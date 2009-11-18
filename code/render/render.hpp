#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "helpers/pass.hpp"
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


class Render : public Transformation
{
public:
    Render()
    {        
    }
    
    void operator()( shared_ptr<Node> context, shared_ptr<Node> root )
    {
        // Parse can only work on a whole program
    	ASSERT( context == root );
        program = dynamic_pointer_cast<Program>(root);
        ASSERT( program );
        AutoPush< shared_ptr<Scope> > cs( scope_stack, program );
              
        string s = RenderDeclarationCollection( program, ";\n", true ); // gets the .hpp stuff directly
    
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
    shared_ptr<Program> program;
    string deferred_decls;
    stack< shared_ptr<Scope> > scope_stack;
    // Remember the orders of collections when we sort them. Mirrors the same
    // map in the parser.
    Map< shared_ptr<Scope>, Sequence<Declaration> > backing_ordering;

    string RenderLiteral( shared_ptr<Literal> sp )
    {
        if( shared_ptr<SpecificString> ss = dynamic_pointer_cast< SpecificString >(sp) )
            return "\"" + Sanitise( ss->value ) + "\"";                     
        else if( shared_ptr<SpecificInteger> ic = dynamic_pointer_cast< SpecificInteger >(sp) )
            return string(ic->value.toString(10)) + 
                   (ic->value.isUnsigned() ? "U" : "") + 
                   (ic->value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ? "L" : "") +
                   (ic->value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_long] ? "L" : ""); 
                   // note, assuming longlong bigger than long, so second L appends first to get LL
        else if( shared_ptr<SpecificFloat> fc = dynamic_pointer_cast< SpecificFloat >(sp) )
        {
            char hs[256];
            // generate hex float since it can be exact
            fc->value.convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
            return string(hs) + 
                   (&(fc->value.getSemantics())==TypeDb::float_semantics ? "F" : "") +
                   (&(fc->value.getSemantics())==TypeDb::long_double_semantics ? "L" : ""); 
        }           
        else if( dynamic_pointer_cast< True >(sp) )
            return string("true");
        else if( dynamic_pointer_cast< False >(sp) )
            return string("false");
        else
            return ERROR_UNSUPPORTED( sp );
    }
    
