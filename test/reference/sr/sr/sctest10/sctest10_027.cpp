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
void (id_12)(auto char id_9, auto short id_8, auto int id_7, auto void *id_10);
/*temp*/ int id_13;
/*temp*/ void *id_23;
};
id_0 id_24("id_24");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=({ /*temp*/ int id_3; id_3=({ /*temp*/ int id_4; id_4=(0); ({ /*temp*/ int id_5; id_5=(0); ({ /*temp*/ int id_6; id_6=(0); ({ {
 ::id_0::id_12(id_6, id_5, id_4, &&id_11);
id_11:;
}
 ::id_0::id_13; }); }); }); }); ({ /*temp*/ int id_14; id_14=(6); ({ /*temp*/ int id_15; id_15=(8); ({ {
 ::id_0::id_12(id_15, id_14, id_3, &&id_16);
id_16:;
}
 ::id_0::id_13; }); }); }); });
cease(  ::id_0::id_2+((2)*({ /*temp*/ int id_17; id_17=(1); ({ /*temp*/ int id_18; id_18=(2); ({ /*temp*/ int id_19; id_19=(3); ({ {
 ::id_0::id_12(id_19, id_18, id_17, &&id_20);
id_20:;
}
 ::id_0::id_13; }); }); }); })) );
return ;
}

void (id_0::id_12)(char id_9, short id_8, int id_7, void *id_10)
{
/*temp*/ void *id_21;
auto int id_22 = id_7+((3)*id_8);
{
 ::id_0::id_13=(id_22+((5)*id_9));
{
id_21=id_10;
return ;
}
}
}
