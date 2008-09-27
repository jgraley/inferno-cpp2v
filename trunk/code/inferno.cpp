
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

#include "tree.hpp"

#define INFERNO_TRIPLE "arm-linux" 

//using namespace clang;

/*
Note: the default implementation of ActOnStartOfFunctionDef() appears in
MinimalAction and can cause spurious ActOnDeclarator() calls if we always
call through. Therefore we don't and instead just call explicitly implemented
functions in MinimalAction where required.
*/
class InfernoAction : public clang::MinimalAction
{
public:
    InfernoAction(clang::IdentifierTable &IT) : MinimalAction(IT)
    {
    }
 
 private:   
    void PrintType( const clang::DeclSpec &DS )
    {
        switch( DS.getTypeSpecType() )
        {
            case clang::DeclSpec::TST_int:
                printf("int ");
                break;
            case clang::DeclSpec::TST_char:
                printf("char ");
                break;
            default:
                printf("Type<%d> ", DS.getTypeSpecType() );
                break;
        }
    }
    
    virtual DeclTy *ActOnDeclarator(clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup)
    {
        PrintType( D.getDeclSpec() );
        printf("%s;\n", D.getIdentifier()->getName() );
        
        return MinimalAction::ActOnDeclarator( S, D, LastInGroup );     
    }
    
    virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
    {
        PrintType( D.getDeclSpec() );
        printf("%s()\n", D.getIdentifier()->getName() );
        printf("{\n");
       
        return MinimalAction::ActOnDeclarator( FnBodyScope, D, NULL );     
    }
    
    virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
    {
        printf("}\n");
        
        return MinimalAction::ActOnFinishFunctionBody( Decl, Body );
    }

};

int main()
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
    
    std::string infile="in.c";
    const clang::FileEntry *file = fm.getFile(infile);
    if (file) 
        sm.createMainFileID(file, clang::SourceLocation());
    if (sm.getMainFileID() == 0) 
    {
        fprintf(stderr, "Error reading '%s'!\n",infile.c_str());
        return true;
    }
    pp.EnterMainSourceFile();

    clang::IdentifierTable it( opts );                 
    InfernoAction actions( it );                 
    
    clang::Parser parser( pp, actions );
    
    parser.ParseTranslationUnit();
}
