#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "helpers/transformation.hpp"
#include "helpers/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_db.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "helpers/scope.hpp"
#include "sort_decls.hpp"

#include "clang/Parse/DeclSpec.h"

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    string( "\n#error " ) + \
    string( V ) + \
    string( " not supported in " ) + \
    string( INFERNO_CURRENT_FUNCTION ) + \
    string( "\n" );

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? typeid(*P).name() : "<NULL>" );


class Render : public OutOfPlaceTransformation
{
public:
	using Transformation::operator();
    Render()
    {        
    }
    
    SharedPtr<Node> operator()( SharedPtr<Node> context, SharedPtr<Node> root )
    {
        // Render can only work on a whole program
    	ASSERT( context == root );
        program = dynamic_pointer_cast<Program>(root);
        ASSERT( program );
        AutoPush< SharedPtr<Scope> > cs( scope_stack, program );
              
        string s = RenderDeclarationCollection( program, ";\n", true ); // gets the .hpp stuff directly
    
        s += deferred_decls; // these could go in a .cpp file
        
        if( ReadArgs::outfile.empty() )
        {
            puts( s.c_str() );
        }
        else
        {
            FILE *fp = fopen( ReadArgs::outfile.c_str(), "wt" );
            ASSERT( fp )( "Cannot open output file" );
            fputs( s.c_str(), fp );
            fclose( fp );
        }    
        program = SharedPtr<Program>();
        return root; // no change
    }

private:
    SharedPtr<Program> program;
    string deferred_decls;
    stack< SharedPtr<Scope> > scope_stack;
    // Remember the orders of collections when we sort them. Mirrors the same
    // map in the parser.
    Map< SharedPtr<Scope>, Sequence<Declaration> > backing_ordering;

    string RenderLiteral( SharedPtr<Literal> sp )
    {
    	return Sanitise( *sp );
    }
    
    string RenderIdentifier( SharedPtr<Identifier> id )
    {
        string ids;
        if( id )
        {
            // TODO maybe just try casting to Named
            if( SharedPtr<SpecificIdentifier> ii = dynamic_pointer_cast<SpecificIdentifier>( id ) )
                ids = *ii;
            else
                ids = ERROR_UNSUPPORTED( (id) );

            TRACE( "%s\n", ids.c_str() );
        }
        else
        {
            TRACE();
        }
        return ids;
    }

    string RenderScopePrefix( SharedPtr<Identifier> id )
    {
        SharedPtr<Scope> scope = GetScope( program, id );
        TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
        if( scope == scope_stack.top() )
            return string(); // local scope
        else if( scope == program )
            return "::"; 
        else if( SharedPtr<Enum> e = dynamic_pointer_cast<Enum>( scope ) ) // <- for enum
            return RenderScopePrefix( e->identifier );    // omit scope for the enum itself
        else if( SharedPtr<Record> r = dynamic_pointer_cast<Record>( scope ) ) // <- for class, struct, union
            return RenderScopedIdentifier( r->identifier ) + "::";       
        else if( dynamic_pointer_cast<Procedure>( scope ) ||  // <- this is for params
                 dynamic_pointer_cast<Compound>( scope ) )    // <- this is for locals in body
            return string(); 
        else
            return ERROR_UNSUPPORTED( scope );
    }        
    
    string RenderScopedIdentifier( SharedPtr<Identifier> id )
    {
        string s = RenderScopePrefix( id ) + RenderIdentifier( id );
        TRACE("Render scoped identifier %s\n", s.c_str() );
        return s;
    }
    
