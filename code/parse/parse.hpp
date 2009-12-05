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
#include "helpers/transformation.hpp"
#include "common/trace.hpp"
#include "tree/type_db.hpp"
#include "helpers/misc.hpp"
#include "helpers/typeof.hpp"

#include "identifier_tracker.hpp"

#define INFERNO_TRIPLE "arm-linux"

class Parse : public Transformation
{
public:
    Parse( string i ) :
        infile(i)
    {
    }

    void operator()( shared_ptr<Node> context, shared_ptr<Node> root )
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
        InfernoAction actions( context, root, it, pp, *ptarget );
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
        InfernoAction(shared_ptr<Node> context, shared_ptr<Node> root, clang::IdentifierTable &IT, clang::Preprocessor &pp, clang::TargetInfo &T) :
            preprocessor(pp),
            target_info(T),
            ident_track( context ),
            global_scope( context ),
            all_decls( new Program )
        {
        	shared_ptr<Scope> root_scope = dynamic_pointer_cast<Scope>(root);
        	ASSERT(root_scope)("Can only parse into a scope");
            inferno_scope_stack.push( root_scope ); // things will be pushed into here
            backing_ordering[inferno_scope_stack.top()].clear();
        }

        ~InfernoAction()
        {
            inferno_scope_stack.pop();
            assert( inferno_scope_stack.empty() );
        }

    private:
        // Parameters are parsed outside function scope, so we defer entering them
        // into the ident_track until we're in the function. This stores the clang identifiers.
        map< shared_ptr<Declaration>, clang::IdentifierInfo * > backing_params;

        // The statement after a label is parsed as a sub-construct under the label which
        // is not how the inferno tree does it. Remember that relationship here and
        // generate the extra nodes when rendering a compound statement.
        map< shared_ptr<Label>, shared_ptr<Statement> > backing_labels;
        map< shared_ptr<SwitchTarget>, shared_ptr<Statement> > backing_targets;
        Map< shared_ptr<Declaration>, shared_ptr<Declaration> > backing_paired_decl;

        // Members of records go in an unordered collection, but when parsing
        // we might need the order, eg for C-style initialisers or auto-generated
        // constructor calls.
        Map< shared_ptr<Scope>, Sequence<Declaration> > backing_ordering;

        // In ActOnTag, when we see a record decl, we store it here and generate it
        // at the next IssueDeclaration, called from ActOnDeclaration. This allows
        // a seperate decl for records, since we so not support anon ones, and only
        // allow one thing to be decl'd at a time.
        shared_ptr<Declaration> decl_to_insert;

        clang::Preprocessor &preprocessor;
        clang::TargetInfo &target_info;

        stack< shared_ptr<Scope> > inferno_scope_stack;
        RCHold<Declaration, DeclTy *> hold_decl;
        RCHold<Base, DeclTy *> hold_base;
        RCHold<Expression, ExprTy *> hold_expr;
        RCHold<Statement, StmtTy *> hold_stmt;
        RCHold<Type, TypeTy *> hold_type;
        RCHold<LabelIdentifier, void *> hold_label_identifier;
        RCHold<Node, CXXScopeTy *> hold_scope;
        IdentifierTracker ident_track;
        shared_ptr<Node> global_scope;
        shared_ptr<Program> all_decls; // not the actual program, just a flattening of the decls
                                       // we maintain this because decls don't always make it
                                       // into the tree by the time we need them, thanks to the
                                       // way clang works. Decls go in here immediately.

        OwningStmtResult ToStmt( shared_ptr<Statement> s )
        {
            return OwningStmtResult( *this, hold_stmt.ToRaw( s ) );
        }

        OwningExprResult ToExpr( shared_ptr<Expression> e )
        {
            return OwningExprResult( *this, hold_expr.ToRaw( e ) );
        }

        shared_ptr<Statement> FromClang( const StmtArg &s )
        {
            return hold_stmt.FromRaw( s.get() );
        }

        shared_ptr<Expression> FromClang( const ExprArg &e )
        {
            return hold_expr.FromRaw( e.get() );
        }

        struct DeclarationAsStatement : Statement
        {
        	NODE_FUNCTIONS
        	SharedPtr<Declaration> d;
        };

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
                shared_ptr<UserType> t = dynamic_pointer_cast<UserType>( n );
                if(t)
                    return hold_type.ToRaw(t->identifier);
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

        shared_ptr<Integral> CreateIntegralType( int bits,
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

            i->width = CreateNumericConstant(bits);
            return i;
        }

        shared_ptr<Floating> CreateFloatingType( const llvm::fltSemantics *s )
        {
            shared_ptr<SpecificFloatSemantics> sem( new SpecificFloatSemantics );
            sem->value = s;
            shared_ptr<Floating> f( new Floating );
            f->semantics = sem;
            return f;
        }

