#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_8);
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
private:
unsigned int id_12;
};
id_0 id_14("id_14");
int id_10;
int id_11;
int id_13;

void id_0::id_8()
{
/*temp*/ bool id_9 = true;
if( (sc_delta_count())==(0) )
{
 ::id_10=(1);
 ::id_11=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_12=((!( ::id_11<(4))) ?  ::id_0::id_5 :  ::id_0::id_3);
id_9=(false);
}
if( id_9&&( ::id_0::id_3== ::id_0::id_12) )
{
 ::id_10+= ::id_11;
 ::id_13=(0);
 ::id_0::id_12=((!( ::id_13<(3))) ?  ::id_0::id_4 :  ::id_0::id_2);
}
if( id_9&&( ::id_0::id_2== ::id_0::id_12) )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_12= ::id_0::id_7;
id_9=(false);
}
if( id_9&&( ::id_0::id_7== ::id_0::id_12) )
{
 ::id_10++;
 ::id_13++;
 ::id_0::id_12=(( ::id_13<(3)) ?  ::id_0::id_2 :  ::id_0::id_4);
}
if( id_9&&( ::id_0::id_4== ::id_0::id_12) )
{
 ::id_10*=(2);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_12= ::id_0::id_6;
id_9=(false);
}
if( id_9&&( ::id_0::id_6== ::id_0::id_12) )
{
 ::id_11++;
 ::id_0::id_12=(( ::id_11<(4)) ?  ::id_0::id_3 :  ::id_0::id_5);
}
if( id_9&&( ::id_0::id_5== ::id_0::id_12) )
{
cease(  ::id_10 );
id_9=(false);
}
if( id_9 )
next_trigger(SC_ZERO_TIME);
}
