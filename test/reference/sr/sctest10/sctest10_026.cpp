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
int id_2;
void (id_10)(auto char id_9, auto short id_8, auto int id_7);
/*temp*/ int id_11;
};
id_0 id_18("id_18");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=({ /*temp*/ int id_3; id_3=({ /*temp*/ int id_4; id_4=(0); ({ /*temp*/ int id_5; id_5=(0); ({ /*temp*/ int id_6; id_6=(0); ({  ::id_0::id_10(id_6, id_5, id_4);  ::id_0::id_11; }); }); }); }); ({ /*temp*/ int id_12; id_12=(6); ({ /*temp*/ int id_13; id_13=(8); ({  ::id_0::id_10(id_13, id_12, id_3);  ::id_0::id_11; }); }); }); });
cease(  ::id_0::id_2+((2)*({ /*temp*/ int id_14; id_14=(1); ({ /*temp*/ int id_15; id_15=(2); ({ /*temp*/ int id_16; id_16=(3); ({  ::id_0::id_10(id_16, id_15, id_14);  ::id_0::id_11; }); }); }); })) );
return ;
}

void (id_0::id_10)(char id_9, short id_8, int id_7)
{
auto int id_17 = id_7+((3)*id_8);
{
 ::id_0::id_11=(id_17+((5)*id_9));
return ;
}
}
