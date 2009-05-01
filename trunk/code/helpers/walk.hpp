#ifndef WALK_HPP
#define WALK_HPP

#include "tree/tree.hpp"

class Walk
{
    struct Frame
    {
        vector< GenericSharedPtr * > children;
        int index;
    };
    
    stack< Frame > state;
    SharedPtr<Node> root;
    
    bool IsValid();
    void Iterate();
    void Push( shared_ptr<Node> n );
    void Pop();

public:
    Walk( shared_ptr<Node> r );        
    bool Done();
    int Depth();
    GenericSharedPtr *GetGeneric();
    shared_ptr<Node> Get();
    string Path();
    void Advance();
};


/*
    Example usage:
    
    Flattener<Record> w(program);
    FOREACH( shared_ptr<Record> r, w )
        printf("%s\n", typeid(*r).name() );
*/

template< class FIND >
class Flattener : public Sequence< FIND >
{
public:
    Flattener( shared_ptr<Node> root ) 
    {
        TRACE("Flattener\n");
        Walk w( root );
        while(!w.Done())
        {
            shared_ptr<Node> x = w.Get();
            if( shared_ptr<FIND> f = dynamic_pointer_cast< FIND >( x ) )
                Sequence< FIND >::push_back( f );
            w.Advance(); 
        }    
    }
};

#endif
