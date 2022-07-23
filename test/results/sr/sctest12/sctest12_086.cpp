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
id_10 = 5,
id_11 = 9,
id_12 = 11,
id_13 = 12,
id_14 = 6,
id_15 = 10,
id_16 = 13,
id_17 = 14,
id_2 = 3,
id_3 = 0,
id_4 = 1,
id_5 = 2,
id_6 = 8,
id_7 = 15,
id_8 = 7,
id_9 = 4,
};
void id_18();
int id_28;
private:
unsigned int id_31;
public:
/*temp*/ int id_30;
/*temp*/ unsigned int id_29;
/*temp*/ unsigned int id_32;
};
id_0 id_33("id_33");

void id_0::id_18()
{
/*temp*/ unsigned int id_19;
static const unsigned int (id_20[]) = { &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21, &&id_21 };
auto int id_22;
auto int id_23;
auto unsigned int id_24;
/*temp*/ int id_25;
/*temp*/ int id_26;
/*temp*/ int id_27;
do
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_28=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_28=(99);
break;
case 0:;
if( ((0)== ::id_0::id_28)||((2)== ::id_0::id_28) )
 ::id_0::id_28=((false) ? (88) : (2));
break;
}
id_22=(0);
wait(SC_ZERO_TIME);
id_24=(((0)==id_22) ?  ::id_0::id_4 :  ::id_0::id_10);
continue;
}
if(  ::id_0::id_10==id_24 )
{
id_24=(((1)==id_22) ?  ::id_0::id_15 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_24 )
{
id_24=(((4)==id_22) ?  ::id_0::id_5 :  ::id_0::id_15);
}
if(  ::id_0::id_15==id_24 )
{
 ::id_0::id_28=(99);
id_24= ::id_0::id_2;
}
if(  ::id_0::id_5==id_24 )
{
 ::id_0::id_28=(44);
id_24= ::id_0::id_4;
}
if(  ::id_0::id_4==id_24 )
{
if( ((0)== ::id_0::id_28)||((2)== ::id_0::id_28) )
 ::id_0::id_28=((false) ? (88) : (2));
id_24= ::id_0::id_2;
}
if(  ::id_0::id_2==id_24 )
{
id_23=(2);
id_24=(((1)==id_23) ?  ::id_0::id_14 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_24 )
{
id_24=(((2)==id_23) ?  ::id_0::id_3 :  ::id_0::id_14);
}
if(  ::id_0::id_14==id_24 )
{
 ::id_0::id_28=(99);
id_24= ::id_0::id_17;
}
if(  ::id_0::id_3==id_24 )
{
id_24=((!(((0)== ::id_0::id_28)||((2)== ::id_0::id_28))) ?  ::id_0::id_17 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_24 )
{
id_24=((!(false)) ?  ::id_0::id_16 :  ::id_0::id_13);
}
if(  ::id_0::id_13==id_24 )
{
id_25=(88);
id_24= ::id_0::id_6;
}
if(  ::id_0::id_16==id_24 )
{
 ::id_0::id_29= ::id_0::id_8;
id_24= ::id_0::id_7;
}
if(  ::id_0::id_8==id_24 )
{
id_26= ::id_0::id_30;
id_25=id_26;
id_24= ::id_0::id_6;
}
if(  ::id_0::id_6==id_24 )
{
id_27=id_25;
 ::id_0::id_28=id_27;
id_24= ::id_0::id_17;
}
if(  ::id_0::id_17==id_24 )
{
cease(  ::id_0::id_28 );
return ;
id_24= ::id_0::id_7;
}
if(  ::id_0::id_7==id_24 )
{
 ::id_0::id_31= ::id_0::id_29;
 ::id_0::id_30=(3);
id_19= ::id_0::id_31;
id_24=id_19;
}
}
while( true );
}
