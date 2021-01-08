#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_1);
}
void id_1();
void (id_7)();
void (id_6)(auto int id_5);
};
id_0 id_9("id_9");
int id_2;
int id_3;
int id_8;

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(4);  ::id_3=((1)+ ::id_3) )
{
 ::id_2=( ::id_2+ ::id_3);
({ /*temp*/ int id_4; id_4=(3);  ::id_0::id_6(id_4); });
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
cease(  ::id_2 );
return ;
}

void (id_0::id_7)()
{
 ::id_2=( ::id_2-(1));
return ;
}

void (id_0::id_6)(int id_5)
{
for(  ::id_8=(0);  ::id_8<id_5;  ::id_8=((1)+ ::id_8) )
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
 ::id_0::id_7();
return ;
}
