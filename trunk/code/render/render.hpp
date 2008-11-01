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
    string( " not supported in " ) + \
    string( BOOST_CURRENT_FUNCTION ) + \
    string( "\n" );

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
        if( id )
        {
            ids = id->identifier;
            TRACE( "%s\n", ids.c_str() );
        }
        else
        {
            TRACE();
        }
        return ids;
    }

    string RenderType( shared_ptr<Type> type, string object=string() )
    {
        TRACE();
        if( dynamic_pointer_cast< Int >(type) )
            return "int " + object;
        else if( dynamic_pointer_cast< Char >(type) )
            return "char " + object;
        else if( dynamic_pointer_cast< Void >(type) )
            return "void " + object;
        else if( shared_ptr<Function> f = dynamic_pointer_cast< Function >(type) )
            return RenderType( f->return_type, "(" + object + ")(" + RenderSequence(f->parameters, ", ", false) + ")" );
        else if( shared_ptr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
            return RenderType( p->destination, "(*" + object + ")" );
        else if( shared_ptr<Reference> r = dynamic_pointer_cast< Reference >(type) )
            return RenderType( r->destination, "(&" + object + ")" );
        else if( shared_ptr<Array> a = dynamic_pointer_cast< Array >(type) )
            return RenderType( a->element, "(" + object + "[" + RenderExpression(a->size) + "])" );
        else if( shared_ptr<UserType> ut = dynamic_pointer_cast< UserType >(type) )
            return ut->identifier + " " + object;
        else
            return ERROR_UNSUPPORTED(type);
    }

    string RenderExpression( shared_ptr<Expression> expression, bool bracketize_operator=false )
    {
        TRACE("%p\n", expression.get());
        
        string before = bracketize_operator ? "(" : "";
        string after = bracketize_operator ? ")" : "";
        
        if( !expression )
            return string();            
        else if( shared_ptr<Label> l = dynamic_pointer_cast< Label >(expression) )
            return before + 
                   "&&" + RenderIdentifier( l ) + // label-as-variable (GCC extension)
                   after;
        else if( shared_ptr<Object> v = dynamic_pointer_cast< Object >(expression) )
            return RenderIdentifier( v );
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
        else if( shared_ptr<Subscript> su = dynamic_pointer_cast< Subscript >(expression) )
            return before + 
                   RenderExpression( su->base, true ) + "[" +
                   RenderExpression( su->index, false ) + "]" +
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
    
    string RenderDeclaration( shared_ptr<Declaration> declaration, string sep, Declaration::Access *access = NULL )
    {
        TRACE();
        string s;
        
        if( declaration->access != *access )
        {
            switch( declaration->access )
            {
                case Declaration::PUBLIC:
                    s += "public:\n";
                    break;
                case Declaration::PRIVATE:
                    s += "private:\n";
                    break;
                case Declaration::PROTECTED:
                    s += "protected:\n";
                    break;
                default:
                    ASSERT(0);
                    break;    
            }
            *access = declaration->access;
        }
        
        /*char hhh[2];
        hhh[0]='A'+(char)(declaration->access);
        hhh[1]=0;
        s += hhh;*/
        
        if( shared_ptr<ObjectDeclaration> od = dynamic_pointer_cast< ObjectDeclaration >(declaration) )
        {
            //TODO storage class, access 
            s += RenderType( od->object->type, RenderIdentifier(od->object) );
            if(od->initialiser)
            {
                bool function_definition = !!dynamic_pointer_cast<Scope>(od->initialiser);
                if( function_definition )
                    s += "\n";
                else
                    s += " = ";
                s += RenderExpression(od->initialiser);
                if( !function_definition )
                    s += sep;
            }
            else
            {
                s += sep;
            }    
            return s;
        }
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(declaration) )
            s += "typedef " + RenderType( t->type, t->identifier ) + sep;
        else if( shared_ptr<Holder> h = dynamic_pointer_cast< Holder >(declaration) )
        {
            Declaration::Access a;
            if( dynamic_pointer_cast< Class >(h) )
            {
                s += "class";
                a = Declaration::PRIVATE;
            }
            else if( dynamic_pointer_cast< Struct >(h) )
            {
                s += "struct";
                a = Declaration::PUBLIC;
            }
            else if( dynamic_pointer_cast< Union >(h) )
            {
                s += "union";
                a = Declaration::PUBLIC;
            }
            else
                return ERROR_UNSUPPORTED(declaration);

            s += " " + h->identifier + "\n" 
                 "{\n" +
                 RenderSequence( h->members, ";\n", true, a ) +
                 "};\n";
        }
        else
            s += ERROR_UNSUPPORTED(declaration);
        return s;    
    }

    string RenderStatement( shared_ptr<Statement> statement, string sep )
    {
        TRACE();
        if( !statement )
            return sep;            
        else if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(statement) )
            return RenderDeclaration( d, sep );
        else if( shared_ptr<Scope> sc = dynamic_pointer_cast< Scope >(statement) ) // Never put ; after a scope - you'd get {blah};
            return RenderExpression(shared_ptr<Expression>(sc));
        else if( shared_ptr<Expression> e = dynamic_pointer_cast< Expression >(statement) )
            return RenderExpression(e) + sep;
        else if( shared_ptr<Return> es = dynamic_pointer_cast<Return>(statement) )
            return "return " + RenderExpression(es->return_value) + sep;
        else if( shared_ptr<LabelMarker> l = dynamic_pointer_cast<LabelMarker>(statement) )
            return RenderIdentifier(l->label) + ":\n"; // no ; after a label
        else if( shared_ptr<Goto> g = dynamic_pointer_cast<Goto>(statement) )
        {
            if( shared_ptr<Label> l = dynamic_pointer_cast< Label >(g->destination) )
                return "goto " + RenderIdentifier(l) + sep;  // regular goto
            else
                return "goto " + RenderExpression(g->destination) + sep; // goto-a-variable (GCC extension)
        }
        else if( shared_ptr<If> i = dynamic_pointer_cast<If>(statement) )
        {
            string s;
            s += "if( " + RenderExpression(i->condition) + " )\n"
                 "{\n" + // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
                 RenderStatement(i->body, ";\n") +
                 "}\n";
            if( i->else_body )  // else is optional
                s += "else\n" +
                     RenderStatement(i->else_body, ";\n");
            return s;
        } 
        else if( shared_ptr<While> w = dynamic_pointer_cast<While>(statement) )
            return "while( " + RenderExpression(w->condition) + " )\n" +
                   RenderStatement(w->body, ";\n");
        else if( shared_ptr<Do> d = dynamic_pointer_cast<Do>(statement) )
            return "do\n" +
                   RenderStatement(d->body, ";\n") +
                   "while( " + RenderExpression(d->condition) + " )" + sep;
        else if( shared_ptr<For> f = dynamic_pointer_cast<For>(statement) )
            return "for( " + RenderStatement(f->initialisation, "") + "; " + RenderExpression(f->condition) + "; "+ RenderStatement(f->increment, "") + " )\n" +
                   RenderStatement(f->body, ";\n");
        else if( shared_ptr<Switch> s = dynamic_pointer_cast<Switch>(statement) )
            return "switch( " + RenderExpression(s->condition) + " )\n" +
                   RenderStatement(s->body, ";\n");
        else if( shared_ptr<Case> c = dynamic_pointer_cast<Case>(statement) )
            return "case " + RenderExpression(c->value) + ":\n";
        else if( shared_ptr<Default> d = dynamic_pointer_cast<Default>(statement) )
            return "default:\n";
        else if( shared_ptr<Continue> d = dynamic_pointer_cast<Continue>(statement) )
            return "continue" + sep;
        else if( shared_ptr<Break> d = dynamic_pointer_cast<Break>(statement) )
            return "break" + sep;
        else
            return ERROR_UNSUPPORTED(statement);
    }
    
    template< class ELEMENT >
    string RenderSequence( Sequence<ELEMENT> spe, 
                           string separator, 
                           bool seperate_last, 
                           Declaration::Access init_access = Declaration::PRIVATE )
    {
        TRACE();
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            string sep = (seperate_last || i+1<spe.size()) ? separator : "";
            shared_ptr<ELEMENT> pe = spe[i];                        
            if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(pe) )
                s += RenderDeclaration( d, sep, &init_access );
            else if( shared_ptr<Statement> st = dynamic_pointer_cast< Statement >(pe) )
                s += RenderStatement( st, sep ); 
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
};

#endif
