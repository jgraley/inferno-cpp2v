#ifndef PARSE_HPP
#define PARSE_HPP

#include "common/common.hpp"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Basic/FileManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Parse/Action.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Parse/Designator.h"
#include "clang/Parse/Scope.h"
#include "clang/Driver/TextDiagnosticPrinter.h"
#include "clang/Lex/LiteralSupport.h"

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"
#include "common/trace.hpp"
#include "common/type_info.hpp"
#include "helpers/misc.hpp"
#include "helpers/typeof.hpp"

#include "identifier_tracker.hpp"

#define INFERNO_TRIPLE "arm-linux" 

class Parse : public Pass
{
public:
    Parse( string i ) :
        infile(i)
    {
    }
    
    void operator()( shared_ptr<Program> program )
    {        
        clang::FileManager fm; 
        llvm::raw_stderr_ostream errstream; // goes to stderr
        clang::TextDiagnosticPrinter diag_printer( errstream );
        clang::Diagnostic diags( &diag_printer ); 
        clang::LangOptions opts;
        opts.CPlusPlus = 1; // Note: always assume input is C++, even if file ends in .c
        clang::TargetInfo* ptarget = clang::TargetInfo::CreateTargetInfo(INFERNO_TRIPLE);
        ASSERT(ptarget);
        clang::SourceManager sm;
        clang::HeaderSearch headers( fm );
        
        clang::Preprocessor pp( diags, opts, *ptarget, sm, headers );
        pp.setPredefines("#define __INFERNO__ 1\n");
        
        const clang::FileEntry *file = fm.getFile(infile);
        if (file) 
            sm.createMainFileID(file, clang::SourceLocation());
        if (sm.getMainFileID() == 0) 
        {
            fprintf(stderr, "Error reading '%s'!\n",infile.c_str());
            exit(1);
        }
        pp.EnterMainSourceFile();
    
        clang::IdentifierTable it( opts );                 
        InfernoAction actions( program, it, pp, *ptarget );                 
        clang::Parser parser( pp, actions );
        TRACE("Start parse\n");
        parser.ParseTranslationUnit();        
        TRACE("End parse\n");
    }
    
private:
    string infile;
    
    class InfernoAction : public clang::Action
    {
    public:
        InfernoAction(shared_ptr<Program> p, clang::IdentifierTable &IT, clang::Preprocessor &pp, clang::TargetInfo &T) : 
            preprocessor(pp),
            target_info(T),
            ident_track( p ),
            global_scope( p )
        {
            inferno_scope_stack.push( &*p );
        }
     
        ~InfernoAction()
        {
            inferno_scope_stack.pop();
            assert( inferno_scope_stack.empty() );
        }
     
    private:   
        // Extend the tree so we can store function parameter decls during prototype parse
        // and read them back at the start of function body, which is a seperate scope.
        struct ParseParameterDeclaration : Instance
        {
            clang::IdentifierInfo *clang_identifier;
        };
        
        // Extend tree so we can insert sub statements at the same level as the label
        struct ParseLabelTarget : LabelTarget
        {
            shared_ptr<Statement> sub;
        };
        struct ParseCase : Case
        {
            shared_ptr<Statement> sub;
        };
        struct ParseDefault : Default
        {
            shared_ptr<Statement> sub;
        };
        struct ParseTwin : Declaration
        {
            shared_ptr<Declaration> d1;
            shared_ptr<Declaration> d2;
        };
        shared_ptr<Declaration> decl_to_insert;
    
        clang::Preprocessor &preprocessor;
        clang::TargetInfo &target_info;
        
        stack< Sequence<Declaration> * > inferno_scope_stack;
        RCHold<Declaration, DeclTy *> hold_decl;
        RCHold<Base, DeclTy *> hold_base;
        RCHold<Operand, ExprTy *> hold_expr;
        RCHold<Statement, StmtTy *> hold_stmt;
        RCHold<Type, TypeTy *> hold_type;
        RCHold<Label, void *> hold_label;
        RCHold<Node, CXXScopeTy *> hold_scope;
        IdentifierTracker ident_track;
        shared_ptr<Node> global_scope;
                
        OwningStmtResult ToStmt( shared_ptr<Statement> s )
        {
            return OwningStmtResult( *this, hold_stmt.ToRaw( s ) );
        }
        
        OwningExprResult ToExpr( shared_ptr<Operand> e )
        {
            return OwningExprResult( *this, hold_expr.ToRaw( e ) );
        }
        
        shared_ptr<Statement> FromClang( const StmtArg &s )
        {
            return hold_stmt.FromRaw( s.get() );
        }

        shared_ptr<Operand> FromClang( const ExprArg &e )
        {
            return hold_expr.FromRaw( e.get() );
        }
        
        // Turn a clang::CXXScopeSpec into a pointer to the corresponding scope node.
        // We have to deal with all the ways of it baing invalid, then just use hold_scope.
        shared_ptr<Node> FromCXXScope( const clang::CXXScopeSpec *SS )
        {
            if( !SS )
                return shared_ptr<Node>();
            
            if( SS->isEmpty() )
                return shared_ptr<Node>();
        
            if( !SS->isSet() )
                return shared_ptr<Node>();
        
            return hold_scope.FromRaw( SS->getScopeRep() );    
        }
        
        clang::Action::TypeTy *isTypeName( clang::IdentifierInfo &II, clang::Scope *S, const clang::CXXScopeSpec *SS) 
        {
            shared_ptr<Node> n = ident_track.TryGet( &II, FromCXXScope( SS ) );                              
            if(n)
            {
                shared_ptr<Type> t = dynamic_pointer_cast<UserType>( n );
                if(t)
                    return hold_type.ToRaw(t);
            }
            
            return 0;
        }
        
        virtual DeclTy *isTemplateName( clang::IdentifierInfo &II, clang::Scope *S, const clang::CXXScopeSpec *SS = 0 )
        {
            return 0; // TODO templates
        }

        virtual bool isCurrentClassName(const clang::IdentifierInfo& II, clang::Scope *S, const clang::CXXScopeSpec *SS) 
        {             
            ident_track.SeenScope( S );
            
            shared_ptr<Node> cur = ident_track.GetCurrent();
            if( !dynamic_pointer_cast<Record>(cur) )
                return false; // not even in a record
            
            shared_ptr<Node> cxxs = FromCXXScope( SS );
            shared_ptr<Node> n = ident_track.TryGet( &II, cxxs );    
            return n == cur;
        }

        virtual void ActOnPopScope(clang::SourceLocation Loc, clang::Scope *S)
        {
            ident_track.PopScope(S);
        }
          
        shared_ptr<Integral> CreateIntegralType( unsigned bits, 
                                                 bool default_signed, 
                                                 clang::DeclSpec::TSS type_spec_signed = clang::DeclSpec::TSS_unspecified )
        {
            shared_ptr<Integral> i;
            bool sign;
            switch( type_spec_signed )
            { 
                case clang::DeclSpec::TSS_signed:
                    sign = true;
                    break;
                case clang::DeclSpec::TSS_unsigned:
                    sign = false;
                    break;
                case clang::DeclSpec::TSS_unspecified:
                    sign = default_signed;
                    break;
            }
            
            if( sign )
                i = shared_ptr<Signed>( new Signed );
            else
                i = shared_ptr<Unsigned>( new Unsigned );
            
            shared_ptr<Literal> l( new Literal );            
            l->value = CreateNumericConstant(bits);
            
            i->width = l;           
            return i;
        }
        
        shared_ptr<Floating> CreateFloatingType( unsigned bits )
        {
            shared_ptr<Literal> l( new Literal );            
            l->value = CreateNumericConstant(bits);

            shared_ptr<Floating> f( new Floating );
            f->width = l;
            return f;
        }
        
        void FillParameters( shared_ptr<Procedure> p, const clang::DeclaratorChunk::FunctionTypeInfo &fchunk )
        {
            for( int i=0; i<fchunk.NumArgs; i++ )
            {
                shared_ptr<Declaration> d = hold_decl.FromRaw( fchunk.ArgInfo[i].Param );
                shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(d); // TODO just push the declarators, no need for dynamic cast?
                p->parameters.push_back( o );
            }            
        }
        
