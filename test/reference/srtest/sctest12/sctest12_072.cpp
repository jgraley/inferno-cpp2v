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
id_11 = 6U,
id_12 = 9U,
id_13 = 10U,
id_14 = 2U,
id_15 = 7U,
id_16 = 11U,
id_17 = 14U,
id_2 = 5U,
id_3 = 3U,
id_4 = 4U,
id_5 = 8U,
id_6 = 13U,
id_7 = 15U,
id_8 = 12U,
id_9 = 0U,
};
void id_18();
int id_23;
private:
int id_24;
int id_26;
unsigned int id_25;
unsigned int id_29;
public:
/*temp*/ int id_28;
/*temp*/ unsigned int id_27;
};
id_0 id_30("id_30");

void id_0::id_18()
{
/*temp*/ unsigned int id_19;
/*temp*/ int id_20;
/*temp*/ int id_21;
/*temp*/ int id_22;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_23=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_23=(99);
break;
case 0:;
if( ((0)== ::id_0::id_23)||((2)== ::id_0::id_23) )
 ::id_0::id_23=((false) ? (88) : (2));
break;
}
 ::id_0::id_24=(0);
wait(SC_ZERO_TIME);
 ::id_0::id_25=(((0)== ::id_0::id_24) ?  ::id_0::id_4 :  ::id_0::id_9);
continue;
}
if(  ::id_0::id_9== ::id_0::id_25 )
{
 ::id_0::id_25=(((4)== ::id_0::id_24) ?  ::id_0::id_3 :  ::id_0::id_10);
}
if(  ::id_0::id_10== ::id_0::id_25 )
{
 ::id_0::id_25=(((1)== ::id_0::id_24) ?  ::id_0::id_14 :  ::id_0::id_14);
}
if(  ::id_0::id_14== ::id_0::id_25 )
{
 ::id_0::id_23=(99);
 ::id_0::id_25= ::id_0::id_2;
}
if(  ::id_0::id_3== ::id_0::id_25 )
{
 ::id_0::id_23=(44);
 ::id_0::id_25= ::id_0::id_4;
}
if(  ::id_0::id_4== ::id_0::id_25 )
{
if( ((0)== ::id_0::id_23)||((2)== ::id_0::id_23) )
 ::id_0::id_23=((false) ? (88) : (2));
 ::id_0::id_25= ::id_0::id_2;
}
if(  ::id_0::id_2== ::id_0::id_25 )
{
 ::id_0::id_26=(2);
 ::id_0::id_25=(((2)== ::id_0::id_26) ?  ::id_0::id_5 :  ::id_0::id_11);
}
if(  ::id_0::id_11== ::id_0::id_25 )
{
 ::id_0::id_25=(((1)== ::id_0::id_26) ?  ::id_0::id_15 :  ::id_0::id_15);
}
if(  ::id_0::id_15== ::id_0::id_25 )
{
 ::id_0::id_23=(99);
 ::id_0::id_25= ::id_0::id_17;
}
if(  ::id_0::id_5== ::id_0::id_25 )
{
 ::id_0::id_25=((!(((0)== ::id_0::id_23)||((2)== ::id_0::id_23))) ?  ::id_0::id_17 :  ::id_0::id_12);
}
if(  ::id_0::id_12== ::id_0::id_25 )
{
 ::id_0::id_25=((!(false)) ?  ::id_0::id_16 :  ::id_0::id_13);
}
if(  ::id_0::id_13== ::id_0::id_25 )
{
id_20=(88);
 ::id_0::id_25= ::id_0::id_6;
}
if(  ::id_0::id_16== ::id_0::id_25 )
{
 ::id_0::id_27= ::id_0::id_8;
 ::id_0::id_25= ::id_0::id_7;
}
if(  ::id_0::id_8== ::id_0::id_25 )
{
id_21= ::id_0::id_28;
id_20=id_21;
 ::id_0::id_25= ::id_0::id_6;
}
if(  ::id_0::id_6== ::id_0::id_25 )
{
id_22=id_20;
 ::id_0::id_23=id_22;
 ::id_0::id_25= ::id_0::id_17;
}
if(  ::id_0::id_17== ::id_0::id_25 )
{
cease(  ::id_0::id_23 );
return ;
 ::id_0::id_25= ::id_0::id_7;
}
if(  ::id_0::id_7== ::id_0::id_25 )
{
 ::id_0::id_29= ::id_0::id_27;
 ::id_0::id_28=(3);
id_19= ::id_0::id_29;
 ::id_0::id_25=id_19;
}
wait(SC_ZERO_TIME);
}
while( true );
}
