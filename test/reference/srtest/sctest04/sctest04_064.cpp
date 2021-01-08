#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_5);
}
enum id_1
{
id_2 = 0U,
id_3 = 2U,
id_4 = 1U,
};
void id_5();
};
id_0 id_13("id_13");
int id_11;
int id_12;

void id_0::id_5()
{
static const unsigned int (id_6[]) = { &&id_7, &&id_8, &&id_9 };
auto unsigned int id_10;
 ::id_11=(1);
 ::id_12=(0);
wait(SC_ZERO_TIME);
id_10=((!( ::id_12<(5))) ?  ::id_0::id_3 :  ::id_0::id_2);
id_9:;
id_8:;
id_7:;
if(  ::id_0::id_2==id_10 )
{
 ::id_11+= ::id_12;
wait(SC_ZERO_TIME);
id_10= ::id_0::id_4;
goto *(id_6[id_10]);
}
if(  ::id_0::id_4==id_10 )
{
 ::id_11*=(2);
 ::id_12++;
id_10=(( ::id_12<(5)) ?  ::id_0::id_2 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_10 )
{
cease(  ::id_11 );
return ;
}
goto *(id_6[id_10]);
}
