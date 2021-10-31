#include "isystemc.h"

class id_0;
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
};
id_0 id_10("id_10");
void id_11;
int id_12 = 0;
int id_13 = 0;
void (id_15)(void id_14);
void (id_5)(void id_3);
void (id_17)(void id_16);
void (id_19)(void id_18);
void (id_9)(void id_8);
void (id_21)(void id_20);
void (id_24)(void id_22, void id_23);

id_0::id_0(char (*id_1))
{
 ::id_5( ::id_0::id_4);
}

void (id_0::id_4)()
{
 ::id_0::id_6++;
 ::id_0::id_7+= ::id_0::id_6;
if( (10)== ::id_0::id_6 )
 ::id_9( ::id_0::id_7);
next_trigger(SC_ZERO_TIME);
}

void (id_15)(void id_14);

void (id_5)(void id_3);

void (id_17)(void id_16);

void (id_19)(void id_18);

void (id_9)(void id_8);

void (id_21)(void id_20);

void (id_24)(void id_22, void id_23);
