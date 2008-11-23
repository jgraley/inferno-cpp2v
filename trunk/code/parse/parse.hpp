#ifndef PARSE_HPP
#define PARSE_HPP

#include "common/common.hpp"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallString.h"

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
#include "clang/Parse/Scope.h"
#include "clang/Driver/TextDiagnosticPrinter.h"
#include "clang/Lex/LiteralSupport.h"

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"
#include "common/trace.hpp"
#include "common/type_info.hpp"

#include "inferno_minimal_action.hpp"

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
        clang::TextDiagnosticPrinter diag_printer;
        clang::Diagnostic diags( &diag_printer ); 
        clang::LangOptions opts;
        opts.CPlusPlus = 1; // TODO set based on input file extension
        clang::TargetInfo* ptarget = clang::TargetInfo::CreateTargetInfo(INFERNO_TRIPLE);
        ASSERT(ptarget);
        clang::SourceManager sm;
        clang::HeaderSearch headers( fm );
        
        clang::Preprocessor pp( diags, opts, *ptarget, sm, headers );
        
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
        InfernoAction actions( program, it, pp );                 
        clang::Parser parser( pp, actions );
        TRACE("Start parse\n");
        parser.ParseTranslationUnit();        
        TRACE("End parse\n");
    }
    
private:
    string infile;
    
    class InfernoAction : public InfernoMinimalAction
    {
    public:
        InfernoAction(shared_ptr<Program> p, clang::IdentifierTable &IT, clang::Preprocessor &pp) : 
            InfernoMinimalAction(IT),
            preprocessor(pp)
        {
            decl_scope.push( &*p );
        }
     
        ~InfernoAction()
        {
            decl_scope.pop();
            assert( decl_scope.empty() );
        }
     
    private:   
        // Extend the tree so we can store function parameter decls during prototype parse
        // and read them back at the start of function body, which is a seperate scope.
        struct ParseParameterDeclaration : ObjectDeclaration
        {
            clang::IdentifierInfo *clang_identifier;
        };
        
        // Extend tree so we can insert sub statements at the same level as the label
        struct ParseLabelMarker : LabelMarker
        {
            StmtTy *sub;
        };
        struct ParseCase : Case
        {
            StmtTy *sub;
        };
        struct ParseDefault : Default
        {
            StmtTy *sub;
        };
    
        clang::Preprocessor &preprocessor;
        stack< Sequence<Declaration> * > decl_scope;
        RCHold<Declaration, DeclTy *> hold_decl;
        RCHold<Expression, ExprTy *> hold_expr;
        RCHold<Statement, StmtTy *> hold_stmt;
        RCHold<Type, TypeTy *> hold_type;
        RCHold<Label, void *> hold_label;
        
        clang::Action::TypeTy *isTypeName(const clang::IdentifierInfo &II, clang::Scope *S) 
        {
            shared_ptr<Node> n = InfernoMinimalAction::isTypeNameima(II, S);
            
            if(n)
            {
                shared_ptr<Type> t = dynamic_pointer_cast<UserType>( n );
                ASSERT( t ); // If the node is not a type, n should be NULL
                return hold_type.ToRaw(t);
            }
            
            return 0;
        }

        bool IsInFunction(clang::Scope *S)
        {
            return !!(S->getFnParent());
        }
        
        shared_ptr<Integral> CreateIntegralType( unsigned bits, bool default_signed, clang::DeclSpec::TSS type_spec_signed )
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
            
            i->width = bits;
            
            return i;
        }
        
        shared_ptr<Floating> CreateFloatingType( unsigned bits )
        {
            shared_ptr<Floating> f( shared_ptr<Floating>( new Floating ) );
            f->width = bits;
            return f;
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
                        TRACE("struct/union/class\n");
                        // Disgustingly, clang casts the DeclTy returned from ActOnTag() to 
                        // a TypeTy. 
                        return dynamic_pointer_cast<Holder>( hold_decl.FromRaw( DS.getTypeRep() ) );
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
                        shared_ptr<Function> f(new Function);
                        for( int i=0; i<fchunk.NumArgs; i++ )
                        {
                            shared_ptr<Declaration> d = hold_decl.FromRaw( fchunk.ArgInfo[i].Param );
                            shared_ptr<ObjectDeclaration> vd = dynamic_pointer_cast<ObjectDeclaration>(d); // TODO just push the declarators, no need for dynamic cast?
                            f->parameters.push_back( vd );
                        }
                        TRACE("function returning...\n");
                        f->return_type = CreateTypeNode( D, depth+1 );                        
                        return f;
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
        
        shared_ptr<Object> CreateObjectNode( clang::Scope *S, clang::Declarator &D )
        { 
            shared_ptr<Object> o(new Object());
            clang::IdentifierInfo *ID = D.getIdentifier();
            const clang::DeclSpec &DS = D.getDeclSpec();
            if(ID)
            {
                o->identifier = ID->getName();
                TRACE("object %s\n", o->identifier.c_str());
            }
            else
            {
                TRACE();
            }

            switch( DS.getStorageClassSpec() )
            {
            case clang::DeclSpec::SCS_unspecified:
                o->storage = Object::DEFAULT;
                break;
            case clang::DeclSpec::SCS_static:
                o->storage = Object::STATIC;
                break;
            case clang::DeclSpec::SCS_auto:
                o->storage = Object::AUTO;
                break;
            default:
                ASSERT(!"Unsupported storage class");
                break;
            }
            
            o->type = CreateTypeNode( D );
            
            (void)InfernoMinimalAction::ActOnDeclarator( S, D, 0, o );     
            return o;
        }

        shared_ptr<Typedef> CreateTypedefNode( clang::Scope *S, clang::Declarator &D )
        { 
            shared_ptr<Typedef> t(new Typedef);
            clang::IdentifierInfo *ID = D.getIdentifier();
            if(ID)
                t->identifier = ID->getName();
            t->type = CreateTypeNode( D );

            (void)InfernoMinimalAction::ActOnDeclarator( S, D, 0, t );     // TODO rename this function
            TRACE("%s %p %p\n", ID->getName(), t.get(), ID );            
            return t;
        }

        shared_ptr<Label> CreateLabelNode( clang::IdentifierInfo *ID )
        { 
            shared_ptr<Label> l(new Label);            
            l->identifier = ID->getName();
            TRACE("%s %p %p\n", ID->getName(), l.get(), ID );            
            return l;
        }
        
        shared_ptr<Declaration> CreateDelcaration( clang::Scope *S, clang::Declarator &D )
        {
            const clang::DeclSpec &DS = D.getDeclSpec();
            shared_ptr<Declaration> d;
            if( DS.getStorageClassSpec() == clang::DeclSpec::SCS_typedef )
            {
                shared_ptr<Typedef> t = CreateTypedefNode( S, D );                
                if( !IsInFunction(S) ) // TODO are typedefs legal in functions?
                    decl_scope.top()->push_back( t );
                TRACE();
                d = t;
            }    
            else
            {                
                shared_ptr<ObjectDeclaration> od(new ObjectDeclaration);
                od->object = CreateObjectNode( S, D );        
                od->initialiser = shared_ptr<Expression>(); // might fill in later if initialised
                TRACE("aod %s %p %p\n", od->object->identifier.c_str(), od->object.get(), od.get() );
                if( !IsInFunction(S) )
                    decl_scope.top()->push_back( od );
                d = od;
            }
            
            // Default the access specifier
            d->access = Declaration::PUBLIC; 
            return d;
        }
        
        virtual DeclTy *ActOnDeclarator( clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup )
        {
            // TODO the spurious char __builtin_va_list; line comes from the target info.
            // Create an inferno target info customised for Inferno that doesn't do this.
            //TRACE("\"%s\"\n", D.getIdentifier()->getName().c_str() ); 
            if( strcmp(D.getIdentifier()->getName(), "__builtin_va_list")==0 )
            {            
                return 0;
            }
 
            shared_ptr<Declaration> d = CreateDelcaration( S, D );            
            return hold_decl.ToRaw( d );
        }
        
        /// ActOnParamDeclarator - This callback is invoked when a parameter
        /// declarator is parsed. This callback only occurs for functions
        /// with prototypes. S is the function prototype scope for the
        /// parameters (C++ [basic.scope.proto]).
        virtual DeclTy *ActOnParamDeclarator(clang::Scope *S, clang::Declarator &D) 
        {
            shared_ptr<ParseParameterDeclaration> p(new ParseParameterDeclaration);
            p->object = CreateObjectNode( S, D );
            p->access = Declaration::PUBLIC;        
            p->initialiser = shared_ptr<Expression>(); // might fill in later if init
            p->clang_identifier = D.getIdentifier(); // allow us to re-register the object
            TRACE("aopd %s %p %p\n", 0, p->object.get(), p.get() );
            return hold_decl.ToRaw( p );
        }

        virtual void AddInitializerToDecl(DeclTy *Dcl, ExprTy *Init) 
        {
            shared_ptr<Declaration> d = hold_decl.FromRaw( Dcl );            
            shared_ptr<ObjectDeclaration> od = dynamic_pointer_cast<ObjectDeclaration>(d);
            ASSERT( od ); // Only objects can be initialised
            
            shared_ptr<Expression> e = hold_expr.FromRaw( Init );
            
            od->initialiser = e;            
        }

        // Clang tends to parse parameters and function bodies in seperate
        // scopes so when we see them being used we don't recognise them
        // and cannot link back to the correct Object node. This function
        // puts all the params back in the current scope assuming:
        // 1. They have been added to the Function node correctly and
        // 2. The pass-specific extension ParseParameterDeclaration has been used
        void AddParamsToScope( shared_ptr<Function> fp, clang::Scope *FnBodyScope )
        {
            ASSERT(fp);
            for( int i=0; i<fp->parameters.size(); i++ )
            {
                TRACE();
                shared_ptr<ParseParameterDeclaration> ppd = dynamic_pointer_cast<ParseParameterDeclaration>( fp->parameters[i] );                
                ASSERT(ppd);
                InfernoMinimalAction::AddNakedIdentifier(FnBodyScope, ppd->clang_identifier, ppd->object, false);
            }
        }

        /*
        Note: the default implementation of ActOnStartOfFunctionDef() appears in
        InfernoMinimalAction and can cause spurious ActOnDeclarator() calls if we always
        call through. Therefore we don't and instead just call explicitly implemented
        functions in InfernoMinimalAction where required.
        TODO consider removing this entirely - clang::Action base class will then
             invoke ActOnDeclaration() followed by ActOnStartOfFunctionDef(DeclTy*)
             and this might be correct.
        */
        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
        {
            TRACE();
            shared_ptr<ObjectDeclaration> p(new ObjectDeclaration);
            clang::Scope *GlobalScope = FnBodyScope->getParent();
            p->object = CreateObjectNode( GlobalScope, D );
            p->access = Declaration::PUBLIC;        // TODO not sure how to get access spec for this     
            shared_ptr<Function> fp = dynamic_pointer_cast<Function>( p->object->type );
            AddParamsToScope( fp, FnBodyScope );
            
            decl_scope.top()->push_back( p );
            return hold_decl.ToRaw( p );     
        }

        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, DeclTy *D) 
        {
            TRACE();
            shared_ptr<ObjectDeclaration> p = dynamic_pointer_cast<ObjectDeclaration>(hold_decl.FromRaw(D));
            ASSERT(p);
            shared_ptr<Function> fp = dynamic_pointer_cast<Function>( p->object->type );
            AddParamsToScope( fp, FnBodyScope );
            
            return hold_decl.ToRaw( p );     
        }
        
        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
        {
            TRACE();
            shared_ptr<ObjectDeclaration> fd( dynamic_pointer_cast<ObjectDeclaration>( hold_decl.FromRaw(Decl) ) );
            ASSERT(fd);
            shared_ptr<Expression> e( dynamic_pointer_cast<Expression>( hold_stmt.FromRaw(Body) ) );
            ASSERT(e); // function body must be a scope or 0
            fd->initialiser = e;
            return Decl;
        }    
        
        virtual StmtResult ActOnExprStmt(ExprTy *Expr) 
        {
            shared_ptr<Expression> e = hold_expr.FromRaw(Expr);
            //TRACE("aoes %p\n", es.get() );
            return hold_stmt.ToRaw( e );
        }

        virtual StmtResult ActOnReturnStmt( clang::SourceLocation ReturnLoc,
                                            ExprTy *RetValExp ) 
        {           
            shared_ptr<Expression> e = hold_expr.FromRaw(RetValExp);
            shared_ptr<Return> r(new Return);
            r->return_value = e;
            TRACE("aors %p\n", r.get() );
            return hold_stmt.ToRaw( r );
        }

        virtual ExprResult ActOnIdentifierExpr( clang::Scope *S, 
                                                clang::SourceLocation Loc,
                                                clang::IdentifierInfo &II,
                                                bool HasTrailingLParen ) 
        {
            TRACE("aoie %s\n", II.getName() );
            shared_ptr<Identifier> i = dynamic_pointer_cast<Identifier>( InfernoMinimalAction::GetCurrentIdentifierRCPtr( II ) );
            ASSERT(i);
            return hold_expr.ToRaw( i );            
        }                                   
        
        llvm::APInt EvaluateNumericConstant(const clang::Token &tok)
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
                // Get the value in the widest-possible width.
                llvm::APInt rv(64, 0);
               
                bool err = literal.GetIntegerValue(rv);
                
                ASSERT( !err && "could not understand numeric literal" );
                return rv;
            }
            ASSERT(!"only integer literals supported"); // todo floating point
        }
        
        virtual ExprResult ActOnNumericConstant(const clang::Token &tok) 
        { 
            shared_ptr<NumericConstant> nc(new NumericConstant);
            *(llvm::APInt *)(nc.get()) = EvaluateNumericConstant( tok );
            return hold_expr.ToRaw( nc );            
        } 
  
        virtual ExprResult ActOnBinOp(clang::Scope *S,
                                      clang::SourceLocation TokLoc, clang::tok::TokenKind Kind,
                                      ExprTy *LHS, ExprTy *RHS) 
        {
            TRACE(); 
            shared_ptr<Infix> o(new Infix);
            o->kind = Kind;
            o->operands.push_back( hold_expr.FromRaw(LHS) );
            o->operands.push_back( hold_expr.FromRaw(RHS) );
            return hold_expr.ToRaw( o );            
        }                     

        virtual ExprResult ActOnPostfixUnaryOp(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
                                               ExprTy *Input) 
        {
            shared_ptr<Postfix> o(new Postfix);
            o->kind = Kind;
            o->operands.push_back( hold_expr.FromRaw(Input) );
            return hold_expr.ToRaw( o );            
        }                     

        virtual ExprResult ActOnUnaryOp(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
                                               ExprTy *Input) 
        {
            shared_ptr<Prefix> o(new Prefix);
            o->kind = Kind;
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

        virtual ExprResult ActOnCallExpr(ExprTy *Fn, clang::SourceLocation LParenLoc,
                                         ExprTy **Args, unsigned NumArgs,
                                         clang::SourceLocation *CommaLocs,
                                         clang::SourceLocation RParenLoc) 
        {
            shared_ptr<Call> c(new Call);
            c->function = hold_expr.FromRaw(Fn);
            for(int i=0; i<NumArgs; i++ )
                c->arguments.push_back( hold_expr.FromRaw(Args[i]) );
            return hold_expr.ToRaw( c );
        }
        
        // Not sure if this one has been tested!!
        virtual TypeResult ActOnTypeName(clang::Scope *S, clang::Declarator &D) 
        {
            shared_ptr<Type> t = CreateTypeNode( D );
            return hold_type.ToRaw( t );
        }
        
        void PushStmt( shared_ptr<Scope> s, StmtTy *stmt )
        {
            shared_ptr<Statement> st = hold_stmt.FromRaw(stmt);
            s->push_back( st );
                
            // Flatten the "sub" statements of labels etc
            if( shared_ptr<ParseLabelMarker> plm = dynamic_pointer_cast<ParseLabelMarker>( st ) )
                PushStmt( s, plm->sub );   
            else if( shared_ptr<ParseCase> pc = dynamic_pointer_cast<ParseCase>( st ) )
                PushStmt( s, pc->sub );   
            else if( shared_ptr<ParseDefault> pc = dynamic_pointer_cast<ParseDefault>( st ) )
                PushStmt( s, pc->sub );   
        }

        virtual StmtResult ActOnCompoundStmt(clang::SourceLocation L, clang::SourceLocation R,
                                             StmtTy **Elts, unsigned NumElts,
                                             bool isStmtExpr) 
        {
            shared_ptr<Scope> s(new Scope);

            for( int i=0; i<NumElts; i++ )
                PushStmt( s, Elts[i] );

            return hold_stmt.ToRaw( s );
        }
        
        virtual StmtResult ActOnDeclStmt(DeclTy *Decl, clang::SourceLocation StartLoc,
                                         clang::SourceLocation EndLoc) 
        {
            shared_ptr<Declaration> d( hold_decl.FromRaw(Decl) );
            return hold_stmt.ToRaw( d );
        }
        
        virtual StmtResult ActOnLabelStmt(clang::SourceLocation IdentLoc, clang::IdentifierInfo *II,
                                          clang::SourceLocation ColonLoc, StmtTy *SubStmt) 
        {
            if( !(II->getFETokenInfo<void *>()) )                        
                II->setFETokenInfo( hold_label.ToRaw( CreateLabelNode( II ) ) );
            
            shared_ptr<ParseLabelMarker> l( new ParseLabelMarker );
            l->label = hold_label.FromRaw( II->getFETokenInfo<void *>() );
            l->sub = SubStmt;
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
        virtual StmtResult ActOnCaseStmt(clang::SourceLocation CaseLoc, ExprTy *LHSVal,
                                         clang::SourceLocation DotDotDotLoc, ExprTy *RHSVal,
                                         clang::SourceLocation ColonLoc, StmtTy *SubStmt) 
        {
            TRACE();
            ASSERT(!RHSVal && "gcc extension ... not supported");
            shared_ptr<ParseCase> c( new ParseCase );
            c->value = hold_expr.FromRaw( LHSVal );
            c->sub = SubStmt;
            return hold_stmt.ToRaw( c );
        }
        
        virtual StmtResult ActOnDefaultStmt(clang::SourceLocation DefaultLoc,
                                            clang::SourceLocation ColonLoc, StmtTy *SubStmt,
                                            clang::Scope *CurScope)
        {
            TRACE();
            shared_ptr<ParseDefault> d( new ParseDefault );
            d->sub = SubStmt;
            return hold_stmt.ToRaw( d );
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
        
        virtual DeclTy *ActOnCXXMemberDeclarator(clang::Scope *S, clang::AccessSpecifier AS,
                                                 clang::Declarator &D, ExprTy *BitfieldWidth,
                                                 ExprTy *Init, DeclTy *LastInGroup) 
        {
            TRACE("Member %p\n", Init);
            shared_ptr<Declaration> d = CreateDelcaration( S, D );
      
            TRACE();
            if( Init )
            {  
                shared_ptr<ObjectDeclaration> od = dynamic_pointer_cast<ObjectDeclaration>(d);
                ASSERT( od ); // Only objects may be initialised
                od->initialiser = hold_expr.FromRaw( Init );
            }
                       
            TRACE("as=%d\n", (unsigned)AS);
            
            // Fill in access specifier
            switch( AS )
            {
                case clang::AS_public:
                    d->access = Declaration::PUBLIC;
                    break;
                case clang::AS_protected:
                    d->access = Declaration::PROTECTED;
                    break;
                case clang::AS_private:
                    d->access = Declaration::PRIVATE;
                    break;
                case clang::AS_none:
                default:
                    ASSERT(0);
                    break;
            }
            
            // TODO set bitfield width (make a worker function for ActOnDeclarator())
            return hold_decl.ToRaw( d );
        }

        virtual DeclTy *ActOnTag(clang::Scope *S, unsigned TagType, TagKind TK,
                                 clang::SourceLocation KWLoc, clang::IdentifierInfo *Name,
                                 clang::SourceLocation NameLoc, clang::AttributeList *Attr) 
        {
            TRACE("Tag type %d\n", TagType);
            // TagType is an instance of DeclSpec::TST, indicating what kind of tag this
            // is (struct/union/enum/class).
            shared_ptr<Holder> h;
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
                    ASSERT(!"TODO add enum");
                    break;
                default:
                    ASSERT(!"Unknown type spec type");            
                    break;        
            }
            
            if(Name)
            {
                h->identifier = Name->getName();
                (void)InfernoMinimalAction::AddNakedIdentifier(S, Name, h, true); 
            }
            else
            {
                // TODO make a general-lurpose anon name generator
                char an[20];
                static int ac=0;
                sprintf( an, "__anon%d", ac++ );
                h->identifier = an;
            }
            
            h->access = Declaration::PUBLIC; // must make all holder type decls public since clang doesnt seem to give us an AS
            
            //TODO should we do something with TagKind? Maybe needed for render.
            //TODO use the attibutes
            
            return hold_decl.ToRaw( h );            
        }
   
        /// ActOnStartCXXClassDef - This is called at the start of a class/struct/union
        /// definition, when on C++.
        virtual void ActOnStartCXXClassDef(clang::Scope *S, DeclTy *TagDecl,
                                           clang::SourceLocation LBrace) 
        {
            TRACE();
            // Just populate the members container for the Holder node
            // we already created. No need to return anything.
            shared_ptr<Declaration> d = hold_decl.FromRaw( TagDecl );
            shared_ptr<Holder> h = dynamic_pointer_cast<Holder>(d);
            decl_scope.push( &(h->members) );      // decls for members will go on this scope      
        }
  
        /// ActOnFinishCXXClassDef - This is called when a class/struct/union has
        /// completed parsing, when on C++.
        virtual void ActOnFinishCXXClassDef(DeclTy *TagDecl) 
        {
            TRACE();
            decl_scope.pop(); // class scope is complete
            // TODO are structs etc definable in functions? If so, this will put the decl outside the function
            decl_scope.top()->push_back( hold_decl.FromRaw(TagDecl) );
        }
        
        virtual ExprResult ActOnMemberReferenceExpr(ExprTy *Base, clang::SourceLocation OpLoc,
                                                    clang::tok::TokenKind OpKind,
                                                    clang::SourceLocation MemberLoc,
                                                    clang::IdentifierInfo &Member) 
        {
            TRACE("kind %d\n", OpKind);
            shared_ptr<Access> a( new Access );
            if( OpKind == clang::tok::arrow )  // Base->Member
            {
                shared_ptr<Prefix> p( new Prefix );
                p->operands.push_back( hold_expr.FromRaw( Base ) );
                p->kind = clang::tok::star;
                a->base = p;
            }
            else if( OpKind == clang::tok::period ) // Base.Member
            {
                a->base = hold_expr.FromRaw( Base );
            }
            else
            {
                ASSERT(!"Unknown token accessing member");
            }            
        
            shared_ptr<Object> o( new Object );
            o->identifier = Member.getName(); // Only the name is filled in TODO fill in (possibly in a pass)
            a->member = o;    
            return hold_expr.ToRaw( a );
        }
                
        virtual ExprResult ActOnArraySubscriptExpr(ExprTy *Base, clang::SourceLocation LLoc,
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
            shared_ptr<NumericConstant> nc(new NumericConstant);
            TRACE("true/false tk %d %d %d\n", Kind, clang::tok::kw_true, clang::tok::kw_false );
            *(llvm::APInt *)(nc.get()) = (Kind == clang::tok::kw_true) ? llvm::APInt( 1, 1 ) : llvm::APInt( 1, 0 );
            return hold_expr.ToRaw( nc );                       
        }

        virtual ExprResult ActOnCastExpr(clang::SourceLocation LParenLoc, TypeTy *Ty,
                                         clang::SourceLocation RParenLoc, ExprTy *Op) 
        {
            shared_ptr<Cast> c(new Cast);
            c->operand = hold_expr.FromRaw( Op );
            c->type = hold_type.FromRaw( Ty );
            return hold_expr.ToRaw( c );                       
        }
        
        virtual StmtResult ActOnNullStmt(clang::SourceLocation SemiLoc) 
        {
            TRACE();
            shared_ptr<Nop> n(new Nop);
            return hold_stmt.ToRaw( n );                       
        }
        
        
        //--------------------------------------------- unimplemented actions -----------------------------------------------     
        // Note: only actions that return something (so we don't get NULL XTy going around the place). No obj-C or GCC 
        // extensions. These all assert out immediately.
        
  virtual DeclTy *ActOnFileScopeAsmDecl(clang::SourceLocation Loc, ExprTy *AsmString) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  /// ParsedFreeStandingDeclSpec - This method is invoked when a declspec with
  /// no declarator (e.g. "struct foo;") is parsed.
  virtual DeclTy *ParsedFreeStandingDeclSpec(clang::Scope *S, DeclSpec &DS) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnField(clang::Scope *S, clang::SourceLocation DeclStart,
                             clang::Declarator &D, ExprTy *BitfieldWidth) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnIvar(clang::Scope *S, clang::SourceLocation DeclStart,
                            clang::Declarator &D, ExprTy *BitfieldWidth,
                            clang::tok::ObjCKeywordKind visibility) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnEnumConstant(clang::Scope *S, DeclTy *EnumDecl,
                                    DeclTy *LastEnumConstant,
                                    clang::SourceLocation IdLoc, clang::IdentifierInfo *Id,
                                    clang::SourceLocation EqualLoc, ExprTy *Val) {
    ASSERT(!"Unimplemented action");
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

  virtual ExprResult ActOnCharacterConstant(const Token &) { return 0; }

  /// ActOnStringLiteral - The specified tokens were lexed as pasted string
  /// fragments (e.g. "foo" "bar" L"baz").
  virtual ExprResult ActOnStringLiteral(const Token *Toks, unsigned NumToks) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult 
    ActOnSizeOfAlignOfTypeExpr(clang::SourceLocation OpLoc, bool isSizeof, 
                               clang::SourceLocation LParenLoc, TypeTy *Ty,
                               clang::SourceLocation RParenLoc) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCompoundLiteral(clang::SourceLocation LParen, TypeTy *Ty,
                                          clang::SourceLocation RParen, ExprTy *Op) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnInitList(clang::SourceLocation LParenLoc,
                                   ExprTy **InitList, unsigned NumInit,
                                   clang::InitListDesignations &Designators,
                                   clang::SourceLocation RParenLoc) {
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

  /// ActOnCXXThis - Parse the C++ 'this' pointer.
  virtual ExprResult ActOnCXXThis(clang::SourceLocation ThisLoc) {
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

  virtual BaseResult ActOnBaseSpecifier(DeclTy *classdecl, 
                                        clang::SourceRange SpecifierRange,
                                        bool Virtual, clang::AccessSpecifier Access,
                                        TypeTy *basetype, 
                                        clang::SourceLocation BaseLoc) {
    ASSERT(!"Unimplemented action");
    return 0;
  }

  virtual MemInitResult ActOnMemInitializer(DeclTy *ConstructorDecl,
                                            clang::Scope *S,
                                            clang::IdentifierInfo *MemberOrBase,
                                            clang::SourceLocation IdLoc,
                                            clang::SourceLocation LParenLoc,
                                            ExprTy **Args, unsigned NumArgs,
                                            clang::SourceLocation *CommaLocs,
                                            clang::SourceLocation RParenLoc) {
    ASSERT(!"Unimplemented action");
    return true;
  }

    };
};   

#endif
