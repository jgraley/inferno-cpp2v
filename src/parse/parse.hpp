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

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "common/trace.hpp"
#include "tree/type_data.hpp"
#include "tree/misc.hpp"
#include "tree/typeof.hpp"

#include "rc_hold.hpp"
#include "identifier_tracker.hpp"

#include <unistd.h> // for get_current_dir_name()

using namespace CPPTree; // TODO put parse in cpp file so this using does not pollute

#define INFERNO_TRIPLE "arm-linux"

class Parse
{
public:
    Parse(string i) :
        infile(i)
    {
        ASSERT( infile!="" );
    }

    TreePtr<Node> DoParse()
    {
        auto root = MakeTreeNode<Program>();
        TreePtr<Scope> root_scope = DynamicTreePtrCast<Scope>(root);
        ASSERT(root_scope)("Can only parse into a scope");

        clang::FileManager fm;
        llvm::raw_stderr_ostream errstream; // goes to stderr
        clang::TextDiagnosticPrinter diag_printer(errstream);
        clang::Diagnostic diags(&diag_printer);
        clang::LangOptions opts;
        opts.CPlusPlus = 1; // Note: always assume input is C++, even if file ends in .c
        clang::TargetInfo* ptarget = clang::TargetInfo::CreateTargetInfo(
                INFERNO_TRIPLE);
        ASSERT(ptarget);
        clang::SourceManager sm;
        clang::HeaderSearch headers(fm);
        
        std::vector<clang::DirectoryLookup> dirs;
        const char *dirname_c = get_current_dir_name();
        ASSERT(dirname_c);
        string dirname(dirname_c);
        free((void *)dirname_c);
        dirs.push_back( clang::DirectoryLookup( fm.getDirectory( dirname + string("/resource/include") ), 
                                                clang::SrcMgr::C_System, 
                                                true, 
                                                false ) ); // TODO would prefer based on location of exe rather than CWD
        headers.SetSearchPaths( dirs, 0, false ); // make the directory in dirs be a system directory

        clang::Preprocessor pp(diags, opts, *ptarget, sm, headers);
        pp.setPredefines("#define __INFERNO__ 1\n");            
            
        const clang::FileEntry *file = fm.getFile(infile);
        if (file)
            sm.createMainFileID(file, clang::SourceLocation());
        if (sm.getMainFileID() == 0)
        {
            fprintf(stderr, "Error reading '%s'!\n", infile.c_str());
            exit(1);
        }
        pp.EnterMainSourceFile();

        clang::IdentifierTable it(opts);
        InfernoAction actions(root, root_scope, it, pp, *ptarget);
        clang::Parser parser(pp, actions);
        TRACE("Start parse\n");
        parser.ParseTranslationUnit();
        ASSERT( !diags.hasErrorOccurred() )("Will not proceed into inferno because clang reported errors\n");
        TRACE("End parse\n");
        return root;
    }

private:
    string infile;

