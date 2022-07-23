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
id_2 = 0,
id_3 = 1,
id_4 = 2,
};
void id_5();
private:
int id_7;
unsigned int id_9;
};
id_0 id_10("id_10");
int id_6;
int id_8;

void id_0::id_5()
{
do
{
if( (sc_delta_count())==(0) )
{
 ::id_6=(1);
 ::id_0::id_7=(5);
 ::id_8=(0);
wait(SC_ZERO_TIME);
 ::id_0::id_9=((!( ::id_8< ::id_0::id_7)) ?  ::id_0::id_3 :  ::id_0::id_2);
continue;
}
if(  ::id_0::id_2== ::id_0::id_9 )
{
 ::id_6+= ::id_8;
wait(SC_ZERO_TIME);
 ::id_0::id_9= ::id_0::id_4;
continue;
}
if(  ::id_0::id_4== ::id_0::id_9 )
{
 ::id_6*=(2);
 ::id_8++;
 ::id_0::id_9=(( ::id_8< ::id_0::id_7) ?  ::id_0::id_2 :  ::id_0::id_3);
}
if(  ::id_0::id_3== ::id_0::id_9 )
{
cease(  ::id_6 );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}
