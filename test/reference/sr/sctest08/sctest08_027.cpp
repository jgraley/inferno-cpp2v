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
void (id_10)(auto void *id_9);
void (id_8)(auto int id_7, auto void *id_5);
/*temp*/ void *id_15;
/*temp*/ void *id_16;
};
id_0 id_17("id_17");
int id_13;
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(4);  ::id_3=((1)+ ::id_3) )
{
 ::id_2=( ::id_2+ ::id_3);
({ /*temp*/ int id_4; id_4=(3); {
 ::id_0::id_8(id_4, &&id_6);
id_6:;
}
});
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
cease(  ::id_2 );
return ;
}

void (id_0::id_10)(void *id_9)
{
/*temp*/ void *id_11;
 ::id_2=( ::id_2-(1));
{
id_11=id_9;
return ;
}
}

void (id_0::id_8)(int id_7, void *id_5)
{
/*temp*/ void *id_12;
for(  ::id_13=(0);  ::id_13<id_7;  ::id_13=((1)+ ::id_13) )
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
{
 ::id_0::id_10(&&id_14);
id_14:;
}
{
id_12=id_5;
return ;
}
}
