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
id_3 = 1U,
id_4 = 0U,
};
void id_5();
private:
unsigned int id_8;
unsigned int id_9;
public:
/*temp*/ unsigned int id_7;
};
id_0 id_10("id_10");

void id_0::id_5()
{
/*temp*/ bool id_6 = true;
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_7= ::id_0::id_4;
next_trigger(SC_ZERO_TIME);
 ::id_0::id_8= ::id_0::id_3;
id_6=(false);
}
if( id_6&&( ::id_0::id_4== ::id_0::id_8) )
{
id_6=(false);
if( id_6 )
 ::id_0::id_8= ::id_0::id_3;
}
if( id_6&&( ::id_0::id_3== ::id_0::id_8) )
{
 ::id_0::id_9= ::id_0::id_7;
 ::id_0::id_1= ::id_0::id_9;
 ::id_0::id_8= ::id_0::id_1;
}
if( id_6 )
next_trigger(SC_ZERO_TIME);
}
