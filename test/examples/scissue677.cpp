#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    int x;
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);        
    }
    void T()
    {
        int a[2];
        // TypeOf(&(a[1])) is pointer not appearing anywhere else.
        // It points to int, which does exist, in the line above.
        int r = &(a[1]) - &(a[0]);
        cease( r );
    }
};

TopLevel top_level("top_level");

