#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *recurser_link;
void (recurser)();
int x;
void T();
/*temp*/ void *recurser_link1;
/*temp*/ int recurser_i;
};
TopLevel top_level("top_level");

void (TopLevel::recurser)()
{
auto void *link;
/*temp*/ void *temp_link;
auto int i;
i= ::TopLevel::recurser_i;
link= ::TopLevel::recurser_link1;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i; temp_i=(i+(1)); {
{
 ::TopLevel::recurser_link1=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser();
}
}
LINK:;
}
});
({ /*temp*/ int temp_i1; temp_i1=(i+(1)); {
{
 ::TopLevel::recurser_link1=(&&LINK1);
{
 ::TopLevel::recurser_i=temp_i1;
 ::TopLevel::recurser();
}
}
LINK1:;
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
 ::TopLevel::recurser_link1=(&&LINK);
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
