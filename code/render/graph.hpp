#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"


class Graph : public Pass
{
public:
    Graph()
    {        
        operator_text.resize( clang::tok::NUM_TOKENS );
        for(int i=0; i<clang::tok::NUM_TOKENS; i++ )
            operator_text[i] = "\n#error Token not defined in operator_text\n";
#define OPERATOR(TOK, TEXT) operator_text[clang::tok::TOK] = TEXT;
#include "operator_text.inc"
    }
    
    void operator()( shared_ptr<Program> program )       
    {
        string s;
        s += "digraph g {\n"; // g is name of graph
        s += "graph [\n";
        s += "rankdir = \"LR\"\n"; // left-to-right looks more like source code
        s += "];\n";
        s += "node [\n";
        s += "fontsize = \"16\"\n";
        s += "shape = \"ellipse\"\n"; // TODO I think this is overridden in all the nodes
        s += "];\n";
        s += "edge [\n";
        s += "];\n";
        //s += Visit( *program );
        s += "}\n"
        if( ReadArgs::outfile.empty() )
        {
            puts( s.c_str() );
        }
        else
        {
            FILE *fp = fopen( ReadArgs::outfile.c_str(), "wt" );
            ASSERT( fp && "Cannot open output file" );
            fputs( s.c_str(), fp );
            fclose( fp );
        }    
    }

private:
    vector<string> operator_text;
    
    enum
    {
        POINTER,
        STRING,
        NUMBER,
        END
    };
    
    string MakeNode( Node *p, ... )
    {
        va_list vl;
        va_start( vl, fmt );
        fprintf(stderr, "%s:%d in %s()\n    ", file, line, function);
        vprintf( fmt, vl );
        va_end( vl );
    }
    
    
#if 0    
    string Visit( shared_ptr<Identifier> id )
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
    
    string Visit( shared_ptr<Integral> type )
    {
        bool ds;
        unsigned width;       
        shared_ptr<IntegralConstant> ic = dynamic_pointer_cast<IntegralConstant>( type->width );
        ASSERT(ic && "width must be constant integral"); 
        if(ic)
            width = ic->value.getLimitedValue();
                  
        TRACE("width %d\n", width);          
                          
        if( width == TypeInfo::char_bits )
            ds = TypeInfo::char_default_signed;
        else
            ds = TypeInfo::int_default_signed;
        
        // Produce signed or unsigned if required
        // Note: literal strings can be converted to char * but not unsigned char * or signed char * 
        string s;
        if( dynamic_pointer_cast< Signed >(type) && !ds )
            s = "signed ";
        else if( dynamic_pointer_cast< Unsigned >(type) && ds )
            s = "unsigned ";

        // Fix the width
        bool bitfield = false;
        if( width == TypeInfo::char_bits )
            s += "char";
        else if( width == TypeInfo::integral_bits[clang::DeclSpec::TSW_unspecified] )
            s += "int";
        else if( width == TypeInfo::integral_bits[clang::DeclSpec::TSW_short] )
            s += "short";
        else if( width == TypeInfo::integral_bits[clang::DeclSpec::TSW_long] )
            s += "long";
        else if( width == TypeInfo::integral_bits[clang::DeclSpec::TSW_longlong] )
            s += "long long";
        else    // unmatched defaults to int for bitfields
        {
            s += "int";
            bitfield = true;
        }
               
        s += " " + object;
        
        if( bitfield )
        {
           char b[100];
           sprintf(b, ":%d", width);
           s += b;
        }
        
        return s;              
    }

    string Visit( shared_ptr<Floating> type )
    {
        string s;
        unsigned base_width;       
        shared_ptr<IntegralConstant> ic = dynamic_pointer_cast<IntegralConstant>( type->width );
        ASSERT(ic && "width must be constant integral"); 
        if(ic)
            base_width = ic->value.getLimitedValue();
    
        // Fix the width
        if( base_width == TypeInfo::float_bits )
            s += "float";
        else if( base_width == TypeInfo::double_bits )
            s += "double";
        else if( base_width == TypeInfo::long_double_bits )
            s += "long double";
        else    
            ASSERT( !"no builtin floating type has required bit width"); // TODO drop in a bit vector
        
        return s;              
    }

