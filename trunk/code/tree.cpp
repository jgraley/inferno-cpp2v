
#include "tree.hpp"

#define DO_KIND( CLASS ) const char *CLASS::kind = #CLASS;

DO_KIND( BaseNode )
DO_KIND( Declarator )
DO_KIND( VariableDeclarator )
DO_KIND( FuncitonDeclarator )
DO_KIND( Identifier )
DO_KIND( Type )
DO_KIND( Int )



