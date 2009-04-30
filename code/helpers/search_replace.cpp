#include "search_replace.hpp"

bool SearchReplace::IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
    ASSERT( !!pattern ); // Disallow NULL pattern for now, could change this if required
    if( !x )
        return false; // NULL target allowed; never matches since pattern is not allwed to be NULL

    TRACE("%s >= %s??\n", TypeInfo(pattern).name().c_str(), TypeInfo(x).name().c_str() );

    // Is node correct class?
    if( !(TypeInfo(pattern) >= TypeInfo(x)) ) // Note >= is "non-strict superset" i.e. pattern is superclass of x or same class
    {
        TRACE("lol no!\n" );
        return false;
    }

    if( shared_ptr<String> pattern_str = dynamic_pointer_cast<String>(pattern) )
    {
        shared_ptr<String> x_str = dynamic_pointer_cast<String>(x);
        ASSERT( x_str );
        return x_str->value == pattern_str->value;
    }    
    else if( shared_ptr<Integer> pattern_int = dynamic_pointer_cast<Integer>(pattern) )
    {
        shared_ptr<Integer> x_int = dynamic_pointer_cast<Integer>(x);
        ASSERT( x_int );
        TRACE("%s %s\n", x_int->value.toString(10).c_str(), pattern_int->value.toString(10).c_str() );
        return x_int->value == pattern_int->value;
    }    
    else if( shared_ptr<Float> pattern_flt = dynamic_pointer_cast<Float>(pattern) )
    {
        shared_ptr<Float> x_flt = dynamic_pointer_cast<Float>(x);
        ASSERT( x_flt );
        return x_flt->value.bitwiseIsEqual( pattern_flt->value );
    }
    else // node is standard pattern, so recurse children (or we have a match)
    {
        vector< Itemiser::Element * > pattern_memb = Itemiser::Itemise( pattern.get() ); 
        vector< Itemiser::Element * > x_memb = Itemiser::Itemise( x.get(),           // The thing we're itemising
                                                                  pattern.get() );   // Just get the members corresponding to pattern's class
        ASSERT( pattern_memb.size() == x_memb.size() );
        
        for( int i=0; i<pattern_memb.size(); i++ )
        {
            ASSERT( pattern_memb[i] && "itemise returned null element");
            ASSERT( x_memb[i] && "itemise returned null element");
            
            if( GenericSequence *pattern_seq = dynamic_cast<GenericSequence *>(pattern_memb[i]) )                
            {
                GenericSequence *x_seq = dynamic_cast<GenericSequence *>(x_memb[i]);
                ASSERT( x_seq && "itemise for target didn't match itemise for pattern");
                
                if( x_seq->size() != pattern_seq->size() )
                    return false;
                
                for( int j=0; j<pattern_seq->size(); j++ )
                {
                    bool match = IsMatchPattern( x_seq->Element(j).Get(), pattern_seq->Element(j).Get() );
                    if( !match )
                        return false;
                }
            }            
            else if( GenericSharedPtr *pattern_ptr = dynamic_cast<GenericSharedPtr *>(pattern_memb[i]) )         
            {
                GenericSharedPtr *x_ptr = dynamic_cast<GenericSharedPtr *>(x_memb[i]);
                ASSERT( x_ptr && "itemise for target didn't match itemise for pattern");
                if( !!x_ptr->Get() != !!pattern_ptr->Get() )
                    return false;
                    
                bool match = IsMatchPattern( x_ptr->Get(), pattern_ptr->Get() );
                if( !match )
                    return false;                     
            }
            else
            {
                ASSERT(!"got something from itemise that isnt a sequence or a shared pointer");               
            }
        }       
        return true;
    }
}


GenericSharedPtr *SearchReplace::Search( shared_ptr<Node> program )
{
    Walk w( program );
    while(!w.Done())
    {
        shared_ptr<Node> x = w.Get();
        if( IsMatchPattern( x, search_pattern ) )
            return w.GetGeneric();                            
        w.Advance(); 
    }    
    
    return NULL;
}

shared_ptr<Node> SearchReplace::DuplicateSubtree( shared_ptr<Node> source )
{
    ASSERT( source );
    shared_ptr<Duplicator> dup_dest = Duplicator::Duplicate( source );
    shared_ptr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    ASSERT(dest);
    
    vector< Itemiser::Element * > source_memb = Itemiser::Itemise( source.get() ); 
    vector< Itemiser::Element * > dest_memb = Itemiser::Itemise( dest.get() );
    ASSERT( source_memb.size() == dest_memb.size() ); // required to be same actual type
    
    for( int i=0; i<source_memb.size(); i++ )
    {
        ASSERT( source_memb[i] && "itemise returned null element" );
        ASSERT( dest_memb[i] && "itemise returned null element" );
        
        if( GenericSequence *source_seq = dynamic_cast<GenericSequence *>(source_memb[i]) )                
        {
            GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]);
            ASSERT( dest_seq && "itemise for dest didn't match itemise for source");
            
            for( int j=0; j<source_seq->size(); j++ )
                dest_seq->Element(j).Set( DuplicateSubtree( source_seq->Element(j).Get() ) );
        }            
        else if( GenericSharedPtr *source_ptr = dynamic_cast<GenericSharedPtr *>(source_memb[i]) )         
        {
            GenericSharedPtr *dest_ptr = dynamic_cast<GenericSharedPtr *>(dest_memb[i]);
            ASSERT( dest_ptr && "itemise for target didn't match itemise for source");
            if( source_ptr->Get() )
                dest_ptr->Set( DuplicateSubtree( source_ptr->Get() ) );
            else
                dest_ptr->Set( shared_ptr<Node>() );
        }
        else
        {
            ASSERT(!"got something from itemise that isnt a sequence or a shared pointer");               
        }
    }       

    return dest;
}

