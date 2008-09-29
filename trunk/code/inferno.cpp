
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

RCPtr<Program> program;
RCPtr< Sequence<ProgramElement> > curseq;

bool isf=false;

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
            default:
                assert(0);
                break;
        }
    }
    
    Identifier *ConvertIdentifier( const clang::IdentifierInfo *ID )
    {
        Identifier *i = new Identifier();
        i->assign( ID->getName() );
        return i;
    }
    
    virtual DeclTy *ActOnDeclarator( clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup )
    {
        RCPtr<VariableDeclarator> p = new VariableDeclarator;
        curseq->push_back(p);
        p->storage_class = VariableDeclarator::STATIC;
        p->type = ConvertType( D.getDeclSpec() );
        p->identifier = ConvertIdentifier( D.getIdentifier() );        
        //printf("cs %p\n", &*curseq );
        //printf("id %s %p %p\n", p->identifier->c_str(), &*p->identifier, &*p );
        return MinimalAction::ActOnDeclarator( S, D, LastInGroup );     
    }
    
    virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
    {
        RCPtr<FunctionDeclarator> p = new FunctionDeclarator;
        curseq->push_back(p);
        p->return_type = ConvertType( D.getDeclSpec() );
        p->body = new Sequence<ProgramElement>;
        p->identifier = ConvertIdentifier( D.getIdentifier() );  

        curseq = p->body;
        return MinimalAction::ActOnDeclarator( FnBodyScope, D, NULL );     
    }
    
    virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
    {
        curseq = program;
        return MinimalAction::ActOnFinishFunctionBody( Decl, Body );
    }

};


std::string RenderType( RCPtr<const Type> type )
{
    if( DynamicCast< const Int >(type) )
        return "int";
    else if( DynamicCast< const Char >(type) )
        return "char";
}

std::string Render( RCPtr< Sequence<ProgramElement> > program )
{
    std::string s;
    for( int i=0; i<program->size(); i++ )
    {
        RCPtr<ProgramElement> pe = (*program)[i];
        if( RCPtr<VariableDeclarator> vd = DynamicCast< VariableDeclarator >(pe) )
        {
            //printf("id %s %p %p\n", vd->identifier->c_str(), &*vd->identifier, &*vd );
            s += RenderType(vd->type)+" "+(*vd->identifier)+";\n";
        }
        else if( RCPtr<FunctionDeclarator> fd = DynamicCast< FunctionDeclarator >(pe) )
        {
            s += RenderType(fd->return_type)+" "+(*fd->identifier)+"()\n{\n" + Render(fd->body) + "}\n";
        }
    }
    return s;
}

int main()
{
    RCTarget::Start();

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
    
    program = new Program();  
    curseq = program;
    parser.ParseTranslationUnit();
    assert( &*curseq==&*program );
    
    std::string ss = Render( program );
    printf( "%s", ss.c_str() );   
        
    RCTarget::Finished();
}
