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
id_0(auto char (*id_4));
};
id_0 id_6("id_6");
void id_7;
int id_2;
int id_3;
void (id_11)(void id_10);
void (id_13)(void id_12);
void (id_15)(void id_14);
void (id_9)(void id_8);
void (id_17)(void id_16);
void (id_20)(void id_18, void id_19);

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(5);  ::id_3++ )
{
 ::id_2+= ::id_3;
if( (0)==( ::id_3%(2)) )
{
wait(SC_ZERO_TIME);
 ::id_2^=(1);
}
 ::id_2*=(2);
}
cease(  ::id_2 );
}

id_0::id_0(char (*id_4))
{
}

void (id_11)(void id_10);

void (id_13)(void id_12);

void (id_15)(void id_14);

void (id_9)(void id_8);

void (id_17)(void id_16);

void (id_20)(void id_18, void id_19);
