#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_8);
}
enum id_1
{
id_2 = 0U,
id_3 = 2U,
id_4 = 1U,
id_5 = 5U,
id_6 = 3U,
id_7 = 4U,
};
void id_8();
};
id_0 id_20("id_20");
int id_17;
int id_18;
int id_19;

void id_0::id_8()
{
static const void *(id_9[]) = { &&id_10, &&id_11, &&id_12, &&id_13, &&id_14, &&id_15 };
auto void *id_16;
 ::id_17=(1);
 ::id_18=(0);
wait(SC_ZERO_TIME);
{
id_16=((!( ::id_18<(4))) ? (id_9[ ::id_0::id_4]) : (id_9[ ::id_0::id_2]));
goto *(id_16);
}
id_10:;
 ::id_17+= ::id_18;
 ::id_19=(0);
{
id_16=((!( ::id_19<(3))) ? (id_9[ ::id_0::id_5]) : (id_9[ ::id_0::id_3]));
goto *(id_16);
}
id_12:;
wait(SC_ZERO_TIME);
{
id_16=(id_9[ ::id_0::id_6]);
goto *(id_16);
}
id_13:;
 ::id_17++;
 ::id_19++;
{
id_16=(( ::id_19<(3)) ? (id_9[ ::id_0::id_3]) : (id_9[ ::id_0::id_5]));
goto *(id_16);
}
id_15:;
 ::id_17*=(2);
wait(SC_ZERO_TIME);
{
id_16=(id_9[ ::id_0::id_7]);
goto *(id_16);
}
id_14:;
 ::id_18++;
{
id_16=(( ::id_18<(4)) ? (id_9[ ::id_0::id_2]) : (id_9[ ::id_0::id_4]));
goto *(id_16);
}
id_11:;
cease(  ::id_17 );
return ;
}
