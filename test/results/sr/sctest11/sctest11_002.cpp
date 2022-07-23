#include "isystemc.h"

class id_0;
class id_12;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
}
id_0(auto char (*id_1));
int id_6;
int id_7;
void (id_4)();
int (id_9)(auto int id_8);
};
class id_12
{
};
id_0 id_13("id_13");
void id_14;
void (id_11)(void id_10);
void (id_16)(void id_15);
void (id_18)(void id_17);
void (id_20)(void id_19);
void (id_22)(void id_21);
void (id_5)(void id_3);
void (id_25)(void id_23, void id_24);

id_0::id_0(char (*id_1))
{
 ::id_5( ::id_0::id_4);
}

void (id_0::id_4)()
{
 ::id_0::id_6=(4);
(++ ::id_0::id_6)&&( ::id_0::id_7=( ::id_0::id_9( ::id_0::id_6)));
(!(++ ::id_0::id_6))||( ::id_0::id_7+=( ::id_0::id_9( ::id_0::id_6)));
for(  ::id_0::id_6=(0);  ::id_0::id_6<(2);  )
( ::id_0::id_6++) ? ( ::id_0::id_7+=( ::id_0::id_9( ::id_0::id_6))) : ( ::id_0::id_7-=( ::id_0::id_9( ::id_0::id_6)));
 ::id_11( ::id_0::id_7);
}

int (id_0::id_9)(int id_8)
{
return (100)/id_8;
}

void (id_11)(void id_10);

void (id_16)(void id_15);

void (id_18)(void id_17);

void (id_20)(void id_19);

void (id_22)(void id_21);

void (id_5)(void id_3);

void (id_25)(void id_23, void id_24);
