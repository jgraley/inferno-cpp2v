#ifndef PARSE_HPP
#define PARSE_HPP

#include "clang/Basic/FileManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Parse/Action.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Driver/TextDiagnosticPrinter.h"

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"

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
        InfernoAction actions( program, it );                 
        
        clang::Parser parser( pp, actions );
        
        parser.ParseTranslationUnit();        
    }
    
private:
    std::string infile;
    
    class InfernoAction : public clang::InfernoMinimalAction
    {
    public:
        InfernoAction(RCPtr<Program> p, clang::IdentifierTable &IT) : 
            InfernoMinimalAction(IT),
            program(p),
            curseq(p)
        {
        }
     
        ~InfernoAction()
        {
            assert( &*curseq==&*program ); // TODO operator== in RCPtr<> 
        }
     
     private:   
        RCPtr<Program> program;
        RCPtr< Sequence<ProgramElement> > curseq;
        RCHold hold;
    
        Type *ConvertType( const clang::DeclSpec &DS )
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
        
        RCPtr<Identifier> ConvertIdentifier( clang::IdentifierInfo *ID )
        { 
            RCPtr<Identifier> i = new Identifier();
            i->assign( ID->getName() );
            printf("ci %s %p %p\n", ID->getName(), i.ptr, ID );            
            return i;
        }
        
        virtual DeclTy *ActOnDeclarator( clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup )
        {
            RCPtr<VariableDeclarator> p = new VariableDeclarator;
            curseq->push_back(p);
            p->storage_class = VariableDeclarator::STATIC;
            p->type = ConvertType( D.getDeclSpec() );
            p->identifier = ConvertIdentifier( D.getIdentifier() );        
            printf("aod %s %p %p\n", p->identifier->c_str(), &*p->identifier, &*p );
            RCPtr<Identifier> i = p->identifier;
            (void)clang::InfernoMinimalAction::ActOnDeclarator( S, D, LastInGroup, i );     
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
            p->return_type = ConvertType( D.getDeclSpec() );
            p->body = new Sequence<ProgramElement>;
            p->identifier = ConvertIdentifier( D.getIdentifier() );  
    
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
            return StmtResult(Expr);
        }

        virtual ExprResult ActOnIdentifierExpr( clang::Scope *S, 
                                                clang::SourceLocation Loc,
                                                clang::IdentifierInfo &II,
                                                bool HasTrailingLParen ) 
        {
            assert( !HasTrailingLParen ); // not done yet
            printf("aoie %s\n", II.getName() );
            printf("aoie2 %p\n", &II );
            RCPtr<IdentifierExpression> ie = new IdentifierExpression;
            RCTarget *rcp = (RCTarget *)(clang::InfernoMinimalAction::GetCurrentIdentifierRCPtr( II ));
            printf("aoie3 %p\n", rcp );
            assert(rcp && "no RCPtr was stored with this identifier");
            ie->identifier = dynamic_cast<Identifier *>(rcp);
            assert(ie->identifier && "The RCPtr stored with this identifier was not pointing to an Identifier");
            RCPtr<Expression> e = ie;
            printf("aoie4 %p\n", e.ptr);
            return ExprResult( hold.ToRaw( e ) );            
        }                                                
    };
};  

#endif
