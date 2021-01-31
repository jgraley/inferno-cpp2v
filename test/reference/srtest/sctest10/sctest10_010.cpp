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
void (id_14)(auto char id_11, auto short id_10, auto int id_9, auto void *id_12);
/*temp*/ int id_15;
/*temp*/ void *id_23;
};
id_0 id_24("id_24");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=({ /*temp*/ int id_3; id_3=(8); ({ /*temp*/ int id_4; id_4=(6); ({ /*temp*/ int id_5; id_5=({ /*temp*/ int id_6; id_6=(0); ({ /*temp*/ int id_7; id_7=(0); ({ /*temp*/ int id_8; id_8=(0); ({ {
 ::id_0::id_14(id_6, id_7, id_8, &&id_13);
id_13:;
}
 ::id_0::id_15; }); }); }); }); ({ {
 ::id_0::id_14(id_3, id_4, id_5, &&id_16);
id_16:;
}
 ::id_0::id_15; }); }); }); });
cease(  ::id_0::id_2+((2)*({ /*temp*/ int id_17; id_17=(3); ({ /*temp*/ int id_18; id_18=(2); ({ /*temp*/ int id_19; id_19=(1); ({ {
 ::id_0::id_14(id_17, id_18, id_19, &&id_20);
id_20:;
}
 ::id_0::id_15; }); }); }); })) );
return ;
}

void (id_0::id_14)(char id_11, short id_10, int id_9, void *id_12)
{
/*temp*/ void *id_21;
auto int id_22 = id_9+((3)*id_10);
{
 ::id_0::id_15=(id_22+((5)*id_11));
{
id_21=id_12;
return ;
}
}
}
