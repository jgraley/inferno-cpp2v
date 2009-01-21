#ifndef WALK_HPP
#define WALK_HPP

#include "tree/tree.hpp"

/*
    Example usage:
    
    Walk<Record> w(program, false);
    for( int i=0; i<w.size(); i++ )
        printf("%s\n", typeid(*w[i]).name() );
*/

template< class FIND >
class Walk : public Sequence< FIND >
{
public:
    Walk( shared_ptr<Node> root, bool r = true ) :
        recurse(r)
    {
        TRACE("Walk\n");
        Visit( root );
    }
    
    template< class ELEMENT >    
    void Visit( const Sequence<ELEMENT> &seq )
    {
        for( int i=0; i<seq.size(); i++ )            
            Visit( seq[i] );   
    }
    
    void Visit( shared_ptr<Node> p )
    {
        if(!p)
            return;
        
        if( shared_ptr<FIND> f = dynamic_pointer_cast< FIND >( p ) )
        {
            Sequence< FIND >::push_back( f );
            if( !recurse )
                return;
        }
        
        if( shared_ptr< Sequence<Declaration> > sd = dynamic_pointer_cast< Sequence<Declaration> >( p ) )        
            Visit( *sd );           
        if( shared_ptr< Sequence<Statement> > ss = dynamic_pointer_cast< Sequence<Statement> >( p ) )
            Visit( *ss );            
        else if( shared_ptr< Object > o = dynamic_pointer_cast< Object >( p ) )
        {
            Visit( o->type );
            Visit( o->initialiser );
        }
        else if( shared_ptr< Record > r = dynamic_pointer_cast< Record >( p ) )
            Visit( r->members );
        else if( shared_ptr< Compound > c = dynamic_pointer_cast< Compound >( p ) )
            Visit( c->statements );
        else if( shared_ptr< Operator > o = dynamic_pointer_cast< Operator >( p ) )
            Visit( o->operands );
        else if( shared_ptr< Procedure > pr = dynamic_pointer_cast< Procedure >( p ) )
            Visit( pr->parameters );
        else if( shared_ptr< For > f = dynamic_pointer_cast< For >( p ) )
        {
            Visit( f->initialisation );
            Visit( f->condition );
            Visit( f->increment );
        }
        // TODO all the rest of these, or automate...
    }    
    
private:
    bool recurse;
};

#endif
