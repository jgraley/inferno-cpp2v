
// Reverse template
template<typename TARGET>
class RCTarget<TARGET>
{
public:
    RCTarget() :
        count(1), // You get a free ref on constructions (assuming local)
        ptr(NULL)
    {
    }

private: friend class RCPtr
    int count;
    TARGET *target_this; // not guaranteed to == this
    
    ~RCTarget()    
    {
        switch( count )
        {
            case 0:
            // Was spawned, and the last pointer was destructed (dynamic destruct)
            return;
            
            case 1:
            // Was not spawned, reached the end of local scope, no pointers created or all were deleted
            return;
            
            default:
            // Was not spawned, reached the end of local scope, pointers still in existence 
            // so spawn now using dynamic allocation
            TARGET *p = new TARGET(*target_this);
            p->count = count-1;    // You lose a ref on destruction (assuming local)
        }
    }
};

template<typename TARGET>
class RCPtr<TARGET>
{
public:
    RCPtr() :
        ptr( NULL )
    {
    }

    RCPtr( const RCPtr &o )
    {
        ptr = o.ptr;
        if( ptr )
            ptr->count++;
    }

    operator =(TARGET *t)
    {
        ptr = t;
        if( ptr )
        {
            ptr->target_this = ptr;
            ptr->count++;
        }
    }
    
    ~RCPtr()
    {
        if( ptr )
        {
            ptr->count--;
            if( ptr->count==0 )
                delete ptr;
        }
    }
    
    operator ->()
    {
        return ptr;
    }

private:
    TARGET *ptr;
};
