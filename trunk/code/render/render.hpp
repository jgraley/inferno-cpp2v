#ifndef RENDER_HPP
#define RENDER_HPP

// TODO indent back to previous level at end of string
#define ERROR_UNSUPPORTED(P) \
    std::string( "\n#error " ) + \
    std::string( typeid(*P).name() ) + \
    std::string( " not supported in "__FILE__"\n" );


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
        else if( DynamicCast< Void >(type) )
            return "void";
        else
            return ERROR_UNSUPPORTED(type);
    }

    std::string RenderExpression( RCPtr<Expression> expression )
    {
        printf("re %p\n", expression.ptr);
        if( RCPtr<IdentifierExpression> ie = DynamicCast< IdentifierExpression >(expression) )
        {
            printf("%p\n", &*ie->identifier);
            return (*ie->identifier);
        }
        else
            return ERROR_UNSUPPORTED(expression);
        printf("ok\n");
    }
    
    std::string RenderSPE( RCPtr< Sequence<ProgramElement> > spe )
    {
        std::string s;
        for( int i=0; i<spe->size(); i++ )
        {
            RCPtr<ProgramElement> pe = (*spe)[i];
            if( RCPtr<VariableDeclarator> vd = DynamicCast< VariableDeclarator >(pe) )
                s += RenderType(vd->type)+" "+(*vd->identifier)+";\n";
            else if( RCPtr<FunctionDeclarator> fd = DynamicCast< FunctionDeclarator >(pe) )
                s += RenderType(fd->return_type)+" "+(*fd->identifier)+"()\n{\n" + RenderSPE(fd->body) + "}\n";
            else if( RCPtr<ExpressionStatement> es = DynamicCast< ExpressionStatement >(pe) )
                s += RenderExpression(es->expression) + ";\n";
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
};

#endif
