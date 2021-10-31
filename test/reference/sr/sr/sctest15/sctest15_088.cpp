#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_4);
}
enum id_1
{
id_2 = 1U,
id_3 = 0U,
};
void id_4();
private:
unsigned int id_8;
public:
/*temp*/ unsigned int id_7;
};
id_0 id_9("id_9");

void id_0::id_4()
{
/*temp*/ unsigned int id_5;
auto unsigned int id_6;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_7= ::id_0::id_3;
wait(SC_ZERO_TIME);
id_6= ::id_0::id_2;
continue;
}
if(  ::id_0::id_3==id_6 )
{
return ;
id_6= ::id_0::id_2;
}
if(  ::id_0::id_2==id_6 )
{
 ::id_0::id_8= ::id_0::id_7;
id_5= ::id_0::id_8;
id_6=id_5;
}
wait(SC_ZERO_TIME);
}
while( true );
}
