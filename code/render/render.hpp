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
    
    void operator()( shared_ptr<Program> program )       
    {
        std::string s = RenderSequence( *program, ";\n", true );
        puts( s.c_str() ); // TODO allow a file to be specified in the constructor
    }

private:
    std::vector<std::string> operator_text;

    std::string RenderType( shared_ptr<Type> type )
    {
        if( dynamic_pointer_cast< Int >(type) )
            return "int";
        else if( dynamic_pointer_cast< Char >(type) )
            return "char";
        else if( dynamic_pointer_cast< Void >(type) )
            return "void";
        else
            return ERROR_UNSUPPORTED(type);
    }

    std::string RenderExpression( shared_ptr<Expression> expression, bool bracketize_operator=false )
    {
        TRACE("re %p\n", &*expression);
        
        std::string before = bracketize_operator ? "(" : "";
        std::string after = bracketize_operator ? ")" : "";
        
        if( shared_ptr<IdentifierExpression> ie = dynamic_pointer_cast< IdentifierExpression >(expression) )
            return (*ie->identifier);
        else if( shared_ptr<NumericConstant> nc = dynamic_pointer_cast< NumericConstant >(expression) )
            return (nc->toString(10, true));
        else if( shared_ptr<Infix> o = dynamic_pointer_cast< Infix >(expression) )
            return before + 
                   RenderExpression( o->operands[0], true ) +
                   operator_text[o->kind] + 
                   RenderExpression( o->operands[1], true ) +
                   after;
        else if( shared_ptr<Postfix> o = dynamic_pointer_cast< Postfix >(expression) )
            return before + 
                   RenderExpression( o->operands[0], true ) + 
                   operator_text[o->kind] +
                   after;
        else if( shared_ptr<Prefix> o = dynamic_pointer_cast< Prefix >(expression) )
            return before + 
                   operator_text[o->kind] + 
                   RenderExpression( o->operands[0], true ) +
                   after;
        else if( shared_ptr<ConditionalOperator> o = dynamic_pointer_cast< ConditionalOperator >(expression) )
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
    std::string RenderSequence( Sequence<ELEMENT> spe, std::string separator, bool seperate_last )
    {
        std::string s;
        for( int i=0; i<spe.size(); i++ )
        {
            std::string sep = (seperate_last || i+1<spe.size()) ? separator : "";
            shared_ptr<ELEMENT> pe = spe[i];            
            if( shared_ptr<VariableDeclarator> vd = dynamic_pointer_cast< VariableDeclarator >(pe) )
                s += RenderType(vd->type) + " " + 
                     (*vd->identifier) + sep;
            else if( shared_ptr<FunctionDeclarator> fd = dynamic_pointer_cast< FunctionDeclarator >(pe) )
                s += RenderType(fd->return_type) + " " + 
                     (*fd->identifier) + "(" + 
                     RenderSequence(fd->parameters, ", ", false) + ")\n{\n" + 
                     RenderSequence(*(fd->body), ";\n", true) + "}\n";
            else if( shared_ptr<ExpressionStatement> es = dynamic_pointer_cast< ExpressionStatement >(pe) )
                s += RenderExpression(es->expression) + sep;
            else if( shared_ptr<Return> es = dynamic_pointer_cast<Return>(pe) )
                s += "return " + RenderExpression(es->return_value) + sep;
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
};

#endif
