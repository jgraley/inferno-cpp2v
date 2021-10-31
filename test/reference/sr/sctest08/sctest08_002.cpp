#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
}
id_0(auto char (*id_1));
void (id_15)();
void (id_4)();
void (id_9)(auto int id_8);
};
id_0 id_17("id_17");
void id_11;
int id_16;
int id_6;
int id_7;
void (id_12)(void id_10);
void (id_19)(void id_18);
void (id_5)(void id_3);
void (id_21)(void id_20);
void (id_23)(void id_22);
void (id_14)(void id_13);
void (id_26)(void id_24, void id_25);

id_0::id_0(char (*id_1))
{
 ::id_5( ::id_0::id_4);
}

void (id_0::id_15)()
{
 ::id_6=( ::id_6-(1));
}

void (id_0::id_4)()
{
 ::id_6=(1);
for(  ::id_7=(0);  ::id_7<(4);  ::id_7=((1)+ ::id_7) )
{
 ::id_6=( ::id_6+ ::id_7);
 ::id_0::id_9(3);
 ::id_6=((2)* ::id_6);
 ::id_12( ::id_11);
}
 ::id_14( ::id_6);
}

void (id_0::id_9)(int id_8)
{
for(  ::id_16=(0);  ::id_16<id_8;  ::id_16=((1)+ ::id_16) )
{
 ::id_12( ::id_11);
 ::id_6=((1)+ ::id_6);
}
 ::id_0::id_15();
return ;
}

void (id_12)(void id_10);

void (id_19)(void id_18);

void (id_5)(void id_3);

void (id_21)(void id_20);

void (id_23)(void id_22);

void (id_14)(void id_13);

void (id_26)(void id_24, void id_25);