        shared_ptr<Type> CreateTypeNode( clang::Declarator &D, int depth=0 )
        {
            ASSERT( depth>=0 );
            ASSERT( depth<=D.getNumTypeObjects() );
            
            if( depth==D.getNumTypeObjects() )
            {
                const clang::DeclSpec &DS = D.getDeclSpec();
                clang::DeclSpec::TST t = DS.getTypeSpecType();
                ASSERT( DS.getTypeSpecComplex() == clang::DeclSpec::TSC_unspecified &&
                        "complex types not supported" );
                switch( t )
                {
                    case clang::DeclSpec::TST_int:
                    case clang::DeclSpec::TST_unspecified:
                        TRACE("int based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateIntegralType( TypeInfo::integral_bits[DS.getTypeSpecWidth()], 
                                                   TypeInfo::int_default_signed,
                                                   DS.getTypeSpecSign() );
                        break;
                    case clang::DeclSpec::TST_char:
                        TRACE("char based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateIntegralType( TypeInfo::char_bits, 
                                                   TypeInfo::char_default_signed,
                                                   DS.getTypeSpecSign() );
                        break;
                    case clang::DeclSpec::TST_void:
                        TRACE("void based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return shared_ptr<Type>(new Void());
                        break;
                    case clang::DeclSpec::TST_bool:
                        TRACE("bool based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return shared_ptr<Type>(new Bool());
                        break;
                    case clang::DeclSpec::TST_float:
                        TRACE("float based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateFloatingType( TypeInfo::float_bits );
                        break;
                    case clang::DeclSpec::TST_double:
                        TRACE("double based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateFloatingType( DS.getTypeSpecWidth()==clang::DeclSpec::TSW_long ?
                                                   TypeInfo::long_double_bits :
                                                   TypeInfo::double_bits );
                        break;
                    case clang::DeclSpec::TST_typedef:
                        TRACE("typedef\n");
                        return hold_type.FromRaw( DS.getTypeRep() );
                        break;
                    case clang::DeclSpec::TST_struct:
                    case clang::DeclSpec::TST_union:
                    case clang::DeclSpec::TST_class:
                    case clang::DeclSpec::TST_enum:
                        TRACE("struct/union/class/enum\n");
                        // Disgustingly, clang casts the DeclTy returned from ActOnTag() to 
                        // a TypeTy. 
                        return dynamic_pointer_cast<Record>( hold_decl.FromRaw( DS.getTypeRep() ) );
                        break;
                    default:                    
                        ASSERT(!"unsupported type");
                        break;
                }
            }
            else
            {
                const clang::DeclaratorChunk &chunk = D.getTypeObject(depth);
                switch( chunk.Kind )
                {
                    case clang::DeclaratorChunk::Function:
                    {
                        const clang::DeclaratorChunk::FunctionTypeInfo &fchunk = chunk.Fun; 
                        switch( D.getKind() )
                        {
                            case clang::Declarator::DK_Normal:
                            {
                                shared_ptr<Function> f(new Function);
                                FillParameters( f, fchunk );
                                f->return_type = CreateTypeNode( D, depth+1 );                        
                                return f;
                            }
                            case clang::Declarator::DK_Constructor:
                            {
                                shared_ptr<Constructor> c(new Constructor);
                                FillParameters( c, fchunk );
                                return c;
                            }
                            case clang::Declarator::DK_Destructor:
                            {
                                shared_ptr<Destructor> d(new Destructor);
                                return d;
                            }
                            default:
                            ASSERT("Unknown function kind\n");
                            break;
                        }
                    }
                    
                    case clang::DeclaratorChunk::Pointer:
                    {
                        // TODO attributes
                        TRACE("pointer to...\n");
                        const clang::DeclaratorChunk::PointerTypeInfo &pchunk = chunk.Ptr; 
                        shared_ptr<Pointer> p(new Pointer);
                        p->destination = CreateTypeNode( D, depth+1 );                        
                        return p;
                    }
                    
                    case clang::DeclaratorChunk::Reference:
                    {
                        // TODO attributes
                        TRACE("reference to...\n");
                        const clang::DeclaratorChunk::ReferenceTypeInfo &rchunk = chunk.Ref; 
                        shared_ptr<Reference> r(new Reference);
                        ASSERT(r);
                        r->destination = CreateTypeNode( D, depth+1 );                        
                        return r;
                    }
                    
                    case clang::DeclaratorChunk::Array:
                    {
                        // TODO attributes
                        const clang::DeclaratorChunk::ArrayTypeInfo &achunk = chunk.Arr; 
                        TRACE("array [%d] of...\n", achunk.NumElts);
                        shared_ptr<Array> a(new Array);
                        ASSERT(a);
                        a->element = CreateTypeNode( D, depth+1 );
                        if( achunk.NumElts )
                            a->size = hold_expr.FromRaw( achunk.NumElts ); // number of elements was specified
                        else
                            a->size = shared_ptr<Expression>();    // number of elements was not specified eg int a[];
                        return a;
                    }
                    
                    default:
                    ASSERT(!"Unknown type chunk");                     
                    break;
                }
            }
        }
        
        shared_ptr<Instance> CreateObjectNode( clang::Scope *S, 
                                               clang::Declarator &D, 
                                               shared_ptr<AccessSpec> access = shared_ptr<AccessSpec>(),
                                               shared_ptr<Expression> init = shared_ptr<Expression>() )
        { 
            if(!access)
                access = shared_ptr<Public>(new Public);
                
            shared_ptr<Instance> o(new Instance());
            clang::IdentifierInfo *ID = D.getIdentifier();
            const clang::DeclSpec &DS = D.getDeclSpec();
            if(ID)
            {
                o->name = ID->getName();
                ident_track.Add( ID, o, S );     
                TRACE("object %s\n", o->name.c_str());
            }
            else
            {
                TRACE();
            }

            switch( DS.getStorageClassSpec() )
            {
            case clang::DeclSpec::SCS_unspecified:
            case clang::DeclSpec::SCS_extern:// linking will be done "automatically" so no need to remember "extern" in the tree
            case clang::DeclSpec::SCS_auto:
            {
                shared_ptr<NonStatic> ns = shared_new<NonStatic>();
                o->storage = ns;
                if( DS.isVirtualSpecified() )
                    ns->virt = shared_new<Virtual>();
                else
                    ns->virt = shared_new<NonVirtual>();
                break;
            }
            case clang::DeclSpec::SCS_static:
                o->storage = shared_new<Static>();
                break;
            default:
                ASSERT(!"Unsupported storage class");
                break;
            }
            if( DS.getTypeQualifiers() & clang::DeclSpec::TQ_const )
                o->constant = shared_new<Const>();
            else
                o->constant = shared_new<NonConst>();
            o->type = CreateTypeNode( D );
            o->access = access;
            o->initialiser = init;
            
            return o;
        }

        shared_ptr<Typedef> CreateTypedefNode( clang::Scope *S, clang::Declarator &D )
        { 
            shared_ptr<Typedef> t(new Typedef);
            clang::IdentifierInfo *ID = D.getIdentifier();
            if(ID)
            {
                t->name = ID->getName();
                ident_track.Add( ID, t, S ); 
            }
            t->type = CreateTypeNode( D );
 
            TRACE("%s %p %p\n", ID->getName(), t.get(), ID );            
            return t;
        }

        shared_ptr<Label> CreateLabelNode( clang::IdentifierInfo *ID )
        { 
            shared_ptr<Label> l(new Label);            
            l->name = ID->getName();
            TRACE("%s %p %p\n", ID->getName(), l.get(), ID );            
            return l;
        }
        
        shared_ptr<Declaration> FindExistingDeclaration( clang::Scope *S, clang::Declarator &D )
        {
            // See if we already have this object in the current scope, or specified scope if Declarator has one
            shared_ptr<Node> cxxs = FromCXXScope( &D.getCXXScopeSpec() );
            
            // Use C++ scope if non-NULL; do not recurse (=precise match only)
            shared_ptr<Node> found_n = ident_track.TryGet( D.getIdentifier(), cxxs, false ); 
            TRACE("Looked for %s, result %p (%p)\n", D.getIdentifier()->getName(), found_n.get(), cxxs.get() );
            if( !found_n )
            {
                ASSERT( !cxxs );
                return shared_ptr<Declaration>();
            }
            
            shared_ptr<Declaration> found_d = dynamic_pointer_cast<Declaration>( found_n );
            // If the found match is not a declaration, cast will fail and we'll return NULL for "not found"
            return found_d; 
        }
        
        shared_ptr<Instance> FindExistingObject( clang::Scope *S, clang::Declarator &D )
        {
            // Get a declaration and see if its an object declaration
            shared_ptr<Declaration> found_d = FindExistingDeclaration( S, D );           
            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(found_d);
            ASSERT( o && "found the name, but not an object - maybe a typedef?");
            TRACE("aod %s %p\n", o->name.c_str(), o.get() );
            return o;
        }
        
        shared_ptr<Declaration> CreateDelcaration( clang::Scope *S, clang::Declarator &D, shared_ptr<AccessSpec> a = shared_ptr<AccessSpec>() )
        {
            if(!a)
                a = shared_ptr<Public>(new Public);
        
            const clang::DeclSpec &DS = D.getDeclSpec();
            shared_ptr<Declaration> d;
            if( DS.getStorageClassSpec() == clang::DeclSpec::SCS_typedef )
            {
                shared_ptr<Typedef> t = CreateTypedefNode( S, D );                
                TRACE();
                d = t;
            }    
            else
            {                
                shared_ptr<Instance> o = CreateObjectNode( S, D, a );        
                TRACE("%s %p\n", o->name.c_str(), o.get() );       
                d = o;
            }
            
            d->access = a;
            
            return d;
        }
        
        // Does 1 thing:
        // 1. Inserts a stored decl if there is one in decl_to_insert
        DeclTy *IssueDeclaration( clang::Scope *S, shared_ptr<Declaration> d )
        {
            // Did we leave a decl lying around to insert later? If so, pack it together with
            // the current decl, for insertion into the code sequence.
            TRACE("Scope flags %x ", S->getFlags() );
            if( decl_to_insert )
            {                
                shared_ptr<ParseTwin> pt( new ParseTwin );
                pt->d1 = decl_to_insert;
                pt->d2 = d;
                inferno_scope_stack.top()->push_back( decl_to_insert );
                inferno_scope_stack.top()->push_back( d );
                decl_to_insert = shared_ptr<Declaration>(); // don't need to generate it again
                TRACE("inserted decl\n" );
                return hold_decl.ToRaw( pt );
            }
            else
            {
                inferno_scope_stack.top()->push_back( d );
                TRACE("no insert\n" );
                return hold_decl.ToRaw( d ); 
            }            
        }
        
        virtual DeclTy *ActOnDeclarator( clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup )
        {
            ident_track.SeenScope( S );
        
            TRACE();
            // TODO the spurious char __builtin_va_list; line comes from the target info.
            // Create an inferno target info customised for Inferno that doesn't do this. 
            if( strcmp(D.getIdentifier()->getName(), "__builtin_va_list")==0 )
            {            
                return 0;
            }
 
            shared_ptr<Declaration> d = FindExistingDeclaration( S, D ); // decl exists already?
            if( d )
            {
                return hold_decl.ToRaw( d ); // just return it
            }
            else
            {
                d = CreateDelcaration( S, D );     // make a new one  
                return IssueDeclaration( S, d );   // add it to the tree and return it
            }
        }
        
        /// ActOnParamDeclarator - This callback is invoked when a parameter
        /// declarator is parsed. This callback only occurs for functions
        /// with prototypes. S is the function prototype scope for the
        /// parameters (C++ [basic.scope.proto]).
        virtual DeclTy *ActOnParamDeclarator(clang::Scope *S, clang::Declarator &D) 
        {
            shared_ptr<ParseParameterDeclaration> p(new ParseParameterDeclaration);
            shared_ptr<Instance> o = CreateObjectNode( S, D );
            p->type = o->type;
            p->access = o->access;
            p->name = o->name;
            p->storage = o->storage;
            p->constant = o->constant;
            p->initialiser = o->initialiser;
            p->clang_identifier = D.getIdentifier(); // allow us to re-register the object
            TRACE("%p\n", p.get() );
            return hold_decl.ToRaw( p );
        }

        virtual void AddInitializerToDecl(DeclTy *Dcl, ExprArg Init) 
        {
            shared_ptr<Declaration> d = hold_decl.FromRaw( Dcl );            
            
            if( shared_ptr<ParseTwin> pt = dynamic_pointer_cast<ParseTwin>(d) )
                d = pt->d2;
                
            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(d);
            ASSERT( o ); // Only objects can be initialised
                        
            o->initialiser = FromClang( Init );            
        }

        // Clang tends to parse parameters and function bodies in seperate
        // scopes so when we see them being used we don't recognise them
        // and cannot link back to the correct Instance node. This function
        // puts all the params back in the current scope assuming:
        // 1. They have been added to the Function node correctly and
        // 2. The pass-specific extension ParseParameterDeclaration has been used
        void AddParamsToScope( shared_ptr<Procedure> pp, clang::Scope *FnBodyScope )
        {
            ASSERT(pp);
            
            for( int i=0; i<pp->parameters.size(); i++ )
            {
                TRACE();
                shared_ptr<ParseParameterDeclaration> ppd = dynamic_pointer_cast<ParseParameterDeclaration>( pp->parameters[i] );                
                ASSERT(ppd);
                if( ppd->clang_identifier )
                    ident_track.Add( ppd->clang_identifier, ppd, FnBodyScope );
            }
        }

        // JSG this is like the defailt in Actions, except it passes the parent of the function
        // body to ActOnDeclarator, since the function decl itself is not inside its own body.
        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
        {
            // Default to ActOnDeclarator.
            return ActOnStartOfFunctionDef(FnBodyScope,
                                           ActOnDeclarator(FnBodyScope->getParent(), D, 0));
        }

        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, DeclTy *D) 
        {
            TRACE();
            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(hold_decl.FromRaw(D));
            ASSERT(o);
    
            if( shared_ptr<Procedure> pp = dynamic_pointer_cast<Procedure>( o->type ) )
                AddParamsToScope( pp, FnBodyScope );
    
            inferno_scope_stack.push( new Sequence<Declaration> ); 
            
            return hold_decl.ToRaw( o );     
        }
        
        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtArg Body) 
        {
            TRACE();
            shared_ptr<Instance> o( dynamic_pointer_cast<Instance>( hold_decl.FromRaw(Decl) ) );
            ASSERT(o);
            shared_ptr<Compound> cb( dynamic_pointer_cast<Compound>( FromClang( Body ) ) );
            ASSERT(cb); // function body must be a scope or 0
            
            if( !o->initialiser )
                o->initialiser = cb;
            else if( shared_ptr<Compound> c = dynamic_pointer_cast<Compound>( o->initialiser ) )
                c->statements = c->statements + cb->statements;
            else
                ASSERT(!"wrong thing in function instance");

            TRACE("finish fn %s with %d statements %d total\n", o->name.c_str(), cb->statements.size(), (dynamic_pointer_cast<Compound>(o->initialiser))->statements.size() );
                
            inferno_scope_stack.pop(); // we dont use these - we use the clang-managed compound statement instead (passed in via Body)
            return Decl;
        }    
        
        virtual OwningStmtResult ActOnExprStmt(ExprArg Expr) 
        {
            if( shared_ptr<Expression> e = dynamic_pointer_cast<Expression>( FromClang(Expr) ) )
            {
                return ToStmt( e );
            }
            else
            {
                // Operands that are not Expressions have no side effects and so
                // they do nothing as Statements
                shared_ptr<Nop> n(new Nop);
                return ToStmt( n );
            }                       
        }

        virtual StmtResult ActOnReturnStmt( clang::SourceLocation ReturnLoc,
                                            ExprTy *RetValExp ) 
        {           
            shared_ptr<Operand> e = hold_expr.FromRaw(RetValExp);
            shared_ptr<Return> r(new Return);
            r->return_value = e;
            TRACE("aors %p\n", r.get() );
            return hold_stmt.ToRaw( r );
        }

        virtual ExprResult ActOnIdentifierExpr( clang::Scope *S, 
                                                clang::SourceLocation Loc,
                                                clang::IdentifierInfo &II,
                                                bool HasTrailingLParen,
                                                const clang::CXXScopeSpec *SS = 0 ) 
        {
            shared_ptr<Node> n = ident_track.Get( &II, FromCXXScope( SS ) );
            TRACE("aoie %s %s\n", II.getName(), typeid(*n).name() );
            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>( n );
            ASSERT( o );
            return hold_expr.ToRaw( o );            
        }                                   
        
        shared_ptr<AnyNumber> CreateNumericConstant( unsigned value, int bits=-1 )        
        {
            if( bits == -1 )
                bits = TypeInfo::integral_bits[clang::DeclSpec::TSW_unspecified];
            llvm::APSInt rv( bits, true );
            rv = value;
            shared_ptr<Integer> nc( new Integer );
            nc->value = rv;
            return nc;            
        }
        
        shared_ptr<AnyNumber> CreateNumericConstant( int value, int bits=-1 )        
        {
            if( bits == -1 )
                bits = TypeInfo::integral_bits[clang::DeclSpec::TSW_unspecified];
            llvm::APSInt rv( bits, false );
            rv = value;
            shared_ptr<Integer> nc( new Integer );
            nc->value = rv;
            return nc;            
        }
        
        shared_ptr<AnyNumber> CreateNumericConstant(const clang::Token &tok)
        {
            llvm::SmallString<512> int_buffer;
            int_buffer.resize(tok.getLength());
            const char *this_tok_begin = &int_buffer[0];
          
            // Get the spelling of the token, which eliminates trigraphs, etc.
            unsigned actual_length = preprocessor.getSpelling(tok, this_tok_begin);
            clang::NumericLiteralParser literal(this_tok_begin, this_tok_begin+actual_length, 
                                                tok.getLocation(), preprocessor);
            ASSERT(!literal.hadError);

            if (literal.isIntegerLiteral()) 
            {
                int bits;
                if( literal.isLong )
                    bits = TypeInfo::integral_bits[clang::DeclSpec::TSW_long];
                else if( literal.isLongLong )
                    bits = TypeInfo::integral_bits[clang::DeclSpec::TSW_longlong];
                else
                    bits = TypeInfo::integral_bits[clang::DeclSpec::TSW_unspecified];
                    
                llvm::APSInt rv(bits, literal.isUnsigned);
                bool err = literal.GetIntegerValue(rv);
                
                ASSERT( !err && "numeric literal too big for its own type" );
                shared_ptr<Integer> nc( new Integer );
                nc->value = rv;
                return nc;
            }
            else if( literal.isFloatingLiteral() )
            {
                const llvm::fltSemantics *semantics;
                if( literal.isLong )
                    semantics = TypeInfo::floating_semantics[clang::DeclSpec::TSW_long];
                else if( literal.isFloat )
                    semantics = TypeInfo::floating_semantics[clang::DeclSpec::TSW_short];
                else
                    semantics = TypeInfo::floating_semantics[clang::DeclSpec::TSW_unspecified];
                llvm::APFloat rv( literal.GetFloatValue( *semantics ) );

                shared_ptr<Float> fc( new Float( rv ) );
                return fc;
            }
            ASSERT(!"this sort of literal is not supported");         
        }
        
        virtual ExprResult ActOnNumericConstant(const clang::Token &tok) 
        { 
            shared_ptr<Literal> l(new Literal);
            l->value = CreateNumericConstant( tok );
            return hold_expr.ToRaw( l );            
        } 
  
        virtual ExprResult ActOnBinOp(clang::Scope *S,
                                      clang::SourceLocation TokLoc, clang::tok::TokenKind Kind,
                                      ExprTy *LHS, ExprTy *RHS) 
        {
            TRACE(); 
            shared_ptr<Operator> o = shared_ptr<Operator>();            
            switch( Kind )
            {            
#define BINARY(TOK, TEXT, NODE, ASS, BASE) case clang::tok::TOK: o=shared_ptr<NODE>(new NODE); o->assign=shared_ptr<ASS>( new ASS ); break;
#define ALTBIN(TOK, NODE, ASS) BINARY(TOK, "", NODE, ASS, 0)
#include "tree/operator_info.inc"
            }
            ASSERT( o );
            o->operands.push_back( hold_expr.FromRaw(LHS) );
            o->operands.push_back( hold_expr.FromRaw(RHS) );
            return hold_expr.ToRaw( o );            
        }                     

        virtual ExprResult ActOnPostfixUnaryOp(clang::Scope *S, clang::SourceLocation OpLoc, 
                                               clang::tok::TokenKind Kind, ExprTy *Input) 
        {
            shared_ptr<Operator> o = shared_ptr<Operator>();            
            
            switch( Kind )
            {
#define POSTFIX(TOK, TEXT, NODE, ASS, BASE) case clang::tok::TOK: o=shared_ptr<NODE>(new NODE); o->assign=shared_ptr<ASS>( new ASS ); break;
#include "tree/operator_info.inc"
            }
            ASSERT( o );
            o->operands.push_back( hold_expr.FromRaw(Input) );
            return hold_expr.ToRaw( o );            
        }                     

        virtual ExprResult ActOnUnaryOp( clang::Scope *S, clang::SourceLocation OpLoc, 
                                         clang::tok::TokenKind Kind, ExprTy *Input) 
        {
            shared_ptr<Operator> o = shared_ptr<Operator>();            
            
            switch( Kind )
            {
#define PREFIX(TOK, TEXT, NODE, ASS, BASE) case clang::tok::TOK: o=shared_ptr<NODE>(new NODE); o->assign=shared_ptr<ASS>( new ASS ); break;
#include "tree/operator_info.inc"
            }
            ASSERT( o );
            o->operands.push_back( hold_expr.FromRaw(Input) );
            return hold_expr.ToRaw( o );            
        }                     

       virtual ExprResult ActOnConditionalOp(clang::SourceLocation QuestionLoc, 
                                             clang::SourceLocation ColonLoc,
                                             ExprTy *Cond, ExprTy *LHS, ExprTy *RHS)
        {
            shared_ptr<ConditionalOperator> o(new ConditionalOperator);
            o->condition = hold_expr.FromRaw(Cond);
            ASSERT(LHS && "gnu extension not supported");
            o->if_true = hold_expr.FromRaw(LHS);
            o->if_false = hold_expr.FromRaw(RHS);
            return hold_expr.ToRaw( o );                        
        }                     

        virtual ExprResult ActOnCallExpr(clang::Scope *S, ExprTy *Fn, clang::SourceLocation LParenLoc,
                                         ExprTy **Args, unsigned NumArgs,
                                         clang::SourceLocation *CommaLocs,
                                         clang::SourceLocation RParenLoc) 
        {
            shared_ptr<Call> c(new Call);
            c->function = hold_expr.FromRaw(Fn);
            CollectArgs( &(c->operands), Args, NumArgs );
            return hold_expr.ToRaw( c );
        }
        
        // Not sure if this one has been tested!!
        virtual TypeResult ActOnTypeName(clang::Scope *S, clang::Declarator &D) 
        {
            shared_ptr<Type> t = CreateTypeNode( D );
            return hold_type.ToRaw( t );
        }
        
        void PushStmt( shared_ptr<Compound> s, shared_ptr<Statement> st )
        {
            if( shared_ptr<ParseTwin> pt = dynamic_pointer_cast<ParseTwin>( st ) )    
            {
                PushStmt( s, pt->d1 );
                PushStmt( s, pt->d2 );
                return;
            }
            
            s->statements.push_back( st );
                
            // Flatten the "sub" statements of labels etc
            if( shared_ptr<ParseLabelTarget> plm = dynamic_pointer_cast<ParseLabelTarget>( st ) )
                PushStmt( s, plm->sub );   
            else if( shared_ptr<ParseCase> pc = dynamic_pointer_cast<ParseCase>( st ) )
                PushStmt( s, pc->sub );   
            else if( shared_ptr<ParseDefault> pc = dynamic_pointer_cast<ParseDefault>( st ) )
                PushStmt( s, pc->sub );                      
        }

        virtual OwningStmtResult ActOnCompoundStmt(clang::SourceLocation L, clang::SourceLocation R,
                                                   MultiStmtArg Elts,
                                                   bool isStmtExpr) 
        {
            // TODO helper fn for MultiStmtArg, like FromClang. Maybe.
            shared_ptr<Compound> s(new Compound);

            for( int i=0; i<Elts.size(); i++ )
                PushStmt( s, hold_stmt.FromRaw( Elts.get()[i] ) );
                
            return ToStmt( s );
        }
        
        virtual OwningStmtResult ActOnDeclStmt(DeclTy *Decl, clang::SourceLocation StartLoc,
                                               clang::SourceLocation EndLoc) 
        {
            shared_ptr<Declaration> d( hold_decl.FromRaw(Decl) );
            
            return ToStmt( d );
        }
        
        virtual StmtResult ActOnLabelStmt(clang::SourceLocation IdentLoc, clang::IdentifierInfo *II,
                                          clang::SourceLocation ColonLoc, StmtTy *SubStmt) 
        {
            if( !(II->getFETokenInfo<void *>()) )                        
                II->setFETokenInfo( hold_label.ToRaw( CreateLabelNode( II ) ) );
            
            shared_ptr<ParseLabelTarget> l( new ParseLabelTarget );
            l->label = hold_label.FromRaw( II->getFETokenInfo<void *>() );
            l->sub = hold_stmt.FromRaw( SubStmt );
            return hold_stmt.ToRaw( l );
        }
        
        virtual StmtResult ActOnGotoStmt(clang::SourceLocation GotoLoc,
                                         clang::SourceLocation LabelLoc,
                                         clang::IdentifierInfo *LabelII) 
        {
            if( !(LabelII->getFETokenInfo<void *>()) )                        
                LabelII->setFETokenInfo( hold_label.ToRaw( CreateLabelNode( LabelII ) ) );

            shared_ptr<Goto> g( new Goto );
            g->destination = hold_label.FromRaw( LabelII->getFETokenInfo<void *>() );
            return hold_stmt.ToRaw( g );
        }
        
        virtual StmtResult ActOnIndirectGotoStmt(clang::SourceLocation GotoLoc,
                                                 clang::SourceLocation StarLoc,
                                                 ExprTy *DestExp) 
        {
            shared_ptr<Goto> g( new Goto );
            g->destination = hold_expr.FromRaw( DestExp );
            return hold_stmt.ToRaw( g );
        }

        virtual ExprResult ActOnAddrLabel(clang::SourceLocation OpLoc, clang::SourceLocation LabLoc,
                                          clang::IdentifierInfo *LabelII)  // "&&foo"
        {                                 
            if( !(LabelII->getFETokenInfo<void *>()) )                        
                LabelII->setFETokenInfo( hold_label.ToRaw( CreateLabelNode( LabelII ) ) );

            return hold_expr.ToRaw( hold_label.FromRaw( LabelII->getFETokenInfo<void *>() ) );
        }
        
        virtual StmtResult ActOnIfStmt(clang::SourceLocation IfLoc, ExprTy *CondVal,
                                       StmtTy *ThenVal, clang::SourceLocation ElseLoc,
                                       StmtTy *ElseVal) 
        {
            shared_ptr<If> i( new If );
            i->condition = hold_expr.FromRaw( CondVal );
            i->body = hold_stmt.FromRaw( ThenVal );
            if( ElseVal )
                i->else_body = hold_stmt.FromRaw( ElseVal );
            return hold_stmt.ToRaw( i );
        }
        
        virtual StmtResult ActOnWhileStmt(clang::SourceLocation WhileLoc, ExprTy *Cond,
                                          StmtTy *Body) 
        {
            shared_ptr<While> w( new While );
            w->condition = hold_expr.FromRaw( Cond );
            w->body = hold_stmt.FromRaw( Body );
            return hold_stmt.ToRaw( w );
        }
        
        virtual StmtResult ActOnDoStmt(clang::SourceLocation DoLoc, StmtTy *Body,
                                       clang::SourceLocation WhileLoc, ExprTy *Cond) 
        {
            shared_ptr<Do> d( new Do );
            d->body = hold_stmt.FromRaw( Body );
            d->condition = hold_expr.FromRaw( Cond );
            return hold_stmt.ToRaw( d );
        }
        
        virtual StmtResult ActOnForStmt(clang::SourceLocation ForLoc, 
                                        clang::SourceLocation LParenLoc, 
                                        StmtTy *First, ExprTy *Second, ExprTy *Third,
                                        clang::SourceLocation RParenLoc, StmtTy *Body)
        {
            shared_ptr<For> f( new For );
            if( First )
                f->initialisation = hold_stmt.FromRaw( First );
            if( Second )
                f->condition = hold_expr.FromRaw( Second );
            StmtTy *third = (StmtTy *)Third; // Third is really a statement, the Actions API is wrong
            if( third )
                f->increment = hold_stmt.FromRaw( third );
            f->body = hold_stmt.FromRaw( Body );
            return hold_stmt.ToRaw( f );
        }

        virtual StmtResult ActOnStartOfSwitchStmt(ExprTy *Cond) 
        {
            shared_ptr<Switch> s( new Switch );
            s->condition = hold_expr.FromRaw( Cond );
            return hold_stmt.ToRaw( s );
        }
  
        virtual StmtResult ActOnFinishSwitchStmt(clang::SourceLocation SwitchLoc, 
                                                 StmtTy *rsw, ExprTy *Body) 
        {
            shared_ptr<Statement> s( hold_stmt.FromRaw( rsw ) );
            shared_ptr<Switch> sw( dynamic_pointer_cast<Switch>(s) );
            ASSERT(sw && "expecting a switch statement");
        
            StmtTy *body = (StmtTy *)Body; // Third is really a statement, the Actions API is wrong                        
            sw->body = hold_stmt.FromRaw( body );
            return hold_stmt.ToRaw( s );
        }

        /// ActOnCaseStmt - Note that this handles the GNU 'case 1 ... 4' extension,
        /// which can specify an RHS value.
        virtual OwningStmtResult ActOnCaseStmt(clang::SourceLocation CaseLoc, ExprArg LHSVal,
                                               clang::SourceLocation DotDotDotLoc, ExprArg RHSVal,
                                               clang::SourceLocation ColonLoc, StmtArg SubStmt) 
        {
            TRACE();
            shared_ptr<ParseCase> c( new ParseCase );
            if( RHSVal.get() )
                c->value_hi = FromClang( RHSVal ); 
            else
                c->value_hi = FromClang( LHSVal );
            c->value_lo = FromClang( LHSVal );
            c->sub = FromClang( SubStmt ); 
            return ToStmt( c );
        }
        
        virtual OwningStmtResult ActOnDefaultStmt(clang::SourceLocation DefaultLoc,
                                                  clang::SourceLocation ColonLoc, StmtArg SubStmt,
                                                  clang::Scope *CurScope)
        {
            TRACE();
            shared_ptr<ParseDefault> d( new ParseDefault );
            d->sub = FromClang( SubStmt );
            return ToStmt( d );
        }
                
        virtual StmtResult ActOnContinueStmt(clang::SourceLocation ContinueLoc,
                                             clang::Scope *CurScope) 
        {
            return hold_stmt.ToRaw( shared_ptr<Continue>( new Continue ) );
        }
  
        virtual StmtResult ActOnBreakStmt(clang::SourceLocation GotoLoc, clang::Scope *CurScope) 
        {        
            return hold_stmt.ToRaw( shared_ptr<Break>( new Break ) );
        }
        
        shared_ptr<AccessSpec> ConvertAccess( clang::AccessSpecifier AS, shared_ptr<Record> rec = shared_ptr<Record>() )
        {
            switch( AS )
            {
                case clang::AS_public:
                    return shared_ptr<Public>(new Public);
                    break;
                case clang::AS_protected:
                    return shared_ptr<Protected>(new Protected);
                    break;
                case clang::AS_private:
                    return shared_ptr<Private>(new Private);
                    break;
                case clang::AS_none:
                    ASSERT( rec && "no access specifier and record not supplied so cannot deduce");
                    // members are never AS_none because clang deals. Bases can be AS_none, so we supply the enclosing record type
                    if( dynamic_pointer_cast<Class>(rec) )
                        return shared_ptr<Private>(new Private);
                    else 
                        return shared_ptr<Public>(new Public);
                    break;
                default:
                    ASSERT(!"Invalid access specfier");
                    return shared_ptr<Public>(new Public);
                    break;
            }
        }
        
        virtual DeclTy *ActOnCXXMemberDeclarator(clang::Scope *S, clang::AccessSpecifier AS,
                                                 clang::Declarator &D, ExprTy *BitfieldWidth,
                                                 ExprTy *Init, DeclTy *LastInGroup) 
        {
            const clang::DeclSpec &DS = D.getDeclSpec();
            TRACE("Member %p\n", Init);
            shared_ptr<Declaration> d = CreateDelcaration( S, D, ConvertAccess( AS ) );
            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(d);
      
            if( BitfieldWidth )
            {
                ASSERT( o && "only objects may be bitfields" );
                shared_ptr<Numeric> n( dynamic_pointer_cast<Numeric>( o->type ) );
                ASSERT( n && "cannot specify width of non-numeric type" );
                n->width = hold_expr.FromRaw(BitfieldWidth);
            }
            
            if( Init )
            {  
                ASSERT( o && "only instances may have initialisers"); 
                o->initialiser = hold_expr.FromRaw( Init );
            }
                       
            return IssueDeclaration( S, d );
        }

        virtual DeclTy *ActOnTag(clang::Scope *S, unsigned TagType, TagKind TK,
                                 clang::SourceLocation KWLoc, const clang::CXXScopeSpec &SS,
                                 clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, 
                                 clang::AttributeList *Attr,
                                 MultiTemplateParamsArg TemplateParameterLists) 
        {
            ASSERT( !FromCXXScope(&SS) && "We're not doing anything with the C++ scope"); // TODO do something with the C++ scope
        
            TRACE("Tag type %d\n", TagType);
            // TagType is an instance of DeclSpec::TST, indicating what kind of tag this
            // is (struct/union/enum/class).
            shared_ptr<Record> h;
            switch( (clang::DeclSpec::TST)TagType )
            {
                case clang::DeclSpec::TST_union:
                    h = shared_ptr<Union>(new Union);
                    break;
                case clang::DeclSpec::TST_struct:
                    h = shared_ptr<Struct>(new Struct);
                    break;
                case clang::DeclSpec::TST_class:
                    h = shared_ptr<Class>(new Class);
                    break;
                case clang::DeclSpec::TST_enum:
                    h = shared_ptr<Enum>(new Enum);
                    break;
                default:
                    ASSERT(!"Unknown type spec type");            
                    break;        
            }
            
            if(Name)
            {
                h->name = Name->getName();
                ident_track.Add(Name, h, S); 
            }
            else
            {
                // TODO make a general-lurpose anon name generator
                char an[20];
                static int ac=0;
                sprintf( an, "__anon%d", ac++ );
                h->name = an;
                ident_track.Add(NULL, h, S); 
            }
            
            h->access = shared_ptr<Public>(new Public); // must make all holder type decls public since clang doesnt seem to give us an AS
            h->incomplete = true;
            
            //TODO should we do something with TagKind? Maybe needed for render.
            //TODO use the attibutes
            
            // struct/class/union pushed by ActOnFinishCXXClassDef()
            if( (clang::DeclSpec::TST)TagType == clang::DeclSpec::TST_enum )
                decl_to_insert = h; 
                                   
            TRACE("done tag\n");
            
            return hold_decl.ToRaw( h );            
        }
        
   
        /// ActOnStartCXXClassDef - This is called at the start of a class/struct/union
        /// definition, when on C++.
        virtual void ActOnStartCXXClassDef(clang::Scope *S, DeclTy *TagDecl,
                                           clang::SourceLocation LBrace) 
        {
            TRACE();
            
            // Just populate the members container for the Record node
            // we already created. No need to return anything.
            shared_ptr<Declaration> d = hold_decl.FromRaw( TagDecl );
            shared_ptr<Record> h = dynamic_pointer_cast<Record>(d);
            h->incomplete = false;
            
            ident_track.SetNextRecord( h );
                
            inferno_scope_stack.push( &(h->members) );      // decls for members will go on this scope      
        }
  
        /// ActOnFinishCXXClassDef - This is called when a class/struct/union has
        /// completed parsing, when on C++.
        virtual void ActOnFinishCXXClassDef(DeclTy *TagDecl) 
        {
            inferno_scope_stack.pop(); // class scope is complete
            ident_track.SetNextRecord();

            // TODO are structs etc definable in functions? If so, this will put the decl outside the function            
            shared_ptr<Declaration> d = hold_decl.FromRaw( TagDecl );
            shared_ptr<Record> h = dynamic_pointer_cast<Record>(d);
            decl_to_insert = h; 
        }
        
        virtual ExprResult ActOnMemberReferenceExpr(clang::Scope *S, ExprTy *Base, 
                                                    clang::SourceLocation OpLoc,
                                                    clang::tok::TokenKind OpKind,
                                                    clang::SourceLocation MemberLoc,
                                                    clang::IdentifierInfo &Member) 
        {
            TRACE("kind %d\n", OpKind);
            shared_ptr<Lookup> a( new Lookup );
            
            // Turn -> into * and .
            if( OpKind == clang::tok::arrow )  // Base->Member
            {            
                shared_ptr<Dereference> ou( new Dereference );
                ou->operands.push_back( hold_expr.FromRaw( Base ) );
                a->base = ou;
            }
            else if( OpKind == clang::tok::period ) // Base.Member
            {
                a->base = hold_expr.FromRaw( Base );
            }
            else
            {
                ASSERT(!"Unknown token accessing member");
            }            
        
            // Find the specified member in the record implied by the expression on the left of .
            shared_ptr<Type> tbase = TypeOf().Get(a->base);
            shared_ptr<Record> rbase = dynamic_pointer_cast<Record>(tbase);
            ASSERT( rbase && "thing on left of ./-> is not a record/record ptr" );
            
            a->member = FindMemberByName( rbase, string(Member.getName()) );

            ASSERT(a->member && "in r.m or (&r)->m, could not find m in r");        
            
            return hold_expr.ToRaw( a );
        }
                
        virtual ExprResult ActOnArraySubscriptExpr(clang::Scope *S,
                                                   ExprTy *Base, clang::SourceLocation LLoc,
                                                   ExprTy *Idx, clang::SourceLocation RLoc) 
        {
            shared_ptr<Subscript> su( new Subscript );
            su->base = hold_expr.FromRaw( Base );
            su->index = hold_expr.FromRaw( Idx );
            return hold_expr.ToRaw( su );
        }

        /// ActOnCXXBoolLiteral - Parse {true,false} literals.
        virtual ExprResult ActOnCXXBoolLiteral(clang::SourceLocation OpLoc,
                                               clang::tok::TokenKind Kind) //TODO not working - get node has no info
        {
            shared_ptr<Literal> ic(new Literal);
            TRACE("true/false tk %d %d %d\n", Kind, clang::tok::kw_true, clang::tok::kw_false );
            
            int v = (Kind == clang::tok::kw_true);            
            ic->value = CreateNumericConstant( v );
            return hold_expr.ToRaw( ic );                       
        }

        virtual ExprResult ActOnCastExpr(clang::SourceLocation LParenLoc, TypeTy *Ty,
                                         clang::SourceLocation RParenLoc, ExprTy *Op) 
        {
            shared_ptr<Cast> c(new Cast);
            c->operand = hold_expr.FromRaw( Op );
            c->type = hold_type.FromRaw( Ty );
            return hold_expr.ToRaw( c );                       
        }
        
        virtual OwningStmtResult ActOnNullStmt(clang::SourceLocation SemiLoc) 
        {
            TRACE();
            shared_ptr<Nop> n(new Nop);
            return ToStmt( n );
        }
                
        virtual ExprResult ActOnCharacterConstant(const clang::Token &tok) 
        { 
            string t = preprocessor.getSpelling(tok);
        
            clang::CharLiteralParser literal(t.c_str(), t.c_str()+t.size(), tok.getLocation(), preprocessor);
  
            if (literal.hadError())
                return ExprResult(true);
                
            shared_ptr<Integer> nc( new Integer );
            llvm::APSInt rv(TypeInfo::char_bits, !TypeInfo::char_default_signed);
            rv = literal.getValue();
            nc->value = rv;   
            
            shared_ptr<Literal> l( new Literal );
            l->value = nc;
            return hold_expr.ToRaw( l );
        }
        
        virtual ExprResult ActOnInitList(clang::SourceLocation LParenLoc,
                                         ExprTy **InitList, unsigned NumInit,
                                         clang::InitListDesignations &Designators,
                                         clang::SourceLocation RParenLoc) 
        {
            ASSERT( !Designators.hasAnyDesignators() && "Designators in init lists unsupported" ); 
         
            shared_ptr<Aggregate> ao(new Aggregate);
            for(int i=0; i<NumInit; i++)
            {
                shared_ptr<Operand> e = hold_expr.FromRaw( InitList[i] );
                ao->operands.push_back( e );
            }
            return hold_expr.ToRaw( ao );                                 
        }

        /// ActOnStringLiteral - The specified tokens were lexed as pasted string
        /// fragments (e.g. "foo" "bar" L"baz"). 
        virtual ExprResult ActOnStringLiteral(const clang::Token *Toks, unsigned NumToks) 
        {
            clang::StringLiteralParser literal(Toks, NumToks, preprocessor, target_info);
            if (literal.hadError)
                return ExprResult(true);
                
            shared_ptr<String> s(new String);
            s->value = literal.GetString();  

            shared_ptr<Literal> l(new Literal);
            l->value = s;  

            return hold_expr.ToRaw( l );                                 
        }
        
        /// ActOnCXXThis - Parse the C++ 'this' pointer.
        virtual ExprResult ActOnCXXThis(clang::SourceLocation ThisLoc) 
        {
            return hold_expr.ToRaw( shared_ptr<This>( new This ) );
        }

        virtual DeclTy *ActOnEnumConstant(clang::Scope *S, DeclTy *EnumDecl,
                                          DeclTy *LastEnumConstant,
                                          clang::SourceLocation IdLoc, clang::IdentifierInfo *Id,
                                          clang::SourceLocation EqualLoc, ExprTy *Val) 
        {
            int enumbits = TypeInfo::integral_bits[clang::DeclSpec::TSW_unspecified];
            shared_ptr<Instance> o(new Instance());
            o->name = Id->getName();
            o->storage = shared_new<Static>();
            o->constant = shared_new<Const>(); // static const member does not consume storage!!
            o->type = CreateIntegralType( enumbits, false );
            o->access = shared_ptr<Public>(new Public);
            if( Val )
            {
                o->initialiser = hold_expr.FromRaw( Val );
            }
            else if( LastEnumConstant )
            {                 
                shared_ptr<Declaration> lastd( hold_decl.FromRaw( LastEnumConstant ) );
                shared_ptr<Instance> lasto( dynamic_pointer_cast<Instance>(lastd) );
                ASSERT(lasto && "unexpected kind of declaration inside an enum");
                shared_ptr<Add> inf( new Add );
                inf->operands.push_back( lasto->initialiser );
                shared_ptr<Literal> l( new Literal );
                l->value = CreateNumericConstant( 1, enumbits );
                inf->operands.push_back( l );
                o->initialiser = inf;
            }
            else
            {
                shared_ptr<Literal> l( new Literal );
                l->value = CreateNumericConstant( 0, enumbits );
                o->initialiser = l;
            }
            ident_track.Add(Id, o, S); 
            return hold_decl.ToRaw( o );
        }
        
        virtual void ActOnEnumBody(clang::SourceLocation EnumLoc, DeclTy *EnumDecl,
                                   DeclTy **Elements, unsigned NumElements) 
        {
            shared_ptr<Declaration> d( hold_decl.FromRaw( EnumDecl ) );
            shared_ptr<Enum> e( dynamic_pointer_cast<Enum>(d) );
            ASSERT( e && "expected the declaration to be an enum");
            for( int i=0; i<NumElements; i++ )
               e->members.push_back( hold_decl.FromRaw( Elements[i] ) );
            e->incomplete = false;   
        }

        /// ParsedFreeStandingDeclSpec - This method is invoked when a declspec with
        /// no declarator (e.g. "struct foo;") is parsed.
        // JSG likely bug with C++ integration in clang means it parses the struct
        // including stuff in {} and then detects the ; as meaning free standing.
        virtual DeclTy *ParsedFreeStandingDeclSpec(clang::Scope *S, clang::DeclSpec &DS) 
        {
            TRACE();
            shared_ptr<Declaration> d( hold_decl.FromRaw( DS.getTypeRep() ) );
            shared_ptr<Record> h( dynamic_pointer_cast<Record>( d ) );
            ASSERT( h );
            if( decl_to_insert )
            {                
                d = decl_to_insert;
                decl_to_insert = shared_ptr<Declaration>();
            }
            inferno_scope_stack.top()->push_back( d );
            return hold_decl.ToRaw( d );
        }
        
        virtual ExprResult 
           ActOnSizeOfAlignOfExpr( clang::SourceLocation OpLoc, bool isSizeof, bool isType,
                                   void *TyOrEx, const clang::SourceRange &ArgRange) 
        {
            shared_ptr<Operator> p;
            if( isSizeof )
                p = shared_ptr<SizeOf>(new SizeOf);
            else
                p = shared_ptr<AlignOf>(new AlignOf);
            if( isType )
                p->operands.push_back( hold_type.FromRaw(TyOrEx) );                   
            else
                p->operands.push_back( hold_expr.FromRaw(TyOrEx) );
            return hold_expr.ToRaw( p );                       
        }
        
        virtual BaseResult ActOnBaseSpecifier(DeclTy *classdecl, 
                                              clang::SourceRange SpecifierRange,
                                              bool Virt, clang::AccessSpecifier AccessSpec,
                                              TypeTy *basetype, 
                                              clang::SourceLocation BaseLoc) 
        {
            shared_ptr<Type> t( hold_type.FromRaw( basetype ) );
            shared_ptr<InheritanceRecord> ir( dynamic_pointer_cast<InheritanceRecord>(t) );
            ASSERT( ir );
            shared_ptr<Declaration> d = hold_decl.FromRaw( classdecl );
            shared_ptr<Record> r = dynamic_pointer_cast<Record>( d );
            ASSERT( r );
            
            shared_ptr<Base> base( new Base );
            base->record = ir;
          /*  if( Virt )
                base->storage = shared_new<Virtual>();
            else    
                base->storage = shared_new<NonStatic>();
            base->constant = shared_new<NonConst>(); */
            base->access = ConvertAccess( AccessSpec, r );       
            return hold_base.ToRaw( base );
        }
        
        virtual void ActOnBaseSpecifiers(DeclTy *ClassDecl, BaseTy **Bases, 
                                         unsigned NumBases) 
        {
            shared_ptr<Declaration> cd( hold_decl.FromRaw( ClassDecl ) );
            shared_ptr<InheritanceRecord> ih( dynamic_pointer_cast<InheritanceRecord>(cd) );
            ASSERT( ih );
            
            for( int i=0; i<NumBases; i++ )
            {
                ih->bases.push_back( hold_base.FromRaw( Bases[i] ) );  
            }
        }
        
        /// ActOnCXXNestedNameSpecifier - Called during parsing of a
        /// nested-name-specifier. e.g. for "foo::bar::" we parsed "foo::" and now
        /// we want to resolve "bar::". 'SS' is empty or the previously parsed
        /// nested-name part ("foo::"), 'IdLoc' is the source location of 'bar',
        /// 'CCLoc' is the location of '::' and 'II' is the identifier for 'bar'.
        /// Returns a CXXScopeTy* object representing the C++ scope.
        virtual CXXScopeTy *ActOnCXXNestedNameSpecifier(clang::Scope *S,
                                                        const clang::CXXScopeSpec &SS,
                                                        clang::SourceLocation IdLoc,
                                                        clang::SourceLocation CCLoc,
                                                        clang::IdentifierInfo &II) 
        {
            shared_ptr<Node> n( ident_track.Get( &II, FromCXXScope( &SS ) ) );
            
            return hold_scope.ToRaw( n );
        }
        
        /// ActOnCXXGlobalScopeSpecifier - Return the object that represents the
        /// global scope ('::').
        virtual CXXScopeTy *ActOnCXXGlobalScopeSpecifier(clang::Scope *S,
                                                         clang::SourceLocation CCLoc) 
        {
            return hold_scope.ToRaw( global_scope );
        }

        /// ActOnCXXEnterDeclaratorScope - Called when a C++ scope specifier (global
        /// scope or nested-name-specifier) is parsed, part of a declarator-id.
        /// After this method is called, according to [C++ 3.4.3p3], names should be
        /// looked up in the declarator-id's scope, until the declarator is parsed and
        /// ActOnCXXExitDeclaratorScope is called.
        /// The 'SS' should be a non-empty valid CXXScopeSpec.
        virtual void ActOnCXXEnterDeclaratorScope(clang::Scope *S, const clang::CXXScopeSpec &SS) 
        {
            TRACE();
            shared_ptr<Node> n = FromCXXScope( &SS );
            ASSERT(n); 
            ident_track.PushScope( S, n );
        }
    
        /// ActOnCXXExitDeclaratorScope - Called when a declarator that previously
        /// invoked ActOnCXXEnterDeclaratorScope(), is finished. 'SS' is the same
        /// CXXScopeSpec that was passed to ActOnCXXEnterDeclaratorScope as well.
        /// Used to indicate that names should revert to being looked up in the
        /// defining scope.
        virtual void ActOnCXXExitDeclaratorScope(clang::Scope *S, const clang::CXXScopeSpec &SS) 
        {
            TRACE();
            ident_track.PopScope( S );
        }
        
        shared_ptr<Instance> GetConstructor( shared_ptr<Type> t )
        {
            if( shared_ptr<Typedef> td = dynamic_pointer_cast<Typedef>(t) )
            {
                return GetConstructor( td->type );
            }
            else if( shared_ptr<Record> r = dynamic_pointer_cast<Record>(t) )
            {
                FOREACH( shared_ptr<Declaration> d, r->members )
                {
                    shared_ptr<Instance> o( dynamic_pointer_cast<Instance>(d) );
                    if( !o )
                        continue;
                    if( dynamic_pointer_cast<Constructor>(o->type) )
                        return o;
                }
                ASSERT(!"missing constructor");
                return shared_ptr<Instance>();
            }
            else 
            {
                ASSERT(!"initialisers for PODs not yet implemented"); // TODO
                return shared_ptr<Instance>();
            }            
        }

        virtual MemInitResult ActOnMemInitializer( DeclTy *ConstructorDecl,
                                                   clang::Scope *S,
                                                   clang::IdentifierInfo *MemberOrBase,
                                                   clang::SourceLocation IdLoc,
                                                   clang::SourceLocation LParenLoc,
                                                   ExprTy **Args, unsigned NumArgs,
                                                   clang::SourceLocation *CommaLocs,
                                                   clang::SourceLocation RParenLoc ) 
        {            
            // Get (or make) the constructor we're invoking
            shared_ptr<Node> n = ident_track.Get( MemberOrBase );
            shared_ptr<Instance> om( dynamic_pointer_cast<Instance>(n) );
            shared_ptr<Instance> cm = GetConstructor( om->type );
            
            // Build a lookup to the constructor, using the speiciifed subobject and the matching constructor
            shared_ptr<Lookup> lu(new Lookup);
            lu->base = om;
            lu->member = cm;            
            
            // Build a call to the constructor with supplied args
            shared_ptr<Call> call(new Call);
            call->function = lu;
            CollectArgs( &(call->operands), Args, NumArgs );
            
            // Get the constructor whose init list we're adding to
            shared_ptr<Declaration> d( hold_decl.FromRaw( ConstructorDecl ) );
            shared_ptr<Instance> o( dynamic_pointer_cast<Instance>(d) );
            ASSERT(o);
            shared_ptr<Compound> comp;
            if( o->initialiser )
            {
                comp = dynamic_pointer_cast<Compound>(o->initialiser);
                ASSERT(comp);       
                TRACE();
            }
            else
            {
                comp = shared_ptr<Compound>( new Compound );
                o->initialiser = comp;
                TRACE();
            }
            
            // Add it
            comp->statements.push_back( call );
            return 0;
        }
        
        void CollectArgs( Sequence<Operand> *ps, ExprTy **Args, unsigned NumArgs )
        {
            for(int i=0; i<NumArgs; i++ )
                ps->push_back( hold_expr.FromRaw(Args[i]) );
        } 
        
        /// ActOnCXXNew - Parsed a C++ 'new' expression. UseGlobal is true if the
        /// new was qualified (::new). In a full new like
        /// @code new (p1, p2) type(c1, c2) @endcode
        /// the p1 and p2 expressions will be in PlacementArgs and the c1 and c2
        /// expressions in ConstructorArgs. The type is passed as a declarator.
        virtual ExprResult ActOnCXXNew( clang::SourceLocation StartLoc, bool UseGlobal,
                                        clang::SourceLocation PlacementLParen,
                                        ExprTy **PlacementArgs, unsigned NumPlaceArgs,
                                        clang::SourceLocation PlacementRParen,
                                        bool ParenTypeId, clang::Declarator &D,
                                        clang::SourceLocation ConstructorLParen,
                                        ExprTy **ConstructorArgs, unsigned NumConsArgs,
                                        clang::SourceLocation ConstructorRParen ) 
        {
            shared_ptr<New> n( new New );
            n->type = CreateTypeNode( D );
            CollectArgs( &(n->placement_arguments), PlacementArgs, NumPlaceArgs );
            CollectArgs( &(n->constructor_arguments), ConstructorArgs, NumConsArgs );

            if( UseGlobal )
                n->global = shared_ptr<GlobalNew>( new GlobalNew );
            else
                n->global = shared_ptr<NonGlobalNew>( new NonGlobalNew );
            // TODO cant figure out meaning of ParenTypeId
            
            return hold_expr.ToRaw( n );         
        }
        
        /// ActOnCXXDelete - Parsed a C++ 'delete' expression. UseGlobal is true if
        /// the delete was qualified (::delete). ArrayForm is true if the array form
        /// was used (delete[]).
        virtual ExprResult ActOnCXXDelete( clang::SourceLocation StartLoc, bool UseGlobal,
                                           bool ArrayForm, ExprTy *Operand ) 
        {
            shared_ptr<Delete> d( new Delete );            
            d->pointer = hold_expr.FromRaw( Operand );
            
            if( ArrayForm )
                d->array = shared_ptr<ArrayNew>( new ArrayNew );
            else
                d->array = shared_ptr<NonArrayNew>( new NonArrayNew );
                
            if( UseGlobal )
                d->global = shared_ptr<GlobalNew>( new GlobalNew );
            else
                d->global = shared_ptr<NonGlobalNew>( new NonGlobalNew );
            
            return hold_expr.ToRaw( d ); 
        }
        
        //--------------------------------------------- unimplemented actions -----------------------------------------------     
        // Note: only actions that return something (so we don't get NULL XTy going around the place). No obj-C or GCC 
        // extensions. These all assert out immediately.
        
  virtual DeclTy *ActOnFileScopeAsmDecl(clang::SourceLocation Loc, ExprArg AsmString) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnField(clang::Scope *S, DeclTy *TagD, clang::SourceLocation DeclStart,
                             clang::Declarator &D, ExprTy *BitfieldWidth) {
    ASSERT(!"Unimplemented action"); // TODO is this C-not-C++ or ObjC?
    return 0;
  }

  virtual StmtResult ActOnAsmStmt(clang::SourceLocation AsmLoc,
                                  bool IsSimple,                                  
                                  bool IsVolatile,
                                  unsigned NumOutputs,
                                  unsigned NumInputs,
                                  std::string *Names,
                                  ExprTy **Constraints,
                                  ExprTy **Exprs,
                                  ExprTy *AsmString,
                                  unsigned NumClobbers,
                                  ExprTy **Clobbers,
                                  clang::SourceLocation RParenLoc) {
    ASSERT(!"Unimplemented action"); 
    return 0;
  }

  virtual ExprResult ActOnPredefinedExpr(clang::SourceLocation Loc,
                                         clang::tok::TokenKind Kind) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCompoundLiteral(clang::SourceLocation LParen, TypeTy *Ty,
                                          clang::SourceLocation RParen, ExprTy *Op) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnStartNamespaceDef(clang::Scope *S, clang::SourceLocation IdentLoc,
                                        clang::IdentifierInfo *Ident,
                                        clang::SourceLocation LBrace) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXNamedCast(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
                                       clang::SourceLocation LAngleBracketLoc, TypeTy *Ty,
                                       clang::SourceLocation RAngleBracketLoc,
                                       clang::SourceLocation LParenLoc, ExprTy *Op,
                                       clang::SourceLocation RParenLoc) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXThrow(clang::SourceLocation OpLoc,
                                   ExprTy *Op = 0) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXTypeConstructExpr(clang::SourceRange TypeRange,
                                               TypeTy *TypeRep,
                                               clang::SourceLocation LParenLoc,
                                               ExprTy **Exprs,
                                               unsigned NumExprs,
                                               clang::SourceLocation *CommaLocs,
                                               clang::SourceLocation RParenLoc) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXConditionDeclarationExpr(clang::Scope *S,
                                                      clang::SourceLocation StartLoc,
                                                      clang::Declarator &D,
                                                      clang::SourceLocation EqualLoc,
                                                      ExprTy *AssignExprVal) {
    ASSERT(!"Unimplemented action");
    return 0;
  }
  
  virtual DeclTy *ActOnExceptionDeclarator(clang::Scope *S, clang::Declarator &D) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual OwningStmtResult ActOnCXXCatchBlock(clang::SourceLocation CatchLoc,
                                              DeclTy *ExceptionDecl,
                                              StmtArg HandlerBlock) {
    ASSERT(!"Unimplemented action");
    return StmtEmpty();
  }

  virtual OwningStmtResult ActOnCXXTryBlock(clang::SourceLocation TryLoc,
                                            StmtArg TryBlock,
                                            MultiStmtArg Handlers) {
    ASSERT(!"Unimplemented action");
    return StmtEmpty();
  }
 /// ActOnUsingDirective - This is called when using-directive is parsed.
  virtual DeclTy *ActOnUsingDirective(clang::Scope *CurScope,
                                      clang::SourceLocation UsingLoc,
                                      clang::SourceLocation NamespcLoc,
                                      const clang::CXXScopeSpec &SS,
                                      clang::SourceLocation IdentLoc,
                                      clang::IdentifierInfo *NamespcName,
                                      clang::AttributeList *AttrList)
  {
    ASSERT(!"Unimplemented action");
    return 0;
  }
  
  /// ActOnParamUnparsedDefaultArgument - We've seen a default
  /// argument for a function parameter, but we can't parse it yet
  /// because we're inside a class definition. Note that this default
  /// argument will be parsed later.
  virtual void ActOnParamUnparsedDefaultArgument(DeclTy *param, 
                                                 clang::SourceLocation EqualLoc) 
  { 
      ASSERT(!"Unimplemented action"); 
  }

  /// ActOnParamDefaultArgumentError - Parsing or semantic analysis of
  /// the default argument for the parameter param failed.
  virtual void ActOnParamDefaultArgumentError(DeclTy *param)
  {
      ASSERT(!"Unimplemented action");
   }
 
  virtual void ActOnEnumStartDefinition(clang::Scope *S, DeclTy *EnumDecl) 
  {
     ASSERT(!"Unimplemented action");
  }  
 };
};   

#endif
