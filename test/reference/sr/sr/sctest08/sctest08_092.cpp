#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_21(0U)
{
SC_METHOD(id_14);
}
/*temp*/ unsigned int id_1;
/*temp*/ unsigned int id_2;
enum id_3
{
id_10 = 3U,
id_11 = 7U,
id_12 = 2U,
id_13 = 6U,
id_4 = 4U,
id_5 = 9U,
id_6 = 1U,
id_7 = 8U,
id_8 = 0U,
id_9 = 5U,
};
void id_14();
private:
int (id_23[10U]);
unsigned int (id_22[10U]);
unsigned int id_17;
unsigned int id_21;
unsigned int id_26;
public:
/*temp*/ int id_18;
/*temp*/ int id_19;
/*temp*/ unsigned int id_20;
/*temp*/ unsigned int id_25;
};
id_0 id_27("id_27");
int id_15;
int id_16;
int id_24;

void id_0::id_14()
{
if( (sc_delta_count())==(0U) )
{
 ::id_15=(1);
 ::id_16=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_17=((!( ::id_16<(4))) ?  ::id_0::id_10 :  ::id_0::id_8);
return ;
}
if(  ::id_0::id_8== ::id_0::id_17 )
{
 ::id_15=( ::id_15+ ::id_16);
 ::id_0::id_18=(3);
 ::id_0::id_19= ::id_0::id_18;
 ::id_0::id_20= ::id_0::id_6;
 ::id_0::id_17= ::id_0::id_4;
}
if(  ::id_0::id_6== ::id_0::id_17 )
{
 ::id_15=((2)* ::id_15);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_17= ::id_0::id_12;
return ;
}
if(  ::id_0::id_12== ::id_0::id_17 )
{
 ::id_16=((1)+ ::id_16);
 ::id_0::id_17=(( ::id_16<(4)) ?  ::id_0::id_8 :  ::id_0::id_10);
}
if(  ::id_0::id_10== ::id_0::id_17 )
{
cease(  ::id_15 );
return ;
 ::id_0::id_17= ::id_0::id_4;
}
if(  ::id_0::id_4== ::id_0::id_17 )
{
 ::id_0::id_21++;
( ::id_0::id_22[ ::id_0::id_21])= ::id_0::id_20;
( ::id_0::id_23[ ::id_0::id_21])= ::id_0::id_19;
 ::id_24=(0);
 ::id_0::id_17=((!( ::id_24<( ::id_0::id_23[ ::id_0::id_21]))) ?  ::id_0::id_11 :  ::id_0::id_9);
}
if(  ::id_0::id_9== ::id_0::id_17 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_17= ::id_0::id_13;
return ;
}
if(  ::id_0::id_13== ::id_0::id_17 )
{
 ::id_15=((1)+ ::id_15);
 ::id_24=((1)+ ::id_24);
 ::id_0::id_17=(( ::id_24<( ::id_0::id_23[ ::id_0::id_21])) ?  ::id_0::id_9 :  ::id_0::id_11);
}
if(  ::id_0::id_11== ::id_0::id_17 )
{
 ::id_0::id_25= ::id_0::id_7;
 ::id_0::id_17= ::id_0::id_5;
}
if(  ::id_0::id_7== ::id_0::id_17 )
{
 ::id_0::id_1=( ::id_0::id_22[ ::id_0::id_21]);
 ::id_0::id_21--;
 ::id_0::id_17= ::id_0::id_1;
}
if(  ::id_0::id_5== ::id_0::id_17 )
{
 ::id_0::id_26= ::id_0::id_25;
 ::id_15=( ::id_15-(1));
 ::id_0::id_2= ::id_0::id_26;
 ::id_0::id_17= ::id_0::id_2;
}
next_trigger(SC_ZERO_TIME);
}
