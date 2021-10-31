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
id_0 id_11("id_11");
int id_10;
int id_9;

void id_0::id_5()
{
static const unsigned int (id_6[]) = { &&id_7, &&id_7, &&id_7 };
auto unsigned int id_8;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_9=(1);
 ::id_10=(0);
wait(SC_ZERO_TIME);
id_8=((!( ::id_10<(5))) ?  ::id_0::id_3 :  ::id_0::id_2);
continue;
}
if(  ::id_0::id_2==id_8 )
{
 ::id_9+= ::id_10;
wait(SC_ZERO_TIME);
id_8= ::id_0::id_4;
continue;
}
if(  ::id_0::id_4==id_8 )
{
 ::id_9*=(2);
 ::id_10++;
id_8=(( ::id_10<(5)) ?  ::id_0::id_2 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_8 )
{
cease(  ::id_9 );
return ;
}
}
while( true );
}
