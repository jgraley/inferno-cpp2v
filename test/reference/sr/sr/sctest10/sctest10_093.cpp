#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_7);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_3 = 3U,
id_4 = 0U,
id_5 = 1U,
id_6 = 2U,
};
void id_7();
private:
char id_38;
short id_39;
int id_40;
int id_41;
public:
int id_9;
private:
unsigned int id_17;
unsigned int id_37;
public:
/*temp*/ char id_15;
/*temp*/ short id_14;
/*temp*/ int id_10;
/*temp*/ int id_11;
/*temp*/ int id_12;
/*temp*/ int id_13;
/*temp*/ int id_18;
/*temp*/ int id_19;
/*temp*/ int id_20;
/*temp*/ int id_21;
/*temp*/ int id_22;
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ int id_25;
/*temp*/ int id_26;
/*temp*/ int id_27;
/*temp*/ int id_28;
/*temp*/ int id_29;
/*temp*/ int id_30;
/*temp*/ int id_31;
/*temp*/ int id_32;
/*temp*/ int id_33;
/*temp*/ int id_34;
/*temp*/ int id_35;
/*temp*/ int id_36;
/*temp*/ unsigned int id_16;
};
id_0 id_42("id_42");

void id_0::id_7()
{
/*temp*/ bool id_8 = true;
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_9=(0);
 ::id_0::id_10=(0);
 ::id_0::id_11=(0);
 ::id_0::id_12=(0);
 ::id_0::id_13= ::id_0::id_10;
 ::id_0::id_14= ::id_0::id_11;
 ::id_0::id_15= ::id_0::id_12;
 ::id_0::id_16= ::id_0::id_4;
next_trigger(SC_ZERO_TIME);
 ::id_0::id_17= ::id_0::id_3;
id_8=(false);
}
if( id_8 )
{
if(  ::id_0::id_4== ::id_0::id_17 )
{
 ::id_0::id_18= ::id_0::id_19;
 ::id_0::id_20= ::id_0::id_18;
 ::id_0::id_21= ::id_0::id_20;
 ::id_0::id_22= ::id_0::id_21;
 ::id_0::id_23= ::id_0::id_22;
 ::id_0::id_24=(6);
 ::id_0::id_25=(8);
 ::id_0::id_13= ::id_0::id_23;
 ::id_0::id_14= ::id_0::id_24;
 ::id_0::id_15= ::id_0::id_25;
 ::id_0::id_16= ::id_0::id_5;
 ::id_0::id_17= ::id_0::id_3;
}
}
if( id_8 )
{
if(  ::id_0::id_5== ::id_0::id_17 )
{
 ::id_0::id_26= ::id_0::id_19;
 ::id_0::id_27= ::id_0::id_26;
 ::id_0::id_28= ::id_0::id_27;
 ::id_0::id_29= ::id_0::id_28;
 ::id_0::id_9= ::id_0::id_29;
 ::id_0::id_30=(1);
 ::id_0::id_31=(2);
 ::id_0::id_32=(3);
 ::id_0::id_13= ::id_0::id_30;
 ::id_0::id_14= ::id_0::id_31;
 ::id_0::id_15= ::id_0::id_32;
 ::id_0::id_16= ::id_0::id_6;
 ::id_0::id_17= ::id_0::id_3;
}
}
if( id_8 )
{
if(  ::id_0::id_6== ::id_0::id_17 )
{
 ::id_0::id_33= ::id_0::id_19;
 ::id_0::id_34= ::id_0::id_33;
 ::id_0::id_35= ::id_0::id_34;
 ::id_0::id_36= ::id_0::id_35;
cease(  ::id_0::id_9+((2)* ::id_0::id_36) );
id_8=(false);
if( id_8 )
 ::id_0::id_17= ::id_0::id_3;
}
}
if( id_8 )
{
if(  ::id_0::id_3== ::id_0::id_17 )
{
 ::id_0::id_37= ::id_0::id_16;
 ::id_0::id_38= ::id_0::id_15;
 ::id_0::id_39= ::id_0::id_14;
 ::id_0::id_40= ::id_0::id_13;
 ::id_0::id_41=( ::id_0::id_40+((3)* ::id_0::id_39));
 ::id_0::id_19=( ::id_0::id_41+((5)* ::id_0::id_38));
 ::id_0::id_1= ::id_0::id_37;
 ::id_0::id_17= ::id_0::id_1;
}
}
if( id_8 )
next_trigger(SC_ZERO_TIME);
}
