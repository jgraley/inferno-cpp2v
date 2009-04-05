#ifndef WALK_HPP
#define WALK_HPP

#include "tree/tree.hpp"

class Walk
{
    struct Frame
    {
        vector< shared_ptr<Node> > children;
        int index;
    };
    
    stack< Frame > state;

    bool IsValid()
    {
        if( state.empty() )
            return true;
    
        Frame &f = state.top();

        return f.index < f.children.size();                      
    }

    void Iterate()
    {
        Frame &f = state.top();
        if( state.empty() )
            return;

        if( f.index == f.children.size() )
        {
            Pop();
            if( state.empty() )
                return;
            Iterate();
            return;
        }

        f.index++;                
    }

    void Push( shared_ptr<Node> n )
    { 
        Frame f;

        vector< Itemiser::Itemisable * > members = n->Itemise();
        for( int i=0; i<members.size(); i++ )
        {
            if( GenericSequence *seq = dynamic_cast<GenericSequence *>(members[i]) )                
            {
                for( int j=0; j<seq->size(); j++ )
                    f.children.push_back( seq->Get(j) );
            }            
            else if( GenericPointer *ptr = dynamic_cast<GenericPointer *>(members[i]) )         
            {
                f.children.push_back( ptr->Get() );
            }
            else
            {
                ASSERT(!"got something from itemise that isnt a sequence or a shared pointer");               
            }
        }

        f.index = 0;
        state.push( f );
    }        

    void Pop()
    {
        state.pop();
    }

public:
    Walk( shared_ptr<Node> root )
    {
        Push( root );
    }        
        
    bool Done()
    {
        return state.empty();
    }    
    
    int Depth()
    {
        return state.size();
    }
        
    shared_ptr<Node> Get()
    {
        if( state.empty() )
            return shared_ptr<Node>(); // all done TODO could cause infinite loop, maybe should be an error
        
        ASSERT( IsValid() );
            
        Frame f = state.top();
        return f.children[f.index];
    }

    void Advance()
    {
        if( IsValid() && Get() )
            Push( Get() );
        else
            Iterate();
                    
        while( !IsValid() )
            Iterate();        
    }
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
};

#define IET(I) if( ITypeInfo(n) I ITypeInfo(pp) ) printf(" " #I);

void WalkAndPrint( shared_ptr<Program> program )
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

#endif