    string RenderIdentifier( shared_ptr<Identifier> id )
    {
        string ids;
        if( id )
        {
            // TODO maybe just try casting to Named
            if( shared_ptr<SpecificIdentifier> ii = dynamic_pointer_cast<SpecificIdentifier>( id ) )
                ids = ii->name;
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

    string RenderScopePrefix( shared_ptr<Identifier> id )
    {
        shared_ptr<Scope> scope = GetScope( program, id );
        TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
        if( scope == scope_stack.top() )
            return string(); // local scope
        else if( scope == program )
            return "::"; 
        else if( shared_ptr<Enum> e = dynamic_pointer_cast<Enum>( scope ) ) // <- for enum
            return RenderScopePrefix( e->identifier );    // omit scope for the enum itself
        else if( shared_ptr<Record> r = dynamic_pointer_cast<Record>( scope ) ) // <- for class, struct, union
            return RenderScopedIdentifier( r->identifier ) + "::";       
        else if( dynamic_pointer_cast<Procedure>( scope ) ||  // <- this is for params
                 dynamic_pointer_cast<Compound>( scope ) )    // <- this is for locals in body
            return string(); 
        else
            return ERROR_UNSUPPORTED( scope );
    }        
    
    string RenderScopedIdentifier( shared_ptr<Identifier> id )
    {
        string s = RenderScopePrefix( id ) + RenderIdentifier( id );
        TRACE("Render scoped identifier %s\n", s.c_str() );
        return s;
    }
    
    string RenderIntegralType( shared_ptr<Integral> type, string object=string() )
    {
        bool ds;
        unsigned width;       
        shared_ptr<SpecificInteger> ic = dynamic_pointer_cast<SpecificInteger>( type->width );
        ASSERT(ic && "width must be integer"); 
        width = ic->value.getLimitedValue();
                  
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

    string RenderFloatingType( shared_ptr<Floating> type )
    {
        string s;
        shared_ptr<SpecificFloatSemantics> sem = dynamic_pointer_cast<SpecificFloatSemantics>(type->semantics);
        ASSERT(sem);
    
        if( sem->value == TypeDb::float_semantics )
            s += "float";
        else if( sem->value == TypeDb::double_semantics )
            s += "double";
        else if( sem->value == TypeDb::long_double_semantics )
            s += "long double";
        else    
            ASSERT( !"no builtin floating type has required semantics"); // TODO drop in a bit vector
        
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
        else if( dynamic_pointer_cast< Boolean >(type) )
            return "bool" + sobject;
        else if( shared_ptr<Constructor> c = dynamic_pointer_cast< Constructor >(type) )
            return object + "(" + RenderDeclarationCollection(c, ", ", false) + ")";
        else if( shared_ptr<Destructor> f = dynamic_pointer_cast< Destructor >(type) )
            return object + "()";
        else if( shared_ptr<Function> f = dynamic_pointer_cast< Function >(type) )
            return RenderType( f->return_type, "(" + object + ")(" + RenderDeclarationCollection(f, ", ", false) + ")" );
        else if( shared_ptr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
            return RenderType( p->destination, "(*" + object + ")" );
        else if( shared_ptr<Reference> r = dynamic_pointer_cast< Reference >(type) )
            return RenderType( r->destination, "(&" + object + ")" );
        else if( shared_ptr<Array> a = dynamic_pointer_cast< Array >(type) )
            return RenderType( a->element, object.empty() ? "[" + RenderOperand(a->size) + "]" : "(" + object + "[" + RenderOperand(a->size) + "])" );
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(type) )
            return RenderIdentifier(t->identifier) + sobject;
        else if( shared_ptr<SpecificTypeIdentifier> ti = dynamic_pointer_cast< SpecificTypeIdentifier >(type) )
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
            sprintf( c, "\\x%02x", s[i] );
            o += c;
        }
        return o;
    }

    string RenderOperator( shared_ptr<Operator> op, Sequence<Expression> &operands )
    {
    	ASSERT(op);
        if( shared_ptr<ConditionalOperator> o = dynamic_pointer_cast< ConditionalOperator >(op) )
            return RenderOperand( operands[0], true ) + "?" +
                   RenderOperand( operands[1], true ) + ":" +
                   RenderOperand( operands[2], true );
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
    	else if( dynamic_pointer_cast<NODE>(op) ) \
			return RenderOperand( operands[0], true ) +\
				   TEXT +\
				   RenderOperand( operands[1], true );
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
    	else if( dynamic_pointer_cast<NODE>(op) ) \
			return TEXT +\
				   RenderOperand( operands[0], true );
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
    	else if( dynamic_pointer_cast<NODE>(op) ) \
			return RenderOperand( operands[0], true ) +\
				   TEXT;
#include "tree/operator_db.inc"
    	else
    		return ERROR_UNSUPPORTED(op);
    }

    string RenderCall( shared_ptr<Call> call )
    {
    	string s;

    	// Render the expression that resolves to the function name unless this is
    	// a constructor call in which case just the name of the thing being constructed.
        if( shared_ptr<Expression> base = TypeOf(program).IsConstructorCall( call ) )
            s += RenderOperand( base, true );
        else
        	s += RenderOperand( call->callee, true );

        s += "(";

        // If Procedure or Function, generate some arguments, resolving the order using the original function type
        shared_ptr<Type> ctype = TypeOf(program).Get( call->callee );
        ASSERT( ctype );
        if( shared_ptr<Procedure> proc = dynamic_pointer_cast<Procedure>(ctype) )
            s += RenderMapInOrder( call, proc, ", ", false );

        s += ")";
        return s;
    }

    string RenderOperand( shared_ptr<Initialiser> expression, bool bracketize_operator=false )
    {
        TRACE("%p\n", expression.get());
        
        string before = bracketize_operator ? "(" : "";
        string after = bracketize_operator ? ")" : "";
        
        if( dynamic_pointer_cast< Uninitialised >(expression) )
            return string();            
        else if( shared_ptr<SpecificLabelIdentifier> li = dynamic_pointer_cast< SpecificLabelIdentifier >(expression) )
            return before + 
                   "&&" + RenderIdentifier( li ) + // label-as-variable (GCC extension)
                   after;
        else if( shared_ptr<InstanceIdentifier> ii = dynamic_pointer_cast< InstanceIdentifier >(expression) )
            return RenderScopedIdentifier( ii );
        else if( shared_ptr<SizeOf> pot = dynamic_pointer_cast< SizeOf >(expression) )
            return before + 
                   "sizeof(" + RenderType( pot->operand, "" ) + ")" +
                   after;
        else if( shared_ptr<AlignOf> pot = dynamic_pointer_cast< AlignOf >(expression) )
            return before + 
                   "alignof(" + RenderType( pot->operand, "" ) + ")" +
                   after;
        else if( shared_ptr<NonCommutativeOperator> nco = dynamic_pointer_cast< NonCommutativeOperator >(expression) )
            return before +
                   RenderOperator( nco, nco->operands ) +
                   after;
        else if( shared_ptr<CommutativeOperator> co = dynamic_pointer_cast< CommutativeOperator >(expression) )
        {
        	Sequence<Expression> seq_operands;
        	// Operands are in collection, so move them to a container
            FOREACH( SharedPtr<Expression> o, co->operands )
          	    seq_operands.push_back( o );
            return before +
                   RenderOperator( co, seq_operands ) +
                   after;
        }
        else if( shared_ptr<Call> c = dynamic_pointer_cast< Call >(expression) )
            return before +
                   RenderCall( c ) +
                   after;
        else if( shared_ptr<New> n = dynamic_pointer_cast< New >(expression) )
            return before +
                   (dynamic_pointer_cast<Global>(n->global) ? "::" : "") +
                   "new(" + RenderOperandSequence( n->placement_arguments, ", ", false ) + ") " +
                   RenderType( n->type, "" ) + 
                   (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( n->constructor_arguments, ", ", false ) + ")" ) +
                   after;
        else if( shared_ptr<Delete> d = dynamic_pointer_cast< Delete >(expression) )
            return before +
                   (dynamic_pointer_cast<Global>(d->global) ? "::" : "") +
                   "delete" + 
                   (dynamic_pointer_cast<DeleteArray>(d->array) ? "[]" : "") +
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
                   RenderScopedIdentifier( a->member ) +
                   after;
        else if( shared_ptr<Cast> c = dynamic_pointer_cast< Cast >(expression) )
            return before + 
                   "(" + RenderType( c->type, "" ) + ")" +
                   RenderOperand( c->operand, false ) + 
                   after;
        else if( shared_ptr<ArrayLiteral> ao = dynamic_pointer_cast< ArrayLiteral >(expression) )
            return before + 
                   "{ " + RenderOperandSequence( ao->elements, ", ", false ) + " }" +
                   after;
        else if( shared_ptr<RecordLiteral> ro = dynamic_pointer_cast< RecordLiteral >(expression) )
            return before +
                   RenderRecordLiteral( ro ) +
                   after;
        else if( shared_ptr<Literal> l = dynamic_pointer_cast< Literal >(expression) )
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
    
    string RenderRecordLiteral( shared_ptr<RecordLiteral> ro )
    {
    	string s;

    	// Get the record
    	shared_ptr<TypeIdentifier> id = dynamic_pointer_cast<TypeIdentifier>(ro->type);
    	ASSERT(id);
    	shared_ptr<Record> r = GetRecordDeclaration(program, id);

    	s += "(";
    	s += RenderType( ro->type, "" );
    	s += "){ ";
        s += RenderMapInOrder( ro, r, ", ", false );
    	s += " }";
        return s;
    }

    string RenderMapInOrder( shared_ptr<MapOperator> ro,
    		                 shared_ptr<Scope> r,
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
    		if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
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
    			        	s += RenderOperand( mi->value );
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

    string RenderAccess( shared_ptr<AccessSpec> current_access )
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
    
    string RenderStorage( shared_ptr<StorageClass> st )
    {
        if( dynamic_pointer_cast<Program>( scope_stack.top() ) )
            return ""; // at top-level scope, everything is set to static, but don't actually output the word
        else if( dynamic_pointer_cast<Static>( st ) )
            return "static "; 
        else if( dynamic_pointer_cast<Auto>( st ) )
            return "auto "; 
        else if( dynamic_pointer_cast<Temp>( st ) )
            return "/*temp*/ "; 
        else if( shared_ptr<Member> ns = dynamic_pointer_cast<Member>( st ) )
        {
            shared_ptr<Virtuality> v = ns->virt;
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
        FOREACH( shared_ptr<Statement> s, body )
        {
            if( shared_ptr<Call> o = dynamic_pointer_cast< Call >(s) )
            {
                if( TypeOf(program).IsConstructorCall( o ) )
                {
                    inits.push_back(s);
                    continue;
                }
            }
            remainder.push_back(s);    
        }
    }
    
    string RenderInstance( shared_ptr<Instance> o, string sep, bool showtype = true, 
                           bool showstorage = true, bool showinit = true, bool showscope = false )
    {
        string s;
        
        ASSERT(o->type);
        
        if( showstorage )
        {
            if( dynamic_pointer_cast<Const>(o->constancy) )
                s += "const ";
            s += RenderStorage(o->storage);
        }
        
        string name;

        if( showscope )
            name = RenderScopePrefix(o->identifier);

        shared_ptr<Constructor> con = dynamic_pointer_cast<Constructor>(o->type);
        shared_ptr<Destructor> de = dynamic_pointer_cast<Destructor>(o->type);
        if( con || de )
        {
            shared_ptr<Record> rec = dynamic_pointer_cast<Record>( GetScope( program, o->identifier ) );
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
        else if( shared_ptr<Compound> comp = dynamic_pointer_cast<Compound>(o->initialiser) )
        {                                 
            // Render initialiser list then let RenderStatement() do the rest
            AutoPush< shared_ptr<Scope> > cs( scope_stack, GetScope( program, o->identifier ) );

            Sequence<Statement> inits;
            Sequence<Statement> remainder;
            ExtractInits( comp->statements, inits, remainder );
            if( !inits.empty() )
            {
                s += " : ";
                s += RenderSequence( inits, ", ", false, shared_ptr<Public>(), true );                
            }
            
            shared_ptr<Compound> r( new Compound );
            r->members = comp->members;
            r->statements = remainder;
            s += "\n" + RenderStatement(r, "");
        }
        else if( shared_ptr<Expression> ei = dynamic_pointer_cast<Expression>( o->initialiser ) )
        {
            // Render expression with an assignment
            AutoPush< shared_ptr<Scope> > cs( scope_stack, GetScope( program, o->identifier ) );
            s += " = " + RenderOperand(ei) + sep;
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
    bool ShouldSplitInstance( shared_ptr<Instance> o )
    {
        bool isfunc = !!dynamic_pointer_cast<Subroutine>( o->type );
        return dynamic_pointer_cast<Record>( scope_stack.top() ) &&
                   ( (dynamic_pointer_cast<Static>(o->storage) && dynamic_pointer_cast<NonConst>(o->constancy)) ||
                     isfunc );
    }
    
    string RenderDeclaration( shared_ptr<Declaration> declaration, string sep, shared_ptr<AccessSpec> *current_access = NULL, 
                              bool showtype = true, bool force_incomplete = false )
    {
        TRACE();
        string s;
        
        // Decide access spec for this declaration (explicit if physical, otherwise force to Public)
        shared_ptr<AccessSpec> this_access = shared_new<Public>();        
        if( shared_ptr<Physical> ph = dynamic_pointer_cast<Physical>(declaration) )
            this_access = ph->access;
                
        // Now decide whether we actually need to render an access spec (ie has it changed?)
        if( current_access && // NULL means dont ever render access specs
            TypeInfo(this_access) != TypeInfo(*current_access) ) // current_access spec must have changed
        {
            s += RenderAccess( this_access ) + ":\n";
            *current_access = this_access;
        }
                                         
        if( shared_ptr<Instance> o = dynamic_pointer_cast< Instance >(declaration) )
        {                
            if( ShouldSplitInstance(o) )
            {
                s += RenderInstance( o, sep, showtype, showtype, false, false );
                {
                    AutoPush< shared_ptr<Scope> > cs( scope_stack, program );
                    deferred_decls += string("\n") + RenderInstance( o, sep, showtype, false, true, true );
                }
            }
            else
            {
                // Otherwise, render everything directly using the default settings
                s += RenderInstance( o, sep, showtype, showtype, true, false );
            }
        }
        else if( shared_ptr<Typedef> t = dynamic_pointer_cast< Typedef >(declaration) )
        {
            s += "typedef " + RenderType( t->type, RenderIdentifier(t->identifier) ) + sep;
        }
        else if( shared_ptr<Record> r = dynamic_pointer_cast< Record >(declaration) )
        {
            shared_ptr<AccessSpec> a;
            bool showtype=true;
            string sep2=";\n";
            if( dynamic_pointer_cast< Class >(r) )
            {
                s += "class";
                a = shared_ptr<Private>(new Private);
            }
            else if( dynamic_pointer_cast< Struct >(r) )
            {
                s += "struct";
                a = shared_ptr<Public>(new Public);
            }
            else if( dynamic_pointer_cast< Union >(r) )
            {
                s += "union";
                a = shared_ptr<Public>(new Public);
            }
            else if( dynamic_pointer_cast< Enum >(r) )
            {
                s += "enum";
                a = shared_ptr<Public>(new Public);
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
                if( shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast< InheritanceRecord >(declaration) )
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
                AutoPush< shared_ptr<Scope> > cs( scope_stack, r );
                s += "\n{\n" +
                     RenderDeclarationCollection( r, sep2, true, a, showtype ) +
                     "}";
            }
            
            s += ";\n";
        }
        else if( shared_ptr<Label> l = dynamic_pointer_cast<Label>(declaration) )
            return RenderIdentifier(l->identifier) + ":\n"; // no ; after a label        
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
        //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
        if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(statement) )
            return RenderDeclaration( d, sep );
        else if( shared_ptr<Compound> c = dynamic_pointer_cast< Compound >(statement) )
        {
            AutoPush< shared_ptr<Scope> > cs( scope_stack, c );
            string s = "{\n";
            s += RenderDeclarationCollection( c, ";\n", true ); // Must do this first to populate backing list
            s += RenderSequence( c->statements, ";\n", true );
            return s + "}\n";
        }
        else if( shared_ptr<Expression> e = dynamic_pointer_cast< Expression >(statement) )
            return RenderOperand(e) + sep;
        else if( shared_ptr<Return> es = dynamic_pointer_cast<Return>(statement) )
            return "return " + RenderOperand(es->return_value) + sep;
        else if( shared_ptr<Goto> g = dynamic_pointer_cast<Goto>(statement) )
        {
            if( shared_ptr<SpecificLabelIdentifier> li = dynamic_pointer_cast< SpecificLabelIdentifier >(g->destination) )
                return "goto " + RenderIdentifier(li) + sep;  // regular goto
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
            if( !dynamic_pointer_cast<Nop>(i->else_body) )  // Nop means no else clause
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
                           bool separate_last,
                           shared_ptr<AccessSpec> init_access = shared_ptr<AccessSpec>(),
                           bool showtype=true )
    {
        TRACE();
        string s;
        for( int i=0; i<spe.size(); i++ )
        {
            TRACE("%d %p\n", i, &i);
            string sep = (separate_last || i+1<spe.size()) ? separator : "";
            shared_ptr<ELEMENT> pe = spe[i];                        
            if( shared_ptr<Declaration> d = dynamic_pointer_cast< Declaration >(pe) )
                s += RenderDeclaration( d, sep, init_access ? &init_access : NULL, showtype );
            else if( shared_ptr<Statement> st = dynamic_pointer_cast< Statement >(pe) )
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
            shared_ptr<Expression> pe = spe[i];                        
            s += RenderOperand( pe ) + sep;
        }
        return s;
    }
    
    string RenderDeclarationCollection( shared_ptr<Scope> sd,
			                            string separator, 
			                            bool separate_last,
			                            shared_ptr<AccessSpec> init_access = shared_ptr<AccessSpec>(),
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
            if( shared_ptr<Record> r = dynamic_pointer_cast<Record>(pd) ) // is a record
                if( !dynamic_pointer_cast<Enum>(r) ) // but not an enum 
                    s += RenderDeclaration( r, separator, init_access ? &init_access : NULL, showtype, true );
        
        s += RenderSequence( sorted, separator, separate_last, init_access, showtype );
        TRACE();
        return s;        
    }
};

#endif
