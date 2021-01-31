#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_22(0U)
{
SC_METHOD(id_14);
}
/*temp*/ unsigned int id_1;
/*temp*/ unsigned int id_2;
enum id_3
{
id_10 = 3U,
id_11 = 9U,
id_12 = 2U,
id_13 = 7U,
id_4 = 4U,
id_5 = 5U,
id_6 = 1U,
id_7 = 8U,
id_8 = 0U,
id_9 = 6U,
};
void id_14();
private:
int (id_23[10U]);
unsigned int (id_24[10U]);
unsigned int id_18;
unsigned int id_22;
unsigned int id_27;
public:
/*temp*/ int id_19;
/*temp*/ int id_21;
/*temp*/ unsigned int id_20;
/*temp*/ unsigned int id_26;
};
id_0 id_28("id_28");
int id_16;
int id_17;
int id_25;

void id_0::id_14()
{
/*temp*/ bool id_15 = true;
if( (sc_delta_count())==(0U) )
{
 ::id_16=(1);
 ::id_17=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_18=((!( ::id_17<(4))) ?  ::id_0::id_10 :  ::id_0::id_8);
id_15=(false);
}
if( id_15 )
{
if(  ::id_0::id_8== ::id_0::id_18 )
{
 ::id_16=( ::id_16+ ::id_17);
 ::id_0::id_19=(3);
 ::id_0::id_20= ::id_0::id_6;
 ::id_0::id_21= ::id_0::id_19;
 ::id_0::id_18= ::id_0::id_4;
}
}
if( id_15 )
{
if(  ::id_0::id_6== ::id_0::id_18 )
{
 ::id_16=((2)* ::id_16);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_18= ::id_0::id_12;
id_15=(false);
}
}
if( id_15 )
{
if(  ::id_0::id_12== ::id_0::id_18 )
{
 ::id_17=((1)+ ::id_17);
 ::id_0::id_18=(( ::id_17<(4)) ?  ::id_0::id_8 :  ::id_0::id_10);
}
}
if( id_15 )
{
if(  ::id_0::id_10== ::id_0::id_18 )
{
cease(  ::id_16 );
id_15=(false);
if( id_15 )
 ::id_0::id_18= ::id_0::id_4;
}
}
if( id_15 )
{
if(  ::id_0::id_4== ::id_0::id_18 )
{
 ::id_0::id_22++;
( ::id_0::id_23[ ::id_0::id_22])= ::id_0::id_21;
( ::id_0::id_24[ ::id_0::id_22])= ::id_0::id_20;
 ::id_25=(0);
 ::id_0::id_18=((!( ::id_25<( ::id_0::id_23[ ::id_0::id_22]))) ?  ::id_0::id_11 :  ::id_0::id_9);
}
}
if( id_15 )
{
if(  ::id_0::id_9== ::id_0::id_18 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_18= ::id_0::id_13;
id_15=(false);
}
}
if( id_15 )
{
if(  ::id_0::id_13== ::id_0::id_18 )
{
 ::id_16=((1)+ ::id_16);
 ::id_25=((1)+ ::id_25);
 ::id_0::id_18=(( ::id_25<( ::id_0::id_23[ ::id_0::id_22])) ?  ::id_0::id_9 :  ::id_0::id_11);
}
}
if( id_15 )
{
if(  ::id_0::id_11== ::id_0::id_18 )
{
 ::id_0::id_26= ::id_0::id_7;
 ::id_0::id_18= ::id_0::id_5;
}
}
if( id_15 )
{
if(  ::id_0::id_7== ::id_0::id_18 )
{
 ::id_0::id_1=( ::id_0::id_24[ ::id_0::id_22]);
 ::id_0::id_22--;
 ::id_0::id_18= ::id_0::id_1;
}
}
if( id_15 )
{
if(  ::id_0::id_5== ::id_0::id_18 )
{
 ::id_0::id_27= ::id_0::id_26;
 ::id_16=( ::id_16-(1));
 ::id_0::id_2= ::id_0::id_27;
 ::id_0::id_18= ::id_0::id_2;
}
}
if( id_15 )
next_trigger(SC_ZERO_TIME);
}
