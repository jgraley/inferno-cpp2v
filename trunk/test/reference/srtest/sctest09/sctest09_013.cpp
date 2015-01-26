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
/*temp*/ void *recurser_link;
int x;
/*temp*/ void *recurser_link1;
void T();
void (recurser)();
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
auto void *link;
/*temp*/ void *temp_link;
auto int i;
i= ::TopLevel::recurser_i;
link= ::TopLevel::recurser_link;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i; temp_i=((1)+i); {
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
({ /*temp*/ int temp_i1; temp_i1=((1)+i); {
{
 ::TopLevel::recurser_link=(&&LINK1);
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
