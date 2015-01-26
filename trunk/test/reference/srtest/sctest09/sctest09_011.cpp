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
/*temp*/ void *recurser_link;
/*temp*/ void *recurser_link1;
int x;
/*temp*/ int recurser_i;
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
/*temp*/ void *temp_link;
auto int i =  ::TopLevel::recurser_i;
auto void *link =  ::TopLevel::recurser_link;
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
({ /*temp*/ int temp_i1; temp_i1=(i+(1)); {
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
