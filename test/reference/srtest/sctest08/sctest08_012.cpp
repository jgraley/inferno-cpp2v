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
void (id_15)();
void (id_8)();
/*temp*/ int id_7;
/*temp*/ void *id_13;
/*temp*/ void *id_18;
/*temp*/ void *id_19;
/*temp*/ void *id_5;
};
id_0 id_20("id_20");
int id_12;
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(4);  ::id_3=((1)+ ::id_3) )
{
 ::id_2=( ::id_2+ ::id_3);
({ /*temp*/ int id_4; id_4=(3); {
{
 ::id_0::id_5=(&&id_6);
{
 ::id_0::id_7=id_4;
 ::id_0::id_8();
}
}
id_6:;
}
});
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
cease(  ::id_2 );
return ;
}

void (id_0::id_15)()
{
/*temp*/ void *id_16;
auto void *id_17;
id_17= ::id_0::id_13;
 ::id_2=( ::id_2-(1));
{
id_16=id_17;
return ;
}
}

void (id_0::id_8)()
{
/*temp*/ void *id_9;
auto int id_10;
auto void *id_11;
id_10= ::id_0::id_7;
id_11= ::id_0::id_5;
for(  ::id_12=(0);  ::id_12<id_10;  ::id_12=((1)+ ::id_12) )
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
{
{
 ::id_0::id_13=(&&id_14);
 ::id_0::id_15();
}
id_14:;
}
{
id_9=id_11;
return ;
}
}
