#ifndef ORDERING_HPP
#define ORDERING_HPP

class Orderer
{
    // Compare nodes for ordering. -ve for less, 0 for equal, +ve for greater
public:
    virtual int GetLocalCmp( const Orderer &other ) const
    {
        const char *tn = typeid( *this ).name(); 
        const char *on = typeid( other ).name(); 
        return strcmp(tn, on);
    }
};

#endif
