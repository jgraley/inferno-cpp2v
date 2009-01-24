#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/pass.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "helpers/scope.hpp"

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    string( "\n#error " ) + \
    string( V ) + \
    string( " not supported in " ) + \
    string( INFERNO_CURRENT_FUNCTION ) + \
    string( "\n" );

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? typeid(*P).name() : "<NULL>" );


class Render : public Pass
{
public:
    Render()
    {        
        operator_text.resize( clang::tok::NUM_TOKENS );
        for(int i=0; i<clang::tok::NUM_TOKENS; i++ )
            operator_text[i] = "\n#error Token not defined in operator_text\n";
#define OPERATOR(TOK, TEXT) operator_text[clang::tok::TOK] = TEXT;
#include "operator_text.inc"
    }
    
    void operator()( shared_ptr<Program> prog )       
    {
        program = prog;
        AutoPush< shared_ptr<Node> > cs( scope_stack, program );
        string s = RenderSequence( *program, ";\n", true ); // gets the .hpp stuff directly
        s += deferred_decls; // these could go in a .cpp file
        
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
        program = shared_ptr<Program>();
    }

private:
    vector<string> operator_text;
    shared_ptr<Program> program;
    string deferred_decls;
    stack< shared_ptr<Node> > scope_stack;
    
    string RenderIdentifier( shared_ptr<Identifier> id )
    {
        // TODO detect Constructor/Destructor and force identifier to empty string if so
        string ids;
        if( id )
        {
            ids = id->name;
            TRACE( "%s\n", ids.c_str() );
        }
        else
        {
            TRACE();
        }
        return ids;
    }

    string RenderScope( shared_ptr<Identifier> id )
    {
        shared_ptr<Node> scope = GetScope( program, id );
        TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
        if( scope == scope_stack.top() )
            return string(); // local scope
        if( scope == program )
            return "::"; 
        else if( shared_ptr<Enum> e = dynamic_pointer_cast<Enum>( scope ) ) // <- for enum
            return RenderScopedIdentifier( e, true );    // omit scope for the enum itself   
        else if( shared_ptr<Record> r = dynamic_pointer_cast<Record>( scope ) ) // <- for class, struct, union
            return RenderScopedIdentifier( r ) + "::";       
        else if( dynamic_pointer_cast<Procedure>( scope ) ||  // <- this is for params
                 dynamic_pointer_cast<Compound>( scope ) )    // <- this is for locals in body
            return string(); 
        else
            return ERROR_UNSUPPORTED( scope );
    }        
    
    string RenderScopedIdentifier( shared_ptr<Identifier> id, bool skip=false )
    {
        string s;
        if( skip )
            s = RenderScope( id );
        else
            s = RenderScope( id ) + RenderIdentifier( id );
        TRACE("Render scoped identifier %s\n", s.c_str() );
        return s;
    }
    
    string RenderIntegralType( shared_ptr<Integral> type, string object=string() )
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

    string RenderFloatingType( shared_ptr<Floating> type )
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

