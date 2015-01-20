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
/*temp*/ void *recurser_link;
void T();
void (recurser)();
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_link=(&&LINK);
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

void (TopLevel::recurser)()
{
auto int i;
auto void *link;
/*temp*/ void *temp_link;
i= ::TopLevel::recurser_i;
link= ::TopLevel::recurser_link;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i; temp_i=(i+(1)); {
{
 ::TopLevel::recurser_link=(&&LINK);
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
 ::TopLevel::recurser_link=(&&LINK_1);
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