    string Visit( shared_ptr<Type> type )
    {
        TRACE();
        if( shared_ptr<Integral> i = dynamic_pointer_cast< Integral >(type) )
            return Visit( i, object );
        if( shared_ptr<Floating> f = dynamic_pointer_cast< Floating >(type) )
            return Visit( f ) + " " + object;
        else if( dynamic_pointer_cast< Void >(type) )
            return "void " + object;
        else if( dynamic_pointer_cast< Bool >(type) )
            return "bool " + object;
        else if( shared_ptr<Function> f = dynamic_pointer_cast< Function >(type) )
            return Visit( f->return_type, "(" + object + ")(" + Visit(f->parameters, ", ", false) + ")" );
        else if( shared_ptr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
            return Visit( p->destination, "(*" + object + ")" );
        else if( shared_ptr<Reference> r = dynamic_pointer_cast< Reference >(type) )
            return Visit( r->destination, "(&" + object + ")" );
        else if( shared_ptr<Array> a = dynamic_pointer_cast< Array >(type) )
            return Visit( a->element, "(" + object + "[" + Visit(a->size) + "])" );
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(type) )
            return t->identifier + " " + object;
#if 0 // Should we include the word "class" etc when referencing holders as types?
       else if( shared_ptr<Struct> ss = dynamic_pointer_cast< Struct >(type) )
            return "struct " + ss->identifier + " " + object;
        else if( shared_ptr<Class> c = dynamic_pointer_cast< Class >(type) )
            return "class " + c->identifier + " " + object;
        else if( shared_ptr<Union> u = dynamic_pointer_cast< Union >(type) )
            return "union " + u->identifier + " " + object;
        else if( shared_ptr<Enum> e = dynamic_pointer_cast< Enum >(type) )
            return "enum " + e->identifier + " " + object;
#else
        else if( shared_ptr<UserType> ut = dynamic_pointer_cast< UserType >(type) )
            return ut->identifier + " " + object;
#endif
        else
            return ERROR_UNSUPPORTED(type);
    }
    
