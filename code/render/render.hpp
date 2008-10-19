#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"
#include "common/trace.hpp"

// TODO indent back to previous level at end of string
#define ERROR_UNSUPPORTED(P) \
    string( "\n#error " ) + \
    string( typeid(*P).name() ) + \
    string( " not supported in "__FILE__"\n" );

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
        string s = RenderSequence( *program, ";\n", true );
        puts( s.c_str() ); // TODO allow a file to be specified in the constructor
    }

private:
    vector<string> operator_text;
    
    string RenderIdentifier( shared_ptr<Identifier> id )
    {
        string ids;
        TRACE();
        if( id )
            ids = *id;
        TRACE();
        return ids;
    }

    string RenderType( shared_ptr<Type> type, string object=string() )
    {
        if( dynamic_pointer_cast< Int >(type) )
            return "int " + object;
        else if( dynamic_pointer_cast< Char >(type) )
            return "char " + object;
        else if( dynamic_pointer_cast< Void >(type) )
            return "void " + object;
        else if( shared_ptr<FunctionPrototype> f = dynamic_pointer_cast< FunctionPrototype >(type) )
            return RenderType( f->return_type, "(" + object + ")(" + RenderSequence(f->parameters, ", ", false) + ")" );
        else if( shared_ptr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
            return RenderType( p->destination, "(*" + object + ")" );
        else if( shared_ptr<Reference> r = dynamic_pointer_cast< Reference >(type) )
            return RenderType( r->destination, "(&" + object + ")" );
        else
            return ERROR_UNSUPPORTED(type);
    }

    string RenderExpression( shared_ptr<Expression> expression, bool bracketize_operator=false )
    {
        TRACE("re %p\n", &*expression);
        
        string before = bracketize_operator ? "(" : "";
        string after = bracketize_operator ? ")" : "";
        
        if( shared_ptr<IdentifierExpression> ie = dynamic_pointer_cast< IdentifierExpression >(expression) )
            return RenderIdentifier( ie->identifier );
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
        else if( shared_ptr<Call> o = dynamic_pointer_cast< Call >(expression) )
            return before + 
                   RenderExpression( o->function, true ) + "(" +
                   RenderSequence( o->arguments, ", ", false ) + ")" +
                   after;
        else if( shared_ptr<Scope> o = dynamic_pointer_cast< Scope >(expression) )
            return before + 
                   "{\n" + 
                   RenderSequence(*o, ";\n", true) +
                   "}\n" +
                   after;
        else
            return ERROR_UNSUPPORTED(expression);
        TRACE("ok\n");
    }
    
    template< class ELEMENT >
    string RenderSequence( Sequence<ELEMENT> spe, string separator, bool seperate_last )
    {
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            string sep = (seperate_last || i+1<spe.size()) ? separator : "";
            shared_ptr<ELEMENT> pe = spe[i];            
            if( shared_ptr<VariableDeclarator> vd = dynamic_pointer_cast< VariableDeclarator >(pe) )
            {
                s += RenderType( vd->type, RenderIdentifier(vd->identifier) );
                if(vd->initialiser)
                    s += " = " + RenderExpression(vd->initialiser);
                s += sep;
            }
            else if( shared_ptr<FunctionDeclarator> fd = dynamic_pointer_cast< FunctionDeclarator >(pe) )
                s += RenderType( fd->type, RenderIdentifier( fd->identifier ) ) + "\n" + 
                     RenderExpression(fd->initialiser);
            else if( shared_ptr<Scope> sc = dynamic_pointer_cast< Scope >(pe) ) // Never put ; after a scope - you'd get {blah};
                s += RenderExpression(shared_ptr<Expression>(sc));
            else if( shared_ptr<Expression> e = dynamic_pointer_cast< Expression >(pe) )
                s += RenderExpression(e) + sep;
            else if( shared_ptr<Return> es = dynamic_pointer_cast<Return>(pe) )
                s += "return " + RenderExpression(es->return_value) + sep;
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
};

#endif
