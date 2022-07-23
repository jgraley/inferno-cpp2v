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
id_0(auto char (*id_6));
void (id_8)();
void (id_5)(auto int id_4);
};
id_0 id_10("id_10");
void id_11;
int id_2;
int id_3;
int id_9;
void (id_13)(void id_12);
void (id_15)(void id_14);
void (id_17)(void id_16);
void (id_19)(void id_18);
void (id_21)(void id_20);
void (id_24)(void id_22, void id_23);

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(4);  ::id_3=((1)+ ::id_3) )
{
 ::id_2=( ::id_2+ ::id_3);
 ::id_0::id_5(3);
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
cease(  ::id_2 );
}

id_0::id_0(char (*id_6))
{
}

void (id_0::id_8)()
{
 ::id_2=( ::id_2-(1));
}

void (id_0::id_5)(int id_4)
{
for(  ::id_9=(0);  ::id_9<id_4;  ::id_9=((1)+ ::id_9) )
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
 ::id_0::id_8();
return ;
}

void (id_13)(void id_12);

void (id_15)(void id_14);

void (id_17)(void id_16);

void (id_19)(void id_18);

void (id_21)(void id_20);

void (id_24)(void id_22, void id_23);
