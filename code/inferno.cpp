
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

#define INFERNO_TRIPLE "none-none-inferno" // Oh yes.

using namespace clang;

int main()
{
    FileManager fm; 

    Diagnostic diags; 
    LangOptions opts;
    TargetInfo* ptarget = TargetInfo::CreateTargetInfo(INFERNO_TRIPLE);
    SourceManager sm;
    HeaderSearch headers( fm );
    
    Preprocessor pp( diags, opts, *ptarget,
                     sm, headers );
    
    IdentifierTable it( opts );                 
    MinimalAction actions( it );                 
    
    Parser parser( pp, actions );
}
