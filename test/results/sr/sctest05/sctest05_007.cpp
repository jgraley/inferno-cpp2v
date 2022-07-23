#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
}
id_0(auto char (*id_1));
void (id_4)();
};
id_0 id_10("id_10");
void id_11;
int id_6;
int id_7;
void (id_13)(void id_12);
void (id_15)(void id_14);
void (id_17)(void id_16);
void (id_9)(void id_8);
void (id_19)(void id_18);
void (id_5)(void id_3);
void (id_22)(void id_20, void id_21);

id_0::id_0(char (*id_1))
{
 ::id_5( ::id_0::id_4);
}

void (id_0::id_4)()
{
 ::id_6=(1);
for(  ::id_7=(0);  ::id_7<(5);  ::id_7++ )
{
 ::id_6+= ::id_7;
if( (0)==( ::id_7%(2)) )
{
wait(SC_ZERO_TIME);
 ::id_6^=(1);
}
 ::id_6*=(2);
}
 ::id_9( ::id_6);
}

void (id_13)(void id_12);

void (id_15)(void id_14);

void (id_17)(void id_16);

void (id_9)(void id_8);

void (id_19)(void id_18);

void (id_5)(void id_3);

void (id_22)(void id_20, void id_21);
