#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_5);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_3 = 1,
id_4 = 0,
};
void id_5();
private:
unsigned int id_7;
unsigned int id_8;
public:
/*temp*/ unsigned int id_6;
};
id_0 id_9("id_9");

void id_0::id_5()
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_6= ::id_0::id_4;
next_trigger(SC_ZERO_TIME);
 ::id_0::id_7= ::id_0::id_3;
return ;
}
if(  ::id_0::id_4== ::id_0::id_7 )
{
return ;
 ::id_0::id_7= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_7 )
{
 ::id_0::id_8= ::id_0::id_6;
 ::id_0::id_1= ::id_0::id_8;
 ::id_0::id_7= ::id_0::id_1;
}
next_trigger(SC_ZERO_TIME);
}
