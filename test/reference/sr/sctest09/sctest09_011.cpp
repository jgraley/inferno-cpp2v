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
id_0(auto char (*id_5));
int id_2;
void (id_4)(auto int id_3);
};
id_0 id_7("id_7");
void id_8;
void (id_10)(void id_9);
void (id_12)(void id_11);
void (id_14)(void id_13);
void (id_16)(void id_15);
void (id_18)(void id_17);
void (id_21)(void id_19, void id_20);

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_4(1);
cease(  ::id_0::id_2 );
}

id_0::id_0(char (*id_5))
{
}

void (id_0::id_4)(int id_3)
{
 ::id_0::id_2++;
if( id_3<(5) )
{
 ::id_0::id_4((1)+id_3);
 ::id_0::id_4((1)+id_3);
}
}

void (id_10)(void id_9);

void (id_12)(void id_11);

void (id_14)(void id_13);

void (id_16)(void id_15);

void (id_18)(void id_17);

void (id_21)(void id_19, void id_20);