    string RenderType( shared_ptr<Type> type, string object=string() )
    {
        string sobject;
        if( !object.empty() )
            sobject = " " + object;
            
        TRACE();
        if( shared_ptr<Integral> i = dynamic_pointer_cast< Integral >(type) )
            return RenderIntegralType( i, object );
        if( shared_ptr<Floating> f = dynamic_pointer_cast< Floating >(type) )
            return RenderFloatingType( f ) + sobject;
        else if( dynamic_pointer_cast< Void >(type) )
            return "void" + sobject;
        else if( dynamic_pointer_cast< Bool >(type) )
            return "bool" + sobject;
        else if( shared_ptr<Constructor> c = dynamic_pointer_cast< Constructor >(type) )
            return object + "(" + RenderSequence(c->parameters, ", ", false) + ")";
        else if( shared_ptr<Destructor> f = dynamic_pointer_cast< Destructor >(type) )
            return object + "()";
        else if( shared_ptr<Function> f = dynamic_pointer_cast< Function >(type) )
            return RenderType( f->return_type, "(" + object + ")(" + RenderSequence(f->parameters, ", ", false) + ")" );
        else if( shared_ptr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
            return RenderType( p->destination, "(*" + object + ")" );
        else if( shared_ptr<Reference> r = dynamic_pointer_cast< Reference >(type) )
            return RenderType( r->destination, "(&" + object + ")" );
        else if( shared_ptr<Array> a = dynamic_pointer_cast< Array >(type) )
            return RenderType( a->element, object.empty() ? "[" + RenderOperand(a->size) + "]" : "(" + object + "[" + RenderOperand(a->size) + "])" );
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(type) )
            return t->name + sobject;
#if 0 // Should we include the word "class" etc when referencing holders as types?
       else if( shared_ptr<Struct> ss = dynamic_pointer_cast< Struct >(type) )
            return "struct " + ss->name + sobject;
        else if( shared_ptr<Class> c = dynamic_pointer_cast< Class >(type) )
            return "class " + c->name + sobject;
        else if( shared_ptr<Union> u = dynamic_pointer_cast< Union >(type) )
            return "union " + u->name + sobject;
        else if( shared_ptr<Enum> e = dynamic_pointer_cast< Enum >(type) )
            return "enum " + e->name + sobject;
#else
        else if( shared_ptr<UserType> ut = dynamic_pointer_cast< UserType >(type) )
            return ut->name + sobject;
#endif
        else
            return ERROR_UNSUPPORTED(type);
    }
    
    // Insert escapes into a string so it can be put in source code
    // TODO use \n \r etc and let printable ascii through
    string Sanitise( string s )
    {
        string o;
        for( int i=0; i<s.size(); i++ )
        {
            char c[10];
            sprintf( c, "\\x%02x", s[i] );
            o += c;
        }
        return o;
    }

    string RenderOperand( shared_ptr<Operand> expression, bool bracketize_operator=false )
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
        else if( shared_ptr<Instance> v = dynamic_pointer_cast< Instance >(expression) )
            return RenderScopedIdentifier( v );
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
                   RenderOperand( o->operands[0], true ) +
                   operator_text[o->kind] + 
                   RenderOperand( o->operands[1], true ) +
                   after;
        else if( shared_ptr<Postfix> o = dynamic_pointer_cast< Postfix >(expression) )
            return before + 
                   RenderOperand( o->operands[0], true ) + 
                   operator_text[o->kind] +
                   after;
        else if( shared_ptr<Prefix> o = dynamic_pointer_cast< Prefix >(expression) )
            return before + 
                   operator_text[o->kind] + 
                   RenderOperand( o->operands[0], true ) +
                   (operator_text[o->kind][operator_text[o->kind].size()-1]=='(' ? ")" : "") + // if the token is "x(", add ")"
                   after;
        else if( shared_ptr<PrefixOnType> pot = dynamic_pointer_cast< PrefixOnType >(expression) )
            return before + 
                   operator_text[pot->kind] + 
                   RenderType( pot->operand, "" ) +
                   (operator_text[pot->kind][operator_text[pot->kind].size()-1]=='(' ? ")" : "") + // if the token is "x(", add ")"
                   after;
        else if( shared_ptr<ConditionalOperator> o = dynamic_pointer_cast< ConditionalOperator >(expression) )
            return before + 
                   RenderOperand( o->condition, true ) + "?" +
                   RenderOperand( o->if_true, true ) + ":" +
                   RenderOperand( o->if_false, true ) +
                   after;
        else if( shared_ptr<Call> o = dynamic_pointer_cast< Call >(expression) )
            return before + 
                   RenderOperand( o->function, true ) + "(" +
                   RenderOperandSequence( o->arguments, ", ", false ) + ")" +
                   after;
        else if( shared_ptr<Invoke> in = dynamic_pointer_cast< Invoke >(expression) )
        {
            if( dynamic_pointer_cast<Constructor>(in->member->type) )
                return before +  // invoking costructors as found in init lists and locals
                       RenderOperand( in->base, true ) + "(" +
                       RenderOperandSequence( in->arguments, ", ", false ) + ")" +
                       after;
            else
                return before + 
                       RenderOperand( in->base, true ) + "." +
                       RenderIdentifier( in->member ) + "(" +
                       RenderOperandSequence( in->arguments, ", ", false ) + ")" +
                       after;
        }
        else if( shared_ptr<New> n = dynamic_pointer_cast< New >(expression) )
            return before +
                   (n->global ? "::" : "") +
                   "new(" + RenderOperandSequence( n->placement_arguments, ", ", false ) + ") " +
                   RenderType( n->type, "" ) + 
                   (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( n->constructor_arguments, ", ", false ) + ")" ) +
                   after;
        else if( shared_ptr<Delete> d = dynamic_pointer_cast< Delete >(expression) )
            return before +
                   (d->global ? "::" : "") +
                   "delete" + (d->array ? "[]" : "") +
                   " " + RenderOperand( d->pointer, true ) +
                   after;
        else if( shared_ptr<Subscript> su = dynamic_pointer_cast< Subscript >(expression) )
            return before + 
                   RenderOperand( su->base, true ) + "[" +
                   RenderOperand( su->index, false ) + "]" +
                   after;
        else if( shared_ptr<Lookup> a = dynamic_pointer_cast< Lookup >(expression) )
            return before + 
                   RenderOperand( a->base, true ) + "." +
                   RenderIdentifier( a->member ) +
                   after;
            // TODO: this should use RenderScopedIdentifier for the member, since it could have a 
            // C++ scope specified (eg o.c::m) but cannot do this until members are set properly
            // in parser, which in turn requires a TypeOfExpression cvapbability in the helpers.     
                   
