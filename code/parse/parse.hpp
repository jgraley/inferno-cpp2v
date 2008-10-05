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
    Parse( std::string i ) :
        infile(i)
    {
    }
    
    void operator()( RCPtr<Program> program )
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
    std::string infile;
    
    class InfernoAction : public clang::InfernoMinimalAction
    {
    public:
        InfernoAction(RCPtr<Program> p, clang::IdentifierTable &IT, clang::Preprocessor &pp) : 
            InfernoMinimalAction(IT),
            preprocessor(pp),
            program(p),
            curseq(p)
        {
        }
     
        ~InfernoAction()
        {
            assert( &*curseq==&*program ); // TODO operator== in RCPtr<> 
        }
     
     private:   
        clang::Preprocessor &preprocessor;
        RCPtr<Program> program;
        RCPtr< Sequence<ProgramElement> > curseq;
        RCHold hold;
    
        Type *CreateTypeNode( const clang::DeclSpec &DS )
        {
            switch( DS.getTypeSpecType() )
            {
                case clang::DeclSpec::TST_int:
                    return new Int();
                    break;
                case clang::DeclSpec::TST_char:
                    return new Char();
                    break;
                case clang::DeclSpec::TST_void:
                    return new Void();
                    break;
                default:
                    assert(0);
                    break;
            }
        }
        
        RCPtr<Identifier> CreateIdenifierNode( clang::IdentifierInfo *ID )
        { 
            RCPtr<Identifier> i = new Identifier();
            i->assign( ID->getName() );
            TRACE("ci %s %p %p\n", ID->getName(), i.ptr, ID );            
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
            RCPtr<VariableDeclarator> p = new VariableDeclarator;
            curseq->push_back(p);
            p->storage_class = VariableDeclarator::STATIC;
            p->type = CreateTypeNode( D.getDeclSpec() );
            p->identifier = CreateIdenifierNode( D.getIdentifier() );        
            TRACE("aod %s %p %p\n", p->identifier->c_str(), &*p->identifier, &*p );
            RCPtr<Identifier> i = p->identifier;
            (void)clang::InfernoMinimalAction::ActOnDeclarator( S, D, LastInGroup, i );     
            return hold.ToRaw( p );
        }
        
          /// ActOnParamDeclarator - This callback is invoked when a parameter
          /// declarator is parsed. This callback only occurs for functions
          /// with prototypes. S is the function prototype scope for the
          /// parameters (C++ [basic.scope.proto]).
        virtual DeclTy *ActOnParamDeclarator(clang::Scope *S, clang::Declarator &D) 
        {
            RCPtr<VariableDeclarator> p = new VariableDeclarator;
            p->storage_class = VariableDeclarator::STATIC;
            p->type = CreateTypeNode( D.getDeclSpec() );
            p->identifier = CreateIdenifierNode( D.getIdentifier() );        
            TRACE("aod %s %p %p\n", p->identifier->c_str(), &*p->identifier, &*p );
            RCPtr<Identifier> i = p->identifier;   
            return hold.ToRaw( p );
        }

        /*
        Note: the default implementation of ActOnStartOfFunctionDef() appears in
        InfernoMinimalAction and can cause spurious ActOnDeclarator() calls if we always
        call through. Therefore we don't and instead just call explicitly implemented
        functions in InfernoMinimalAction where required.
        */
        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
        {
            RCPtr<FunctionDeclarator> p = new FunctionDeclarator;
            curseq->push_back(p);
            p->return_type = CreateTypeNode( D.getDeclSpec() );
            p->body = new Sequence<ProgramElement>;
            p->identifier = CreateIdenifierNode( D.getIdentifier() );  
            clang::DeclaratorChunk::FunctionTypeInfo &pti = D.getTypeObject(0).Fun; // TODO deal with compounded types
            for( int i=0; i<pti.NumArgs; i++ )
            {
                RCPtr<VariableDeclarator> vd = new VariableDeclarator;
                //vd->storage_class = VariableDeclarator::AUTO;
                //printf("gtgtg\n");
                //assert(pti.ArgInfo[i].Param);
                vd = hold.FromRaw<VariableDeclarator>( pti.ArgInfo[i].Param );
                //vd->identifier = CreateIdenifierNode( pti.ArgInfo[i].Ident );
                p->params.push_back( vd );
            }
            curseq = p->body;
            return hold.ToRaw( p );     
        }
        
        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
        {
            curseq = program;
            return Decl;
        }    
        
        virtual StmtResult ActOnExprStmt(ExprTy *Expr) 
        {
            RCPtr<Expression> e = hold.FromRaw<Expression>(Expr);
            RCPtr<ExpressionStatement> es = new ExpressionStatement;
            es->expression = e;
            curseq->push_back( es );
            return hold.ToRaw( es );
        }

        virtual StmtResult ActOnReturnStmt( clang::SourceLocation ReturnLoc,
                                            ExprTy *RetValExp ) 
        {           
            RCPtr<Expression> e = hold.FromRaw<Expression>(RetValExp);
            RCPtr<Return> r = new Return;
            r->return_value = e;
            curseq->push_back( r );
            return hold.ToRaw( r );
        }

        virtual ExprResult ActOnIdentifierExpr( clang::Scope *S, 
                                                clang::SourceLocation Loc,
                                                clang::IdentifierInfo &II,
                                                bool HasTrailingLParen ) 
        {
            assert( !HasTrailingLParen ); // not done yet
            TRACE("aoie %s\n", II.getName() );
            TRACE("aoie2 %p\n", &II );
            RCPtr<IdentifierExpression> ie = new IdentifierExpression;
            RCPtr<RCTarget> rcp = clang::InfernoMinimalAction::GetCurrentIdentifierRCPtr( II );
            assert(rcp && "no RCPtr was stored with this identifier");
            ie->identifier = RCPtr<Identifier>::Specialise(rcp);
            assert(ie->identifier && "The RCPtr stored with this identifier was not pointing to an Identifier node");
            RCPtr<Expression> e = ie;
            TRACE("aoie4 %p\n", e.ptr);
            return ExprResult( hold.ToRaw( e ) );            
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
            RCPtr<NumericConstant> nc = new NumericConstant;
            *(llvm::APInt *)nc = EvaluateNumericConstant( tok );
            return ExprResult( hold.ToRaw( nc ) );            
        }
  
        virtual ExprResult ActOnBinOp(clang::SourceLocation TokLoc, clang::tok::TokenKind Kind,
                                      ExprTy *LHS, ExprTy *RHS) 
        {
            RCPtr<Infix> o = new Infix;
            o->kind = Kind;
            o->operands.push_back( hold.FromRaw<Expression>(LHS) );
            o->operands.push_back( hold.FromRaw<Expression>(RHS) );
            return hold.ToRaw( o );            
        }                     

        virtual ExprResult ActOnPostfixUnaryOp(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
                                               ExprTy *Input) 
        {
            RCPtr<Postfix> o = new Postfix;
            o->kind = Kind;
            o->operands.push_back( hold.FromRaw<Expression>(Input) );
            return hold.ToRaw( o );            
        }                     

        virtual ExprResult ActOnUnaryOp(clang::SourceLocation OpLoc, clang::tok::TokenKind Kind,
                                               ExprTy *Input) 
        {
            RCPtr<Prefix> o = new Prefix;
            o->kind = Kind;
            o->operands.push_back( hold.FromRaw<Expression>(Input) );
            return hold.ToRaw( o );            
        }                     

       virtual ExprResult ActOnConditionalOp(clang::SourceLocation QuestionLoc, 
                                             clang::SourceLocation ColonLoc,
                                             ExprTy *Cond, ExprTy *LHS, ExprTy *RHS)
        {
            RCPtr<ConditionalOperator> o = new ConditionalOperator;
            o->condition = hold.FromRaw<Expression>(Cond);
            o->if_true = hold.FromRaw<Expression>(LHS);
            o->if_false = hold.FromRaw<Expression>(RHS);
            return hold.ToRaw( o );            
        }                     

        virtual TypeResult ActOnTypeName(clang::Scope *S, clang::Declarator &D) 
        {printf("lkjdfas\n");
            RCPtr<Type> t = CreateTypeNode( D.getDeclSpec() );
            return hold.ToRaw( t );
        }
    };
};  

#endif
