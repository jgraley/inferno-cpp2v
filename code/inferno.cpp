
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

class Parse
{
public:
    void operator()( RCPtr<Program> program, std::string infile )
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
    class InfernoAction : public clang::MinimalAction
    {
    public:
        InfernoAction(RCPtr<Program> p, clang::IdentifierTable &IT) : 
            MinimalAction(IT),
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
            (void)MinimalAction::ActOnDeclarator( S, D, LastInGroup );     
            return 0;
        }
        
        /*
        Note: the default implementation of ActOnStartOfFunctionDef() appears in
        MinimalAction and can cause spurious ActOnDeclarator() calls if we always
        call through. Therefore we don't and instead just call explicitly implemented
        functions in MinimalAction where required.
        */
        virtual DeclTy *ActOnStartOfFunctionDef(clang::Scope *FnBodyScope, clang::Declarator &D) 
        {
            RCPtr<FunctionDeclarator> p = new FunctionDeclarator;
            curseq->push_back(p);
            p->return_type = ConvertType( D.getDeclSpec() );
            p->body = new Sequence<ProgramElement>;
            p->identifier = ConvertIdentifier( D.getIdentifier() );  
    
            curseq = p->body;
            return 0;     
        }
        
        virtual DeclTy *ActOnFinishFunctionBody(DeclTy *Decl, StmtTy *Body) 
        {
            curseq = program;
            return 0;
        }    
    };
};    
    


class Render
{
public:
    std::string operator()( RCPtr<Program> program )       
    {
        return RenderSPE( program );
    }

private:
    std::string RenderType( RCPtr<Type> type )
    {
        if( DynamicCast< Int >(type) )
            return "int";
        else if( DynamicCast< Char >(type) )
            return "char";
    }
    
    std::string RenderSPE( RCPtr< Sequence<ProgramElement> > spe )
    {
        std::string s;
        for( int i=0; i<spe->size(); i++ )
        {
            RCPtr<ProgramElement> pe = (*spe)[i];
            if( RCPtr<VariableDeclarator> vd = DynamicCast< VariableDeclarator >(pe) )
            {
                s += RenderType(vd->type)+" "+(*vd->identifier)+";\n";
            }
            else if( RCPtr<FunctionDeclarator> fd = DynamicCast< FunctionDeclarator >(pe) )
            {
                s += RenderType(fd->return_type)+" "+(*fd->identifier)+"()\n{\n" + RenderSPE(fd->body) + "}\n";
            }
        }
        return s;
    }
};

static std::string infile;
void read_args( int argc, char *argv[] )
{
    int i=1;
    while( i<argc )
    {
        if( argv[i]==std::string("-i") && argc>i+1 )
        {
            infile = argv[++i];
        }
        else 
        {
            fprintf(stderr, "Usage: inferno -i <infile>\n");
            exit(1);
        }
        i++;    
    }    
}


int main( int argc, char *argv[] )
{
    RCTarget::Start();

    read_args( argc, argv );

    RCPtr<Program> program = new Program();  

    Parse p;    
    p( program, infile );
    
    Render r;
    std::string ss = r( program );
    
    printf( "%s", ss.c_str() );   
        
    RCTarget::Finished();
}
