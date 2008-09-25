
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

#define INFERNO_TRIPLE "arm-linux" 

using namespace clang;

/*
Note: the default implementation of ActOnStartOfFunctionDef() appears in
MinimalAction and can cause spurious ActOnDeclarator() calls if we always
call through. Therefore we don't and instead just call explicitly implemented
functions in MinimalAction where required.
*/
class InfernoAction : public MinimalAction
{
public:
    InfernoAction(IdentifierTable &IT) : MinimalAction(IT)
    {
    }
 
 private:   
    void PrintType( const DeclSpec &DS )
    {
        switch( DS.getTypeSpecType() )
        {
            case DeclSpec::TST_int:
                printf("int ");
                break;
            case DeclSpec::TST_char:
                printf("char ");
                break;
            default:
                printf("Type<%d> ", DS.getTypeSpecType() );
                break;
        }
    }
    
    virtual DeclTy *ActOnDeclarator(Scope *S, Declarator &D, DeclTy *LastInGroup)
    {
        PrintType( D.getDeclSpec() );
        printf("%s;\n", D.getIdentifier()->getName() );
        
        return MinimalAction::ActOnDeclarator( S, D, LastInGroup );     
    }
    
    virtual DeclTy *ActOnStartOfFunctionDef(Scope *FnBodyScope, Declarator &D) 
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
    FileManager fm; 
    TextDiagnosticPrinter diag_printer;
    Diagnostic diags( &diag_printer ); 
    LangOptions opts;
    TargetInfo* ptarget = TargetInfo::CreateTargetInfo(INFERNO_TRIPLE);
    assert(ptarget);
    SourceManager sm;
    HeaderSearch headers( fm );
    
    Preprocessor pp( diags, opts, *ptarget,
                     sm, headers );
    
    std::string infile="in.c";
    const FileEntry *file = fm.getFile(infile);
    if (file) 
        sm.createMainFileID(file, SourceLocation());
    if (sm.getMainFileID() == 0) 
    {
        fprintf(stderr, "Error reading '%s'!\n",infile.c_str());
        return true;
    }
    pp.EnterMainSourceFile();

    IdentifierTable it( opts );                 
    InfernoAction actions( it );                 
    
    Parser parser( pp, actions );
    
    parser.ParseTranslationUnit();
}