        else if( shared_ptr<Compound> c = dynamic_pointer_cast< Compound >(expression) )
        {
            AutoPush< shared_ptr<Node> > cs( scope_stack, c );
            return before + 
                   "{\n" + 
                   RenderSequence(c->statements, ";\n", true) +
                   "}\n" +
                   after;
        }
        else if( shared_ptr<Cast> c = dynamic_pointer_cast< Cast >(expression) )
            return before + 
                   "(" + RenderType( c->type, "" ) + ")" +
                   RenderOperand( c->operand, false ) + 
                   after;
        else if( shared_ptr<Aggregate> ao = dynamic_pointer_cast< Aggregate >(expression) )
            return before + 
                   "{ " + RenderOperandSequence( ao->elements, ", ", false ) + " }" +
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
    
    string RenderAccess( Declaration::Access access )
    {
        switch( access )
        {
            case Declaration::PUBLIC:
                return "public";
            case Declaration::PRIVATE:
                return "private";
            case Declaration::PROTECTED:
                return "protected";
            default:
                return ERROR_UNKNOWN("access spec"); 
        }        
    }
    
    string RenderStorage( Physical::Storage st )
    {
        switch( st )
        {
        case Physical::STATIC:
            return "static "; 
            break;
        case Physical::EXTERN:
            return "extern ";
            break;
        case Physical::AUTO:
            return "auto ";
            break;
        case Physical::VIRTUAL:
       // case Physical::PURE:
            return "virtual ";
            break;
        case Physical::DEFAULT:
            return "";
            break;
        default:
            return ERROR_UNKNOWN("storage class");
            break;
        }
    }
    
    void ExtractInits( const Sequence<Statement> &body, Sequence<Statement> &inits, Sequence<Statement> &remainder )
    {
        FOREACH( shared_ptr<Statement> s, body )
        {
            if( shared_ptr<Invoke> in = dynamic_pointer_cast< Invoke >(s) )
                if( dynamic_pointer_cast<Constructor>(in->member->type) )
                {
                    inits.push_back(s);
                    continue;
                }
            remainder.push_back(s);    
        }
    }
    
