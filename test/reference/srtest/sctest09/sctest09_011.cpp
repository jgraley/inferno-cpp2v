#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int recurser_i;
void (recurser)();
/*temp*/ void *recurser_link;
/*temp*/ void *recurser_link_1;
int x;
void T();
};
TopLevel top_level("top_level");

void (TopLevel::recurser)()
{
/*temp*/ void *temp_link;
auto int i =  ::TopLevel::recurser_i;
auto void *link =  ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i; temp_i=(i+(1)); {
{
 ::TopLevel::recurser_link_1=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser();
}
}
LINK:;
}
});
({ /*temp*/ int temp_i_1; temp_i_1=(i+(1)); {
{
 ::TopLevel::recurser_link_1=(&&LINK_1);
{
 ::TopLevel::recurser_i=temp_i_1;
 ::TopLevel::recurser();
}
}
LINK_1:;
}
});
}
{
temp_link=link;
return ;
}
}

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_link_1=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser();
}
}
LINK:;
}
});
cease(  ::TopLevel::x );
return ;
}
