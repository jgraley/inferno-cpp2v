#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_18);
}
enum id_1
{
id_10 = 1U,
id_11 = 8U,
id_12 = 14U,
id_13 = 15U,
id_14 = 2U,
id_15 = 10U,
id_16 = 13U,
id_17 = 6U,
id_2 = 7U,
id_3 = 3U,
id_4 = 4U,
id_5 = 12U,
id_6 = 9U,
id_7 = 5U,
id_8 = 11U,
id_9 = 0U,
};
void id_18();
int id_26;
private:
unsigned int id_29;
public:
/*temp*/ int id_28;
/*temp*/ unsigned int id_27;
};
id_0 id_30("id_30");

void id_0::id_18()
{
/*temp*/ unsigned int id_19;
auto int id_20;
auto int id_21;
auto unsigned int id_22;
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ int id_25;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_26=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_26=(99);
break;
case 0:;
if( ((0)== ::id_0::id_26)||((2)== ::id_0::id_26) )
 ::id_0::id_26=((false) ? (88) : (2));
break;
}
id_20=(0);
wait(SC_ZERO_TIME);
id_22=(((0)==id_20) ?  ::id_0::id_4 :  ::id_0::id_9);
continue;
}
if(  ::id_0::id_9==id_22 )
{
id_22=(((4)==id_20) ?  ::id_0::id_3 :  ::id_0::id_10);
}
if(  ::id_0::id_10==id_22 )
{
id_22=(((1)==id_20) ?  ::id_0::id_14 :  ::id_0::id_14);
}
if(  ::id_0::id_14==id_22 )
{
 ::id_0::id_26=(99);
id_22= ::id_0::id_2;
}
if(  ::id_0::id_3==id_22 )
{
 ::id_0::id_26=(44);
id_22= ::id_0::id_4;
}
if(  ::id_0::id_4==id_22 )
{
if( ((0)== ::id_0::id_26)||((2)== ::id_0::id_26) )
 ::id_0::id_26=((false) ? (88) : (2));
id_22= ::id_0::id_2;
}
if(  ::id_0::id_2==id_22 )
{
id_21=(2);
id_22=(((2)==id_21) ?  ::id_0::id_5 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_22 )
{
id_22=(((1)==id_21) ?  ::id_0::id_15 :  ::id_0::id_15);
}
if(  ::id_0::id_15==id_22 )
{
 ::id_0::id_26=(99);
id_22= ::id_0::id_17;
}
if(  ::id_0::id_5==id_22 )
{
id_22=((!(((0)== ::id_0::id_26)||((2)== ::id_0::id_26))) ?  ::id_0::id_17 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_22 )
{
id_22=((!(false)) ?  ::id_0::id_16 :  ::id_0::id_13);
}
if(  ::id_0::id_13==id_22 )
{
id_23=(88);
id_22= ::id_0::id_6;
}
if(  ::id_0::id_16==id_22 )
{
 ::id_0::id_27= ::id_0::id_8;
id_22= ::id_0::id_7;
}
if(  ::id_0::id_8==id_22 )
{
id_24= ::id_0::id_28;
id_23=id_24;
id_22= ::id_0::id_6;
}
if(  ::id_0::id_6==id_22 )
{
id_25=id_23;
 ::id_0::id_26=id_25;
id_22= ::id_0::id_17;
}
if(  ::id_0::id_17==id_22 )
{
cease(  ::id_0::id_26 );
return ;
id_22= ::id_0::id_7;
}
if(  ::id_0::id_7==id_22 )
{
 ::id_0::id_29= ::id_0::id_27;
 ::id_0::id_28=(3);
id_19= ::id_0::id_29;
id_22=id_19;
}
}
while( true );
}
