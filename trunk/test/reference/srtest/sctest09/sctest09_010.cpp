#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
void T();
/*temp*/ void *recurser_link;
void (recurser)(auto void *link, auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
 ::TopLevel::recurser(&&LINK, temp_i);
LINK:;
}
});
cease(  ::TopLevel::x );
return ;
}

void (TopLevel::recurser)(void *link, int i)
{
/*temp*/ void *temp_link;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i; temp_i=(i+(1)); {
 ::TopLevel::recurser(&&LINK, temp_i);
LINK:;
}
});
({ /*temp*/ int temp_i_1; temp_i_1=(i+(1)); {
 ::TopLevel::recurser(&&LINK_1, temp_i_1);
LINK_1:;
}
});
}
{
temp_link=link;
return ;
}
}
