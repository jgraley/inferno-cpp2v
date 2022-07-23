#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_7);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_3 = 2,
id_4 = 0,
id_5 = 1,
id_6 = 3,
};
void id_7();
private:
char id_37;
short id_36;
int id_39;
int id_40;
public:
int id_8;
private:
unsigned int id_16;
unsigned int id_38;
public:
/*temp*/ char id_14;
/*temp*/ short id_15;
/*temp*/ int id_10;
/*temp*/ int id_11;
/*temp*/ int id_12;
/*temp*/ int id_17;
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
/*temp*/ int id_9;
/*temp*/ unsigned int id_13;
};
id_0 id_41("id_41");

void id_0::id_7()
{
do
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_8=(0);
 ::id_0::id_9=(0);
 ::id_0::id_10=(0);
 ::id_0::id_11=(0);
 ::id_0::id_12= ::id_0::id_9;
 ::id_0::id_13= ::id_0::id_6;
 ::id_0::id_14= ::id_0::id_10;
 ::id_0::id_15= ::id_0::id_11;
wait(SC_ZERO_TIME);
 ::id_0::id_16= ::id_0::id_3;
continue;
}
if(  ::id_0::id_6== ::id_0::id_16 )
{
 ::id_0::id_17= ::id_0::id_18;
 ::id_0::id_19= ::id_0::id_17;
 ::id_0::id_20= ::id_0::id_19;
 ::id_0::id_21= ::id_0::id_20;
 ::id_0::id_22= ::id_0::id_21;
 ::id_0::id_23=(8);
 ::id_0::id_24=(6);
 ::id_0::id_12= ::id_0::id_22;
 ::id_0::id_13= ::id_0::id_5;
 ::id_0::id_14= ::id_0::id_23;
 ::id_0::id_15= ::id_0::id_24;
 ::id_0::id_16= ::id_0::id_3;
}
if(  ::id_0::id_5== ::id_0::id_16 )
{
 ::id_0::id_25= ::id_0::id_18;
 ::id_0::id_26= ::id_0::id_25;
 ::id_0::id_27= ::id_0::id_26;
 ::id_0::id_28= ::id_0::id_27;
 ::id_0::id_8= ::id_0::id_28;
 ::id_0::id_29=(1);
 ::id_0::id_30=(3);
 ::id_0::id_31=(2);
 ::id_0::id_12= ::id_0::id_29;
 ::id_0::id_13= ::id_0::id_4;
 ::id_0::id_14= ::id_0::id_30;
 ::id_0::id_15= ::id_0::id_31;
 ::id_0::id_16= ::id_0::id_3;
}
if(  ::id_0::id_4== ::id_0::id_16 )
{
 ::id_0::id_32= ::id_0::id_18;
 ::id_0::id_33= ::id_0::id_32;
 ::id_0::id_34= ::id_0::id_33;
 ::id_0::id_35= ::id_0::id_34;
cease(  ::id_0::id_8+((2)* ::id_0::id_35) );
return ;
 ::id_0::id_16= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_16 )
{
 ::id_0::id_36= ::id_0::id_15;
 ::id_0::id_37= ::id_0::id_14;
 ::id_0::id_38= ::id_0::id_13;
 ::id_0::id_39= ::id_0::id_12;
 ::id_0::id_40=( ::id_0::id_39+((3)* ::id_0::id_36));
 ::id_0::id_18=( ::id_0::id_40+((5)* ::id_0::id_37));
 ::id_0::id_1= ::id_0::id_38;
 ::id_0::id_16= ::id_0::id_1;
}
wait(SC_ZERO_TIME);
}
while( true );
}
