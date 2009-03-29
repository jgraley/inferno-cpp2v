#ifndef WALK_HPP
#define WALK_HPP

#include "tree/tree.hpp"

/*
class Walk
{
    enum Position
    {
        BEFORE,
        CHILDREN,
        AFTER
    };
    struct Frame
    {
        shared_ptr<Node> node;
        Position position;
        vector< Itemiser::ItemisableBase * > members;
        int member_index;
        int sequence_index;
    };
    stack< Frame > state;

    void Push( shared_ptr<Node> n )
    {
        Frame f;
        f.node = n;
        f.position = BEFORE;
        f.members = n->Itemise();
        f.member_index = 0;
        f.sequence_index = 0;
        state.push( f );
    }        

    void Pop()
    {
        state.pop();
    }

    void AdvanceMember( const Frame &f )
    {
        ASSERT( f.member_index < f.members.size() )
        if( f.member_index == f.members.size()-1 )                            
        {
            f.position = AFTER;
            f.member_index = 0;
        }
        else
        {
            f.member_index++;
        }
    }

    void AdvanceSequence( const Frame &f, const Sequence *seq )
    {
        ASSERT( f.sequence_index < seq->size() )
        if( f.sequence_index == seq->size()-1 )                            
        {
            AdvanceMember( f );
            f.sequence_index = 0;
        }
        else
        {
            f.sequence_index++;
        }
    }

public:
    Walk( shared_ptr<Node> root )
    {
        Push( root );
    }        
        
    shared_ptr<Node> Get()
    {
        if( stack.empty() )
            return shared_ptr<Node>(); // all done TODO could cause infinite loop, maybe should be an error
            
        Frame f = stack.top();
        switch( f.position )
        {
            case BEFORE:
            case AFTER:
                return f.node;
            case CHILDREN:
                if( Sequence *seq = dynamic_cast<Sequence *>(f.members[f.member_index]) )                
                    return (*seq)[f.sequence_index];
                else if( dynamic_cast<SharedPtr *>(f.members[f.member_index]) )         
                    return f.members[f.member_index];
                else
                    ASSERT(!"got something from itemise that isnt a sequence or a shared pointer");               
            default:
                ASSERT(!"corrupted state");
        }
    }

    void Advance()
    {
        if( stack.empty() )
            return;
            
        Frame &f = stack.top();
        switch( f.position )
        {
            case BEFORE:
                f.position = CHILDREN;
                break;
                
            case CHILDREN:
                if( Sequence *seq = dynamic_cast<Sequence *>(f.members[f.member_index]) )                
                    AdvanceSequence( f, seq );
                else if( dynamic_cast<SharedPtr *>(f.members[f.member_index]) )         
                    AdvanceMember( f );
                else
                    ASSERT(!"got something from itemise that isnt a sequence or a shared pointer");
                shared_ptr<Node> n = Get();
                if( n ) // pointers can be NULL in which case we don't recurse
                    Push( n );
                break;
                
            case AFTER:
                Pop();
                Advance();                
                break;
                
            default:
                ASSERT(!"corrupted state");
        }        
    }    
};

*/






/*
    Example usage:
    
    Flattener<Record> w(program, false);
    FOREACH( shared_ptr<Record> r, w )
        printf("%s\n", typeid(*r).name() );
*/

template< class FIND >
class Flattener : public Sequence< FIND >
{
public:
    Flattener( shared_ptr<Node> root, bool r = true ) :
        recurse(r)
    {
        TRACE("Flattener\n");
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
        else if( shared_ptr< Instance > o = dynamic_pointer_cast< Instance >( p ) )
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
