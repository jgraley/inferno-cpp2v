#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_7);
}
enum id_1
{
id_2 = 4,
id_3 = 1,
id_4 = 3,
id_5 = 0,
id_6 = 2,
};
void id_7();
};
id_0 id_17("id_17");
int id_15;
int id_16;

void id_0::id_7()
{
static const unsigned int (id_8[]) = { &&id_9, &&id_10, &&id_11, &&id_12, &&id_13 };
auto unsigned int id_14;
 ::id_15=(1);
 ::id_16=(0);
wait(SC_ZERO_TIME);
id_14=((!( ::id_16<(5))) ?  ::id_0::id_4 :  ::id_0::id_3);
id_12:;
id_9:;
id_11:;
id_13:;
id_10:;
if(  ::id_0::id_3==id_14 )
{
 ::id_15+= ::id_16;
id_14=((!((0)==( ::id_16%(2)))) ?  ::id_0::id_5 :  ::id_0::id_2);
}
if(  ::id_0::id_2==id_14 )
{
wait(SC_ZERO_TIME);
id_14= ::id_0::id_6;
goto *(id_8[id_14]);
}
if(  ::id_0::id_6==id_14 )
{
 ::id_15^=(1);
id_14= ::id_0::id_5;
}
if(  ::id_0::id_5==id_14 )
{
 ::id_15*=(2);
 ::id_16++;
id_14=(( ::id_16<(5)) ?  ::id_0::id_3 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_14 )
{
cease(  ::id_15 );
return ;
}
goto *(id_8[id_14]);
}
