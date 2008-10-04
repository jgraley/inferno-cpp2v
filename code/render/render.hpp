#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"
#include "common/trace.hpp"

// TODO indent back to previous level at end of string
#define ERROR_UNSUPPORTED(P) \
    std::string( "\n#error " ) + \
    std::string( typeid(*P).name() ) + \
    std::string( " not supported in "__FILE__"\n" );

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
        if( RCPtr< Int >::Specialise(type) )
            return "int";
        else if( RCPtr< Char >::Specialise(type) )
            return "char";
        else if( RCPtr< Void >::Specialise(type) )
            return "void";
        else
            return ERROR_UNSUPPORTED(type);
    }

    std::string RenderExpression( RCPtr<Expression> expression )
    {
        TRACE("re %p\n", expression.ptr);
        if( RCPtr<IdentifierExpression> ie = RCPtr< IdentifierExpression >::Specialise(expression) )
        {
            TRACE("%p\n", &*ie->identifier);
            return (*ie->identifier);
        }
        else if( RCPtr<NumericConstant> nc = RCPtr< NumericConstant >::Specialise(expression) )
        {
           // TRACE("%p\n", &*ie->identifier);
            return (nc->toString(10, true));
        }
        else
            return ERROR_UNSUPPORTED(expression);
        TRACE("ok\n");
    }
    
    std::string RenderSPE( RCPtr< Sequence<ProgramElement> > spe )
    {
        std::string s;
        for( int i=0; i<spe->size(); i++ )
        {
            RCPtr<ProgramElement> pe = (*spe)[i];
            if( RCPtr<VariableDeclarator> vd = RCPtr< VariableDeclarator >::Specialise(pe) )
                s += RenderType(vd->type) + " " + (*vd->identifier) + ";\n";
            else if( RCPtr<FunctionDeclarator> fd = RCPtr< FunctionDeclarator >::Specialise(pe) )
                s += RenderType(fd->return_type) + " " + (*fd->identifier) + "()\n{\n" + RenderSPE(fd->body) + "}\n";
            else if( RCPtr<ExpressionStatement> es = RCPtr< ExpressionStatement >::Specialise(pe) )
                s += RenderExpression(es->expression) + ";\n";
            else if( RCPtr<Return> es = RCPtr<Return>::Specialise(pe) )
                s += "return " + RenderExpression(es->return_value) + ";\n";
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
};

#endif
