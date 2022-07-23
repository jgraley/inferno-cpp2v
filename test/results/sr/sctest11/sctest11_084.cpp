#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_16);
}
enum id_1
{
id_10 = 2,
id_11 = 5,
id_12 = 12,
id_13 = 6,
id_14 = 10,
id_15 = 4,
id_2 = 1,
id_3 = 13,
id_4 = 3,
id_5 = 7,
id_6 = 9,
id_7 = 11,
id_8 = 0,
id_9 = 8,
};
void id_16();
int id_31;
int id_35;
private:
int id_37;
unsigned int id_36;
public:
/*temp*/ int id_32;
/*temp*/ int id_34;
/*temp*/ unsigned int id_33;
/*temp*/ unsigned int id_38;
};
id_0 id_39("id_39");

void id_0::id_16()
{
/*temp*/ unsigned int id_17;
static const unsigned int (id_18[]) = { &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19, &&id_19 };
auto unsigned int id_20;
/*temp*/ int id_21;
/*temp*/ int id_22;
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ int id_25;
/*temp*/ int id_26;
/*temp*/ bool id_27;
/*temp*/ bool id_28;
/*temp*/ bool id_29;
/*temp*/ bool id_30;
id_19:;
if( (sc_delta_count())==(0) )
{
 ::id_0::id_31=(4);
id_27=(++ ::id_0::id_31);
wait(SC_ZERO_TIME);
id_20=((!id_27) ?  ::id_0::id_15 :  ::id_0::id_8);
goto *(id_18[id_20]);
}
if(  ::id_0::id_8==id_20 )
{
 ::id_0::id_32= ::id_0::id_31;
 ::id_0::id_33= ::id_0::id_4;
id_20= ::id_0::id_3;
}
if(  ::id_0::id_4==id_20 )
{
id_22= ::id_0::id_34;
id_27=( ::id_0::id_35=id_22);
id_20= ::id_0::id_15;
}
if(  ::id_0::id_15==id_20 )
{
id_29=id_27;
id_29;
id_28=(!(++ ::id_0::id_31));
id_20=((!id_28) ?  ::id_0::id_13 :  ::id_0::id_10);
}
if(  ::id_0::id_13==id_20 )
{
 ::id_0::id_32= ::id_0::id_31;
 ::id_0::id_33= ::id_0::id_5;
id_20= ::id_0::id_3;
}
if(  ::id_0::id_5==id_20 )
{
id_23= ::id_0::id_34;
id_28=( ::id_0::id_35+=id_23);
id_20= ::id_0::id_10;
}
if(  ::id_0::id_10==id_20 )
{
id_30=id_28;
id_30;
 ::id_0::id_31=(0);
id_20=((!( ::id_0::id_31<(2))) ?  ::id_0::id_12 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_20 )
{
id_20=((!( ::id_0::id_31++)) ?  ::id_0::id_14 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_20 )
{
 ::id_0::id_32= ::id_0::id_31;
 ::id_0::id_33= ::id_0::id_6;
id_20= ::id_0::id_3;
}
if(  ::id_0::id_6==id_20 )
{
id_24= ::id_0::id_34;
id_21=( ::id_0::id_35+=id_24);
id_20= ::id_0::id_2;
}
if(  ::id_0::id_14==id_20 )
{
 ::id_0::id_32= ::id_0::id_31;
 ::id_0::id_33= ::id_0::id_7;
id_20= ::id_0::id_3;
}
if(  ::id_0::id_7==id_20 )
{
id_25= ::id_0::id_34;
id_21=( ::id_0::id_35-=id_25);
id_20= ::id_0::id_2;
}
if(  ::id_0::id_2==id_20 )
{
id_26=id_21;
id_26;
id_20=(( ::id_0::id_31<(2)) ?  ::id_0::id_11 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_20 )
{
cease(  ::id_0::id_35 );
return ;
id_20= ::id_0::id_3;
}
if(  ::id_0::id_3==id_20 )
{
 ::id_0::id_36= ::id_0::id_33;
 ::id_0::id_37= ::id_0::id_32;
 ::id_0::id_34=((100)/ ::id_0::id_37);
id_17= ::id_0::id_36;
id_20=id_17;
}
goto *(id_18[id_20]);
}
