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
id_0 id_13("id_13");
int id_10;
int id_11;
int id_12;

void id_0::id_8()
{
auto unsigned int id_9;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_10=(1);
 ::id_11=(0);
wait(SC_ZERO_TIME);
id_9=((!( ::id_11<(4))) ?  ::id_0::id_4 :  ::id_0::id_2);
continue;
}
if(  ::id_0::id_2==id_9 )
{
 ::id_10+= ::id_11;
 ::id_12=(0);
id_9=((!( ::id_12<(3))) ?  ::id_0::id_5 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_9 )
{
wait(SC_ZERO_TIME);
id_9= ::id_0::id_6;
continue;
}
if(  ::id_0::id_6==id_9 )
{
 ::id_10++;
 ::id_12++;
id_9=(( ::id_12<(3)) ?  ::id_0::id_3 :  ::id_0::id_5);
}
if(  ::id_0::id_5==id_9 )
{
 ::id_10*=(2);
wait(SC_ZERO_TIME);
id_9= ::id_0::id_7;
continue;
}
if(  ::id_0::id_7==id_9 )
{
 ::id_11++;
id_9=(( ::id_11<(4)) ?  ::id_0::id_2 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_9 )
{
cease(  ::id_10 );
return ;
}
}
while( true );
}
