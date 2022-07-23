#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_17);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_10 = 8,
id_11 = 2,
id_12 = 5,
id_13 = 12,
id_14 = 6,
id_15 = 10,
id_16 = 4,
id_3 = 1,
id_4 = 13,
id_5 = 3,
id_6 = 7,
id_7 = 9,
id_8 = 11,
id_9 = 0,
};
void id_17();
int id_18;
int id_25;
private:
int id_35;
unsigned int id_20;
unsigned int id_34;
public:
/*temp*/ int id_21;
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ int id_28;
/*temp*/ int id_30;
/*temp*/ int id_31;
/*temp*/ int id_32;
/*temp*/ int id_33;
/*temp*/ bool id_19;
/*temp*/ bool id_26;
/*temp*/ bool id_27;
/*temp*/ bool id_29;
/*temp*/ unsigned int id_22;
};
id_0 id_36("id_36");

void id_0::id_17()
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_18=(4);
 ::id_0::id_19=(++ ::id_0::id_18);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_20=((! ::id_0::id_19) ?  ::id_0::id_16 :  ::id_0::id_9);
return ;
}
if(  ::id_0::id_9== ::id_0::id_20 )
{
 ::id_0::id_21= ::id_0::id_18;
 ::id_0::id_22= ::id_0::id_5;
 ::id_0::id_20= ::id_0::id_4;
}
if(  ::id_0::id_5== ::id_0::id_20 )
{
 ::id_0::id_23= ::id_0::id_24;
 ::id_0::id_19=( ::id_0::id_25= ::id_0::id_23);
 ::id_0::id_20= ::id_0::id_16;
}
if(  ::id_0::id_16== ::id_0::id_20 )
{
 ::id_0::id_26= ::id_0::id_19;
 ::id_0::id_26;
 ::id_0::id_27=(!(++ ::id_0::id_18));
 ::id_0::id_20=((! ::id_0::id_27) ?  ::id_0::id_14 :  ::id_0::id_11);
}
if(  ::id_0::id_14== ::id_0::id_20 )
{
 ::id_0::id_21= ::id_0::id_18;
 ::id_0::id_22= ::id_0::id_6;
 ::id_0::id_20= ::id_0::id_4;
}
if(  ::id_0::id_6== ::id_0::id_20 )
{
 ::id_0::id_28= ::id_0::id_24;
 ::id_0::id_27=( ::id_0::id_25+= ::id_0::id_28);
 ::id_0::id_20= ::id_0::id_11;
}
if(  ::id_0::id_11== ::id_0::id_20 )
{
 ::id_0::id_29= ::id_0::id_27;
 ::id_0::id_29;
 ::id_0::id_18=(0);
 ::id_0::id_20=((!( ::id_0::id_18<(2))) ?  ::id_0::id_13 :  ::id_0::id_12);
}
if(  ::id_0::id_12== ::id_0::id_20 )
{
 ::id_0::id_20=((!( ::id_0::id_18++)) ?  ::id_0::id_15 :  ::id_0::id_10);
}
if(  ::id_0::id_10== ::id_0::id_20 )
{
 ::id_0::id_21= ::id_0::id_18;
 ::id_0::id_22= ::id_0::id_7;
 ::id_0::id_20= ::id_0::id_4;
}
if(  ::id_0::id_7== ::id_0::id_20 )
{
 ::id_0::id_30= ::id_0::id_24;
 ::id_0::id_31=( ::id_0::id_25+= ::id_0::id_30);
 ::id_0::id_20= ::id_0::id_3;
}
if(  ::id_0::id_15== ::id_0::id_20 )
{
 ::id_0::id_21= ::id_0::id_18;
 ::id_0::id_22= ::id_0::id_8;
 ::id_0::id_20= ::id_0::id_4;
}
if(  ::id_0::id_8== ::id_0::id_20 )
{
 ::id_0::id_32= ::id_0::id_24;
 ::id_0::id_31=( ::id_0::id_25-= ::id_0::id_32);
 ::id_0::id_20= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_20 )
{
 ::id_0::id_33= ::id_0::id_31;
 ::id_0::id_33;
 ::id_0::id_20=(( ::id_0::id_18<(2)) ?  ::id_0::id_12 :  ::id_0::id_13);
}
if(  ::id_0::id_13== ::id_0::id_20 )
{
cease(  ::id_0::id_25 );
return ;
 ::id_0::id_20= ::id_0::id_4;
}
if(  ::id_0::id_4== ::id_0::id_20 )
{
 ::id_0::id_34= ::id_0::id_22;
 ::id_0::id_35= ::id_0::id_21;
 ::id_0::id_24=((100)/ ::id_0::id_35);
 ::id_0::id_1= ::id_0::id_34;
 ::id_0::id_20= ::id_0::id_1;
}
next_trigger(SC_ZERO_TIME);
}
