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
int (id_12)(auto char id_11, auto short id_10, auto int id_9);
};
id_0 id_17("id_17");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=({ /*temp*/ int id_3; id_3=(8); ({ /*temp*/ int id_4; id_4=(6); ({ /*temp*/ int id_5; id_5=({ /*temp*/ int id_6; id_6=(0); ({ /*temp*/ int id_7; id_7=(0); ({ /*temp*/ int id_8; id_8=(0);  ::id_0::id_12(id_6, id_7, id_8); }); }); });  ::id_0::id_12(id_3, id_4, id_5); }); }); });
cease(  ::id_0::id_2+((2)*({ /*temp*/ int id_13; id_13=(3); ({ /*temp*/ int id_14; id_14=(2); ({ /*temp*/ int id_15; id_15=(1);  ::id_0::id_12(id_13, id_14, id_15); }); }); })) );
}

int (id_0::id_12)(char id_11, short id_10, int id_9)
{
auto int id_16 = id_9+((3)*id_10);
return id_16+((5)*id_11);
}