    string RenderIntegralType( SharedPtr<Integral> type, string object=string() )
    {
        bool ds;
        unsigned width;       
        SharedPtr<SpecificInteger> ic = dynamic_pointer_cast<SpecificInteger>( type->width );
        ASSERT(ic)("width must be integer");
        width = ic->getLimitedValue();
                  
        TRACE("width %d\n", width);          
                          
        if( width == TypeDb::char_bits )
            ds = TypeDb::char_default_signed;
        else
            ds = TypeDb::int_default_signed;
        
        // Produce signed or unsigned if required
        // Note: literal strings can be converted to char * but not unsigned char * or signed char * 
        string s;
        if( dynamic_pointer_cast< Signed >(type) && !ds )
            s = "signed ";
        else if( dynamic_pointer_cast< Unsigned >(type) && ds )
            s = "unsigned ";

        // Fix the width
        bool bitfield = false;
        if( width == TypeDb::char_bits )
            s += "char";
        else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] )
            s += "int";
        else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_short] )
            s += "short";
        else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_long] )
            s += "long";
        else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] )
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

    string RenderFloatingType( SharedPtr<Floating> type )
    {
        string s;
        SharedPtr<SpecificFloatSemantics> sem = dynamic_pointer_cast<SpecificFloatSemantics>(type->semantics);
        ASSERT(sem);
    
        if( &(const llvm::fltSemantics &)*sem == TypeDb::float_semantics )
            s += "float";
        else if( &(const llvm::fltSemantics &)*sem == TypeDb::double_semantics )
            s += "double";
        else if( &(const llvm::fltSemantics &)*sem == TypeDb::long_double_semantics )
            s += "long double";
        else    
            ASSERT(0)("no builtin floating type has required semantics"); // TODO drop in a bit vector
        
        return s;              
    }

    string RenderType( SharedPtr<Type> type, string object=string() )
    {
        string sobject;
        if( !object.empty() )
            sobject = " " + object;
            
        TRACE();
        if( SharedPtr<Integral> i = dynamic_pointer_cast< Integral >(type) )
            return RenderIntegralType( i, object );
        if( SharedPtr<Floating> f = dynamic_pointer_cast< Floating >(type) )
            return RenderFloatingType( f ) + sobject;
        else if( dynamic_pointer_cast< Void >(type) )
            return "void" + sobject;
        else if( dynamic_pointer_cast< Boolean >(type) )
            return "bool" + sobject;
        else if( SharedPtr<Constructor> c = dynamic_pointer_cast< Constructor >(type) )
            return object + "(" + RenderDeclarationCollection(c, ", ", false) + ")";
        else if( SharedPtr<Destructor> f = dynamic_pointer_cast< Destructor >(type) )
            return object + "()";
        else if( SharedPtr<Function> f = dynamic_pointer_cast< Function >(type) )
            return RenderType( f->return_type, "(" + object + ")(" + RenderDeclarationCollection(f, ", ", false) + ")" );
        else if( SharedPtr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
            return RenderType( p->destination, "(*" + object + ")" );
        else if( SharedPtr<Reference> r = dynamic_pointer_cast< Reference >(type) )
            return RenderType( r->destination, "(&" + object + ")" );
        else if( SharedPtr<Array> a = dynamic_pointer_cast< Array >(type) )
            return RenderType( a->element, object.empty() ? "[" + RenderExpression(a->size) + "]" : "(" + object + "[" + RenderExpression(a->size) + "])" );
        else if( SharedPtr<Typedef> t = dynamic_pointer_cast< Typedef >(type) )
            return RenderIdentifier(t->identifier) + sobject;
        else if( SharedPtr<SpecificTypeIdentifier> ti = dynamic_pointer_cast< SpecificTypeIdentifier >(type) )
            return RenderIdentifier(ti) + sobject;
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
            if( s[i] < ' ' )
            	o += SSPrintf( c, "\\x%02x", s[i] );
            else
            	o += s[i];
        }
        return o;
    }

    string RenderOperator( SharedPtr<Operator> op, Sequence<Expression> &operands )
    {
    	ASSERT(op);
        if( dynamic_pointer_cast< MakeArray >(op) )
            return "{ " + RenderOperandSequence( operands, ", ", false ) + " }";
        else if( dynamic_pointer_cast< Multiplexor >(op) )
            return RenderExpression( operands[0], true ) + "?" +
                   RenderExpression( operands[1], true ) + ":" +
                   RenderExpression( operands[2], true );
        else if( dynamic_pointer_cast< Subscript >(op) )
            return RenderExpression( operands[0], true ) + "[" +
                   RenderExpression( operands[1], false ) + "]";
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
    	else if( dynamic_pointer_cast<NODE>(op) ) \
			return RenderExpression( operands[0], true ) +\
				   TEXT +\
				   RenderExpression( operands[1], true );
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
    	else if( dynamic_pointer_cast<NODE>(op) ) \
			return TEXT +\
				   RenderExpression( operands[0], true );
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
    	else if( dynamic_pointer_cast<NODE>(op) ) \
			return RenderExpression( operands[0], true ) +\
				   TEXT;
#include "tree/operator_db.inc"
    	else
    		return ERROR_UNSUPPORTED(op);
    }

    string RenderCall( SharedPtr<Call> call )
    {
    	string s;

    	// Render the expression that resolves to the function name unless this is
    	// a constructor call in which case just the name of the thing being constructed.
        if( SharedPtr<Expression> base = TypeOf().IsConstructorCall( program, call ) )
            s += RenderExpression( base, true );
        else
        	s += RenderExpression( call->callee, true );

        s += "(";

        // If Procedure or Function, generate some arguments, resolving the order using the original function type
        SharedPtr<Node> ctype = TypeOf()( program, call->callee );
        ASSERT( ctype );
        if( SharedPtr<Procedure> proc = dynamic_pointer_cast<Procedure>(ctype) )
            s += RenderMapInOrder( call, proc, ", ", false );

        s += ")";
        return s;
    }

    string RenderExpression( SharedPtr<Initialiser> expression, bool bracketize_operator=false )
    {
        TRACE("%p\n", expression.get());
        
        string before = bracketize_operator ? "(" : "";
        string after = bracketize_operator ? ")" : "";
        
        if( dynamic_pointer_cast< Uninitialised >(expression) )
            return string();            
        else if( SharedPtr<SpecificLabelIdentifier> li = dynamic_pointer_cast< SpecificLabelIdentifier >(expression) )
            return before + 
                   "&&" + RenderIdentifier( li ) + // label-as-variable (GCC extension)
                   after;
        else if( SharedPtr<InstanceIdentifier> ii = dynamic_pointer_cast< InstanceIdentifier >(expression) )
            return RenderScopedIdentifier( ii );
        else if( SharedPtr<SizeOf> pot = dynamic_pointer_cast< SizeOf >(expression) )
            return before + 
                   "sizeof(" + RenderType( pot->operand, "" ) + ")" +
                   after;
        else if( SharedPtr<AlignOf> pot = dynamic_pointer_cast< AlignOf >(expression) )
            return before + 
                   "alignof(" + RenderType( pot->operand, "" ) + ")" +
                   after;
        else if( SharedPtr<NonCommutativeOperator> nco = dynamic_pointer_cast< NonCommutativeOperator >(expression) )
            return before +
                   RenderOperator( nco, nco->operands ) +
                   after;
        else if( SharedPtr<CommutativeOperator> co = dynamic_pointer_cast< CommutativeOperator >(expression) )
        {
        	Sequence<Expression> seq_operands;
        	// Operands are in collection, so move them to a container
            FOREACH( SharedPtr<Expression> o, co->operands )
          	    seq_operands.push_back( o );
            return before +
                   RenderOperator( co, seq_operands ) +
                   after;
        }
        else if( SharedPtr<Call> c = dynamic_pointer_cast< Call >(expression) )
            return before +
                   RenderCall( c ) +
                   after;
        else if( SharedPtr<New> n = dynamic_pointer_cast< New >(expression) )
            return before +
                   (dynamic_pointer_cast<Global>(n->global) ? "::" : "") +
                   "new(" + RenderOperandSequence( n->placement_arguments, ", ", false ) + ") " +
                   RenderType( n->type, "" ) + 
                   (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( n->constructor_arguments, ", ", false ) + ")" ) +
                   after;
        else if( SharedPtr<Delete> d = dynamic_pointer_cast< Delete >(expression) )
            return before +
                   (dynamic_pointer_cast<Global>(d->global) ? "::" : "") +
                   "delete" + 
                   (dynamic_pointer_cast<DeleteArray>(d->array) ? "[]" : "") +
                   " " + RenderExpression( d->pointer, true ) +
                   after;
        else if( SharedPtr<Lookup> a = dynamic_pointer_cast< Lookup >(expression) )
            return before + 
                   RenderExpression( a->base, true ) + "." +
                   RenderScopedIdentifier( a->member ) +
                   after;
        else if( SharedPtr<Cast> c = dynamic_pointer_cast< Cast >(expression) )
            return before + 
                   "(" + RenderType( c->type, "" ) + ")" +
                   RenderExpression( c->operand, false ) +
                   after;
        else if( SharedPtr<MakeRecord> ro = dynamic_pointer_cast< MakeRecord >(expression) )
            return before +
                   RenderMakeRecord( ro ) +
                   after;
        else if( SharedPtr<Literal> l = dynamic_pointer_cast< Literal >(expression) )
            return before + 
                   RenderLiteral( l ) +
                   after;
        else if( dynamic_pointer_cast< This >(expression) )
            return before + 
                   "this" + 
                   after;
        else
            return ERROR_UNSUPPORTED(expression);
    }
    
    string RenderMakeRecord( SharedPtr<MakeRecord> ro )
    {
    	string s;

    	// Get the record
    	SharedPtr<TypeIdentifier> id = dynamic_pointer_cast<TypeIdentifier>(ro->type);
    	ASSERT(id);
    	SharedPtr<Record> r = GetRecordDeclaration(program, id);

    	s += "(";
    	s += RenderType( ro->type, "" );
    	s += "){ ";
        s += RenderMapInOrder( ro, r, ", ", false );
    	s += " }";
        return s;
    }

    string RenderMapInOrder( SharedPtr<MapOperator> ro,
    		                 SharedPtr<Scope> r,
                             string separator,
                             bool separate_last )
    {
    	string s;

    	// Get a reference to the ordered list of members for this record from a backing list
    	ASSERT( backing_ordering.IsExist( r ) );
    	Sequence<Declaration> &sd = backing_ordering[r];
    	ASSERT( sd.size() == r->members.size() );
    	bool first = true;
    	FOREACH( SharedPtr<Declaration> d, sd )
    	{
    		// We only care about instances...
    		if( SharedPtr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
    		{
    			// ...and not function instances
    			if( !dynamic_pointer_cast<Subroutine>( i->type ) )
    			{
    				// search init for matching member (TODO could avoid O(n^2) by exploiting the map)
    				FOREACH( SharedPtr<MapOperand> mi, ro->operands )
    		        {
    			        if( i->identifier == mi->identifier )
    			        {
    			        	if( !first )
    			        		s += separator;
    			        	s += RenderExpression( mi->value );
    			        	first = false;
    			        }
    		        }
    			}
    		}
    		if( separate_last )
    			s += separator;
    	}
        return s;
    }

    string RenderAccess( SharedPtr<AccessSpec> current_access )
    {
        if( dynamic_pointer_cast<Public>( current_access ) )
            return "public";
        else if( dynamic_pointer_cast<Private>( current_access ) )
            return "private";
        else if( dynamic_pointer_cast<Protected>( current_access ) )
            return "protected";
        else
            return ERROR_UNKNOWN("current_access spec"); 
    }
    
    string RenderStorage( SharedPtr<Instance> st )
    {
        if( dynamic_pointer_cast<Program>( scope_stack.top() ) )
            return ""; // at top-level scope, everything is set to static, but don't actually output the word
        else if( dynamic_pointer_cast<Static>( st ) )
            return "static "; 
        else if( dynamic_pointer_cast<Automatic>( st ) )
            return "auto "; 
        else if( dynamic_pointer_cast<Temporary>( st ) )
            return "/*temp*/ "; 
        else if( SharedPtr<Field> no = dynamic_pointer_cast<Field>( st ) )
        {
            SharedPtr<Virtuality> v = no->virt;
            if( dynamic_pointer_cast<Virtual>( v ) )
                return "virtual ";
            else if( dynamic_pointer_cast<NonVirtual>( v ) )
                return "";
            else
                return ERROR_UNKNOWN("virtualness");    
        }
        else
            return ERROR_UNKNOWN("storage class");
    }
    
    void ExtractInits( Sequence<Statement> &body, Sequence<Statement> &inits, Sequence<Statement> &remainder )
    {
        FOREACH( SharedPtr<Statement> s, body )
        {
            if( SharedPtr<Call> o = dynamic_pointer_cast< Call >(s) )
            {
                if( TypeOf().IsConstructorCall( program, o ) )
                {
                    inits.push_back(s);
                    continue;
                }
            }
            remainder.push_back(s);    
        }
    }
    
    string RenderInstance( SharedPtr<Instance> o, string sep, bool showtype = true,
                           bool showstorage = true, bool showinit = true, bool showscope = false )
    {
        string s;
        
        ASSERT(o->type);
        
        if( showstorage )
        {
       	    if( SharedPtr<Static> st = dynamic_pointer_cast<Static>(o) )
       		    if( dynamic_pointer_cast<Const>(st->constancy) )
                    s += "const ";
    	    if( SharedPtr<Field> f = dynamic_pointer_cast<Field>(o) )
    		    if( dynamic_pointer_cast<Const>(f->constancy) )
                    s += "const ";
            s += RenderStorage(o);
        }
        
        string name;

        if( showscope )
            name = RenderScopePrefix(o->identifier);

        SharedPtr<Constructor> con = dynamic_pointer_cast<Constructor>(o->type);
        SharedPtr<Destructor> de = dynamic_pointer_cast<Destructor>(o->type);
        if( con || de )
        {
            SharedPtr<Record> rec = dynamic_pointer_cast<Record>( GetScope( program, o->identifier ) );
            ASSERT( rec );
            name += (de ? "~" : "");
            name += RenderIdentifier(rec->identifier);
        }
        else
        {
            name += RenderIdentifier(o->identifier);
        }
        
                        
        if( showtype )
            s += RenderType( o->type, name );
        else
            s = name;
          
        if( !showinit || dynamic_pointer_cast<Uninitialised>(o->initialiser) )
        {
            // Don't render any initialiser
            s += sep;
        }    
        else if( SharedPtr<Compound> comp = dynamic_pointer_cast<Compound>(o->initialiser) )
        {                                 
            // Render initialiser list then let RenderStatement() do the rest
            AutoPush< SharedPtr<Scope> > cs( scope_stack, GetScope( program, o->identifier ) );

            Sequence<Statement> inits;
            Sequence<Statement> remainder;
            ExtractInits( comp->statements, inits, remainder );
            if( !inits.empty() )
            {
                s += " : ";
                s += RenderSequence( inits, ", ", false, SharedPtr<Public>(), true );
            }
            
            SharedPtr<Compound> r( new Compound );
            r->members = comp->members;
            r->statements = remainder;
            s += "\n" + RenderStatement(r, "");
        }
        else if( SharedPtr<Expression> ei = dynamic_pointer_cast<Expression>( o->initialiser ) )
        {
            // Render expression with an assignment
            AutoPush< SharedPtr<Scope> > cs( scope_stack, GetScope( program, o->identifier ) );
            s += " = " + RenderExpression(ei) + sep;
        }
        else
        {
            s += ERROR_UNSUPPORTED(o->initialiser);            
        }

        return s;
    }
    
    // Non-const static objects and functions in records 
    // get split into a part that goes into the record (main line of rendering) and
    // a part that goes separately (deferred_decls gets appended at the very end)
    bool ShouldSplitInstance( SharedPtr<Instance> o )
    {
        bool isfunc = !!dynamic_pointer_cast<Subroutine>( o->type );
        bool is_non_const_static = false;
        if( SharedPtr<Static> s = dynamic_pointer_cast<Static>(o) )
        	if( dynamic_pointer_cast<NonConst>(s->constancy) )
        		is_non_const_static = true;
        return dynamic_pointer_cast<Record>( scope_stack.top() ) &&
                   ( is_non_const_static || isfunc );
    }
    
    string RenderDeclaration( SharedPtr<Declaration> declaration, string sep, SharedPtr<AccessSpec> *current_access = NULL,
                              bool showtype = true, bool force_incomplete = false )
    {
        TRACE();
        string s;
        
        SharedPtr<AccessSpec> this_access;

        // Decide access spec for this declaration (explicit if instance, otherwise force to Public)
        if( SharedPtr<Field> f = dynamic_pointer_cast<Field>(declaration) )
            this_access = f->access;
        else
        	this_access = shared_new<Public>();
                
        // Now decide whether we actually need to render an access spec (ie has it changed?)
        if( current_access && // NULL means dont ever render access specs
            typeid(*this_access) != typeid(**current_access) ) // current_access spec must have changed
        {
            s += RenderAccess( this_access ) + ":\n";
            *current_access = this_access;
        }
                                         
        if( SharedPtr<Instance> o = dynamic_pointer_cast<Instance>(declaration) )
        {                
            if( ShouldSplitInstance(o) )
            {
                s += RenderInstance( o, sep, showtype, showtype, false, false );
                {
                    AutoPush< SharedPtr<Scope> > cs( scope_stack, program );
                    deferred_decls += string("\n") + RenderInstance( o, sep, showtype, false, true, true );
                }
            }
            else
            {
                // Otherwise, render everything directly using the default settings
                s += RenderInstance( o, sep, showtype, showtype, true, false );
            }
        }
        else if( SharedPtr<Typedef> t = dynamic_pointer_cast< Typedef >(declaration) )
        {
            s += "typedef " + RenderType( t->type, RenderIdentifier(t->identifier) ) + sep;
        }
        else if( SharedPtr<Record> r = dynamic_pointer_cast< Record >(declaration) )
        {
            SharedPtr<AccessSpec> a;
            bool showtype=true;
            string sep2=";\n";
            if( dynamic_pointer_cast< Class >(r) )
            {
                s += "class";
                a = SharedPtr<Private>(new Private);
            }
            else if( dynamic_pointer_cast< Struct >(r) )
            {
                s += "struct";
                a = SharedPtr<Public>(new Public);
            }
            else if( dynamic_pointer_cast< Union >(r) )
            {
                s += "union";
                a = SharedPtr<Public>(new Public);
            }
            else if( dynamic_pointer_cast< Enum >(r) )
            {
                s += "enum";
                a = SharedPtr<Public>(new Public);
                sep2 = ",\n";
                showtype = false;
            }
            else
                return ERROR_UNSUPPORTED(declaration);

            // Name of the record
            s += " " + RenderIdentifier(r->identifier);
            
            if( !force_incomplete )
            {
                // Base classes
                if( SharedPtr<InheritanceRecord> ir = dynamic_pointer_cast< InheritanceRecord >(declaration) )
                {
                    if( !ir->bases.empty() )
                    {
                        s += " : ";
                        bool first=true;
                        FOREACH( SharedPtr<Base> b, ir->bases )
                        {   
                            if( !first )
                                s += ", ";
                            first=false;
                            ASSERT( b );
                            s += RenderAccess(b->access) + " " /*+ RenderStorage(b->storage)*/ + RenderIdentifier(b->record);
                        }
                    }
                }
                
                // Contents
                AutoPush< SharedPtr<Scope> > cs( scope_stack, r );
                s += "\n{\n" +
                     RenderDeclarationCollection( r, sep2, true, a, showtype ) +
                     "}";
            }
            
            s += ";\n";
        }
        else if( SharedPtr<Label> l = dynamic_pointer_cast<Label>(declaration) )
            return RenderIdentifier(l->identifier) + ":\n"; // no ; after a label        
        else
            s += ERROR_UNSUPPORTED(declaration);
            
            TRACE();
        return s;    
    }

    string RenderStatement( SharedPtr<Statement> statement, string sep )
    {
        TRACE();
        if( !statement )
            return sep;            
        //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
        if( SharedPtr<Declaration> d = dynamic_pointer_cast< Declaration >(statement) )
            return RenderDeclaration( d, sep );
        else if( SharedPtr<Compound> c = dynamic_pointer_cast< Compound >(statement) )
        {
            AutoPush< SharedPtr<Scope> > cs( scope_stack, c );
            string s = "{\n";
            s += RenderDeclarationCollection( c, ";\n", true ); // Must do this first to populate backing list
            s += RenderSequence( c->statements, ";\n", true );
            return s + "}\n";
        }
        else if( SharedPtr<Expression> e = dynamic_pointer_cast< Expression >(statement) )
            return RenderExpression(e) + sep;
        else if( SharedPtr<Return> es = dynamic_pointer_cast<Return>(statement) )
            return "return " + RenderExpression(es->return_value) + sep;
        else if( SharedPtr<Goto> g = dynamic_pointer_cast<Goto>(statement) )
        {
            if( SharedPtr<SpecificLabelIdentifier> li = dynamic_pointer_cast< SpecificLabelIdentifier >(g->destination) )
                return "goto " + RenderIdentifier(li) + sep;  // regular goto
            else
                return "goto *" + RenderExpression(g->destination) + sep; // goto-a-variable (GCC extension)
        }
        else if( SharedPtr<If> i = dynamic_pointer_cast<If>(statement) )
        {
            string s;
            s += "if( " + RenderExpression(i->condition) + " )\n"
                 "{\n" + // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
                 RenderStatement(i->body, ";\n") +
                 "}\n";
            if( !dynamic_pointer_cast<Nop>(i->else_body) )  // Nop means no else clause
                s += "else\n" +
                     RenderStatement(i->else_body, ";\n");
            return s;
        } 
        else if( SharedPtr<While> w = dynamic_pointer_cast<While>(statement) )
            return "while( " + RenderExpression(w->condition) + " )\n" +
                   RenderStatement(w->body, ";\n");
        else if( SharedPtr<Do> d = dynamic_pointer_cast<Do>(statement) )
            return "do\n" +
                   RenderStatement(d->body, ";\n") +
                   "while( " + RenderExpression(d->condition) + " )" + sep;
        else if( SharedPtr<For> f = dynamic_pointer_cast<For>(statement) )
            return "for( " + RenderStatement(f->initialisation, "") + "; " + RenderExpression(f->condition) + "; "+ RenderStatement(f->increment, "") + " )\n" +
                   RenderStatement(f->body, ";\n");
        else if( SharedPtr<Switch> s = dynamic_pointer_cast<Switch>(statement) )
            return "switch( " + RenderExpression(s->condition) + " )\n" +
                   RenderStatement(s->body, ";\n");
        else if( SharedPtr<Case> c = dynamic_pointer_cast<Case>(statement) )
            return "case " + RenderExpression(c->value) + ":\n";
        else if( SharedPtr<RangeCase> rc = dynamic_pointer_cast<RangeCase>(statement) )
            return "case " + RenderExpression(rc->value_lo) + " ... " + RenderExpression(rc->value_hi) + ":\n";
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
                           bool separate_last,
                           SharedPtr<AccessSpec> init_access = SharedPtr<AccessSpec>(),
                           bool showtype=true )
    {
        TRACE();
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            TRACE("%d %p\n", i, &i);
            string sep = (separate_last || i+1<spe.size()) ? separator : "";
            SharedPtr<ELEMENT> pe = spe[i];
            if( SharedPtr<Declaration> d = dynamic_pointer_cast< Declaration >(pe) )
                s += RenderDeclaration( d, sep, init_access ? &init_access : NULL, showtype );
            else if( SharedPtr<Statement> st = dynamic_pointer_cast< Statement >(pe) )
                s += RenderStatement( st, sep ); 
            else
                s += ERROR_UNSUPPORTED(pe);
        }
        return s;
    }
    
    string RenderOperandSequence( Sequence<Expression> spe, 
                                  string separator, 
                                  bool separate_last )
    {
        TRACE();
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            TRACE("%d %p\n", i, &i);
            string sep = (separate_last || i+1<spe.size()) ? separator : "";
            SharedPtr<Expression> pe = spe[i];
            s += RenderExpression( pe ) + sep;
        }
        return s;
    }
    
    string RenderDeclarationCollection( SharedPtr<Scope> sd,
			                            string separator, 
			                            bool separate_last,
			                            SharedPtr<AccessSpec> init_access = SharedPtr<AccessSpec>(),
			                            bool showtype=true )
    {
        TRACE();        
        
        // Uncomment one of these to stress the sorter
        //sd = ReverseDecls( sd );
        //sd = JumbleDecls( sd );
        
        Sequence<Declaration> sorted = SortDecls( sd->members, true );
        backing_ordering[sd] = sorted;
        
        string s;
        // Emit an incomplete for each record
        FOREACH( SharedPtr<Declaration> pd, sorted ) //for( int i=0; i<sorted.size(); i++ )
            if( SharedPtr<Record> r = dynamic_pointer_cast<Record>(pd) ) // is a record
                if( !dynamic_pointer_cast<Enum>(r) ) // but not an enum 
                    s += RenderDeclaration( r, separator, init_access ? &init_access : NULL, showtype, true );
        
        s += RenderSequence( sorted, separator, separate_last, init_access, showtype );
        TRACE();
        return s;        
    }
};

#endif