        void FillParameters( shared_ptr<Procedure> p, const clang::DeclaratorChunk::FunctionTypeInfo &fchunk )
        {
        	backing_ordering[p].clear(); // ensure at least an empty sequence is in the map
            for( int i=0; i<fchunk.NumArgs; i++ )
            {
                shared_ptr<Declaration> d = hold_decl.FromRaw( fchunk.ArgInfo[i].Param );
                shared_ptr<Instance> inst = dynamic_pointer_cast<Instance>(d);
                ASSERT( inst );
                backing_ordering[p].push_back( inst );
                p->members.insert( inst );
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
                        return CreateIntegralType( TypeDb::integral_bits[DS.getTypeSpecWidth()],
                                                   TypeDb::int_default_signed,
                                                   DS.getTypeSpecSign() );
                        break;
                    case clang::DeclSpec::TST_char:
                        TRACE("char based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateIntegralType( TypeDb::char_bits,
                                                   TypeDb::char_default_signed,
                                                   DS.getTypeSpecSign() );
                        break;
                    case clang::DeclSpec::TST_void:
                        TRACE("void based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return shared_ptr<Type>(new Void());
                        break;
                    case clang::DeclSpec::TST_bool:
                        TRACE("bool based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return shared_ptr<Type>(new Boolean());
                        break;
                    case clang::DeclSpec::TST_float:
                        TRACE("float based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateFloatingType( TypeDb::float_semantics );
                        break;
                    case clang::DeclSpec::TST_double:
                        TRACE("double based %d %d\n", DS.getTypeSpecWidth(), DS.getTypeSpecSign() );
                        return CreateFloatingType( DS.getTypeSpecWidth()==clang::DeclSpec::TSW_long ?
                                                   TypeDb::long_double_semantics :
                                                   TypeDb::double_semantics );
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
                        return dynamic_pointer_cast<Record>( hold_decl.FromRaw( DS.getTypeRep() ) )->identifier;
                        break;
                    default:
                        ASSERTFAIL("unsupported type");
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
                            a->size = shared_new<Uninitialised>();    // number of elements was not specified eg int a[];
                        return a;
                    }

                    default:
                    ASSERTFAIL("Unknown type chunk");
                    break;
                }
            }
        }

        shared_ptr<InstanceIdentifier> CreateInstanceIdentifier( clang::IdentifierInfo *ID = 0 )
        {
            if(ID)
            {
            	shared_ptr<SpecificInstanceIdentifier> ii( new SpecificInstanceIdentifier(ID->getName()) );
                return ii;
            }
            else
            {
            	shared_ptr<SpecificInstanceIdentifier> ii( new SpecificInstanceIdentifier );
                return ii;
            }
        }

        shared_ptr<TypeIdentifier> CreateTypeIdentifier( clang::IdentifierInfo *ID )
        {
        	ASSERT( ID );
            shared_ptr<SpecificTypeIdentifier> ti( new SpecificTypeIdentifier(ID->getName()) );
            return ti;
        }

        shared_ptr<TypeIdentifier> CreateTypeIdentifier( string s )
        {
            shared_ptr<SpecificTypeIdentifier> ti( new SpecificTypeIdentifier( s ) );
            return ti;
        }

        shared_ptr<LabelIdentifier> CreateLabelIdentifier( clang::IdentifierInfo *ID )
        {
        	ASSERT( ID );
            shared_ptr<SpecificLabelIdentifier> li( new SpecificLabelIdentifier(ID->getName()) );
            return li;
        }

        shared_ptr<Instance> CreateInstanceNode( clang::Scope *S,
                                                 clang::Declarator &D,
                                                 shared_ptr<AccessSpec> access = shared_ptr<AccessSpec>(),
                                                 shared_ptr<StorageClass> storage = shared_ptr<StorageClass>() )
        {
            const clang::DeclSpec &DS = D.getDeclSpec();

            if( !storage )
            {
                clang::DeclSpec::SCS scs = DS.getStorageClassSpec();
                switch( scs )
                {
                case clang::DeclSpec::SCS_unspecified:
                {
                	TRACE("scope flags 0x%x\n", S->getFlags());
                    if( S->getFlags() & clang::Scope::CXXClassScope ) // record scope
                    {
                        shared_ptr<Member> ns = shared_new<Member>();
                        storage = ns;
                        if( DS.isVirtualSpecified() )
                            ns->virt = shared_new<Virtual>();
                        else
                            ns->virt = shared_new<NonVirtual>();
                    }
                    else if( S->getFnParent() ) // in code
                    {
                    	storage = shared_new<Auto>();
                    }
                    else // top level
                    {
                        storage = shared_new<Static>();
                        if( !access )
                            access = shared_ptr<Public>(new Public); // unspecified at top level implies "extern", which we call public
                    }
                    break;
                }
                case clang::DeclSpec::SCS_auto:
                    storage = shared_new<Auto>();
                    break;
                case clang::DeclSpec::SCS_extern:// linking will be done "automatically" so no need to remember "extern" in the tree
                    storage = shared_new<Static>();
                    if( !access )
                        access = shared_ptr<Public>(new Public); // we call extern public
                    break;
                case clang::DeclSpec::SCS_static:
                    storage = shared_new<Static>();
                    break;
                default:
                    ASSERTFAIL("Unsupported storage class");
                    break;
                }
            }

            if(!access)
                access = shared_ptr<Private>(new Private); // Most scopes are private unless specified otherwise

            shared_ptr<Instance> o(new Instance());
            all_decls->members.insert(o);

            clang::IdentifierInfo *ID = D.getIdentifier();
            if(ID)
            {
                o->identifier = CreateInstanceIdentifier(ID);
                ident_track.Add( ID, o, S );
            }
            else
            {
                o->identifier = CreateInstanceIdentifier();
            }
            if( DS.getTypeQualifiers() & clang::DeclSpec::TQ_const )
                o->constancy = shared_new<Const>();
            else
                o->constancy = shared_new<NonConst>();
            o->type = CreateTypeNode( D );
            o->storage = storage;
            o->access = access;
            o->initialiser = shared_new<Uninitialised>();

            return o;
        }

        shared_ptr<Typedef> CreateTypedefNode( clang::Scope *S, clang::Declarator &D )
        {
            shared_ptr<Typedef> t(new Typedef);
            all_decls->members.insert(t);
            clang::IdentifierInfo *ID = D.getIdentifier();
            if(ID)
            {
                t->identifier = CreateTypeIdentifier(ID);
                ident_track.Add( ID, t, S );
            }
            t->type = CreateTypeNode( D );

            TRACE("%s %p %p\n", ID->getName(), t.get(), ID );
            return t;
        }
/*
        shared_ptr<Label> CreateLabelNode( clang::IdentifierInfo *ID )
        {
            shared_ptr<Label> l(new Label);
            all_decls->members.insert(l);
            l->access = shared_new<Public>();
            l->identifier = CreateLabelIdentifier(ID);
            TRACE("%s %p %p\n", ID->getName(), l.get(), ID );
            return l;
        }
*/
        shared_ptr<Declaration> FindExistingDeclaration( const clang::CXXScopeSpec &SS, clang::IdentifierInfo *ID, bool recurse )
        {
        	if( !ID )
        	    return shared_ptr<Declaration>(); // No name specified => doesn't match anything

            // See if we already have this record in the current scope, or specified scope
            // if Declarator has one
            shared_ptr<Node> cxxs = FromCXXScope( &SS );

            // Use C++ scope if non-NULL; do not recurse (=precise match only)
            shared_ptr<Node> found_n = ident_track.TryGet( ID, cxxs, recurse );
            TRACE("Looked for %s, result %p (%p)\n", ID->getName(), found_n.get(), cxxs.get() );
            if( !found_n )
            {
            	// Nothing was found with the supplied name
                ASSERT( !cxxs ); // If C++ scope was given explicitly, require successful find
                return shared_ptr<Declaration>();
            }

            shared_ptr<Declaration> found_d = dynamic_pointer_cast<Declaration>( found_n );
            // If the found match is not a declaration, cast will fail and we'll return NULL for "not found"
            return found_d;
        }

        // Alternative parameters
        shared_ptr<Declaration> FindExistingDeclaration( clang::Declarator &D, bool recurse )
        {
        	return FindExistingDeclaration( D.getCXXScopeSpec(), D.getIdentifier(), recurse );
        }

        shared_ptr<Declaration> CreateDelcaration( clang::Scope *S, clang::Declarator &D, shared_ptr<AccessSpec> a = shared_ptr<AccessSpec>() )
        {
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
                shared_ptr<Instance> o = CreateInstanceNode( S, D, a );
                d = o;
            }

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
                inferno_scope_stack.top()->members.insert( decl_to_insert );
                backing_ordering[inferno_scope_stack.top()].push_back( decl_to_insert );
                backing_paired_decl[d] = decl_to_insert;
                decl_to_insert = shared_ptr<Declaration>(); // don't need to generate it again
                TRACE("inserted decl\n" );
            }