    class InfernoAction: public clang::Action, public Traceable
    {
    public:
        InfernoAction(TreePtr<Node> context, TreePtr<Scope> root_scope,
                clang::IdentifierTable &IT, clang::Preprocessor &pp,
                clang::TargetInfo &T) :
            preprocessor(pp), target_info(T), ident_track(context),
                    global_scope(context), all_decls(MakeTreeNode<Program>()) // TODO Scope not Program
        {
            ASSERT( context );
            ASSERT( root_scope );
            inferno_scope_stack.push(root_scope); // things will be pushed into here
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
        map<TreePtr<Declaration> , clang::IdentifierInfo *> backing_params;

        // The statement after a label is parsed as a sub-construct under the label which
        // is not how the inferno tree does it. Remember that relationship here and
        // generate the extra nodes when rendering a compound statement.
        map<TreePtr<Label> , TreePtr<Statement> > backing_labels;
        map<TreePtr<SwitchTarget> , TreePtr<Statement> > backing_targets;
        map<TreePtr<Declaration> , TreePtr<Declaration> >
                backing_paired_decl;

        // Members of records go in an unordered collection, but when parsing
        // we might need the order, eg for C-style initialisers or auto-generated
        // constructor calls.
        map<TreePtr<Scope> , Sequence<Declaration> > backing_ordering;

        // In ActOnTag, when we see a record decl, we store it here and generate it
        // at the next IssueDeclaration, called from ActOnDeclaration. This allows
        // a seperate decl for records, since we so not support anon ones, and only
        // allow one thing to be decl'd at a time.
        TreePtr<Declaration> decl_to_insert;

        clang::Preprocessor &preprocessor;
        clang::TargetInfo &target_info;

        stack<TreePtr<Scope> > inferno_scope_stack;
        RCHold<Declaration, DeclTy *> hold_decl;
        RCHold<Base, DeclTy *> hold_base;
        RCHold<Expression, ExprTy *> hold_expr;
        RCHold<Statement, StmtTy *> hold_stmt;
        RCHold<Type, TypeTy *> hold_type;
        RCHold<LabelIdentifier, void *> hold_label_identifier;
        RCHold<Node, CXXScopeTy *> hold_scope;
        IdentifierTracker ident_track;
        TreePtr<Node> global_scope;
        TreePtr<Program> all_decls; // not the actual program, just a flattening of the decls
        // we maintain this because decls don't always make it
        // into the tree by the time we need them, thanks to the
        // way clang works. Decls go in here immediately.

        OwningStmtResult ToStmt(TreePtr<Statement> s)
        {
            return OwningStmtResult(*this, hold_stmt.ToRaw(s));
        }

        OwningExprResult ToExpr(TreePtr<Expression> e)
        {
            return OwningExprResult(*this, hold_expr.ToRaw(e));
        }

        TreePtr<Statement> FromClang(const StmtArg &s)
        {
            return hold_stmt.FromRaw(s.get());
        }

        TreePtr<Expression> FromClang(const ExprArg &e)
        {
            return hold_expr.FromRaw(e.get());
        }

        struct DeclarationAsStatement: Statement
        {
            NODE_FUNCTIONS
            TreePtr<Declaration> d;
        };

        struct DeclarationChain: Declaration
        {
            NODE_FUNCTIONS
            TreePtr<Declaration> first;
            TreePtr<Declaration> second;
        };

        // Turn a clang::CXXScopeSpec into a pointer to the corresponding scope node.
        // We have to deal with all the ways of it baing invalid, then just use hold_scope.
        TreePtr<Node> FromCXXScope(const clang::CXXScopeSpec *SS)
        {
            if (!SS)
                return TreePtr<Node> ();

            if (SS->isEmpty())
                return TreePtr<Node> ();

            if (!SS->isSet())
                return TreePtr<Node> ();

            return hold_scope.FromRaw(SS->getScopeRep());
        }

        clang::Action::TypeTy *isTypeName(clang::IdentifierInfo &II,
                clang::Scope *S, const clang::CXXScopeSpec *SS)
        {
            TreePtr<Node> n( ident_track.TryGet(&II, FromCXXScope(SS)) );
            if (n)
            {
                TreePtr<UserType> t = DynamicTreePtrCast<UserType> (n);
                if (t)
                    return hold_type.ToRaw(t->identifier);
            }

            return 0;
        }

        virtual DeclTy *isTemplateName(clang::IdentifierInfo &II,
                clang::Scope *S, const clang::CXXScopeSpec *SS = 0)
        {
            return 0; // TODO templates
        }

        virtual bool isCurrentClassName(const clang::IdentifierInfo& II,
                clang::Scope *S, const clang::CXXScopeSpec *SS)
        {
            TRACE();
            ident_track.SeenScope(S);

            TreePtr<Node> cur( ident_track.GetCurrent() );
            if (!DynamicTreePtrCast<Record> (cur))
                return false; // not even in a record

            TreePtr<Node> cxxs = FromCXXScope(SS);
            TreePtr<Node> n( ident_track.TryGet(&II, cxxs) );
            return n == cur;
        }

        virtual void ActOnPopScope(clang::SourceLocation Loc, clang::Scope *S)
        {
            TRACE();
            ident_track.PopScope(S);
        }

        TreePtr<Integral> CreateIntegralType(int bits, bool default_signed,
                clang::DeclSpec::TSS type_spec_signed =
                        clang::DeclSpec::TSS_unspecified)
        {
            TreePtr<Integral> i;
            bool sign;
            switch (type_spec_signed)
            {
            case clang::DeclSpec::TSS_signed:
                sign = true;
                break;
            case clang::DeclSpec::TSS_unsigned:
                sign = false;
                break;
            case clang::DeclSpec::TSS_unspecified:
            default:
                sign = default_signed;
                break;
            }

            if (sign)
                i = MakeTreeNode<Signed>();
            else
                i = MakeTreeNode<Unsigned>();

            i->width = CreateNumericConstant(bits);
            return i;
        }

        TreePtr<Floating> CreateFloatingType(const llvm::fltSemantics *s)
        {
            ASSERT(s);
            auto sem = MakeTreeNode<SpecificFloatSemantics>(s);
            auto f = MakeTreeNode<Floating>();
            f->semantics = sem;
            return f;
        }

        void FillParameters(TreePtr<CallableParams> p,
                            const clang::DeclaratorChunk::FunctionTypeInfo &fchunk)
        {
            backing_ordering[p].clear(); // ensure at least an empty sequence is in the map
            for (unsigned i = 0; i < fchunk.NumArgs; i++)
            {
                TreePtr<Declaration> d = hold_decl.FromRaw(
                        fchunk.ArgInfo[i].Param);
                TreePtr<Instance> inst = DynamicTreePtrCast<Instance> (d);
                ASSERT( inst );
                backing_ordering[p].push_back(inst);
                p->members.insert(inst);
            }
        }

        TreePtr<Type> CreateTypeNode(clang::Declarator &D, unsigned depth = 0)
        {
            ASSERT( depth>=0 );
            ASSERT( depth<=D.getNumTypeObjects() );

            if (depth == D.getNumTypeObjects())
            {
                const clang::DeclSpec &DS = D.getDeclSpec();
                clang::DeclSpec::TST t = DS.getTypeSpecType();
                ASSERT( DS.getTypeSpecComplex() == clang::DeclSpec::TSC_unspecified )(
                        "complex types not supported");
                switch (t)
                {
                case clang::DeclSpec::TST_int:
                case clang::DeclSpec::TST_unspecified:
                    TRACE("int based %d %d\n", DS.getTypeSpecWidth(),
                            DS.getTypeSpecSign());
                    return CreateIntegralType(
                            TypeDb::integral_bits[DS.getTypeSpecWidth()],
                            true, DS.getTypeSpecSign());
                    break;
                case clang::DeclSpec::TST_char:
                    TRACE("char based %d %d\n", DS.getTypeSpecWidth(),
                            DS.getTypeSpecSign());
                    return CreateIntegralType(TypeDb::char_bits,
                            TypeDb::char_default_signed, DS.getTypeSpecSign());
                    break;
                case clang::DeclSpec::TST_void:
                    TRACE("void based %d %d\n", DS.getTypeSpecWidth(),
                            DS.getTypeSpecSign());
                    return MakeTreeNode<Void>();
                    break;
                case clang::DeclSpec::TST_bool:
                    TRACE("bool based %d %d\n", DS.getTypeSpecWidth(),
                            DS.getTypeSpecSign());
                    return MakeTreeNode<Boolean>();
                    break;
                case clang::DeclSpec::TST_float:
                    TRACE("float based %d %d\n", DS.getTypeSpecWidth(),
                            DS.getTypeSpecSign());
                    return CreateFloatingType(TypeDb::float_semantics);
                    break;
                case clang::DeclSpec::TST_double:
                    TRACE("double based %d %d\n", DS.getTypeSpecWidth(),
                            DS.getTypeSpecSign());
                    return CreateFloatingType(
                            DS.getTypeSpecWidth() == clang::DeclSpec::TSW_long ? TypeDb::long_double_semantics
                                    : TypeDb::double_semantics);
                    break;
                case clang::DeclSpec::TST_typedef:
                    TRACE("typedef\n");
                    return hold_type.FromRaw(DS.getTypeRep());
                    break;
                case clang::DeclSpec::TST_struct:
                case clang::DeclSpec::TST_union:
                case clang::DeclSpec::TST_class:
                case clang::DeclSpec::TST_enum:
                    TRACE("struct/union/class/enum\n");
                    // Disgustingly, clang casts the DeclTy returned from ActOnTag() to
                    // a TypeTy.
                    return DynamicTreePtrCast<Record> (hold_decl.FromRaw(
                            DS.getTypeRep()))->identifier;
                    break;
                default:
                    ASSERTFAIL("unsupported type")
                    ;
                    break;
                }
            }
            else
            {
                const clang::DeclaratorChunk &chunk = D.getTypeObject(depth);
                switch (chunk.Kind)
                {
                case clang::DeclaratorChunk::Function:
                {
                    const clang::DeclaratorChunk::FunctionTypeInfo &fchunk =
                            chunk.Fun;
                    switch (D.getKind())
                    {
                    case clang::Declarator::DK_Normal:
                    {
                        auto f = MakeTreeNode<Function>();
                        FillParameters(f, fchunk);
                        f->return_type = CreateTypeNode(D, depth + 1);
                        return f;
                    }
                    case clang::Declarator::DK_Constructor:
                    {
                        auto c = MakeTreeNode<Constructor>();
                        FillParameters(c, fchunk);
                        return c;
                    }
                    case clang::Declarator::DK_Destructor:
                    {
                        auto d = MakeTreeNode<Destructor>();
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
                    //const clang::DeclaratorChunk::PointerTypeInfo &pchunk = chunk.Ptr;
                    auto p = MakeTreeNode<Pointer>();
                    p->destination = CreateTypeNode(D, depth + 1);
                    return p;
                }

                case clang::DeclaratorChunk::Reference:
                {
                    // TODO attributes
                    TRACE("reference to...\n");
                    //const clang::DeclaratorChunk::ReferenceTypeInfo &rchunk = chunk.Ref;
                    auto r = MakeTreeNode<Reference>();
                    ASSERT(r);
                    r->destination = CreateTypeNode(D, depth + 1);
                    return r;
                }

                case clang::DeclaratorChunk::Array:
                {
                    // TODO attributes
                    const clang::DeclaratorChunk::ArrayTypeInfo &achunk = chunk.Arr;
                    TRACE("array [%d] of...\n", achunk.NumElts);
                    auto a = MakeTreeNode<Array>();
                    ASSERT(a);
                    a->element = CreateTypeNode(D, depth + 1);
                    if (achunk.NumElts)
                        a->size = hold_expr.FromRaw(achunk.NumElts); // number of elements was specified
                    else
                        a->size = MakeTreeNode<Uninitialised>(); // number of elements was not specified eg int a[];
                    return a;
                }

                default:
                    ASSERTFAIL("Unknown type chunk")
                    ;
                    break;
                }
            }
        }

        TreePtr<InstanceIdentifier> CreateInstanceIdentifier(
                clang::IdentifierInfo *ID = 0)
        {
            if (ID)            
                return MakeTreeNode<SpecificInstanceIdentifier>( ID->getName() );            
            else
                return MakeTreeNode<SpecificInstanceIdentifier>();            
        }

        TreePtr<TypeIdentifier> CreateTypeIdentifier(
                clang::IdentifierInfo *ID)
        {
            return MakeTreeNode<SpecificTypeIdentifier>( ID->getName() );
        }

        TreePtr<TypeIdentifier> CreateTypeIdentifier(string s)
        {
            return MakeTreeNode<SpecificTypeIdentifier>(s);
        }

        TreePtr<LabelIdentifier> CreateLabelIdentifier(
                clang::IdentifierInfo *ID)
        {
            ASSERT( ID );
            return MakeTreeNode<SpecificLabelIdentifier>( ID->getName() );
        }

        TreePtr<Instance> CreateInstanceNode(clang::Scope *S,
                clang::Declarator &D, TreePtr<AccessSpec> access =
                        TreePtr<AccessSpec> (), bool automatic = false)
        {
            TRACE();
            const clang::DeclSpec &DS = D.getDeclSpec();
            if (!access)
                access = MakeTreeNode<Private>(); // Most scopes are private unless specified otherwise

            TreePtr<Constancy> constancy;
            if (DS.getTypeQualifiers() & clang::DeclSpec::TQ_const)
                constancy = MakeTreeNode<Const>();
            else
                constancy = MakeTreeNode<NonConst>();

            TreePtr<Instance> o;

            if (automatic)
            {
                o = MakeTreeNode<Automatic>();
            }
            else
            {
                clang::DeclSpec::SCS scs = DS.getStorageClassSpec();
                switch (scs)
                {
                case clang::DeclSpec::SCS_unspecified:
                {
                    TRACE("scope flags 0x%x\n", S->getFlags());
                    if (S->getFlags() & clang::Scope::CXXClassScope) // record scope
                    {
                        TreePtr<Field> no = MakeTreeNode<Field> ();
                        o = no;
                        if (DS.isVirtualSpecified())
                        {
                            no->virt = MakeTreeNode<Virtual> ();
                        }
                        else
                        {
                            no->virt = MakeTreeNode<NonVirtual> ();
                        }
                        no->access = access;
                        no->constancy = constancy;
                    }
                    else if (S->getFnParent()) // in code
                    {
                        o = MakeTreeNode<Automatic> ();
                    }
                    else // top level
                    {
                        TreePtr<Static> no = MakeTreeNode<Static> ();
                        o = no;
                        no->constancy = constancy;
                    }
                    break;
                }
                case clang::DeclSpec::SCS_auto:
                    o = MakeTreeNode<Automatic> ();
                    break;
                case clang::DeclSpec::SCS_extern:// linking will be done "automatically" so no need to remember "extern" in the tree
                {
                    TreePtr<Static> no = MakeTreeNode<Static> ();
                    o = no;
                    no->constancy = constancy;
                }
                    break;
                case clang::DeclSpec::SCS_static:
                {
                    TreePtr<Static> no = MakeTreeNode<Static> ();
                    o = no;
                    no->constancy = constancy;
                }
                    break;
                default:
                    ASSERTFAIL("Unsupported storage class")
                    ;
                    break;
                }
            }

            all_decls->members.insert(o);

            clang::IdentifierInfo *ID = D.getIdentifier();
            if (ID)
            {
                o->identifier = CreateInstanceIdentifier(ID);
                ident_track.Add(ID, o, S);
            }
            else
            {
                o->identifier = CreateInstanceIdentifier();
            }
            o->type = CreateTypeNode(D);
            o->initialiser = MakeTreeNode<Uninitialised> ();

            return o;
        }

        TreePtr<Typedef> CreateTypedefNode(clang::Scope *S,
                clang::Declarator &D)
        {
            auto t = MakeTreeNode<Typedef>();
            all_decls->members.insert(t);
            clang::IdentifierInfo *ID = D.getIdentifier();
            if (ID)
            {
                t->identifier = CreateTypeIdentifier(ID);
                ident_track.Add(ID, t, S);
            }
            t->type = CreateTypeNode(D);

            //TRACE("%s %p %p\n", ID->getName(), t.get(), ID);
            return t;
        }
        /*
         TreePtr<Label> CreateLabelNode( clang::IdentifierInfo *ID )
         {
         auto l = MakeTreeNode<Label>();
         all_decls->members.insert(l);
         l->access = MakeTreeNode<Public>();
         l->identifier = CreateLabelIdentifier(ID);
         //TRACE("%s %p %p\n", ID->getName(), l.get(), ID );
         return l;
         }
         */
        TreePtr<Declaration> FindExistingDeclaration(
                const clang::CXXScopeSpec &SS, clang::IdentifierInfo *ID,
                bool recurse)
        {
            if (!ID)
                return TreePtr<Declaration> (); // No name specified => doesn't match anything

            // See if we already have this record in the current scope, or specified scope
            // if Declarator has one
            TreePtr<Node> cxxs = FromCXXScope(&SS);

            // Use C++ scope if non-nullptr; do not recurse (=precise match only)
            TreePtr<Node> found_n( ident_track.TryGet(ID, cxxs, recurse) );
            //TRACE("Looked for %s, result %p (%p)\n", ID->getName(),
            //        found_n.get(), cxxs.get());
            if (!found_n)
            {
                // Nothing was found with the supplied name
                ASSERT( !cxxs ); // If C++ scope was given explicitly, require successful find
                return TreePtr<Declaration> ();
            }

            TreePtr<Declaration> found_d =
                    DynamicTreePtrCast<Declaration> (found_n);
            // If the found match is not a declaration, cast will fail and we'll return nullptr for "not found"
            return found_d;
        }

        // Alternative parameters
        TreePtr<Declaration> FindExistingDeclaration(clang::Declarator &D,
                bool recurse)
        {
            return FindExistingDeclaration(D.getCXXScopeSpec(),
                    D.getIdentifier(), recurse);
        }

        TreePtr<Declaration> CreateDelcaration(clang::Scope *S,
                clang::Declarator &D, TreePtr<AccessSpec> a = TreePtr<
                        AccessSpec> ())
        {
            const clang::DeclSpec &DS = D.getDeclSpec();
            TreePtr<Declaration> d;
            if (DS.getStorageClassSpec() == clang::DeclSpec::SCS_typedef)
            {
                TreePtr<Typedef> t = CreateTypedefNode(S, D);
                TRACE();
                d = t;
            }
            else
            {
                TreePtr<Instance> o = CreateInstanceNode(S, D, a);
                d = o;
            }

            return d;
        }

        // Does 1 thing:
        // 1. Inserts a stored decl if there is one in decl_to_insert
        void IssueDeclaration(clang::Scope *S, TreePtr<Declaration> d)
        {
            int os = inferno_scope_stack.top()->members.size();
            // Did we leave a record decl lying around to insert later? If so, pack it together with
            // the current instance decl, for insertion into the code sequence.
            TRACE("Issuing instance decl ")(*d)(" scope flags %x ", S->getFlags());
            if (decl_to_insert)
            {
                inferno_scope_stack.top()->members.insert(decl_to_insert);
                backing_ordering[inferno_scope_stack.top()].push_back(
                        decl_to_insert);
                backing_paired_decl[d] = decl_to_insert;
                decl_to_insert = TreePtr<Declaration> (); // don't need to generate it again
                TRACE("inserted record decl\n");
            }

            TRACE("no insert record decl\n");
            inferno_scope_stack.top()->members.insert(d);
            backing_ordering[inferno_scope_stack.top()].push_back(d);
            TRACE("From %d to %d decls\n", os, inferno_scope_stack.top()->members.size() );
        }

        virtual DeclTy *ActOnDeclarator(clang::Scope *S, clang::Declarator &D,
                DeclTy *LastInGroup)
        {
            //TRACE("Scope S%p\n", S);
            ident_track.SeenScope(S);
            // TODO the spurious char __builtin_va_list; line comes from the target info.
            // Create an inferno target info customised for Inferno that doesn't do this.
            if (strcmp(D.getIdentifier()->getName(), "__builtin_va_list") == 0)
            {
                return 0;
            }

            TreePtr<Declaration> d = FindExistingDeclaration(D, false); // decl exists already?
            if (!d)
            {
                d = CreateDelcaration(S, D); // make a new one
                IssueDeclaration(S, d);
            }
            
            // Clang refuses to store all the DeclTys we return in cases like int a, b, c;
            // instead it provides us with the last one we parsed (LastInGroup) and we are
            // expected to chain them. Use a local node for this and untangle in AddStatementToCompound. 
            TRACE("LIG=%x\n", LastInGroup);
            if( LastInGroup )
            {
                TRACE("Chaining declarations\n");
                auto dc = MakeTreeNode<DeclarationChain>();
                dc->first = hold_decl.FromRaw(LastInGroup);
                dc->second = d;
                d = dc;
            }     
            
            return hold_decl.ToRaw( d );
        }

        /// ActOnParamDeclarator - This callback is invoked when a parameter
        /// declarator is parsed. This callback only occurs for functions
        /// with prototypes. S is the function prototype scope for the
        /// parameters (C++ [basic.scope.proto]).
        virtual DeclTy *ActOnParamDeclarator(clang::Scope *S,
                clang::Declarator &D)
        {

            TreePtr<Instance> p = CreateInstanceNode(S, D, MakeTreeNode<Public>(), true);
            backing_params[p] = D.getIdentifier(); // allow us to register the object with ident_track once we're in the function body scope
            return hold_decl.ToRaw(p);
        }

        virtual void AddInitializerToDecl(DeclTy *Dcl, ExprArg Init)
        {
            TRACE();
            TreePtr<Declaration> d = hold_decl.FromRaw(Dcl);

            TreePtr<Instance> o = DynamicTreePtrCast<Instance> (d);
            ASSERT( o ); // Only objects can be initialised

            o->initialiser = FromClang(Init);

            // At this point, when we have the instance (and hence the type) and the initialiser
            // we can detect when an array initialiser has been inserted for a record instance and
            // change it.
            DefaultTransUtils utils(all_decls);
            TransKit kit { &utils };
            if ( TreePtr<MakeArray> ai = DynamicTreePtrCast<MakeArray>(o->initialiser) )
                if ( TreePtr<TypeIdentifier> ti = DynamicTreePtrCast<TypeIdentifier>(o->type) )
                    if ( TreePtr<Record> r = GetRecordDeclaration(kit, ti).GetTreePtr() )
                        o->initialiser = CreateRecordLiteralFromArrayLiteral(
                                ai, r);
        }

        /// AddCXXDirectInitializerToDecl - This action is called immediately after 
        /// ActOnDeclarator, when a C++ direct initializer is present.
        /// e.g: "int x(1);"
        virtual void AddCXXDirectInitializerToDecl(DeclTy *Dcl,
                                                   clang::SourceLocation LParenLoc,
                                                   ExprTy **Exprs, unsigned NumExprs,
                                                   clang::SourceLocation *CommaLocs,
                                                   clang::SourceLocation RParenLoc) 
        {
            TRACE();
            TreePtr<Declaration> d = hold_decl.FromRaw(Dcl);
            TreePtr<Instance> o = DynamicTreePtrCast<Instance> (d);
            TRACE("Ignoring C++ direct initialiser for SC Modules, not supported in general\n"); // TODO try the code below...
           // TreePtr<Instance> cm = GetConstructor( d->type );
           // ASSERT( cm );
           // ASSERT( cm->identifier );
           // Sequence<Expression> args;
           // CollectArgs( &args, Exprs, NumExprs );
           // TreePtr<Call> c = CreateCall( args, cm->identifier );
        }
        
        // Clang tends to parse parameters and function bodies in seperate
        // scopes so when we see them being used we don't recognise them
        // and cannot link back to the correct Instance node. This function
        // puts all the params back in the current scope assuming:
        // 1. They have been added to the Function node correctly and
        // 2. They feature in the backing list for params
        void AddParamsToScope( TreePtr<CallableParams> pp,
                               clang::Scope *FnBodyScope)
        {
            ASSERT(pp);

            for(TreePtr<Declaration> param : pp->members )
            {        
                TRACE();
                clang::IdentifierInfo *paramII = backing_params[param];
                backing_params.erase( param );
                //TRACE("%p %p %s S%p\n", param.get(), paramII, paramII->getName(), FnBodyScope);
                if( paramII )
                ident_track.Add( paramII, param, FnBodyScope );
            }
        }

    // JSG this is like the default in Actions, except it passes the parent of the function
    // body to ActOnDeclarator, since the function decl itself is not inside its own body.
    virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D)
    {
        TRACE();
        // Declarator is outside function sope, otherwise it would be in its own scope and unfindable
        DeclTy *DT = ActOnDeclarator(FnBodyScope->getParent(), D, 0);
        // Now enter function's scope. Use node from existing declaration so we get any enclosing classes
        TreePtr<Node> n = FindExistingDeclaration(D, false);
        ident_track.PushScope( FnBodyScope->getParent(), n );
        // Default to ActOnDeclarator.
        return ActOnStartOfFunctionDef(FnBodyScope, DT);
    }

    virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, DeclTy *D)
    {
        //TRACE("FnBodyScope S%p\n", FnBodyScope);
        TreePtr<Instance> o = DynamicTreePtrCast<Instance>(hold_decl.FromRaw(D));
        ASSERT(o);        
        
        // Note: this line was commented out as of mid August, but I'm sure I put it 
        // in recently, to fix a bug. Can't remember why it was commented out. Uncommented
        // to fix scopes being popped without being pushed.
        ident_track.SeenScope( FnBodyScope );

        if( TreePtr<CallableParams> pp = DynamicTreePtrCast<CallableParams>( o->type ) )
        AddParamsToScope( pp, FnBodyScope );

        // This is just a junk scope because we will not use scopes collected
        // via the inferno_scope_stack mechanism within functions; instead, they
        // will appear among the Statements managed by clang and passed into
        // ActOnFinishFunctionBody() as a hierarchy of Compounds.
        // If we tried to do this ourselves we'd lose the nested compound
        // statement hierarchy.
        inferno_scope_stack.push( MakeTreeNode<Scope>() );
 
        return hold_decl.ToRaw( o );
    }

    virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtArg Body)
    {
        TRACE();
        TreePtr<Instance> o( DynamicTreePtrCast<Instance>( hold_decl.FromRaw(Decl) ) );
        ASSERT(o);
        TreePtr<Compound> cb( DynamicTreePtrCast<Compound>( FromClang( Body ) ) );
        ASSERT(cb); // function body must be a scope or 0

        if( DynamicTreePtrCast<Uninitialised>( o->initialiser ) )
        o->initialiser = cb;
        else if( TreePtr<Compound> c = DynamicTreePtrCast<Compound>( o->initialiser ) )
        c->statements = c->statements + cb->statements;
        else
        ASSERTFAIL("wrong thing in function instance");

        TRACE("finish fn %d statements %d total\n", cb->statements.size(), (DynamicTreePtrCast<Compound>(o->initialiser))->statements.size() );

        inferno_scope_stack.pop(); // we dont use these - we use the clang-managed compound statement instead (passed in via Body)
        ident_track.PopScope(nullptr);
        return Decl;
    }

