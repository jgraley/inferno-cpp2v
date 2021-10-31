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
id_0(auto char (*id_7));
int id_2;
int (id_6)(auto char id_5, auto short id_4, auto int id_3);
};
id_0 id_10("id_10");
void id_11;
void (id_13)(void id_12);
void (id_15)(void id_14);
void (id_17)(void id_16);
void (id_19)(void id_18);

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=( ::id_0::id_6(8, 6,  ::id_0::id_6(0, 0, 0)));
cease(  ::id_0::id_2+((2)*( ::id_0::id_6(3, 2, 1))) );
}

id_0::id_0(char (*id_7))
{
}

int (id_0::id_6)(char id_5, short id_4, int id_3)
{
auto int id_9 = id_3+((3)*id_4);
return id_9+((5)*id_5);
}

void (id_13)(void id_12);

void (id_15)(void id_14);

void (id_17)(void id_16);

void (id_19)(void id_18);
