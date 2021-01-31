#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_7);
}
enum id_1
{
id_2 = 0U,
id_3 = 2U,
id_4 = 1U,
id_5 = 4U,
id_6 = 3U,
};
void id_7();
private:
unsigned int id_11;
};
id_0 id_13("id_13");
int id_10;
int id_12;
int id_9;

void id_0::id_7()
{
/*temp*/ bool id_8 = true;
if( (sc_delta_count())==(0U) )
{
 ::id_9=(1);
 ::id_10=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_11=((!( ::id_10<(4))) ?  ::id_0::id_4 :  ::id_0::id_2);
id_8=(false);
}
if( id_8 )
{
if(  ::id_0::id_2== ::id_0::id_11 )
{
 ::id_9+= ::id_10;
 ::id_12=(0);
 ::id_0::id_11=((!( ::id_12<(3))) ?  ::id_0::id_5 :  ::id_0::id_3);
}
}
if( id_8 )
{
if(  ::id_0::id_3== ::id_0::id_11 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_11= ::id_0::id_6;
id_8=(false);
}
}
if( id_8 )
{
if(  ::id_0::id_6== ::id_0::id_11 )
{
 ::id_9++;
 ::id_12++;
 ::id_0::id_11=(( ::id_12<(3)) ?  ::id_0::id_3 :  ::id_0::id_5);
}
}
if( id_8 )
{
if(  ::id_0::id_5== ::id_0::id_11 )
{
 ::id_9*=(2);
 ::id_10++;
 ::id_0::id_11=(( ::id_10<(4)) ?  ::id_0::id_2 :  ::id_0::id_4);
}
}
if( id_8 )
{
if(  ::id_0::id_4== ::id_0::id_11 )
{
cease(  ::id_9 );
id_8=(false);
}
}
if( id_8 )
next_trigger(SC_ZERO_TIME);
}
