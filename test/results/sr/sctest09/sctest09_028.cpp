#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
int x;
void (recurser)(auto int i, auto void *link);
/*temp*/ void *recurser_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
 ::TopLevel::recurser(temp_i, &&LINK);
LINK:;
}
});
cease(  ::TopLevel::x );
return ;
}

void (TopLevel::recurser)(int i, void *link)
{
/*temp*/ void *temp_link;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i_1; temp_i_1=((1)+i); {
 ::TopLevel::recurser(temp_i_1, &&LINK_1);
LINK_1:;
}
});
({ /*temp*/ int temp_i_2; temp_i_2=((1)+i); {
 ::TopLevel::recurser(temp_i_2, &&LINK_2);
LINK_2:;
}
});
}
{
temp_link=link;
return ;
}
}