    virtual OwningStmtResult ActOnExprStmt(ExprArg Expr)
    {
        // TODO most of this is now unnecessary
        if( TreePtr<Expression> e = DynamicTreePtrCast<Expression>( FromClang(Expr) ) )
        {
            return ToStmt( e );
        }
        else
        {
            // Operands that are not Expressions have no side effects and so
            // they do nothing as Statements
            auto n = MakeTreeNode<Nop>();
            return ToStmt( n );
        }
    }

    virtual StmtResult ActOnReturnStmt( clang::SourceLocation ReturnLoc,
            ExprTy *RetValExp )
    {
        auto r = MakeTreeNode<Return>();
        if( RetValExp )
        r->return_value = hold_expr.FromRaw(RetValExp);
        else
        r->return_value = MakeTreeNode<Uninitialised>();
        //TRACE("aors %p\n", r.get() );
        return hold_stmt.ToRaw( r );
    }

    virtual ExprResult ActOnIdentifierExpr( clang::Scope *S,
            clang::SourceLocation Loc,
            clang::IdentifierInfo &II,
            bool HasTrailingLParen,
            const clang::CXXScopeSpec *SS = 0 )
    {
        //TRACE("S%p\n", S);
        TreePtr<Node> n( ident_track.Get( &II, FromCXXScope( SS ) ) );
        TRACE("aoie %s %s\n", II.getName(), typeid(*n).name() );
        TreePtr<Instance> o = DynamicTreePtrCast<Instance>( n );
        ASSERT( o );
        return hold_expr.ToRaw( o->identifier );
    }