            inferno_scope_stack.top()->members.insert( d );
            backing_ordering[inferno_scope_stack.top()].push_back( d );
            TRACE("no insert\n" );
            return hold_decl.ToRaw( d );
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

            shared_ptr<Declaration> d = FindExistingDeclaration( D, false ); // decl exists already?
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

            shared_ptr<Instance> p = CreateInstanceNode( S, D, shared_new<Public>(), shared_new<Auto>() );
            backing_params[p] = D.getIdentifier(); // allow us to register the object with ident_track once we're in the function body scope
            return hold_decl.ToRaw( p );
        }

        virtual void AddInitializerToDecl(DeclTy *Dcl, ExprArg Init)
        {
            shared_ptr<Declaration> d = hold_decl.FromRaw( Dcl );

            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(d);
            ASSERT( o ); // Only objects can be initialised

            o->initialiser = FromClang( Init );

            // At this point, when we have the instance (and hence the type) and the initialiser
            // we can detect when an array initialiser has been inserted for a record instance and
            // change it.
            if( shared_ptr<ArrayLiteral> ai = dynamic_pointer_cast<ArrayLiteral>(o->initialiser) )
                if( shared_ptr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(o->type) )
                	if( shared_ptr<Record> r = GetRecordDeclaration(all_decls, ti) )
                		o->initialiser = CreateRecordLiteralFromArrayLiteral( ai, r );
        }

        // Clang tends to parse parameters and function bodies in seperate
        // scopes so when we see them being used we don't recognise them
        // and cannot link back to the correct Instance node. This function
        // puts all the params back in the current scope assuming:
        // 1. They have been added to the Function node correctly and
        // 2. They feature in the backing list for params
        void AddParamsToScope( shared_ptr<Procedure> pp, clang::Scope *FnBodyScope )
        {
            ASSERT(pp);

            FOREACH(shared_ptr<Declaration> param, pp->members )
            {
                TRACE();
                clang::IdentifierInfo *paramII = backing_params[param];
                backing_params.erase( param );
                TRACE("%p %p %s\n", param.get(), paramII, paramII->getName());
                if( paramII )
                    ident_track.Add( paramII, param, FnBodyScope );
            }
        }

        // JSG this is like the default in Actions, except it passes the parent of the function
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

            // This is just a junk scope because we will not use scopes collected
            // via the inferno_scope_stack mechanism within functions; instead, they
            // will appear among the Statements managed by clang and passed into
            // ActOnFinishFunctionBody() as a hierarchy of Compounds.
            // If we tried to do this ourselves we'd lose the nested compound
            // statement hierarchy.
            inferno_scope_stack.push( shared_ptr<Scope>(new Scope) );

            return hold_decl.ToRaw( o );
        }

        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtArg Body)
        {
            TRACE();
            shared_ptr<Instance> o( dynamic_pointer_cast<Instance>( hold_decl.FromRaw(Decl) ) );
            ASSERT(o);
            shared_ptr<Compound> cb( dynamic_pointer_cast<Compound>( FromClang( Body ) ) );
            ASSERT(cb); // function body must be a scope or 0

            if( dynamic_pointer_cast<Uninitialised>( o->initialiser ) )
                o->initialiser = cb;
            else if( shared_ptr<Compound> c = dynamic_pointer_cast<Compound>( o->initialiser ) )
                c->statements = c->statements + cb->statements;
            else
                ASSERTFAIL("wrong thing in function instance");

            TRACE("finish fn %d statements %d total\n", cb->statements.size(), (dynamic_pointer_cast<Compound>(o->initialiser))->statements.size() );

            inferno_scope_stack.pop(); // we dont use these - we use the clang-managed compound statement instead (passed in via Body)
            return Decl;
        }

        virtual OwningStmtResult ActOnExprStmt(ExprArg Expr)
        {
        	// TODO most of this is now unnecessary
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
            shared_ptr<Return> r(new Return);
            if( RetValExp )
                r->return_value = hold_expr.FromRaw(RetValExp);
            else
                r->return_value = shared_new<Uninitialised>();
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
            return hold_expr.ToRaw( o->identifier );
        }

        shared_ptr<Integer> CreateNumericConstant( int value )
        {
            shared_ptr<SpecificInteger> nc( new SpecificInteger(value) );
            return nc;
        }

        shared_ptr<Literal> CreateLiteral( int value )
        {
            return CreateNumericConstant( value );
        }

        shared_ptr<Number> CreateNumericConstant(const clang::Token &tok)
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
                    bits = TypeDb::integral_bits[clang::DeclSpec::TSW_long];
                else if( literal.isLongLong )
                    bits = TypeDb::integral_bits[clang::DeclSpec::TSW_longlong];
                else
                    bits = TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified];

                llvm::APSInt rv(bits, literal.isUnsigned);
                bool err = literal.GetIntegerValue(rv);

                ASSERT( !err && "numeric literal too big for its own type" );
                shared_ptr<SpecificInteger> nc( new SpecificInteger );
                nc->value = rv;
                return nc;
            }
            else if( literal.isFloatingLiteral() )
            {
                const llvm::fltSemantics *semantics;
                if( literal.isLong )
                    semantics = TypeDb::long_double_semantics;
                else if( literal.isFloat )
                    semantics = TypeDb::float_semantics;
                else
                    semantics = TypeDb::double_semantics;
                llvm::APFloat rv( literal.GetFloatValue( *semantics ) );

                shared_ptr<SpecificFloat> fc( new SpecificFloat( rv ) );
                return fc;
            }
            ASSERTFAIL("this sort of literal is not supported");
        }

        virtual ExprResult ActOnNumericConstant(const clang::Token &tok)
        {
            return hold_expr.ToRaw( CreateNumericConstant( tok ) );
        }

        virtual ExprResult ActOnBinOp(clang::Scope *S,
                                      clang::SourceLocation TokLoc, clang::tok::TokenKind Kind,
                                      ExprTy *LHS, ExprTy *RHS)
        {
            TRACE();
            shared_ptr<Operator> o = shared_ptr<Operator>();
            switch( Kind )
            {
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
	        case clang::tok::TOK: \
                o=shared_ptr<NODE>(new NODE);\
                break;
#include "tree/operator_db.inc"
            }
            ASSERT( o );
            if( shared_ptr<NonCommutativeOperator> nco = dynamic_pointer_cast< NonCommutativeOperator >(o) )
            {
            	nco->operands.push_back( hold_expr.FromRaw(LHS) );
                nco->operands.push_back( hold_expr.FromRaw(RHS) );
            }
            else if( shared_ptr<CommutativeOperator> co = dynamic_pointer_cast< CommutativeOperator >(o) )
            {
               	co->operands.insert( hold_expr.FromRaw(LHS) );
                co->operands.insert( hold_expr.FromRaw(RHS) );
            }
            else
            	ASSERTFAIL("Binop was apparently neither Commutative nor NonCommutative");

            return hold_expr.ToRaw( o );
        }

        virtual ExprResult ActOnPostfixUnaryOp(clang::Scope *S, clang::SourceLocation OpLoc,
                                               clang::tok::TokenKind Kind, ExprTy *Input)
        {
            shared_ptr<NonCommutativeOperator> o = shared_ptr<NonCommutativeOperator>();

            switch( Kind )
            {
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
	        case clang::tok::TOK: \
	            o=shared_ptr<NODE>(new NODE); \
	            break;
#include "tree/operator_db.inc"
            }
            ASSERT( o );
            o->operands.push_back( hold_expr.FromRaw(Input) );
            return hold_expr.ToRaw( o );
        }

        virtual ExprResult ActOnUnaryOp( clang::Scope *S, clang::SourceLocation OpLoc,
                                         clang::tok::TokenKind Kind, ExprTy *Input)
        {
            shared_ptr<NonCommutativeOperator> o = shared_ptr<NonCommutativeOperator>();

            switch( Kind )
            {
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
	        case clang::tok::TOK:\
                o=shared_ptr<NODE>(new NODE); \
                break;
#include "tree/operator_db.inc"
            }
            ASSERT( o );
            o->operands.push_back( hold_expr.FromRaw(Input) );
            return hold_expr.ToRaw( o );
        }

       virtual ExprResult ActOnConditionalOp(clang::SourceLocation QuestionLoc,
                                             clang::SourceLocation ColonLoc,
                                             ExprTy *Cond, ExprTy *LHS, ExprTy *RHS)
        {
            shared_ptr<ConditionalOperator> co(new ConditionalOperator);
            co->operands.push_back( hold_expr.FromRaw(Cond) );
            ASSERT(LHS && "gnu extension not supported");
            co->operands.push_back( hold_expr.FromRaw(LHS) );
            co->operands.push_back( hold_expr.FromRaw(RHS) );
            return hold_expr.ToRaw( co );
        }

        shared_ptr<Call> CreateCall( Sequence<Expression> &args, shared_ptr<Expression> callee )
        {
            // Make the Call node and fill in the called function
             shared_ptr<Call> c(new Call);
             c->callee = callee;

             // If Procedure or Function, fill in the args map based on the supplied args and original function type
             shared_ptr<Type> t = TypeOf(all_decls).Get(callee);
             if( shared_ptr<Procedure> p = dynamic_pointer_cast<Procedure>(t) )
                 PopulateMapOperator( c, args, p );

             return c;
        }

        virtual ExprResult ActOnCallExpr(clang::Scope *S, ExprTy *Fn, clang::SourceLocation LParenLoc,
                                         ExprTy **Args, unsigned NumArgs,
                                         clang::SourceLocation *CommaLocs,
                                         clang::SourceLocation RParenLoc)
        {
        	// Get the args in a Sequence
            Sequence<Expression> args;
            CollectArgs( &args, Args, NumArgs );
            shared_ptr<Call> c = CreateCall( args, hold_expr.FromRaw(Fn) );
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
           /* if( shared_ptr<ParseTwin> pt = dynamic_pointer_cast<ParseTwin>( st ) )
            {
                PushStmt( s, pt->d1 );
                PushStmt( s, pt->d2 );
                return;
            }
            */
            if( shared_ptr<Declaration> d = dynamic_pointer_cast<Declaration>( st ) )
            {
                if( backing_paired_decl.IsExist(d) )
                {
                	shared_ptr<Declaration> bd = backing_paired_decl[d];
                    ASSERT( bd );

                    if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(bd) )
                    	PushStmt( s, i ); // Instances can have inits that require being in order, so append as a statement
                    else
                    	s->members.insert( bd );

                    backing_paired_decl.erase(d);
                }
            }

            if( shared_ptr<DeclarationAsStatement> das = dynamic_pointer_cast<DeclarationAsStatement>(st) )
            	s->members.insert( das->d );
            else
            	s->statements.push_back( st );

            // Flatten the "sub" statements of labels etc
            if( shared_ptr<Label> l = dynamic_pointer_cast<Label>( st ) )
            {
                ASSERT( backing_labels[l] );
                PushStmt( s, backing_labels[l] );
                backing_labels.erase(l);
            }
            else if( shared_ptr<SwitchTarget> t = dynamic_pointer_cast<SwitchTarget>( st ) )
            {
                ASSERT( backing_targets[t] );
                PushStmt( s, backing_targets[t] );
                backing_targets.erase(t);
            }
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
            // Basically we are being asked to turn a Declaration, which has already been parsed,
            // into a Statement. Instances are already both Declarations and Statements, so that's
            // OK. In other cases, we have to package up the Declaration in a special kind of
            // Statement node and pass it through that way. We will unpack later.
            if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(d) )
            {
            	return ToStmt( i );
            }
            else
            {
            	shared_ptr<DeclarationAsStatement> das( new DeclarationAsStatement );
            	das->d = d;
            	return ToStmt( das );
            }
        }

        // Create a label identifier if there isn't already one with the same name (TODO scopes?)
        shared_ptr<LabelIdentifier> MaybeCreateLabelIdentifier( clang::IdentifierInfo *II )
        {
            if( !(II->getFETokenInfo<void *>()) )
                II->setFETokenInfo( hold_label_identifier.ToRaw( CreateLabelIdentifier( II ) ) );

            return hold_label_identifier.FromRaw( II->getFETokenInfo<void *>() );
        }

        virtual StmtResult ActOnLabelStmt(clang::SourceLocation IdentLoc, clang::IdentifierInfo *II,
                                          clang::SourceLocation ColonLoc, StmtTy *SubStmt)
        {
            shared_ptr<Label> l( new Label );
            l->identifier = MaybeCreateLabelIdentifier(II);
            backing_labels[l] = hold_stmt.FromRaw( SubStmt );
            return hold_stmt.ToRaw( l );
        }

        virtual StmtResult ActOnGotoStmt(clang::SourceLocation GotoLoc,
                                         clang::SourceLocation LabelLoc,
                                         clang::IdentifierInfo *LabelII)
        {
            shared_ptr<Goto> g( new Goto );
            g->destination = MaybeCreateLabelIdentifier(LabelII);
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
            // TODO doesn't && meaning label-as-variable conflict with C++0x right-reference thingy?
            return hold_expr.ToRaw( MaybeCreateLabelIdentifier(LabelII) );
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
            else
                i->else_body = shared_new<Nop>(); // empty else clause
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
            else
                f->initialisation = shared_new<Nop>();

            if( Second )
                f->condition = hold_expr.FromRaw( Second );
            else
                f->condition = shared_new<True>();

            StmtTy *third = (StmtTy *)Third; // Third is really a statement, the Actions API is wrong
            if( third )
                f->increment = hold_stmt.FromRaw( third );
            else
                f->increment = shared_new<Nop>();

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
            shared_ptr<Case> c( new Case );
            if( RHSVal.get() )
                c->value_hi = FromClang( RHSVal );
            else
                c->value_hi = FromClang( LHSVal );
            c->value_lo = FromClang( LHSVal );
            backing_targets[c] = FromClang( SubStmt );
            return ToStmt( c );
        }

        virtual OwningStmtResult ActOnDefaultStmt(clang::SourceLocation DefaultLoc,
                                                  clang::SourceLocation ColonLoc, StmtArg SubStmt,
                                                  clang::Scope *CurScope)
        {
            TRACE();
            shared_ptr<Default> d( new Default );
            backing_targets[d] = FromClang( SubStmt );
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
                    ASSERTFAIL("Invalid access specfier");
                    return shared_ptr<Public>(new Public);
                    break;
            }
        }

        virtual DeclTy *ActOnCXXMemberDeclarator(clang::Scope *S, clang::AccessSpecifier AS,
                                                 clang::Declarator &D, ExprTy *BitfieldWidth,
                                                 ExprTy *Init, DeclTy *LastInGroup)
        {
            const clang::DeclSpec &DS = D.getDeclSpec();
            TRACE("Element %p\n", Init);
            shared_ptr<Declaration> d = CreateDelcaration( S, D, ConvertAccess( AS ) );
            shared_ptr<Instance> o = dynamic_pointer_cast<Instance>(d);

            if( BitfieldWidth )
            {
                ASSERT( o && "only Instances may be bitfields" );
                shared_ptr<Integral> n( dynamic_pointer_cast<Integral>( o->type ) );
                ASSERT( n && "cannot specify width of non-numeric type" );
                shared_ptr<Expression> ee = hold_expr.FromRaw(BitfieldWidth);
                shared_ptr<Literal> ll = dynamic_pointer_cast<Literal>(ee);
                ASSERT(ll && "bitfield width must be literal, not expression"); // TODO evaluate
                shared_ptr<SpecificInteger> ii = dynamic_pointer_cast<SpecificInteger>(ll);
                ASSERT(ll && "bitfield width must be integer");
                n->width = ii;
            }

            if( Init )
            {
                ASSERT( o && "only Instances may have initialisers");
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
            //ASSERT( !FromCXXScope(&SS) && "We're not doing anything with the C++ scope"); // TODO do something with the C++ scope
            // Now we're using it to link up with forward decls eg class foo { struct s; }; struct foo::s { blah } TODO is this even legal C++?

            ident_track.SeenScope( S );

            TRACE("Tag type %d, kind %d\n", TagType, (int)TK);
            // TagType is an instance of DeclSpec::TST, indicating what kind of tag this
            // is (struct/union/enum/class).

            // Proceed based on the context around the tag
            if( TK == clang::Action::TK_Reference )
            {
            	// Tag is a reference, that is a usage rather than a definition. We therefore
            	// expect to be able to find a previous definition/declaration for it. Recurse
            	// the search through enclosing scopes until we find it.
                shared_ptr<Declaration> ed = FindExistingDeclaration( SS, Name, true );
                ASSERT(ed)("Cannot find declaration of \"%s\"", Name->getName());

            	return hold_decl.ToRaw( ed );
            }

			// Tag is a definition or declaration. Create if it doesn't already
            // exist, *but* don't recurse into enclosing scopes.
			if( shared_ptr<Declaration> ed = FindExistingDeclaration( SS, Name, false ) )
			{
				// Note: members will be filled in later, so nothing to do here
				// even if the is the "complete" version of the record (=definition).
				return hold_decl.ToRaw( ed );
			}

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
					ASSERTFAIL("Unknown type spec type");
					break;
			}
			all_decls->members.insert(h);

			if(Name)
			{
				h->identifier = CreateTypeIdentifier(Name);
				ident_track.Add(Name, h, S);
			}
			else
			{
				// TODO make a general-lurpose anon name generator
				char an[20];
				static int ac=0;
				sprintf( an, "__anon%d", ac++ );
				h->identifier = CreateTypeIdentifier(an);
				ident_track.Add(NULL, h, S);
			}

			//TODO should we do something with TagKind? Maybe needed for render.
			//TODO use the attibutes

			// struct/class/union pushed by ActOnFinishCXXClassDef()
			if( (clang::DeclSpec::TST)TagType == clang::DeclSpec::TST_enum )
				decl_to_insert = h;

			TRACE("done tag %p\n", h.get());

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

            // We're about to populate the Record; if it has been populated already
            // then something's wrong
            ASSERT( h->members.empty() )("Record has already been defined");

            ident_track.SetNextRecord( h );

            inferno_scope_stack.push( h );      // decls for members will go on this scope
            backing_ordering[inferno_scope_stack.top()].clear();
        }

        /// ActOnFinishCXXClassDef - This is called when a class/struct/union has
        /// completed parsing, when on C++.
        virtual void ActOnFinishCXXClassDef(DeclTy *TagDecl)
        {
            TRACE();

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
                ASSERTFAIL("Unknown token accessing member");
            }

            // Find the specified member in the record implied by the expression on the left of .
            shared_ptr<Type> tbase = TypeOf( all_decls ).Get( a->base );
            shared_ptr<TypeIdentifier> tibase = dynamic_pointer_cast<TypeIdentifier>(tbase);
            ASSERT( tibase );
            shared_ptr<Record> rbase = GetRecordDeclaration(all_decls, tibase);
            ASSERT( rbase && "thing on left of ./-> is not a record/record ptr" );
            shared_ptr<SpecificInstanceIdentifier> sii( new SpecificInstanceIdentifier( string(Member.getName()) ) );
            a->member = FindMemberByName( all_decls, rbase, sii )->identifier;

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
            shared_ptr<Literal> ic;
            TRACE("true/false tk %d %d %d\n", Kind, clang::tok::kw_true, clang::tok::kw_false );

            if(Kind == clang::tok::kw_true)
                ic = shared_new<True>();
            else
                ic = shared_new<False>();
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

            shared_ptr<SpecificInteger> nc( new SpecificInteger );
            llvm::APSInt rv(TypeDb::char_bits, !TypeDb::char_default_signed);
            rv = literal.getValue();
            nc->value = rv;

            return hold_expr.ToRaw( nc );
        }

        virtual ExprResult ActOnInitList(clang::SourceLocation LParenLoc,
                                         ExprTy **InitList, unsigned NumInit,
                                         clang::InitListDesignations &Designators,
                                         clang::SourceLocation RParenLoc)
        {
            ASSERT( !Designators.hasAnyDesignators() && "Designators in init lists unsupported" );
            // Assume initialiser is for an Array, and create an ArrayInitialiser node
            // even if it's really a struct init. We'll come along later and replace with a
            // RecordInitialiser when we can see what the struct is.
            shared_ptr<ArrayLiteral> ao(new ArrayLiteral);
            for(int i=0; i<NumInit; i++)
            {
                shared_ptr<Expression> e = hold_expr.FromRaw( InitList[i] );
                ao->elements.push_back( e );
            }
            return hold_expr.ToRaw( ao );
        }

        // Create a RecordInitialiser using the elements of the supplied ArrayInitialiser and matching
        // them against the members of the supplied record. Records are stored using an unordered
        // collection for the members, so we have to use the ordered backing map. Array inits are ordered.
        shared_ptr<RecordLiteral> CreateRecordLiteralFromArrayLiteral( shared_ptr<ArrayLiteral> ai,
        		                                                       shared_ptr<Record> r )
        {
        	// Make new record initialiser and fill in the type
        	shared_ptr<RecordLiteral> ri( new RecordLiteral );
        	ri->type = r->identifier;

        	// Fill in the RecordLiteral operands collection with pairs that relate operands to their member ids
        	shared_ptr<Scope> s = r;
        	PopulateMapOperator( ri, ai->elements, s );

        	return ri;
        }

        // Populate a map operator using elements from a sequence of expressions
        void PopulateMapOperator( shared_ptr<MapOperator> mapop, // MapOperands corresponding to the elements of ai go in here
        		                  Sequence<Expression> &seq, // Operands to insert, ordered as per the input program
        	                      shared_ptr<Scope> scope ) // Original Scope that established ordering, must be in backing_ordering
        {
          	// Get a reference to the ordered list of members for this scope from a backing list
        	ASSERT( backing_ordering.IsExist(scope) )("Supplied scope did not make it into the backing ordering list");
        	Sequence<Declaration> &scope_ordered = backing_ordering[scope];
        	TRACE("%p %p\n", &scope->members, scope.get());

        	// Go over the entire scope, keeping track of where we are in the Sequence
        	int seq_index=0; // TODO rename
        	FOREACH( SharedPtr<Declaration> d, scope_ordered )
        	{
        		TRACE();
        		// We only care about instances...
        		if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
        		{
        			// ...and not function instances
        			if( !dynamic_pointer_cast<Subroutine>( i->type ) )
        			{
        				TRACE();
        				// Get value out of array init and put it in record init together with member instance id
        				shared_ptr<Expression> v = seq[seq_index];
        				shared_ptr<MapOperand> mi( new MapOperand );
        				mi->identifier = i->identifier;
        				mi->value = v;
        				mapop->operands.insert( mi );

        				seq_index++;
        			}
        		}
        	}
        	ASSERT( seq_index == seq.size() );
        }

        shared_ptr<String> CreateString( const char *s )
        {
            shared_ptr<SpecificString> st( new SpecificString );
            st->value = s;
            return st;
        }

        shared_ptr<String> CreateString( clang::IdentifierInfo *Id )
        {
            return CreateString( Id->getName() );
        }

        /// ActOnStringLiteral - The specified tokens were lexed as pasted string
        /// fragments (e.g. "foo" "bar" L"baz").
        virtual ExprResult ActOnStringLiteral(const clang::Token *Toks, unsigned NumToks)
        {
            clang::StringLiteralParser literal(Toks, NumToks, preprocessor, target_info);
            if (literal.hadError)
                return ExprResult(true);

            return hold_expr.ToRaw( CreateString( literal.GetString() ) );
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
            shared_ptr<Instance> o(new Instance());
            all_decls->members.insert(o);
            o->identifier = CreateInstanceIdentifier(Id);
            o->storage = shared_new<Static>();
            o->constancy = shared_new<Const>(); // static const member does not consume storage!!
            o->type = CreateIntegralType( TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified], false );
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
                shared_ptr<Expression> ei = lasto->identifier;
                inf->operands.insert( ei );
                inf->operands.insert( CreateNumericConstant( 1 ) );
                o->initialiser = inf;
            }
            else
            {
                o->initialiser = CreateNumericConstant( 0 );
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
               e->members.insert( hold_decl.FromRaw( Elements[i] ) );
        }

        /// ParsedFreeStandingDeclSpec - This method is invoked when a declspec with
        /// no declarator (e.g. "struct foo;") is parsed.
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

            // See if the declaration is already there (due to forwarding using
            // incomplete struct). If so, do not add it again
            Collection<Declaration> &sd = inferno_scope_stack.top()->members;
            FOREACH( const SharedPtr<Declaration> &p, sd ) // TODO find()?
                if( shared_ptr<Declaration>(p) == d )
                    return hold_decl.ToRaw( d );

            inferno_scope_stack.top()->members.insert( d );
            backing_ordering[inferno_scope_stack.top()].push_back( d );
            return hold_decl.ToRaw( d );
        }

        virtual ExprResult
           ActOnSizeOfAlignOfExpr( clang::SourceLocation OpLoc, bool isSizeof, bool isType,
                                   void *TyOrEx, const clang::SourceRange &ArgRange)
        {
            shared_ptr<TypeOperator> p;
            if( isSizeof )
                p = shared_ptr<SizeOf>(new SizeOf);
            else
                p = shared_ptr<AlignOf>(new AlignOf);

            if( isType )
                p->operand = hold_type.FromRaw(TyOrEx);
            else
                p->operand = TypeOf( all_decls ).Get( hold_expr.FromRaw(TyOrEx) );
            return hold_expr.ToRaw( p );
        }

        virtual BaseResult ActOnBaseSpecifier(DeclTy *classdecl,
                                              clang::SourceRange SpecifierRange,
                                              bool Virt, clang::AccessSpecifier AccessSpec,
                                              TypeTy *basetype,
                                              clang::SourceLocation BaseLoc)
        {
            shared_ptr<Type> t( hold_type.FromRaw( basetype ) );
            shared_ptr<SpecificTypeIdentifier> ti = dynamic_pointer_cast<SpecificTypeIdentifier>(t);
            ASSERT( ti );
            shared_ptr<Declaration> d = hold_decl.FromRaw( classdecl );
            shared_ptr<Record> r = dynamic_pointer_cast<Record>( d );
            ASSERT( r );

            shared_ptr<Base> base( new Base );
            base->record = ti;
          /*  if( Virt )
                base->storage = shared_new<Virtual>();
            else
                base->storage = shared_new<NonStatic>();
            base->constancy = shared_new<NonConst>(); */
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
                ih->bases.insert( hold_base.FromRaw( Bases[i] ) );
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
            shared_ptr<TypeIdentifier> id = dynamic_pointer_cast<TypeIdentifier>(t);
            ASSERT(id);
            shared_ptr<Record> r = GetRecordDeclaration( all_decls, id );

            FOREACH( shared_ptr<Declaration> d, r->members )
            {
                shared_ptr<Instance> o( dynamic_pointer_cast<Instance>(d) );
                if( !o )
                    continue;
                if( dynamic_pointer_cast<Constructor>(o->type) )
                    return o;
            }
            ASSERTFAIL("missing constructor");
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
            ASSERT( om );
            shared_ptr<Instance> cm = GetConstructor( om->type );
            ASSERT( cm );
            ASSERT( cm->identifier );

            // Build a lookup to the constructor, using the speiciifed subobject and the matching constructor
            shared_ptr<Lookup> lu(new Lookup);
            lu->base = om->identifier;
            lu->member = cm->identifier;

            // Build a call to the constructor with supplied args
            Sequence<Expression> args;
            CollectArgs( &args, Args, NumArgs );
            shared_ptr<Call> call = CreateCall( args, lu );

            // Get the constructor whose init list we're adding to (may need to start a
            // new compound statement)
            shared_ptr<Declaration> d( hold_decl.FromRaw( ConstructorDecl ) );
            shared_ptr<Instance> o( dynamic_pointer_cast<Instance>(d) );
            ASSERT(o);
            shared_ptr<Compound> comp = dynamic_pointer_cast<Compound>(o->initialiser);
            if( !comp )
            {
                comp = shared_ptr<Compound>( new Compound );
                o->initialiser = comp;
                TRACE();
            }

            // Add it
            comp->statements.push_back( call );
            return 0;
        }

        void CollectArgs( Sequence<Expression> *ps, ExprTy **Args, unsigned NumArgs )
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
                n->global = shared_ptr<Global>( new Global );
            else
                n->global = shared_ptr<NonGlobal>( new NonGlobal );

            // TODO cant figure out meaning of ParenTypeId

            return hold_expr.ToRaw( n );
        }

        /// ActOnCXXDelete - Parsed a C++ 'delete' expression. UseGlobal is true if
        /// the delete was qualified (::delete). ArrayForm is true if the array form
        /// was used (delete[]).
        virtual ExprResult ActOnCXXDelete( clang::SourceLocation StartLoc, bool UseGlobal,
                                           bool ArrayForm, ExprTy *Expression )
        {
            shared_ptr<Delete> d( new Delete );
            d->pointer = hold_expr.FromRaw( Expression );

            if( ArrayForm )
                d->array = shared_ptr<DeleteArray>( new DeleteArray );
            else
                d->array = shared_ptr<DeleteNonArray>( new DeleteNonArray );

            if( UseGlobal )
                d->global = shared_ptr<Global>( new Global );
            else
                d->global = shared_ptr<NonGlobal>( new NonGlobal );

            return hold_expr.ToRaw( d );
        }

        virtual ExprResult ActOnCompoundLiteral(clang::SourceLocation LParen, TypeTy *Ty,
                                                clang::SourceLocation RParen, ExprTy *Op)
        {
            shared_ptr<Type> t = hold_type.FromRaw( Ty );
      	    shared_ptr<Expression> e = hold_expr.FromRaw( Op );

      	    TRACE("%p\n", t.get() );

            // At this point, when we have the instance (and hence the type) and the initialiser
            // we can detect when an array initialiser has been inserted for a record instance and
            // change it.
            if( shared_ptr<ArrayLiteral> ai = dynamic_pointer_cast<ArrayLiteral>(e) )
                if( shared_ptr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(t) )
                	if( shared_ptr<Record> r = GetRecordDeclaration(all_decls, ti) )
                		e = CreateRecordLiteralFromArrayLiteral( ai, r );

            return hold_expr.ToRaw( e );
        }


        //--------------------------------------------- unimplemented actions -----------------------------------------------
        // Note: only actions that return something (so we don't get NULL XTy going around the place). No obj-C or GCC
        // extensions. These all assert out immediately.

  virtual DeclTy *ActOnFileScopeAsmDecl(clang::SourceLocation Loc, ExprArg AsmString) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnField(clang::Scope *S, DeclTy *TagD, clang::SourceLocation DeclStart,
                             clang::Declarator &D, ExprTy *BitfieldWidth) {
    ASSERTFAIL("Unimplemented action"); // TODO is this C-not-C++ or ObjC?
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
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnPredefinedExpr(clang::SourceLocation Loc,
                                         clang::tok::TokenKind Kind) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }


  virtual DeclTy *ActOnStartNamespaceDef(clang::Scope *S, clang::SourceLocation IdentLoc,
                                        clang::IdentifierInfo *Ident,
                                        clang::SourceLocation LBrace) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXNamedCast(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
                                       clang::SourceLocation LAngleBracketLoc, TypeTy *Ty,
                                       clang::SourceLocation RAngleBracketLoc,
                                       clang::SourceLocation LParenLoc, ExprTy *Op,
                                       clang::SourceLocation RParenLoc) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXThrow(clang::SourceLocation OpLoc,
                                   ExprTy *Op = 0) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXTypeConstructExpr(clang::SourceRange TypeRange,
                                               TypeTy *TypeRep,
                                               clang::SourceLocation LParenLoc,
                                               ExprTy **Exprs,
                                               unsigned NumExprs,
                                               clang::SourceLocation *CommaLocs,
                                               clang::SourceLocation RParenLoc) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual ExprResult ActOnCXXConditionDeclarationExpr(clang::Scope *S,
                                                      clang::SourceLocation StartLoc,
                                                      clang::Declarator &D,
                                                      clang::SourceLocation EqualLoc,
                                                      ExprTy *AssignExprVal) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual DeclTy *ActOnExceptionDeclarator(clang::Scope *S, clang::Declarator &D) {
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  virtual OwningStmtResult ActOnCXXCatchBlock(clang::SourceLocation CatchLoc,
                                              DeclTy *ExceptionDecl,
                                              StmtArg HandlerBlock) {
    ASSERTFAIL("Unimplemented action");
    return StmtEmpty();
  }

  virtual OwningStmtResult ActOnCXXTryBlock(clang::SourceLocation TryLoc,
                                            StmtArg TryBlock,
                                            MultiStmtArg Handlers) {
    ASSERTFAIL("Unimplemented action");
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
    ASSERTFAIL("Unimplemented action");
    return 0;
  }

  /// ActOnParamUnparsedDefaultArgument - We've seen a default
  /// argument for a function parameter, but we can't parse it yet
  /// because we're inside a class definition. Note that this default
  /// argument will be parsed later.
  virtual void ActOnParamUnparsedDefaultArgument(DeclTy *param,
                                                 clang::SourceLocation EqualLoc)
  {
      ASSERTFAIL("Unimplemented action");
  }

  /// ActOnParamDefaultArgumentError - Parsing or semantic analysis of
  /// the default argument for the parameter param failed.
  virtual void ActOnParamDefaultArgumentError(DeclTy *param)
  {
      ASSERTFAIL("Unimplemented action");
   }

  virtual void ActOnEnumStartDefinition(clang::Scope *S, DeclTy *EnumDecl)
  {
     ASSERTFAIL("Unimplemented action");
  }
 };
};

#endif