void SearchReplace::Replace( GenericSharedPtr *target )
{
    target->Set( DuplicateSubtree( replace_pattern ) );
}

void SearchReplace::operator()( shared_ptr<Node> program )
{
    while(1)
    {
        GenericSharedPtr *gp = Search( program );        
        if( gp )
            Replace( gp );
        else
            break;
    }
}


void SearchReplace::Test()
{
    {
        // single node with topological wildcarding
        shared_ptr<Void> v(new Void);
        ASSERT( SearchReplace::IsMatchPattern( v, v ) == true );
        shared_ptr<Bool> b(new Bool);
        ASSERT( SearchReplace::IsMatchPattern( v, b ) == false );
        ASSERT( SearchReplace::IsMatchPattern( b, v ) == false );
        shared_ptr<Type> t(new Type);
        ASSERT( SearchReplace::IsMatchPattern( v, t ) == true );
        ASSERT( SearchReplace::IsMatchPattern( t, v ) == false );
        ASSERT( SearchReplace::IsMatchPattern( b, t ) == true );
        ASSERT( SearchReplace::IsMatchPattern( t, b ) == false );
        
        // node points directly to another with TC
        shared_ptr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( SearchReplace::IsMatchPattern( p1, b ) == false );
        ASSERT( SearchReplace::IsMatchPattern( p1, p1 ) == true );
        shared_ptr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( SearchReplace::IsMatchPattern( p1, p2 ) == false );
        p2->destination = t;
        ASSERT( SearchReplace::IsMatchPattern( p1, p2 ) == true );
        ASSERT( SearchReplace::IsMatchPattern( p2, p1 ) == false );
    }
    
    {
        // string property
        shared_ptr<String> s1( new String );
        shared_ptr<String> s2( new String );
        s1->value = "here";
        s2->value = "there";
        ASSERT( SearchReplace::IsMatchPattern( s1, s1 ) == true );
        ASSERT( SearchReplace::IsMatchPattern( s1, s2 ) == false );        
    }    
    
    {
        // int property
        llvm::APSInt apsint( 32, true );
        shared_ptr<Integer> i1( new Integer );
        shared_ptr<Integer> i2( new Integer );
        apsint = 3;
        i1->value = apsint;
        apsint = 5;
        i2->value = apsint;
        TRACE("  %s %s\n", i1->value.toString(10).c_str(), i2->value.toString(10).c_str() );
        ASSERT( SearchReplace::IsMatchPattern( i1, i1 ) == true );
        ASSERT( SearchReplace::IsMatchPattern( i1, i2 ) == false );        
    }    
    
    {
        // node with sequence, check lengths 
        shared_ptr<Compound> c1( new Compound );
        ASSERT( SearchReplace::IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( SearchReplace::IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( SearchReplace::IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Compound> c2( new Compound );
        ASSERT( SearchReplace::IsMatchPattern( c1, c2 ) == false );
        shared_ptr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( SearchReplace::IsMatchPattern( c1, c2 ) == false );
        shared_ptr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( SearchReplace::IsMatchPattern( c1, c2 ) == true );        
        shared_ptr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( SearchReplace::IsMatchPattern( c1, c2 ) == false );        
    }

    {
        // node with sequence, TW 
        shared_ptr<Compound> c1( new Compound );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        shared_ptr<Compound> c2( new Compound );
        shared_ptr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( SearchReplace::IsMatchPattern( c1, c2 ) == true );
        ASSERT( SearchReplace::IsMatchPattern( c2, c1 ) == false );
    }
    
    {        
        // topological with extra member in terget node
        shared_ptr<Label> l( new Label );
        shared_ptr<Public> p1( new Public );
        l->access = p1;
        shared_ptr<LabelIdentifier> li( new LabelIdentifier );
        li->value = "mylabel";
        l->identifier = li;
        shared_ptr<Declaration> d( new Declaration );
        shared_ptr<Public> p2( new Public );
        d->access = p2;
        ASSERT( SearchReplace::IsMatchPattern( l, d ) == true );
        ASSERT( SearchReplace::IsMatchPattern( d, l ) == false );
        shared_ptr<Private> p3( new Private );
        d->access = p3;
        ASSERT( SearchReplace::IsMatchPattern( l, d ) == false );
        ASSERT( SearchReplace::IsMatchPattern( d, l ) == false );
        shared_ptr<AccessSpec> p4( new AccessSpec );
        d->access = p4;
        ASSERT( SearchReplace::IsMatchPattern( l, d ) == true );
        ASSERT( SearchReplace::IsMatchPattern( d, l ) == false );
    }
}