    TreePtr<Integer> CreateNumericConstant( int value )
    {
        return MakeTreeNode<SpecificInteger>( value );
    }

    TreePtr<Literal> CreateLiteral( int value )
    {
        return CreateNumericConstant( value );
    }

    TreePtr<Number> CreateNumericConstant(const clang::Token &tok)
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

            ASSERT( !err )( "numeric literal too big for its own type" );
            return MakeTreeNode<SpecificInteger>( rv );
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

            return MakeTreeNode<SpecificFloat>( rv );
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
        TreePtr<Operator> o = TreePtr<Operator>();
        switch( Kind )
        {
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
            case clang::tok::TOK: \
                o=MakeTreeNode<NODE>();\
                break;
#include "tree/operator_data.inc"
            default:
                ASSERTFAIL("Unsupported token kind");
        }
        ASSERT( o );
        if( TreePtr<NonCommutativeOperator> nco = DynamicTreePtrCast< NonCommutativeOperator >(o) )
        {
            nco->operands.push_back( hold_expr.FromRaw(LHS) );
            nco->operands.push_back( hold_expr.FromRaw(RHS) );
        }
        else if( TreePtr<CommutativeOperator> co = DynamicTreePtrCast< CommutativeOperator >(o) )
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
        TreePtr<NonCommutativeOperator> o = TreePtr<NonCommutativeOperator>();

        switch( Kind )
        {
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
            case clang::tok::TOK: \
                o=MakeTreeNode<NODE>(); \
                break;
#include "tree/operator_data.inc"
            default:
                ASSERTFAIL("Unsupported token kind");
        }
        ASSERT( o );
        o->operands.push_back( hold_expr.FromRaw(Input) );
        return hold_expr.ToRaw( o );
    }

    virtual ExprResult ActOnUnaryOp( clang::Scope *S, clang::SourceLocation OpLoc,
            clang::tok::TokenKind Kind, ExprTy *Input)
    {
        TreePtr<NonCommutativeOperator> o = TreePtr<NonCommutativeOperator>();

        switch( Kind )
        {
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
            case clang::tok::TOK:\
                o=MakeTreeNode<NODE>(); \
                break;
#include "tree/operator_data.inc"
            default:
                ASSERTFAIL("Unsupported token kind");
        }
        ASSERT( o );
        o->operands.push_back( hold_expr.FromRaw(Input) );
        return hold_expr.ToRaw( o );
    }

    virtual ExprResult ActOnConditionalOp(clang::SourceLocation QuestionLoc,
            clang::SourceLocation ColonLoc,
            ExprTy *Cond, ExprTy *LHS, ExprTy *RHS)
    {
        auto co = MakeTreeNode<ConditionalOperator>();
        co->operands.push_back( hold_expr.FromRaw(Cond) );
        ASSERT(LHS )( "gnu extension not supported");
        co->operands.push_back( hold_expr.FromRaw(LHS) );
        co->operands.push_back( hold_expr.FromRaw(RHS) );
        return hold_expr.ToRaw( co );
    }

    TreePtr<Call> CreateCall( Sequence<Expression> &args, TreePtr<Expression> callee )
    {
        // Make the Call node and fill in the called function
        auto c = MakeTreeNode<Call>();
        c->callee = callee;

        // If CallableParams, fill in the args map based on the supplied args and original function type
        TreePtr<Node> t = TypeOf::instance(callee, all_decls).GetTreePtr();
        if( TreePtr<CallableParams> p = DynamicTreePtrCast<CallableParams>(t) )
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
        TreePtr<Call> c = CreateCall( args, hold_expr.FromRaw(Fn) );
        return hold_expr.ToRaw( c );
    }

    // Not sure if this one has been tested!!
    virtual TypeResult ActOnTypeName(clang::Scope *S, clang::Declarator &D)
    {
        TreePtr<Type> t = CreateTypeNode( D );
        return hold_type.ToRaw( t );
    }

