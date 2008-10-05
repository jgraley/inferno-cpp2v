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
    Render()
    {
        operator_text.resize( clang::tok::NUM_TOKENS );
#define OPERATOR(TOK, TEXT) operator_text[clang::tok::TOK] = TEXT;
#include "operator_text.inc"
    }
    
    void operator()( RCPtr<Program> program )       
    {
        std::string s = RenderVector( *program, ";\n", true );
        puts( s.c_str() ); // TODO allow a file to be specified in the constructor
    }

private:
    std::vector<std::string> operator_text;

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

    std::string RenderExpression( RCPtr<Expression> expression, bool bracketize_operator=false )
    {
        TRACE("re %p\n", expression.ptr);
        
        std::string before = bracketize_operator ? "(" : "";
        std::string after = bracketize_operator ? ")" : "";
        
        if( RCPtr<IdentifierExpression> ie = RCPtr< IdentifierExpression >::Specialise(expression) )
            return (*ie->identifier);
        else if( RCPtr<NumericConstant> nc = RCPtr< NumericConstant >::Specialise(expression) )
            return (nc->toString(10, true));
        else if( RCPtr<Infix> o = RCPtr< Infix >::Specialise(expression) )
            return before + 
                   RenderExpression( o->operands[0], true ) +
                   operator_text[o->kind] + 
                   RenderExpression( o->operands[1], true ) +
                   after;
        else if( RCPtr<Postfix> o = RCPtr< Postfix >::Specialise(expression) )
            return before + 
                   RenderExpression( o->operands[0], true ) + 
                   operator_text[o->kind] +
                   after;
        else if( RCPtr<Prefix> o = RCPtr< Prefix >::Specialise(expression) )
            return before + 
                   operator_text[o->kind] + 
                   RenderExpression( o->operands[0], true ) +
                   after;
        else if( RCPtr<ConditionalOperator> o = RCPtr< ConditionalOperator >::Specialise(expression) )
            return before + 
                   RenderExpression( o->condition, true ) + "?" +
                   RenderExpression( o->if_true, true ) + ":" +
                   RenderExpression( o->if_false, true ) +
                   after;
        else
            return ERROR_UNSUPPORTED(expression);
        TRACE("ok\n");
    }
    
    template< class ELEMENT >
    std::string RenderVector(  std::vector<ELEMENT> spe, std::string separator, bool seperate_last )
    {
        std::string s;
        for( int i=0; i<spe.size(); i++ )
        {
            std::string sep = (seperate_last || i+1<spe.size()) ? separator : "";
            RCPtr<ProgramElement> pe = spe[i];            
            if( RCPtr<VariableDeclarator> vd = RCPtr< VariableDeclarator >::Specialise(pe) )
                s += RenderType(vd->type) + " " + 
                     (*vd->identifier) + sep;
            else if( RCPtr<FunctionDeclarator> fd = RCPtr< FunctionDeclarator >::Specialise(pe) )
                s += RenderType(fd->return_type) + " " + 
                     (*fd->identifier) + "(" + 
                     RenderVector(fd->params, ", ", false) + ")\n{\n" + 
                     RenderVector(*(fd->body), ";\n", true) + "}\n";
            else if( RCPtr<ExpressionStatement> es = RCPtr< ExpressionStatement >::Specialise(pe) )
                s += RenderExpression(es->expression) + sep;
            else if( RCPtr<Return> es = RCPtr<Return>::Specialise(pe) )
                s += "return " + RenderExpression(es->return_value) + sep;
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
};

#endif