    string Visit( shared_ptr<Expression> expression )
    {
        TRACE("%p\n", expression.get());
        
        string before = bracketize_operator ? "(" : "";
        string after = bracketize_operator ? ")" : "";
        
        if( !expression )
            return string();            
        else if( shared_ptr<Label> l = dynamic_pointer_cast< Label >(expression) )
            return before + 
                   "&&" + Visit( l ) + // label-as-variable (GCC extension)
                   after;
        else if( shared_ptr<Object> v = dynamic_pointer_cast< Object >(expression) )
            return Visit( v );
        else if( shared_ptr<IntegralConstant> ic = dynamic_pointer_cast< IntegralConstant >(expression) )
            return string(ic->value.toString(10)) + 
                   (ic->value.isUnsigned() ? "U" : "") + 
                   (ic->value.getBitWidth()>TypeInfo::integral_bits[clang::DeclSpec::TSW_unspecified] ? "L" : "") +
                   (ic->value.getBitWidth()>TypeInfo::integral_bits[clang::DeclSpec::TSW_long] ? "L" : ""); 
                   // note, assuming longlong bigger than long, so second L appends first to get LL
        else if( shared_ptr<FloatingConstant> fc = dynamic_pointer_cast< FloatingConstant >(expression) )
        {
            char hs[256];
            // generate hex float since it can be exact
            fc->value.convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
            return string(hs) + 
                   (&(fc->value.getSemantics())==TypeInfo::floating_semantics[clang::DeclSpec::TSW_short] ? "F" : "") +
                   (&(fc->value.getSemantics())==TypeInfo::floating_semantics[clang::DeclSpec::TSW_long] ? "L" : ""); 
        }           
        else if( shared_ptr<Infix> o = dynamic_pointer_cast< Infix >(expression) )
            return before + 
                   Visit( o->operands[0], true ) +
                   operator_text[o->kind] + 
                   Visit( o->operands[1], true ) +
                   after;
        else if( shared_ptr<Postfix> o = dynamic_pointer_cast< Postfix >(expression) )
            return before + 
                   Visit( o->operands[0], true ) + 
                   operator_text[o->kind] +
                   after;
        else if( shared_ptr<Prefix> o = dynamic_pointer_cast< Prefix >(expression) )
            return before + 
                   operator_text[o->kind] + 
                   Visit( o->operands[0], true ) +
                   (operator_text[o->kind][operator_text[o->kind].size()-1]=='(' ? ")" : "") + // if the token is "x(", add ")"
                   after;
        else if( shared_ptr<PrefixOnType> pot = dynamic_pointer_cast< PrefixOnType >(expression) )
            return before + 
                   operator_text[pot->kind] + 
                   Visit( pot->operand, "" ) +
                   (operator_text[pot->kind][operator_text[pot->kind].size()-1]=='(' ? ")" : "") + // if the token is "x(", add ")"
                   after;
        else if( shared_ptr<ConditionalOperator> o = dynamic_pointer_cast< ConditionalOperator >(expression) )
            return before + 
                   Visit( o->condition, true ) + "?" +
                   Visit( o->if_true, true ) + ":" +
                   Visit( o->if_false, true ) +
                   after;
        else if( shared_ptr<Call> o = dynamic_pointer_cast< Call >(expression) )
            return before + 
                   Visit( o->function, true ) + "(" +
                   Visit( o->arguments, ", ", false ) + ")" +
                   after;
        else if( shared_ptr<Subscript> su = dynamic_pointer_cast< Subscript >(expression) )
            return before + 
                   Visit( su->base, true ) + "[" +
                   Visit( su->index, false ) + "]" +
                   after;
        else if( shared_ptr<Access> a = dynamic_pointer_cast< Access >(expression) )
            return before + 
                   Visit( a->base, true ) + "." +
                   Visit( a->member ) +
                   after;
        else if( shared_ptr<Compound> c = dynamic_pointer_cast< Compound >(expression) )
            return before + 
                   "{\n" + 
                   Visit(c->statements, ";\n", true) +
                   "}\n" +
                   after;
        else if( shared_ptr<Cast> c = dynamic_pointer_cast< Cast >(expression) )
            return before + 
                   "(" + Visit( c->type, "" ) + ")" +
                   Visit( c->operand, false ) + 
                   after;
        else if( shared_ptr<Aggregate> ao = dynamic_pointer_cast< Aggregate >(expression) )
            return before + 
                   "{ " + Visit( ao->elements, ", ", false ) + " }" +
                   after;
        else if( shared_ptr<String> ss = dynamic_pointer_cast< String >(expression) )
            return before + 
                   "\"" + Sanitise( ss->value ) + "\"" + 
                   after;
        else if( dynamic_pointer_cast< This >(expression) )
            return before + 
                   "this" + 
                   after;
        else
            return ERROR_UNSUPPORTED(expression);
        TRACE("ok\n");
    }
    

    
    string Visit( shared_ptr<Declaration> declaration )
    {
        TRACE();
        string s;
        
        if( access && declaration->access != *access )
        {
            s += VisitAccess( declaration->access ) + ":\n";
            *access = declaration->access;
        }
         
        if( declaration->is_virtual )
            s+= "virtual "; 
                                
        if( shared_ptr<ObjectDeclaration> od = dynamic_pointer_cast< ObjectDeclaration >(declaration) )
        {                
            switch( od->object->storage )
            {
            case Object::STATIC:
                s += "static ";
                break;
            case Object::EXTERN:
                s += "extern ";
                break;
            case Object::AUTO:
                s += "auto ";
                break;
            case Object::DEFAULT:
            case Object::MEMBER:
            case Object::SYMBOL:
                break;
            default:
                s += ERROR_UNKNOWN("storage class");
                break;
            }
            
            if(showtype)
                s += Visit( od->object->type, Visit(od->object) );
            else
                s += Visit(od->object);
                
            if(od->initialiser)
            {
                bool function_definition = !!dynamic_pointer_cast<Compound>(od->initialiser);
                if( function_definition )
                    s += "\n";
                else
                    s += " = ";
                s += Visit(od->initialiser);
                if( !function_definition )
                    s += sep;
            }
            else
            {
                s += sep;
            }    
        }
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(declaration) )
            s += "typedef " + Visit( t->type, t->identifier ) + sep;
        else if( shared_ptr<Record> h = dynamic_pointer_cast< Record >(declaration) )
        {
            Declaration::Access a;
            bool showtype=true;
            string sep2=";\n";
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
            else if( dynamic_pointer_cast< Enum >(h) )
            {
                s += "enum";
                a = Declaration::PUBLIC;
                sep2 = ",\n";
                showtype = false;
            }
            else
                return ERROR_UNSUPPORTED(declaration);

            s += " " + h->identifier;
            
            if( shared_ptr<InheritanceHolder> ih = dynamic_pointer_cast< InheritanceHolder >(declaration) )
            {
                if( !ih->bases.empty() )
                {
                    s += " : ";
                    for( int i=0; i<ih->bases.size(); i++ )
                    {   
                        if( i>0 )
                            s += ", ";
                        shared_ptr<InheritanceHolder> bih = dynamic_pointer_cast< InheritanceHolder >(ih->bases[i]);    
                        ASSERT( bih );
                        s += VisitAccess( bih->access ) + " " + bih->identifier;
                    }
                }
            }
            
            if( !h->incomplete )
            {
                 s += "\n{\n" +
                      Visit( h->members, sep2, true, a, showtype ) +
                      "}";
            }
            
            s += ";\n\n";
        }
        else
            s += ERROR_UNSUPPORTED(declaration);
            
