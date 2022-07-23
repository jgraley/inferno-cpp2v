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
int id_10;
};
id_0 id_12("id_12");
int id_11;
int id_9;

void id_0::id_5()
{
static const unsigned int (id_6[]) = { &&id_7, &&id_7, &&id_7 };
auto unsigned int id_8;
 ::id_9=(1);
 ::id_0::id_10=(5);
 ::id_11=(0);
wait(SC_ZERO_TIME);
id_8=((!( ::id_11< ::id_0::id_10)) ?  ::id_0::id_3 :  ::id_0::id_2);
id_7:;
if(  ::id_0::id_2==id_8 )
{
 ::id_9+= ::id_11;
wait(SC_ZERO_TIME);
id_8= ::id_0::id_4;
goto *(id_6[id_8]);
}
if(  ::id_0::id_4==id_8 )
{
 ::id_9*=(2);
 ::id_11++;
id_8=(( ::id_11< ::id_0::id_10) ?  ::id_0::id_2 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_8 )
{
cease(  ::id_9 );
return ;
}
goto *(id_6[id_8]);
}