    void AddDeclarationToCompound( TreePtr<Compound> s, TreePtr<Declaration> d )
    {
        if( TreePtr<DeclarationChain> dc = DynamicTreePtrCast<DeclarationChain>( d ) )
        {
            TRACE("Walking declaration chain\n");
            AddDeclarationToCompound( s, dc->first );
            AddDeclarationToCompound( s, dc->second );
            return;
        }
          
        if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>(d) )
            AddStatementToCompound( s, i ); // Instances can have inits that require being in order, so append as a statement
        else
            s->members.insert( d );
    }
    
    void AddStatementToCompound( TreePtr<Compound> s, TreePtr<Statement> st )
    {
        /* if( TreePtr<ParseTwin> pt = DynamicTreePtrCast<ParseTwin>( st ) )
         {
         AddStatementToCompound( s, pt->d1 );
         AddStatementToCompound( s, pt->d2 );
         return;
         }
         */
        if( TreePtr<Declaration> d = DynamicTreePtrCast<Declaration>( st ) )
        {
            if( backing_paired_decl.count(d) > 0 )
            {
                TreePtr<Declaration> bd = backing_paired_decl[d];
                ASSERT( bd );
                AddDeclarationToCompound( s, bd );
                backing_paired_decl.erase(d);
            }
        }

        if( TreePtr<DeclarationAsStatement> das = DynamicTreePtrCast<DeclarationAsStatement>(st) )
            AddDeclarationToCompound( s, das->d );
        else
            s->statements.push_back( st );

        // Flatten the "sub" statements of labels etc
        if( TreePtr<Label> l = DynamicTreePtrCast<Label>( st ) )
        {
            ASSERT( backing_labels[l] );
            AddStatementToCompound( s, backing_labels[l] );
            backing_labels.erase(l);
        }
        else if( TreePtr<SwitchTarget> t = DynamicTreePtrCast<SwitchTarget>( st ) )
        {
            ASSERT( backing_targets[t] );
            AddStatementToCompound( s, backing_targets[t] );
            backing_targets.erase(t);
        }
    }

    virtual OwningStmtResult ActOnCompoundStmt(clang::SourceLocation L, clang::SourceLocation R,
            MultiStmtArg Elts,
            bool isStmtExpr)
    {
        // TODO helper fn for MultiStmtArg, like FromClang. Maybe.
        auto s = MakeTreeNode<Compound>();

        for( unsigned i=0; i<Elts.size(); i++ )
        AddStatementToCompound( s, hold_stmt.FromRaw( Elts.get()[i] ) );

        return ToStmt( s );
    }

    virtual OwningStmtResult ActOnDeclStmt(DeclTy *Decl, clang::SourceLocation StartLoc,
            clang::SourceLocation EndLoc)
    {
        TreePtr<Declaration> d( hold_decl.FromRaw(Decl) );
        // Basically we are being asked to turn a Declaration, which has already been parsed,
        // into a Statement. Instances are already both Declarations and Statements, so that's
        // OK. In other cases, we have to package up the Declaration in a special kind of
        // Statement node and pass it through that way. We will unpack later.
        if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>(d) )
        {
            return ToStmt( i );
        }
        else
        {
            auto das = MakeTreeNode<DeclarationAsStatement>();
            das->d = d;
            return ToStmt( das );
        }
    }

    // Create a label identifier if there isn't already one with the same name (TODO scopes?)
    TreePtr<LabelIdentifier> MaybeCreateLabelIdentifier( clang::IdentifierInfo *II )
    {
        if( !(II->getFETokenInfo<void *>()) )
        II->setFETokenInfo( hold_label_identifier.ToRaw( CreateLabelIdentifier( II ) ) );

        return hold_label_identifier.FromRaw( II->getFETokenInfo<void *>() );
    }

    virtual StmtResult ActOnLabelStmt(clang::SourceLocation IdentLoc, clang::IdentifierInfo *II,
            clang::SourceLocation ColonLoc, StmtTy *SubStmt)
    {
        auto l = MakeTreeNode<Label>();
        l->identifier = MaybeCreateLabelIdentifier(II);
        backing_labels[l] = hold_stmt.FromRaw( SubStmt );
        return hold_stmt.ToRaw( l );
    }

    virtual StmtResult ActOnGotoStmt(clang::SourceLocation GotoLoc,
            clang::SourceLocation LabelLoc,
            clang::IdentifierInfo *LabelII)
    {
        auto g = MakeTreeNode<Goto>();
        g->destination = MaybeCreateLabelIdentifier(LabelII);
        return hold_stmt.ToRaw( g );
    }

    virtual StmtResult ActOnIndirectGotoStmt(clang::SourceLocation GotoLoc,
            clang::SourceLocation StarLoc,
            ExprTy *DestExp)
    {
        auto g = MakeTreeNode<Goto>();
        g->destination = hold_expr.FromRaw( DestExp );
        return hold_stmt.ToRaw( g );
    }

    virtual ExprResult ActOnAddrLabel(clang::SourceLocation OpLoc, clang::SourceLocation LabLoc,
            clang::IdentifierInfo *LabelII) // "&&foo"

    {
        // TODO doesn't && meaning label-as-variable conflict with C++0x right-reference thingy?
        return hold_expr.ToRaw( MaybeCreateLabelIdentifier(LabelII) );
    }

    virtual StmtResult ActOnIfStmt(clang::SourceLocation IfLoc, ExprTy *CondVal,
            StmtTy *ThenVal, clang::SourceLocation ElseLoc,
            StmtTy *ElseVal)
    {
        auto i = MakeTreeNode<If>();
        i->condition = hold_expr.FromRaw( CondVal );
        i->body = hold_stmt.FromRaw( ThenVal );
        if( ElseVal )
        i->else_body = hold_stmt.FromRaw( ElseVal );
        else
        i->else_body = MakeTreeNode<Nop>(); // empty else clause
        return hold_stmt.ToRaw( i );
    }

    virtual StmtResult ActOnWhileStmt(clang::SourceLocation WhileLoc, ExprTy *Cond,
            StmtTy *Body)
    {
        auto w = MakeTreeNode<While>();
        w->condition = hold_expr.FromRaw( Cond );
        w->body = hold_stmt.FromRaw( Body );
        return hold_stmt.ToRaw( w );
    }

    virtual StmtResult ActOnDoStmt(clang::SourceLocation DoLoc, StmtTy *Body,
            clang::SourceLocation WhileLoc, ExprTy *Cond)
    {
        auto d = MakeTreeNode<Do>();
        d->body = hold_stmt.FromRaw( Body );
        d->condition = hold_expr.FromRaw( Cond );
        return hold_stmt.ToRaw( d );
    }

    virtual StmtResult ActOnForStmt(clang::SourceLocation ForLoc,
            clang::SourceLocation LParenLoc,
            StmtTy *First, ExprTy *Second, ExprTy *Third,
            clang::SourceLocation RParenLoc, StmtTy *Body)
    {
        auto f = MakeTreeNode<For>();
        if( First )
            f->initialisation = hold_stmt.FromRaw( First );
        else
            f->initialisation = MakeTreeNode<Nop>();

        if( Second )
            f->condition = hold_expr.FromRaw( Second );
        else
            f->condition = MakeTreeNode<True>();

        StmtTy *third = (StmtTy *)Third; // Third is really a statement, the Actions API is wrong
        if( third )
            f->increment = hold_stmt.FromRaw( third );
        else
            f->increment = MakeTreeNode<Nop>();

        f->body = hold_stmt.FromRaw( Body );
        return hold_stmt.ToRaw( f );
    }

    virtual StmtResult ActOnStartOfSwitchStmt(ExprTy *Cond)
    {
        auto s = MakeTreeNode<Switch>();
        s->condition = hold_expr.FromRaw( Cond );
        return hold_stmt.ToRaw( s );
    }

    virtual StmtResult ActOnFinishSwitchStmt(clang::SourceLocation SwitchLoc,
            StmtTy *rsw, ExprTy *Body)
    {
        TreePtr<Statement> s( hold_stmt.FromRaw( rsw ) );
        TreePtr<Switch> sw( DynamicTreePtrCast<Switch>(s) );
        ASSERT(sw)("expecting a switch statement");

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

        if( RHSVal.get() )
        {
            auto rc = MakeTreeNode<RangeCase>();
            rc->value_lo = FromClang( LHSVal );
            rc->value_hi = FromClang( RHSVal );
            backing_targets[rc] = FromClang( SubStmt );
            return ToStmt( rc );
        }
        else
        {
            auto c = MakeTreeNode<Case>();
            c->value = FromClang( LHSVal );
            backing_targets[c] = FromClang( SubStmt );
            return ToStmt( c );
        }
    }

    virtual OwningStmtResult ActOnDefaultStmt(clang::SourceLocation DefaultLoc,
            clang::SourceLocation ColonLoc, StmtArg SubStmt,
            clang::Scope *CurScope)
    {
        TRACE();
        auto d = MakeTreeNode<Default>();
        backing_targets[d] = FromClang( SubStmt );
        return ToStmt( d );
    }

    virtual StmtResult ActOnContinueStmt(clang::SourceLocation ContinueLoc,
            clang::Scope *CurScope)
    {
        return hold_stmt.ToRaw( MakeTreeNode<Continue>() );
    }

    virtual StmtResult ActOnBreakStmt(clang::SourceLocation GotoLoc, clang::Scope *CurScope)
    {
        return hold_stmt.ToRaw( MakeTreeNode<Break>() );
    }

    TreePtr<AccessSpec> ConvertAccess( clang::AccessSpecifier AS, TreePtr<Record> rec = TreePtr<Record>() )
    {
        switch( AS )
        {
            case clang::AS_public:
            return MakeTreeNode<Public>();
            break;
            case clang::AS_protected:
            return MakeTreeNode<Protected>();
            break;
            case clang::AS_private:
            return MakeTreeNode<Private>();
            break;
            case clang::AS_none:
            ASSERT( rec )( "no access specifier and record not supplied so cannot deduce");
            // members are never AS_none because clang deals. Bases can be AS_none, so we supply the enclosing record type
            if( DynamicTreePtrCast<Class>(rec) )
            return MakeTreeNode<Private>();
            else
            return MakeTreeNode<Public>();
            break;
            default:
            ASSERTFAIL("Invalid access specfier");
            return MakeTreeNode<Public>();
            break;
        }
    }

    virtual DeclTy *ActOnCXXMemberDeclarator(clang::Scope *S, clang::AccessSpecifier AS,
            clang::Declarator &D, ExprTy *BitfieldWidth,
            ExprTy *Init, DeclTy *LastInGroup)
    {
        //const clang::DeclSpec &DS = D.getDeclSpec();
        //TRACE("Element %p\n", Init);
        TreePtr<Declaration> d = CreateDelcaration( S, D, ConvertAccess( AS ) );
        TreePtr<Instance> o = DynamicTreePtrCast<Instance>(d);

        if( BitfieldWidth )
        {
            ASSERT( o )( "only Instances may be bitfields" );
            TreePtr<Integral> n( DynamicTreePtrCast<Integral>( o->type ) );
            ASSERT( n )( "cannot specify width of non-numeric type" );
            TreePtr<Expression> ee = hold_expr.FromRaw(BitfieldWidth);
            TreePtr<Literal> ll = DynamicTreePtrCast<Literal>(ee);
            ASSERT(ll )( "bitfield width must be literal, not expression"); // TODO evaluate
            TreePtr<SpecificInteger> ii = DynamicTreePtrCast<SpecificInteger>(ll);
            ASSERT(ll )( "bitfield width must be integer");
            n->width = ii;
        }

        if( Init )
        {
            ASSERT( o )( "only Instances may have initialisers");
            o->initialiser = hold_expr.FromRaw( Init );
        }
        
        IssueDeclaration( S, d );
        return hold_decl.ToRaw( d );
    }

    virtual DeclTy *ActOnTag(clang::Scope *S, unsigned TagType, TagKind TK,
            clang::SourceLocation KWLoc, const clang::CXXScopeSpec &SS,
            clang::IdentifierInfo *Name, clang::SourceLocation NameLoc,
            clang::AttributeList *Attr,
            MultiTemplateParamsArg TemplateParameterLists)
    {
        //ASSERT( !FromCXXScope(&SS) && "We're not doing anything with the C++ scope"); // TODO do something with the C++ scope
        // Now we're using it to link up with forward decls eg class foo { struct s; }; struct foo::s { blah } TODO is this even legal C++?
        
        TRACE("Tag type %d, kind %d\n", TagType, (int)TK);
        ident_track.SeenScope( S );

        // TagType is an instance of DeclSpec::TST, indicating what kind of tag this
        // is (struct/union/enum/class).

        // Proceed based on the context around the tag
        if( TK == clang::Action::TK_Reference )
        {
            // Tag is a reference, that is a usage rather than a definition. We therefore
            // expect to be able to find a previous definition/declaration for it. Recurse
            // the search through enclosing scopes until we find it.
            TreePtr<Declaration> ed = FindExistingDeclaration( SS, Name, true );
            ASSERT(ed)("Cannot find declaration of \"%s\"", Name->getName());

            return hold_decl.ToRaw( ed );
        }

        // Tag is a definition or declaration. Create if it doesn't already
        // exist, *but* don't recurse into enclosing scopes.
        if( TreePtr<Declaration> ed = FindExistingDeclaration( SS, Name, false ) )
        {
            // Note: members will be filled in later, so nothing to do here
            // even if the is the "complete" version of the record (=definition).
            return hold_decl.ToRaw( ed );
        }

        TreePtr<Record> h;
        switch( (clang::DeclSpec::TST)TagType )
        {
            case clang::DeclSpec::TST_union:
            h = MakeTreeNode<Union>();
            break;
            case clang::DeclSpec::TST_struct:
            h = MakeTreeNode<Struct>();
            break;
            case clang::DeclSpec::TST_class:
            h = MakeTreeNode<Class>();
            break;
            case clang::DeclSpec::TST_enum:
            h = MakeTreeNode<Enum>();
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
            ident_track.Add(nullptr, h, S);
        }

        //TODO should we do something with TagKind? Maybe needed for render.
        //TODO use the attibutes

        // struct/class/union pushed by ActOnFinishCXXClassDef()
        if( (clang::DeclSpec::TST)TagType == clang::DeclSpec::TST_enum )
        decl_to_insert = h;

        //TRACE("done tag %p\n", h.get());

        return hold_decl.ToRaw( h );
    }

    /// ActOnStartCXXClassDef - This is called at the start of a class/struct/union
    /// definition, when on C++.
    virtual void ActOnStartCXXClassDef(clang::Scope *S, DeclTy *TagDecl,
            clang::SourceLocation LBrace)
    {
        //TRACE("S%p\n", S);

        // Just populate the members container for the Record node
        // we already created. No need to return anything.
        TreePtr<Declaration> d = hold_decl.FromRaw( TagDecl );
        TreePtr<Record> h = DynamicTreePtrCast<Record>(d);

        // We're about to populate the Record; if it has been populated already
        // then something's wrong
        ASSERT( h->members.empty() )("Record has already been defined");

        ident_track.SetNextRecord( h );

        inferno_scope_stack.push( h ); // decls for members will go on this scope
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
        TreePtr<Declaration> d = hold_decl.FromRaw( TagDecl );
        TreePtr<Record> h = DynamicTreePtrCast<Record>(d);
        decl_to_insert = h;
    }

    virtual ExprResult ActOnMemberReferenceExpr(clang::Scope *S, ExprTy *Base,
            clang::SourceLocation OpLoc,
            clang::tok::TokenKind OpKind,
            clang::SourceLocation MemberLoc,
            clang::IdentifierInfo &Member)
    {
        TRACE("kind %d\n", OpKind);
        auto a = MakeTreeNode<Lookup>();

        // Turn -> into * and .
        if( OpKind == clang::tok::arrow ) // Base->Member

        {
            auto ou = MakeTreeNode<Dereference>();
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
        TreePtr<Node> tbase = TypeOf::instance( a->base, all_decls ).GetTreePtr();
        TreePtr<TypeIdentifier> tibase = DynamicTreePtrCast<TypeIdentifier>(tbase);
        ASSERT( tibase );
        DefaultTransUtils utils(all_decls);
        TransKit kit { &utils };
        TreePtr<Record> rbase = GetRecordDeclaration(kit, tibase).GetTreePtr();
        ASSERT( rbase )( "thing on left of ./-> is not a record/record ptr" );
        TreePtr<Instance> m = FindMemberByName( kit, rbase, string(Member.getName()) ).GetTreePtr();
        ASSERT(m)("in r.m or (&r)->m, could not find m in r\n"
                  "m is %s, r is ", 
                  Member.getName())
                  (*rbase)
                  (" named ")
                  (*tibase);

        a->member = m->identifier;

        return hold_expr.ToRaw( a );
    }

    virtual ExprResult ActOnArraySubscriptExpr(clang::Scope *S,
            ExprTy *Base, clang::SourceLocation LLoc,
            ExprTy *Idx, clang::SourceLocation RLoc)
    {
        auto su = MakeTreeNode<Subscript>();
        su->operands.push_back( hold_expr.FromRaw( Base ) );
        su->operands.push_back( hold_expr.FromRaw( Idx ) );
        return hold_expr.ToRaw( su );
    }

    /// ActOnCXXBoolLiteral - Parse {true,false} literals.
    virtual ExprResult ActOnCXXBoolLiteral(clang::SourceLocation OpLoc,
            clang::tok::TokenKind Kind) //TODO not working - get node has no info

    {
        TreePtr<Literal> ic;
        TRACE("true/false tk %d %d %d\n", Kind, clang::tok::kw_true, clang::tok::kw_false );

        if(Kind == clang::tok::kw_true)
        ic = MakeTreeNode<True>();
        else
        ic = MakeTreeNode<False>();
        return hold_expr.ToRaw( ic );
    }

    virtual ExprResult ActOnCastExpr(clang::SourceLocation LParenLoc, TypeTy *Ty,
            clang::SourceLocation RParenLoc, ExprTy *Op)
    {
        auto c = MakeTreeNode<Cast>();
        c->operand = hold_expr.FromRaw( Op );
        c->type = hold_type.FromRaw( Ty );
        return hold_expr.ToRaw( c );
    }

    virtual OwningStmtResult ActOnNullStmt(clang::SourceLocation SemiLoc)
    {
        TRACE();
        auto n = MakeTreeNode<Nop>();
        return ToStmt( n );
    }

    virtual ExprResult ActOnCharacterConstant(const clang::Token &tok)
    {
        string t = preprocessor.getSpelling(tok);

        clang::CharLiteralParser literal(t.c_str(), t.c_str()+t.size(), tok.getLocation(), preprocessor);

        if (literal.hadError())
        return ExprResult(true);

        llvm::APSInt rv(TypeDb::char_bits, !TypeDb::char_default_signed);
        rv = literal.getValue();
        auto nc = MakeTreeNode<SpecificInteger>( rv );

        return hold_expr.ToRaw( nc );
    }

    virtual ExprResult ActOnInitList(clang::SourceLocation LParenLoc,
            ExprTy **InitList, unsigned NumInit,
            clang::InitListDesignations &Designators,
            clang::SourceLocation RParenLoc)
    {
        ASSERT( !Designators.hasAnyDesignators() )( "Designators in init lists unsupported" );
        // Assume initialiser is for an Array, and create an ArrayInitialiser node
        // even if it's really a struct init. We'll come along later and replace with a
        // RecordInitialiser when we can see what the struct is.
        auto ao = MakeTreeNode<MakeArray>();
        for(unsigned i=0; i<NumInit; i++)
        {
            TreePtr<Expression> e = hold_expr.FromRaw( InitList[i] );
            ao->operands.push_back( e );
        }
        return hold_expr.ToRaw( ao );
    }

    // Create a RecordInitialiser using the elements of the supplied ArrayInitialiser and matching
    // them against the members of the supplied record. Records are stored using an unordered
    // collection for the members, so we have to use the ordered backing map. Array inits are ordered.
    TreePtr<MakeRecord> CreateRecordLiteralFromArrayLiteral( TreePtr<MakeArray> ai,
            TreePtr<Record> r )
    {
        // Make new record initialiser and fill in the type
        auto ri = MakeTreeNode<MakeRecord>();
        ri->type = r->identifier;

        // Fill in the RecordLiteral operands collection with pairs that relate operands to their member ids
        TreePtr<Scope> s = r;
        PopulateMapOperator( ri, ai->operands, s );

        return ri;
    }

    // Populate a map operator using elements from a sequence of expressions
    void PopulateMapOperator( TreePtr<MapOperator> mapop, // MapOperands corresponding to the elements of ai go in here
            Sequence<Expression> &seq, // Operands to insert, ordered as per the input program
            TreePtr<Scope> scope ) // Original Scope that established ordering, must be in backing_ordering

    {
        // Get a reference to the ordered list of members for this scope from a backing list
        ASSERT( backing_ordering.count(scope) > 0 )("Supplied scope did not make it into the backing ordering list");
        Sequence<Declaration> &scope_ordered = backing_ordering[scope];
        //TRACE("%p %p\n", &scope->members, scope.get());

        // Go over the entire scope, keeping track of where we are in the Sequence
        Sequence<Expression>::iterator seq_it = seq.begin();
        for( TreePtr<Declaration> d : scope_ordered )
        {
            if( seq_it == seq.end() )
            {
                TRACE("Early out due to fewer elements in sequence than scope\n"); 
                break;
            }
            // We only care about instances...
            if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>( d ) )
            {
                // ...and not function instances
                if( !DynamicTreePtrCast<Callable>( i->type ) )
                {
                    // Get value out of array init and put it in record init together with member instance id
                    TreePtr<Expression> v = *seq_it;
                    auto mi = MakeTreeNode<MapOperand>();
                    mi->identifier = i->identifier;
                    mi->value = v;
                    mapop->operands.insert( mi );

                    ++seq_it;
                }
            }
        }        
        ASSERT( seq_it == seq.end() )
              ("Too many arguments to function/struct init (we allow too few for poor mans overlading, but not too many)\n")
              ("Scope was ")(*scope)(" for map operator ")(mapop)("\n");
    }

    TreePtr<String> CreateString( const char *s )
    {
        return MakeTreeNode<SpecificString>( s );
    }

    TreePtr<String> CreateString( clang::IdentifierInfo *Id )
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
        return hold_expr.ToRaw( MakeTreeNode<This>() );
    }

    virtual DeclTy *ActOnEnumConstant(clang::Scope *S, DeclTy *EnumDecl,
            DeclTy *LastEnumConstant,
            clang::SourceLocation IdLoc, clang::IdentifierInfo *Id,
            clang::SourceLocation EqualLoc, ExprTy *Val)
    {
        TreePtr<Declaration> d( hold_decl.FromRaw( EnumDecl ) );
        TreePtr<Enum> e( DynamicTreePtrCast<Enum>(d) );
        auto o = MakeTreeNode<Static>();
        all_decls->members.insert(o);
        o->identifier = CreateInstanceIdentifier(Id);
        o->constancy = MakeTreeNode<Const>(); // static const member need not consume storage!!
        o->type = e->identifier;//CreateIntegralType( TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified], false );
        if( Val )
        {
            o->initialiser = hold_expr.FromRaw( Val );
        }
        else if( LastEnumConstant )
        {
            TreePtr<Declaration> lastd( hold_decl.FromRaw( LastEnumConstant ) );
            TreePtr<Instance> lasto( DynamicTreePtrCast<Instance>(lastd) );
            ASSERT(lasto)( "unexpected kind of declaration inside an enum");
            auto inf = MakeTreeNode<Add>();
            TreePtr<Expression> ei = lasto->identifier;
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
        TreePtr<Declaration> d( hold_decl.FromRaw( EnumDecl ) );
        TreePtr<Enum> e( DynamicTreePtrCast<Enum>(d) );
        ASSERT( e )( "expected the declaration to be an enum");
        for( unsigned i=0; i<NumElements; i++ )
            e->members.insert( hold_decl.FromRaw( Elements[i] ) );
    }

    /// ParsedFreeStandingDeclSpec - This method is invoked when a declspec with
    /// no declarator (e.g. "struct foo;") is parsed.
    virtual DeclTy *ParsedFreeStandingDeclSpec(clang::Scope *S, clang::DeclSpec &DS)
    {
        TRACE();
        TreePtr<Declaration> d( hold_decl.FromRaw( DS.getTypeRep() ) );
        TreePtr<Record> h( DynamicTreePtrCast<Record>( d ) );
        ASSERT( h );
        if( decl_to_insert )
        {
            d = decl_to_insert;
            decl_to_insert = TreePtr<Declaration>();
        }

        // See if the declaration is already there (due to forwarding using
        // incomplete struct). If so, do not add it again
        Collection<Declaration> &sd = inferno_scope_stack.top()->members;
        for( const TreePtr<Declaration> &p : sd ) // TODO find()?
        if( TreePtr<Declaration>(p) == d )
        return hold_decl.ToRaw( d );

        inferno_scope_stack.top()->members.insert( d );
        backing_ordering[inferno_scope_stack.top()].push_back( d );
        return hold_decl.ToRaw( d );
    }

    virtual ExprResult
    ActOnSizeOfAlignOfExpr( clang::SourceLocation OpLoc, bool isSizeof, bool isType,
            void *TyOrEx, const clang::SourceRange &ArgRange)
    {
        TreePtr<TypeOperator> p;
        if( isSizeof )
            p = MakeTreeNode<SizeOf>();
        else
            p = MakeTreeNode<AlignOf>();

        if( isType )
        p->operand = hold_type.FromRaw(TyOrEx);
        else
        {
            ASSERT(0)("typeof() only supported on types at the moment");
            // TODO This is wrong because we'll get 2 refs to the type, need to duplicate,
            // or maybe add an alternative node and convert in a S&R
            p->operand = TreePtr<Type>::DynamicCast( TypeOf::instance( hold_expr.FromRaw(TyOrEx), all_decls ).GetTreePtr() );
        }
        return hold_expr.ToRaw( p );
    }

    virtual BaseResult ActOnBaseSpecifier(DeclTy *classdecl,
            clang::SourceRange SpecifierRange,
            bool Virt, clang::AccessSpecifier AccessSpec,
            TypeTy *basetype,
            clang::SourceLocation BaseLoc)
    {
        TreePtr<Type> t( hold_type.FromRaw( basetype ) );
        TreePtr<SpecificTypeIdentifier> ti = DynamicTreePtrCast<SpecificTypeIdentifier>(t);
        ASSERT( ti );
        TreePtr<Declaration> d = hold_decl.FromRaw( classdecl );
        TreePtr<Record> r = DynamicTreePtrCast<Record>( d );
        ASSERT( r );

        auto base = MakeTreeNode<Base>();
        base->record = ti;
        /*  if( Virt )
         base->storage = MakeTreeNode<Virtual>();
         else
         base->storage = MakeTreeNode<NonStatic>();
         base->constancy = MakeTreeNode<NonConst>(); */
        base->access = ConvertAccess( AccessSpec, r );
        return hold_base.ToRaw( base );
    }

    virtual void ActOnBaseSpecifiers(DeclTy *ClassDecl, BaseTy **Bases,
            unsigned NumBases)
    {
        TreePtr<Declaration> cd( hold_decl.FromRaw( ClassDecl ) );
        TreePtr<InheritanceRecord> ih( DynamicTreePtrCast<InheritanceRecord>(cd) );
        ASSERT( ih );

        for( unsigned i=0; i<NumBases; i++ )
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
        TreePtr<Node> n( ident_track.Get( &II, FromCXXScope( &SS ) ) );

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
        TreePtr<Node> n = FromCXXScope( &SS );
        ASSERT(n);
        ident_track.PushScope( nullptr, n );
    }

    /// ActOnCXXExitDeclaratorScope - Called when a declarator that previously
    /// invoked ActOnCXXEnterDeclaratorScope(), is finished. 'SS' is the same
    /// CXXScopeSpec that was passed to ActOnCXXEnterDeclaratorScope as well.
    /// Used to indicate that names should revert to being looked up in the
    /// defining scope.
    virtual void ActOnCXXExitDeclaratorScope(clang::Scope *S, const clang::CXXScopeSpec &SS)
    {
        TRACE();
        ident_track.PopScope( nullptr );
    }

    TreePtr<Instance> GetConstructor( TreePtr<Type> t )
    {
        TreePtr<TypeIdentifier> id = DynamicTreePtrCast<TypeIdentifier>(t);
        ASSERT(id);
        DefaultTransUtils utils(all_decls);
        TransKit kit { &utils };
        TreePtr<Record> r = GetRecordDeclaration( kit, id ).GetTreePtr();

        for( TreePtr<Declaration> d : r->members )
        {
            TreePtr<Instance> o( DynamicTreePtrCast<Instance>(d) );
            if( !o )
            continue;
            if( DynamicTreePtrCast<Constructor>(o->type) )
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
        TreePtr<Node> n( ident_track.Get( MemberOrBase ) );
        TreePtr<Instance> om( DynamicTreePtrCast<Instance>(n) );
        ASSERT( om );
        TreePtr<Instance> cm = GetConstructor( om->type );
        ASSERT( cm );
        ASSERT( cm->identifier );

        // Build a lookup to the constructor, using the speiciifed subobject and the matching constructor
        auto lu = MakeTreeNode<Lookup>();
        lu->base = om->identifier;
        lu->member = cm->identifier;

        // Build a call to the constructor with supplied args
        Sequence<Expression> args;
        CollectArgs( &args, Args, NumArgs );
        TreePtr<Call> call = CreateCall( args, lu );

        // Get the constructor whose init list we're adding to (may need to start a
        // new compound statement)
        TreePtr<Declaration> d( hold_decl.FromRaw( ConstructorDecl ) );
        TreePtr<Instance> o( DynamicTreePtrCast<Instance>(d) );
        ASSERT(o);
        TreePtr<Compound> comp = DynamicTreePtrCast<Compound>(o->initialiser);
        if( !comp )
        {
            comp = MakeTreeNode<Compound>();
            o->initialiser = comp;
            TRACE();
        }

        // Add it
        comp->statements.push_back( call );
        return 0;
    }

    void CollectArgs( Sequence<Expression> *ps, ExprTy **Args, unsigned NumArgs )
    {
        for(unsigned i=0; i<NumArgs; i++ )
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
        auto n = MakeTreeNode<New>();
        n->type = CreateTypeNode( D );
        CollectArgs( &(n->placement_arguments), PlacementArgs, NumPlaceArgs );
        CollectArgs( &(n->constructor_arguments), ConstructorArgs, NumConsArgs );

        if( UseGlobal )
            n->global = MakeTreeNode<Global>();
        else
            n->global = MakeTreeNode<NonGlobal>();

        // TODO cant figure out meaning of ParenTypeId

        return hold_expr.ToRaw( n );
    }

    /// ActOnCXXDelete - Parsed a C++ 'delete' expression. UseGlobal is true if
    /// the delete was qualified (::delete). ArrayForm is true if the array form
    /// was used (delete[]).
    virtual ExprResult ActOnCXXDelete( clang::SourceLocation StartLoc, bool UseGlobal,
            bool ArrayForm, ExprTy *Expression )
    {
        auto d = MakeTreeNode<Delete>();
        d->pointer = hold_expr.FromRaw( Expression );

        if( ArrayForm )
            d->array = MakeTreeNode<DeleteArray>();
        else
            d->array = MakeTreeNode<DeleteNonArray>();

        if( UseGlobal )
            d->global = MakeTreeNode<Global>();
        else
            d->global = MakeTreeNode<NonGlobal>();

        return hold_expr.ToRaw( d );
    }

    virtual ExprResult ActOnCompoundLiteral(clang::SourceLocation LParen, TypeTy *Ty,
            clang::SourceLocation RParen, ExprTy *Op)
    {
        TreePtr<Type> t = hold_type.FromRaw( Ty );
        TreePtr<Expression> e = hold_expr.FromRaw( Op );

        //TRACE("%p\n", t.get() );

        // At this point, when we have the instance (and hence the type) and the initialiser
        // we can detect when an array initialiser has been inserted for a record instance and
        // change it.
        DefaultTransUtils utils(all_decls);
        TransKit kit { &utils };
        if( TreePtr<MakeArray> ai = DynamicTreePtrCast<MakeArray>(e) )
            if( TreePtr<TypeIdentifier> ti = DynamicTreePtrCast<TypeIdentifier>(t) )
                if( TreePtr<Record> r = GetRecordDeclaration(kit, ti).GetTreePtr() )
                    e = CreateRecordLiteralFromArrayLiteral( ai, r );

        return hold_expr.ToRaw( e );
    }

    virtual ExprResult ActOnStmtExpr(clang::SourceLocation LPLoc, StmtTy *SubStmt,
                                     clang::SourceLocation RPLoc) // "({..})"
    {
        TreePtr<Compound> cb( DynamicTreePtrCast<Compound>( hold_stmt.FromRaw( SubStmt ) ) );
        ASSERT(cb); // Let's hope the only statement Clang gives us is a compound

        auto ce = MakeTreeNode<StatementExpression>();
        ce->statements = cb->statements;
        return hold_expr.ToRaw( ce );
    }

    //--------------------------------------------- unimplemented actions -----------------------------------------------
    // Note: only actions that return something (so we don't get nullptr XTy going around the place). No obj-C or GCC
    // extensions. These all assert out immediately.

    virtual DeclTy *ActOnFileScopeAsmDecl(clang::SourceLocation Loc, ExprArg AsmString)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual DeclTy *ActOnField(clang::Scope *S, DeclTy *TagD, clang::SourceLocation DeclStart,
            clang::Declarator &D, ExprTy *BitfieldWidth)
    {
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
            clang::SourceLocation RParenLoc)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual ExprResult ActOnPredefinedExpr(clang::SourceLocation Loc,
            clang::tok::TokenKind Kind)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual DeclTy *ActOnStartNamespaceDef(clang::Scope *S, clang::SourceLocation IdentLoc,
            clang::IdentifierInfo *Ident,
            clang::SourceLocation LBrace)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual ExprResult ActOnCXXNamedCast(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
            clang::SourceLocation LAngleBracketLoc, TypeTy *Ty,
            clang::SourceLocation RAngleBracketLoc,
            clang::SourceLocation LParenLoc, ExprTy *Op,
            clang::SourceLocation RParenLoc)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual ExprResult ActOnCXXThrow(clang::SourceLocation OpLoc,
            ExprTy *Op = 0)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual ExprResult ActOnCXXTypeConstructExpr(clang::SourceRange TypeRange,
            TypeTy *TypeRep,
            clang::SourceLocation LParenLoc,
            ExprTy **Exprs,
            unsigned NumExprs,
            clang::SourceLocation *CommaLocs,
            clang::SourceLocation RParenLoc)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual ExprResult ActOnCXXConditionDeclarationExpr(clang::Scope *S,
            clang::SourceLocation StartLoc,
            clang::Declarator &D,
            clang::SourceLocation EqualLoc,
            ExprTy *AssignExprVal)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual DeclTy *ActOnExceptionDeclarator(clang::Scope *S, clang::Declarator &D)
    {
        ASSERTFAIL("Unimplemented action");
        return 0;
    }

    virtual OwningStmtResult ActOnCXXCatchBlock(clang::SourceLocation CatchLoc,
            DeclTy *ExceptionDecl,
            StmtArg HandlerBlock)
    {
        ASSERTFAIL("Unimplemented action");
        return StmtEmpty();
    }

    virtual OwningStmtResult ActOnCXXTryBlock(clang::SourceLocation TryLoc,
            StmtArg TryBlock,
            MultiStmtArg Handlers)
    {
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
