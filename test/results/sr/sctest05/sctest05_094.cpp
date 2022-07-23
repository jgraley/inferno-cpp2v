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
id_2 = 4,
id_3 = 1,
id_4 = 3,
id_5 = 0,
id_6 = 2,
};
void id_7();
private:
unsigned int id_11;
};
id_0 id_12("id_12");
int id_10;
int id_9;

void id_0::id_7()
{
/*temp*/ bool id_8 = true;
if( (sc_delta_count())==(0) )
{
 ::id_9=(1);
 ::id_10=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_11=((!( ::id_10<(5))) ?  ::id_0::id_4 :  ::id_0::id_3);
id_8=(false);
}
if( id_8 )
{
if(  ::id_0::id_3== ::id_0::id_11 )
{
 ::id_9+= ::id_10;
 ::id_0::id_11=((!((0)==( ::id_10%(2)))) ?  ::id_0::id_5 :  ::id_0::id_2);
}
}
if( id_8 )
{
if(  ::id_0::id_2== ::id_0::id_11 )
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
 ::id_9^=(1);
 ::id_0::id_11= ::id_0::id_5;
}
}
if( id_8 )
{
if(  ::id_0::id_5== ::id_0::id_11 )
{
 ::id_9*=(2);
 ::id_10++;
 ::id_0::id_11=(( ::id_10<(5)) ?  ::id_0::id_3 :  ::id_0::id_4);
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
