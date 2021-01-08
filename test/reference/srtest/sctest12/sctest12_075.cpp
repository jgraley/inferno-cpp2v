#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_19);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_10 = 0U,
id_11 = 1U,
id_12 = 6U,
id_13 = 9U,
id_14 = 10U,
id_15 = 2U,
id_16 = 7U,
id_17 = 11U,
id_18 = 14U,
id_3 = 5U,
id_4 = 3U,
id_5 = 4U,
id_6 = 8U,
id_7 = 13U,
id_8 = 15U,
id_9 = 12U,
};
void id_19();
int id_20;
private:
int id_21;
int id_23;
unsigned int id_22;
unsigned int id_29;
public:
/*temp*/ int id_24;
/*temp*/ int id_26;
/*temp*/ int id_27;
/*temp*/ int id_28;
/*temp*/ unsigned int id_25;
};
id_0 id_30("id_30");

void id_0::id_19()
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_20=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_20=(99);
break;
case 0:;
if( ((0)== ::id_0::id_20)||((2)== ::id_0::id_20) )
 ::id_0::id_20=((false) ? (88) : (2));
break;
}
 ::id_0::id_21=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_22=(((0)== ::id_0::id_21) ?  ::id_0::id_5 :  ::id_0::id_10);
return ;
}
if(  ::id_0::id_10== ::id_0::id_22 )
{
 ::id_0::id_22=(((4)== ::id_0::id_21) ?  ::id_0::id_4 :  ::id_0::id_11);
}
if(  ::id_0::id_11== ::id_0::id_22 )
{
 ::id_0::id_22=(((1)== ::id_0::id_21) ?  ::id_0::id_15 :  ::id_0::id_15);
}
if(  ::id_0::id_15== ::id_0::id_22 )
{
 ::id_0::id_20=(99);
 ::id_0::id_22= ::id_0::id_3;
}
if(  ::id_0::id_4== ::id_0::id_22 )
{
 ::id_0::id_20=(44);
 ::id_0::id_22= ::id_0::id_5;
}
if(  ::id_0::id_5== ::id_0::id_22 )
{
if( ((0)== ::id_0::id_20)||((2)== ::id_0::id_20) )
 ::id_0::id_20=((false) ? (88) : (2));
 ::id_0::id_22= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_22 )
{
 ::id_0::id_23=(2);
 ::id_0::id_22=(((2)== ::id_0::id_23) ?  ::id_0::id_6 :  ::id_0::id_12);
}
if(  ::id_0::id_12== ::id_0::id_22 )
{
 ::id_0::id_22=(((1)== ::id_0::id_23) ?  ::id_0::id_16 :  ::id_0::id_16);
}
if(  ::id_0::id_16== ::id_0::id_22 )
{
 ::id_0::id_20=(99);
 ::id_0::id_22= ::id_0::id_18;
}
if(  ::id_0::id_6== ::id_0::id_22 )
{
 ::id_0::id_22=((!(((0)== ::id_0::id_20)||((2)== ::id_0::id_20))) ?  ::id_0::id_18 :  ::id_0::id_13);
}
if(  ::id_0::id_13== ::id_0::id_22 )
{
 ::id_0::id_22=((!(false)) ?  ::id_0::id_17 :  ::id_0::id_14);
}
if(  ::id_0::id_14== ::id_0::id_22 )
{
 ::id_0::id_24=(88);
 ::id_0::id_22= ::id_0::id_7;
}
if(  ::id_0::id_17== ::id_0::id_22 )
{
 ::id_0::id_25= ::id_0::id_9;
 ::id_0::id_22= ::id_0::id_8;
}
if(  ::id_0::id_9== ::id_0::id_22 )
{
 ::id_0::id_26= ::id_0::id_27;
 ::id_0::id_24= ::id_0::id_26;
 ::id_0::id_22= ::id_0::id_7;
}
if(  ::id_0::id_7== ::id_0::id_22 )
{
 ::id_0::id_28= ::id_0::id_24;
 ::id_0::id_20= ::id_0::id_28;
 ::id_0::id_22= ::id_0::id_18;
}
if(  ::id_0::id_18== ::id_0::id_22 )
{
cease(  ::id_0::id_20 );
return ;
 ::id_0::id_22= ::id_0::id_8;
}
if(  ::id_0::id_8== ::id_0::id_22 )
{
 ::id_0::id_29= ::id_0::id_25;
 ::id_0::id_27=(3);
 ::id_0::id_1= ::id_0::id_29;
 ::id_0::id_22= ::id_0::id_1;
}
next_trigger(SC_ZERO_TIME);
}