    string RenderObject( shared_ptr<Instance> o, string sep, bool showtype = true, 
                         bool showstorage = true, bool showinit = true, bool showscope = false )
    {
        string s;
        
        ASSERT(o->type);
        
        if( showstorage )
            s += RenderStorage(o->storage);
        
        string name;
        shared_ptr<Constructor> con = dynamic_pointer_cast<Constructor>(o->type);
        shared_ptr<Destructor> de = dynamic_pointer_cast<Destructor>(o->type);
        if( con || de )
        {
            shared_ptr<Record> rec = dynamic_pointer_cast<Record>( GetScope( program, o ) );
            ASSERT( rec );
            name = (de ? "~" : "") + RenderIdentifier(rec); 
        }
        else
        {
            name = RenderIdentifier(o);
        }
        
        if( showscope )
            name = RenderScope(o) + name;           
                        
        if( showtype )
            s += RenderType( o->type, name );
        else
            s = name;
        
        if( o->initialiser && showinit )
        {
            AutoPush< shared_ptr<Node> > cs( scope_stack, GetScope( program, o ) );
            
            shared_ptr<Compound> comp = dynamic_pointer_cast<Compound>(o->initialiser);                
            if( comp )
            {                                 
                Sequence<Statement> inits;
                Sequence<Statement> remainder;
                ExtractInits( comp->statements, inits, remainder );
                if( !inits.empty() )
                {
                    s += " : ";
                    s += RenderSequence( inits, ", ", false, Declaration::PUBLIC, true );                
                }
                
                shared_ptr<Compound> r( new Compound );
                r->statements = remainder;
                s += "\n" + RenderOperand(r);
            }
            else
            {
                s += " = " + RenderOperand(o->initialiser) + sep;
            }
        }            
        else
        {
            s += sep;
        }    
        return s;
    }
    
    string RenderDeclaration( shared_ptr<Declaration> declaration, string sep, Declaration::Access *access = NULL, bool showtype = true )
    {
        TRACE();
        string s;
        
        if( access && declaration->access != *access )
        {
            s += RenderAccess( declaration->access ) + ":\n";
            *access = declaration->access;
        }
                                         
        if( shared_ptr<Instance> o = dynamic_pointer_cast< Instance >(declaration) )
        {                
            bool isfunc = !!dynamic_pointer_cast<Subroutine>( o->type );
            if( dynamic_pointer_cast<Record>( scope_stack.top() ) &&
                !dynamic_pointer_cast<Enum>( scope_stack.top() ) &&
                (o->storage==Physical::STATIC || isfunc) )
            {
                // Static things in records (ie static member objects and static emmber functions)
                // get split into a part that goes into the record (main line of rendering) and
                // a part that goes seperately (deferred_decls gets appended at the very end)
                s += RenderObject( o, sep, showtype, showtype, false, false );
                {
                    AutoPush< shared_ptr<Node> > cs( scope_stack, program );
                    deferred_decls += string("\n") + RenderObject( o, sep, showtype, false, true, true );
                }
            }
            else
            {
                // Otherwise, render everything directly using the default settings
                s += RenderObject( o, sep, showtype, showtype, true, false );
            }
        }
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(declaration) )
            s += "typedef " + RenderType( t->type, t->name ) + sep;
        else if( shared_ptr<Record> r = dynamic_pointer_cast< Record >(declaration) )
        {
            Declaration::Access a;
            bool showtype=true;
            string sep2=";\n";
            if( dynamic_pointer_cast< Class >(r) )
            {
                s += "class";
                a = Declaration::PRIVATE;
            }
            else if( dynamic_pointer_cast< Struct >(r) )
            {
                s += "struct";
                a = Declaration::PUBLIC;
            }
            else if( dynamic_pointer_cast< Union >(r) )
            {
                s += "union";
                a = Declaration::PUBLIC;
            }
            else if( dynamic_pointer_cast< Enum >(r) )
            {
                s += "enum";
                a = Declaration::PUBLIC;
                sep2 = ",\n";
                showtype = false;
            }
            else
                return ERROR_UNSUPPORTED(declaration);

            s += " " + r->name;
            
            if( shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast< InheritanceRecord >(declaration) )
            {
                if( !ir->bases.empty() )
                {
                    s += " : ";
                    for( int i=0; i<ir->bases.size(); i++ )
                    {   
                        if( i>0 )
                            s += ", ";
                        shared_ptr<Base> b = ir->bases[i];    
                        ASSERT( b );
                        s += RenderAccess(b->access) + " " + RenderStorage(b->storage) + b->record->name;
                    }
                }
            }
            
            if( !r->incomplete )
            {
                 AutoPush< shared_ptr<Node> > cs( scope_stack, r );
                 s += "\n{\n" +
                      RenderSequence( r->members, sep2, true, a, showtype ) +
                      "}";
            }
            
            s += ";\n";
        }
        else
            s += ERROR_UNSUPPORTED(declaration);
            
