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
void (recurser)();
int x;
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link1;
/*temp*/ void *recurser_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_i=temp_i;
{
 ::TopLevel::recurser_link=(&&LINK);
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
/*temp*/ void *temp_link;
auto int i;
auto void *link;
link= ::TopLevel::recurser_link;
i= ::TopLevel::recurser_i;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i1; temp_i1=((1)+i); {
{
 ::TopLevel::recurser_i=temp_i1;
{
 ::TopLevel::recurser_link=(&&LINK1);
 ::TopLevel::recurser();
}
}
LINK1:;
}
});
({ /*temp*/ int temp_i2; temp_i2=((1)+i); {
{
 ::TopLevel::recurser_i=temp_i2;
{
 ::TopLevel::recurser_link=(&&LINK2);
 ::TopLevel::recurser();
}
}
LINK2:;
}
});
}
{
temp_link=link;
return ;
}
}
