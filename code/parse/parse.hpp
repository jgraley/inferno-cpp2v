#ifndef PARSE_HPP
#define PARSE_HPP

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
        clang::TargetInfo* ptarget = clang::TargetInfo::CreateTargetInfo(INFERNO_TRIPLE);
        assert(ptarget);
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
    
    class InfernoAction : public clang::InfernoMinimalAction
    {
    public:
        InfernoAction(shared_ptr<Program> p, clang::IdentifierTable &IT, clang::Preprocessor &pp) : 
            InfernoMinimalAction(IT),
            preprocessor(pp),
            program(p),
            curseq(p)
        {
        }
     
        ~InfernoAction()
        {
            assert( &*curseq==&*program ); // TODO operator== in shared_ptr<> 
        }
     
     private:   
        clang::Preprocessor &preprocessor;
        shared_ptr<Program> program;
        shared_ptr<Scope> curseq;
        RCHold<Declarator, DeclTy *> hold_decl;
        RCHold<Expression, ExprTy *> hold_expr;
        RCHold<Statement, StmtTy *> hold_stmt;
        RCHold<Type, TypeTy *> hold_type;
     
        shared_ptr<Type> CreateTypeNode( clang::Declarator &D, int depth=0 )
        {
            assert( depth>=0 );
            assert( depth<=D.getNumTypeObjects() );
            
            if( depth==D.getNumTypeObjects() )
            {
                const clang::DeclSpec &DS = D.getDeclSpec();
                switch( DS.getTypeSpecType() )
                {
                    case clang::DeclSpec::TST_int:
                        return shared_ptr<Type>(new Int());
                        break;
                    case clang::DeclSpec::TST_char:
                        return shared_ptr<Type>(new Char());
                        break;
                    case clang::DeclSpec::TST_void:
                        return shared_ptr<Type>(new Void());
                        break;
                    default:
                        assert(0);
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
                        const clang::DeclaratorChunk::FunctionTypeInfo &fchunk = chunk.Fun; // TODO deal with compounded types
                        shared_ptr<FunctionPrototype> f(new FunctionPrototype);
                        for( int i=0; i<fchunk.NumArgs; i++ )
                        {
                            shared_ptr<Declarator> d = hold_decl.FromRaw( fchunk.ArgInfo[i].Param );
                            shared_ptr<VariableDeclarator> vd = dynamic_pointer_cast<VariableDeclarator>(d); // just push the declarators, no need for dynamic cast?
                            f->parameters.push_back( vd );
                        }
                        f->return_type = CreateTypeNode( D, depth+1 );                        
                        return f;
                    }
                    
                default:
                assert(!"Unknown type chunk");                     
                    break;
                }
            }
        }
        
        shared_ptr<Identifier> CreateIdentifierNode( clang::IdentifierInfo *ID )
        { 
            shared_ptr<Identifier> i(new Identifier());
            i->assign( ID->getName() );
            TRACE("ci %s %p %p\n", ID->getName(), &*i, ID );            
            return i;
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
            shared_ptr<VariableDeclarator> p(new VariableDeclarator);
            curseq->push_back(p);
            p->storage_class = VariableDeclarator::STATIC;
            p->identifier = CreateIdentifierNode( D.getIdentifier() );        

            p->type = CreateTypeNode( D );
            TRACE("aod %s %p %p\n", p->identifier->c_str(), &*p->identifier, &*p );

            (void)clang::InfernoMinimalAction::ActOnDeclarator( S, D, LastInGroup, p->identifier );     
            return hold_decl.ToRaw( p );
        }
        
          /// ActOnParamDeclarator - This callback is invoked when a parameter
          /// declarator is parsed. This callback only occurs for functions
          /// with prototypes. S is the function prototype scope for the
          /// parameters (C++ [basic.scope.proto]).
        virtual DeclTy *ActOnParamDeclarator(clang::Scope *S, clang::Declarator &D) 
        {
            shared_ptr<VariableDeclarator> p(new VariableDeclarator);
            p->storage_class = VariableDeclarator::STATIC;
            p->type = CreateTypeNode( D );
            p->identifier = CreateIdentifierNode( D.getIdentifier() );        
            TRACE("aod %s %p %p\n", p->identifier->c_str(), &*p->identifier, &*p );
            (void)clang::InfernoMinimalAction::ActOnDeclarator( S, D, 0, p->identifier );     
            return hold_decl.ToRaw( p );
        }

        /*
        Note: the default implementation of ActOnStartOfFunctionDef() appears in
        InfernoMinimalAction and can cause spurious ActOnDeclarator() calls if we always
        call through. Therefore we don't and instead just call explicitly implemented
        functions in InfernoMinimalAction where required.
        */
        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
        {
            shared_ptr<FunctionDeclarator> p(new FunctionDeclarator);
            curseq->push_back(p);
            p->identifier = CreateIdentifierNode( D.getIdentifier() );
            /*  
            clang::DeclaratorChunk::FunctionTypeInfo &pti = D.getTypeObject(0).Fun; // TODO deal with compounded types
            for( int i=0; i<pti.NumArgs; i++ )
            {
                shared_ptr<Declarator> d = hold_decl.FromRaw( pti.ArgInfo[i].Param );
                shared_ptr<VariableDeclarator> vd = dynamic_pointer_cast<VariableDeclarator>(d); // just push the declarators, no need for dynamic cast?
                p->parameters.push_back( vd );
            }
*/
            p->prototype = CreateTypeNode( D );
            curseq = p->body = shared_ptr<Scope>(new Scope);
            clang::Scope *GlobalScope = FnBodyScope->getParent();
            (void)clang::InfernoMinimalAction::ActOnDeclarator( GlobalScope, D, 0, p->identifier );     
            return hold_decl.ToRaw( p );     
        }
        
        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
        {
            curseq = program;
            return Decl;
        }    
        
        virtual StmtResult ActOnExprStmt(ExprTy *Expr) 
        {
            shared_ptr<Expression> e = hold_expr.FromRaw(Expr);
            shared_ptr<ExpressionStatement> es(new ExpressionStatement);
            es->expression = e;
            curseq->push_back( es );
            return hold_stmt.ToRaw( es );
        }

        virtual StmtResult ActOnReturnStmt( clang::SourceLocation ReturnLoc,
                                            ExprTy *RetValExp ) 
        {           
            shared_ptr<Expression> e = hold_expr.FromRaw(RetValExp);
            shared_ptr<Return> r(new Return);
            r->return_value = e;
            curseq->push_back( r );
            return hold_stmt.ToRaw( r );
        }

        virtual ExprResult ActOnIdentifierExpr( clang::Scope *S, 
                                                clang::SourceLocation Loc,
                                                clang::IdentifierInfo &II,
                                                bool HasTrailingLParen ) 
        {
            TRACE("aoie %s\n", II.getName() );
            TRACE("aoie2 %p\n", &II );
            shared_ptr<IdentifierExpression> ie(new IdentifierExpression);
            ie->identifier = clang::InfernoMinimalAction::GetCurrentIdentifierRCPtr( II );
            shared_ptr<Expression> e = ie;
            TRACE("aoie4 %p\n", &*e);
            return hold_expr.ToRaw( e );            
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
            assert(!literal.hadError);

            if (literal.isIntegerLiteral()) 
            {
                // Get the value in the widest-possible width.
                llvm::APInt rv(64, 0);
               
                bool err = literal.GetIntegerValue(rv);
                
                assert( !err );
                return rv;
            }
            assert(!"only ints supported");
        }
        
        virtual ExprResult ActOnNumericConstant(const clang::Token &tok) 
        { 
            shared_ptr<NumericConstant> nc(new NumericConstant);
            *(llvm::APInt *)&*nc = EvaluateNumericConstant( tok );
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
    };
};  

#endif
