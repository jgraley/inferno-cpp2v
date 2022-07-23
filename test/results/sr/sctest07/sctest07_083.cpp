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
id_0 id_15("id_15");
int id_12;
int id_13;
int id_14;

void id_0::id_8()
{
static const unsigned int (id_9[]) = { &&id_10, &&id_10, &&id_10, &&id_10, &&id_10, &&id_10 };
auto unsigned int id_11;
 ::id_12=(1);
 ::id_13=(0);
wait(SC_ZERO_TIME);
id_11=((!( ::id_13<(4))) ?  ::id_0::id_5 :  ::id_0::id_3);
id_10:;
if(  ::id_0::id_3==id_11 )
{
 ::id_12+= ::id_13;
 ::id_14=(0);
id_11=((!( ::id_14<(3))) ?  ::id_0::id_4 :  ::id_0::id_2);
}
if(  ::id_0::id_2==id_11 )
{
wait(SC_ZERO_TIME);
id_11= ::id_0::id_7;
goto *(id_9[id_11]);
}
if(  ::id_0::id_7==id_11 )
{
 ::id_12++;
 ::id_14++;
id_11=(( ::id_14<(3)) ?  ::id_0::id_2 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_11 )
{
 ::id_12*=(2);
wait(SC_ZERO_TIME);
id_11= ::id_0::id_6;
goto *(id_9[id_11]);
}
if(  ::id_0::id_6==id_11 )
{
 ::id_13++;
id_11=(( ::id_13<(4)) ?  ::id_0::id_3 :  ::id_0::id_5);
}
if(  ::id_0::id_5==id_11 )
{
cease(  ::id_12 );
return ;
}
goto *(id_9[id_11]);
}
