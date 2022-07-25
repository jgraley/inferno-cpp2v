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
void (recurser)();
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link;
/*temp*/ void *recurser_link_1;
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
auto void *link =  ::TopLevel::recurser_link;
auto int i =  ::TopLevel::recurser_i;
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i_1; temp_i_1=((1)+i); {
{
 ::TopLevel::recurser_i=temp_i_1;
{
 ::TopLevel::recurser_link=(&&LINK_1);
 ::TopLevel::recurser();
}
}
LINK_1:;
}
});
({ /*temp*/ int temp_i_2; temp_i_2=((1)+i); {
{
 ::TopLevel::recurser_i=temp_i_2;
{
 ::TopLevel::recurser_link=(&&LINK_2);
 ::TopLevel::recurser();
}
}
LINK_2:;
}
});
}
{
temp_link=link;
return ;
}
}
