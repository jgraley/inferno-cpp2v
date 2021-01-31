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
id_2 = 0U,
id_3 = 2U,
id_4 = 1U,
id_5 = 4U,
id_6 = 3U,
};
void id_7();
};
id_0 id_14("id_14");
int id_11;
int id_12;
int id_13;

void id_0::id_7()
{
static const unsigned int (id_8[]) = { &&id_9, &&id_9, &&id_9, &&id_9, &&id_9 };
auto unsigned int id_10;
 ::id_11=(1);
 ::id_12=(0);
wait(SC_ZERO_TIME);
id_10=((!( ::id_12<(4))) ?  ::id_0::id_4 :  ::id_0::id_2);
id_9:;
if(  ::id_0::id_2==id_10 )
{
 ::id_11+= ::id_12;
 ::id_13=(0);
id_10=((!( ::id_13<(3))) ?  ::id_0::id_5 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_10 )
{
wait(SC_ZERO_TIME);
id_10= ::id_0::id_6;
goto *(id_8[id_10]);
}
if(  ::id_0::id_6==id_10 )
{
 ::id_11++;
 ::id_13++;
id_10=(( ::id_13<(3)) ?  ::id_0::id_3 :  ::id_0::id_5);
}
if(  ::id_0::id_5==id_10 )
{
 ::id_11*=(2);
 ::id_12++;
id_10=(( ::id_12<(4)) ?  ::id_0::id_2 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_10 )
{
cease(  ::id_11 );
return ;
}
goto *(id_8[id_10]);
}
