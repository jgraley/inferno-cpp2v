#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_23(0)
{
SC_THREAD(id_12);
}
enum id_1
{
id_10 = 6,
id_11 = 8,
id_2 = 3,
id_3 = 9,
id_4 = 2,
id_5 = 5,
id_6 = 0,
id_7 = 4,
id_8 = 1,
id_9 = 7,
};
void id_12();
private:
int (id_25[10]);
unsigned int (id_24[10]);
unsigned int id_23;
unsigned int id_28;
public:
/*temp*/ int id_21;
/*temp*/ unsigned int id_22;
/*temp*/ unsigned int id_27;
/*temp*/ unsigned int id_29;
/*temp*/ unsigned int id_30;
};
id_0 id_31("id_31");
int id_19;
int id_20;
int id_26;

void id_0::id_12()
{
/*temp*/ unsigned int id_13;
/*temp*/ unsigned int id_14;
static const unsigned int (id_15[]) = { &&id_16, &&id_16, &&id_16, &&id_16, &&id_16, &&id_16, &&id_16, &&id_16, &&id_16, &&id_16 };
auto unsigned int id_17;
/*temp*/ int id_18;
 ::id_19=(1);
 ::id_20=(0);
wait(SC_ZERO_TIME);
id_17=((!( ::id_20<(4))) ?  ::id_0::id_9 :  ::id_0::id_6);
id_16:;
if(  ::id_0::id_6==id_17 )
{
 ::id_19=( ::id_19+ ::id_20);
id_18=(3);
 ::id_0::id_21=id_18;
 ::id_0::id_22= ::id_0::id_5;
id_17= ::id_0::id_2;
}
if(  ::id_0::id_5==id_17 )
{
 ::id_19=((2)* ::id_19);
wait(SC_ZERO_TIME);
id_17= ::id_0::id_10;
goto *(id_15[id_17]);
}
if(  ::id_0::id_10==id_17 )
{
 ::id_20=((1)+ ::id_20);
id_17=(( ::id_20<(4)) ?  ::id_0::id_6 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_17 )
{
cease(  ::id_19 );
return ;
id_17= ::id_0::id_2;
}
if(  ::id_0::id_2==id_17 )
{
 ::id_0::id_23++;
( ::id_0::id_24[ ::id_0::id_23])= ::id_0::id_22;
( ::id_0::id_25[ ::id_0::id_23])= ::id_0::id_21;
 ::id_26=(0);
id_17=((!( ::id_26<( ::id_0::id_25[ ::id_0::id_23]))) ?  ::id_0::id_8 :  ::id_0::id_7);
}
if(  ::id_0::id_7==id_17 )
{
wait(SC_ZERO_TIME);
id_17= ::id_0::id_11;
goto *(id_15[id_17]);
}
if(  ::id_0::id_11==id_17 )
{
 ::id_19=((1)+ ::id_19);
 ::id_26=((1)+ ::id_26);
id_17=(( ::id_26<( ::id_0::id_25[ ::id_0::id_23])) ?  ::id_0::id_7 :  ::id_0::id_8);
}
if(  ::id_0::id_8==id_17 )
{
 ::id_0::id_27= ::id_0::id_4;
id_17= ::id_0::id_3;
}
if(  ::id_0::id_4==id_17 )
{
id_13=( ::id_0::id_24[ ::id_0::id_23]);
 ::id_0::id_23--;
id_17=id_13;
}
if(  ::id_0::id_3==id_17 )
{
 ::id_0::id_28= ::id_0::id_27;
 ::id_19=( ::id_19-(1));
id_14= ::id_0::id_28;
id_17=id_14;
}
goto *(id_15[id_17]);
}
