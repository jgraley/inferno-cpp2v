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
id_2 = 1,
id_3 = 3,
id_4 = 4,
id_5 = 5,
id_6 = 0,
id_7 = 2,
};
void id_8();
};
id_0 id_20("id_20");
int id_17;
int id_18;
int id_19;

void id_0::id_8()
{
static const unsigned int (id_9[]) = { &&id_10, &&id_11, &&id_12, &&id_13, &&id_14, &&id_15 };
auto unsigned int id_16;
 ::id_17=(1);
 ::id_18=(0);
wait(SC_ZERO_TIME);
id_16=((!( ::id_18<(4))) ?  ::id_0::id_5 :  ::id_0::id_3);
goto *(id_9[id_16]);
id_13:;
 ::id_17+= ::id_18;
 ::id_19=(0);
id_16=((!( ::id_19<(3))) ?  ::id_0::id_4 :  ::id_0::id_2);
goto *(id_9[id_16]);
id_11:;
wait(SC_ZERO_TIME);
id_16= ::id_0::id_7;
goto *(id_9[id_16]);
id_12:;
 ::id_17++;
 ::id_19++;
id_16=(( ::id_19<(3)) ?  ::id_0::id_2 :  ::id_0::id_4);
goto *(id_9[id_16]);
id_14:;
 ::id_17*=(2);
wait(SC_ZERO_TIME);
id_16= ::id_0::id_6;
goto *(id_9[id_16]);
id_10:;
 ::id_18++;
id_16=(( ::id_18<(4)) ?  ::id_0::id_3 :  ::id_0::id_5);
goto *(id_9[id_16]);
id_15:;
cease(  ::id_17 );
return ;
}