            TRACE();
        return s;    
    }

    string RenderStatement( shared_ptr<Statement> statement, string sep )
    {
        TRACE();
        if( !statement )
            return sep;            
        else if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(statement) )
            return RenderDeclaration( d, sep );
        else if( shared_ptr<Compound> c = dynamic_pointer_cast< Compound >(statement) ) // Never put ; after a scope - you'd get {blah};
            return RenderOperand(c);
        else if( shared_ptr<Expression> e = dynamic_pointer_cast< Expression >(statement) )
            return RenderOperand(e) + sep;
        else if( shared_ptr<Return> es = dynamic_pointer_cast<Return>(statement) )
            return "return " + RenderOperand(es->return_value) + sep;
        else if( shared_ptr<LabelTarget> l = dynamic_pointer_cast<LabelTarget>(statement) )
            return RenderIdentifier(l->label) + ":\n"; // no ; after a label
        else if( shared_ptr<Goto> g = dynamic_pointer_cast<Goto>(statement) )
        {
            if( shared_ptr<Label> l = dynamic_pointer_cast< Label >(g->destination) )
                return "goto " + RenderIdentifier(l) + sep;  // regular goto
            else
                return "goto *" + RenderOperand(g->destination) + sep; // goto-a-variable (GCC extension)
        }
        else if( shared_ptr<If> i = dynamic_pointer_cast<If>(statement) )
        {
            string s;
            s += "if( " + RenderOperand(i->condition) + " )\n"
                 "{\n" + // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
                 RenderStatement(i->body, ";\n") +
                 "}\n";
            if( i->else_body )  // else is optional
                s += "else\n" +
                     RenderStatement(i->else_body, ";\n");
            return s;
        } 
        else if( shared_ptr<While> w = dynamic_pointer_cast<While>(statement) )
            return "while( " + RenderOperand(w->condition) + " )\n" +
                   RenderStatement(w->body, ";\n");
        else if( shared_ptr<Do> d = dynamic_pointer_cast<Do>(statement) )
            return "do\n" +
                   RenderStatement(d->body, ";\n") +
                   "while( " + RenderOperand(d->condition) + " )" + sep;
        else if( shared_ptr<For> f = dynamic_pointer_cast<For>(statement) )
            return "for( " + RenderStatement(f->initialisation, "") + "; " + RenderOperand(f->condition) + "; "+ RenderStatement(f->increment, "") + " )\n" +
                   RenderStatement(f->body, ";\n");
        else if( shared_ptr<Switch> s = dynamic_pointer_cast<Switch>(statement) )
            return "switch( " + RenderOperand(s->condition) + " )\n" +
                   RenderStatement(s->body, ";\n");
        else if( shared_ptr<Case> c = dynamic_pointer_cast<Case>(statement) )
            return "case " + RenderOperand(c->value_lo) + " ... " + RenderOperand(c->value_hi) + ":\n";
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
    string RenderSequence( Sequence<ELEMENT> spe, 
                           string separator, 
                           bool seperate_last, 
                           Declaration::Access init_access = Declaration::PUBLIC,
                           bool showtype=true )
    {
        TRACE();
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            TRACE("%d %p\n", i, &i);
            string sep = (seperate_last || i+1<spe.size()) ? separator : "";
            shared_ptr<ELEMENT> pe = spe[i];                        
            if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(pe) )
                s += RenderDeclaration( d, sep, &init_access, showtype );
            else if( shared_ptr<Statement> st = dynamic_pointer_cast< Statement >(pe) )
                s += RenderStatement( st, sep ); 
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }

    string RenderOperandSequence( Sequence<Operand> spe, 
                                  string separator, 
                                  bool seperate_last )
    {
        TRACE();
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            TRACE("%d %p\n", i, &i);
            string sep = (seperate_last || i+1<spe.size()) ? separator : "";
            shared_ptr<Operand> pe = spe[i];                        
            s += RenderOperand( pe ) + sep;
        }
        return s;
    }
};

#endif