            TRACE();
        return s;    
    }

    string Visit( shared_ptr<Statement> statement )
    {
        TRACE();
        if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(statement) )
            return Visit( d );
        else if( shared_ptr<Compound> c = dynamic_pointer_cast< Compound >(statement) ) // Never put ; after a scope - you'd get {blah};
            return Visit(c);
        else if( shared_ptr<Expression> e = dynamic_pointer_cast< Expression >(statement) )
            return Visit(e);
        else if( shared_ptr<Return> es = dynamic_pointer_cast<Return>(statement) )
            return "return " + Visit(es->return_value);
        else if( shared_ptr<LabelTarget> l = dynamic_pointer_cast<LabelTarget>(statement) )
            return Visit(l->label) + ":\n"; // no ; after a label
        else if( shared_ptr<Goto> g = dynamic_pointer_cast<Goto>(statement) )
        {
            if( shared_ptr<Label> l = dynamic_pointer_cast< Label >(g->destination) )
                return "goto " + Visit(l) + sep;  // regular goto
            else
                return "goto *" + Visit(g->destination) + sep; // goto-a-variable (GCC extension)
        }
        else if( shared_ptr<If> i = dynamic_pointer_cast<If>(statement) )
        {
            string s;
            s += "if( " + Visit(i->condition) + " )\n"
                 "{\n" + // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
                 Visit(i->body, ";\n") +
                 "}\n";
            if( i->else_body )  // else is optional
                s += "else\n" +
                     Visit(i->else_body, ";\n");
            return s;
        } 
        else if( shared_ptr<While> w = dynamic_pointer_cast<While>(statement) )
            return "while( " + Visit(w->condition) + " )\n" +
                   Visit(w->body, ";\n");
        else if( shared_ptr<Do> d = dynamic_pointer_cast<Do>(statement) )
            return "do\n" +
                   Visit(d->body, ";\n") +
                   "while( " + Visit(d->condition) + " )" + sep;
        else if( shared_ptr<For> f = dynamic_pointer_cast<For>(statement) )
            return "for( " + Visit(f->initialisation, "") + "; " + Visit(f->condition) + "; "+ Visit(f->increment, "") + " )\n" +
                   Visit(f->body, ";\n");
        else if( shared_ptr<Switch> s = dynamic_pointer_cast<Switch>(statement) )
            return "switch( " + Visit(s->condition) + " )\n" +
                   Visit(s->body, ";\n");
        else if( shared_ptr<Case> c = dynamic_pointer_cast<Case>(statement) )
            return "case " + Visit(c->value_lo) + " ... " + Visit(c->value_hi) + ":\n";
        else if( dynamic_pointer_cast<Default>(statement) )
            return "default:\n";
        else if( dynamic_pointer_cast<Continue>(statement) )
            return "continue" + sep;
        else if( dynamic_pointer_cast<Break>(statement) )
            return "break" + sep;
        else if( dynamic_pointer_cast<Nop>(statement) )
            return sep;
        else
            return ERROR_UNSUPPORTED(statement);
    }
    
    template< class ELEMENT >
    string Visit( Sequence<ELEMENT> spe )
    {
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            shared_ptr<ELEMENT> pe = spe[i];                        
            if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(pe) )
                s += Visit( d );
            else if( shared_ptr<Statement> st = dynamic_pointer_cast< Statement >(pe) )
                s += Visit( st ); 
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
#endif
};

#endif
