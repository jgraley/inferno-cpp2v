#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"

class Render : public Pass
{
public:
    void operator()( RCPtr<Program> program )       
    {
        std::string s = RenderSPE( program );
        puts( s.c_str() ); // TODO allow a file to be specified in the constructor
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

#endif
