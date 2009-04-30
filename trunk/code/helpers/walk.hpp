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
    void Advance();
};





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

#define IET(I) if( TypeInfo(n) I TypeInfo(pp) ) printf(" " #I);

    static void WalkAndPrint( shared_ptr<Program> program )
    {
        Walk w( program );
        
        shared_ptr<Record> pp( new Record );
        
        int count=0;
        while(!w.Done())
        {
            shared_ptr<Node> n = w.Get();
            for( int i=0; i<w.Depth(); i++ )
                printf("    ");
            if(!n)
                printf("null");
            else if(shared_ptr<InstanceIdentifier> ii = dynamic_pointer_cast<InstanceIdentifier>(n) )
                printf("%s", ii->value.c_str());
            else if(shared_ptr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(n) )
                printf("type %s", ti->value.c_str());
            else if(shared_ptr<LabelIdentifier> li = dynamic_pointer_cast<LabelIdentifier>(n) )
                printf("%s:", li->value.c_str());
            else
                printf("%s", typeid(*n).name());
            
            if(n)
            {    
                IET(==) 
                IET(!=)
                IET(>)
                IET(<)
                IET(>=)
                IET(<=)
            }
                
            w.Advance(); 
            printf("\n");
            count++;
        }   
    }
};

#endif
