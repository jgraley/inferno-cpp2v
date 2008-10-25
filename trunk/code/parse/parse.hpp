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
        
        parser.ParseTranslationUnit();        
    }
    
private:
    string infile;
    
    class InfernoAction : public InfernoMinimalAction
    {
    public:
        InfernoAction(shared_ptr<Program> p, clang::IdentifierTable &IT, clang::Preprocessor &pp) : 
            InfernoMinimalAction(IT),
            preprocessor(pp),
            program(p)
        {
        }
     
        ~InfernoAction()
        {
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
        shared_ptr<Program> program;
        RCHold<Declaration, DeclTy *> hold_decl;
        RCHold<Expression, ExprTy *> hold_expr;
        RCHold<Statement, StmtTy *> hold_stmt;
        RCHold<Type, TypeTy *> hold_type;
        RCHold<Label, void *> hold_label;
        
        shared_ptr<Type> CreateTypeNode( clang::Declarator &D, int depth=0 )
        {
            TRACE("%d, %d\n", depth, D.getNumTypeObjects() );
            ASSERT( depth>=0 );
            ASSERT( depth<=D.getNumTypeObjects() );
            
            TRACE();
            if( depth==D.getNumTypeObjects() )
            {
                TRACE();
                const clang::DeclSpec &DS = D.getDeclSpec();
                switch( DS.getTypeSpecType() )
                {
                    case clang::DeclSpec::TST_int:
                        TRACE();
                        return shared_ptr<Type>(new Int());
                        break;
                    case clang::DeclSpec::TST_char:
                        return shared_ptr<Type>(new Char());
                        break;
                    case clang::DeclSpec::TST_void:
                        return shared_ptr<Type>(new Void());
                        break;
                    default:                    
                        ASSERT(0);
                        break;
                }
            }
            else
            {
                TRACE();
                const clang::DeclaratorChunk &chunk = D.getTypeObject(depth);
                TRACE();
                switch( chunk.Kind )
                {
                case clang::DeclaratorChunk::Function:
                    {
                        const clang::DeclaratorChunk::FunctionTypeInfo &fchunk = chunk.Fun; 
                        shared_ptr<FunctionPrototype> f(new FunctionPrototype);
                        for( int i=0; i<fchunk.NumArgs; i++ )
                        {
                            shared_ptr<Declaration> d = hold_decl.FromRaw( fchunk.ArgInfo[i].Param );
                            shared_ptr<ObjectDeclaration> vd = dynamic_pointer_cast<ObjectDeclaration>(d); // TODO just push the declarators, no need for dynamic cast?
                            f->parameters.push_back( vd );
                        }
                        f->return_type = CreateTypeNode( D, depth+1 );                        
                        return f;
                    }
                    
                case clang::DeclaratorChunk::Pointer:
                    {
                        // TODO attributes
                        const clang::DeclaratorChunk::PointerTypeInfo &pchunk = chunk.Ptr; 
                        shared_ptr<Pointer> p(new Pointer);
                        p->destination = CreateTypeNode( D, depth+1 );                        
                        return p;
                    }
                    
                case clang::DeclaratorChunk::Reference:
                    {
                        // TODO attributes
                        TRACE("ref\n");
                        const clang::DeclaratorChunk::ReferenceTypeInfo &rchunk = chunk.Ref; 
                        shared_ptr<Reference> r(new Reference);
                        ASSERT(r);
                        r->destination = CreateTypeNode( D, depth+1 );                        
                        return r;
                    }
                    
                default:
                ASSERT(!"Unknown type chunk");                     
                    break;
                }
            }
        }
        
        shared_ptr<Object> CreateObjectNode( clang::IdentifierInfo *ID,
                                             Object::StorageClass storage,
                                             shared_ptr<Type> type )
        { 
            shared_ptr<Object> v(new Object());            
            if(ID)
                v->identifier = ID->getName();
            v->storage_class = storage;
            v->type = type;
            TRACE("ci %s %p %p\n", ID->getName(), v.get(), ID );            
            return v;
        }

        shared_ptr<Label> CreateLabelNode( clang::IdentifierInfo *ID )
        { 
            shared_ptr<Label> l(new Label());            
            l->identifier = ID->getName();
            TRACE("ci %s %p %p\n", ID->getName(), l.get(), ID );            
            return l;
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
            shared_ptr<ObjectDeclaration> p(new ObjectDeclaration);
            p->object = CreateObjectNode( D.getIdentifier(), 
                                                Object::STATIC, 
                                                CreateTypeNode( D ) );        
            p->initialiser = shared_ptr<Expression>(); // might fill in later if initialised
            TRACE("aod %s %p %p\n", p->object->identifier.c_str(), p->object.get(), p.get() );
            (void)InfernoMinimalAction::ActOnDeclarator( S, D, LastInGroup, p->object );                 
            if( !(S->getParent()) )
                program->push_back( p );
            return hold_decl.ToRaw( p );
        }
        
        /// ActOnParamDeclarator - This callback is invoked when a parameter
        /// declarator is parsed. This callback only occurs for functions
        /// with prototypes. S is the function prototype scope for the
        /// parameters (C++ [basic.scope.proto]).
        virtual DeclTy *ActOnParamDeclarator(clang::Scope *S, clang::Declarator &D) 
        {
            shared_ptr<ParseParameterDeclaration> p(new ParseParameterDeclaration);
            p->object = CreateObjectNode( D.getIdentifier(), 
                                                Object::AUTO, 
                                                CreateTypeNode( D ) );        
            p->initialiser = shared_ptr<Expression>(); // might fill in later if init
            p->clang_identifier = D.getIdentifier(); // allow us to re-register the object
            TRACE("aopd %s %p %p\n", 0, p->object.get(), p.get() );
            (void)InfernoMinimalAction::ActOnDeclarator( S, D, 0, p->object );     
            return hold_decl.ToRaw( p );
        }

        virtual void AddInitializerToDecl(DeclTy *Dcl, ExprTy *Init) 
        {
            shared_ptr<Declaration> d = hold_decl.FromRaw( Dcl );
            shared_ptr<Expression> e = hold_expr.FromRaw( Init );
            
            d->initialiser = e;            
        }

        /*
        Note: the default implementation of ActOnStartOfFunctionDef() appears in
        InfernoMinimalAction and can cause spurious ActOnDeclarator() calls if we always
        call through. Therefore we don't and instead just call explicitly implemented
        functions in InfernoMinimalAction where required.
        */
        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
        {
            shared_ptr<FunctionDeclaration> p(new FunctionDeclaration);
            p->object = CreateObjectNode( D.getIdentifier(), 
                                                Object::STATIC, 
                                                CreateTypeNode( D ) );
            
            clang::Scope *GlobalScope = FnBodyScope->getParent();
            (void)InfernoMinimalAction::ActOnDeclarator( GlobalScope, D, 0, p->object );     
            
            shared_ptr<FunctionPrototype> fp = dynamic_pointer_cast<FunctionPrototype>( p->object->type );
            ASSERT(fp);
            for( int i=0; i<fp->parameters.size(); i++ )
            {
                shared_ptr<ParseParameterDeclaration> ppd = dynamic_pointer_cast<ParseParameterDeclaration>( fp->parameters[i] );                
                ASSERT(ppd);
                InfernoMinimalAction::AddNakedIdentifier(FnBodyScope, ppd->clang_identifier, ppd->object, false);
            }
            
            program->push_back( p );
            return hold_decl.ToRaw( p );     
        }
        
        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
        {
            shared_ptr<Declaration> d( hold_decl.FromRaw(Decl) );
            shared_ptr<Expression> e( dynamic_pointer_cast<Expression>( hold_stmt.FromRaw(Body) ) );
            ASSERT(e); // function body must be a scope or 0
            d->initialiser = e;
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
            return hold_expr.ToRaw( InfernoMinimalAction::GetCurrentIdentifierRCPtr( II ) );            
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
                
                ASSERT( !err );
                return rv;
            }
            ASSERT(!"only ints supported");
        }
        
        virtual ExprResult ActOnNumericConstant(const clang::Token &tok) 
        { 
            shared_ptr<NumericConstant> nc(new NumericConstant);
            *(llvm::APInt *)(nc.get()) = EvaluateNumericConstant( tok );
            return hold_expr.ToRaw( nc );            
        }
  
        virtual ExprResult ActOnBinOp(clang::SourceLocation TokLoc, clang::tok::TokenKind Kind,
                                      ExprTy *LHS, ExprTy *RHS) 
        {
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
            ASSERT(!RHSVal); // ... not supported
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
    };
};  

#endif
