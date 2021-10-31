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
int id_43;
private:
unsigned int id_46;
public:
/*temp*/ int id_45;
/*temp*/ unsigned int id_44;
/*temp*/ unsigned int id_47;
};
id_0 id_48("id_48");

void id_0::id_18()
{
/*temp*/ unsigned int id_19;
static const unsigned int (id_20[]) = { &&id_21, &&id_22, &&id_23, &&id_24, &&id_25, &&id_26, &&id_27, &&id_28, &&id_29, &&id_30, &&id_31, &&id_32, &&id_33, &&id_34, &&id_35, &&id_36 };
auto int id_37;
auto int id_38;
auto unsigned int id_39;
/*temp*/ int id_40;
/*temp*/ int id_41;
/*temp*/ int id_42;
 ::id_0::id_43=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_43=(99);
break;
case 0:;
if( ((0)== ::id_0::id_43)||((2)== ::id_0::id_43) )
 ::id_0::id_43=((false) ? (88) : (2));
break;
}
id_37=(0);
wait(SC_ZERO_TIME);
id_39=(((0)==id_37) ?  ::id_0::id_4 :  ::id_0::id_9);
id_36:;
id_35:;
id_34:;
id_33:;
id_32:;
id_31:;
id_30:;
id_29:;
id_28:;
id_27:;
id_26:;
id_25:;
id_24:;
id_23:;
id_22:;
id_21:;
if(  ::id_0::id_9==id_39 )
{
id_39=(((4)==id_37) ?  ::id_0::id_3 :  ::id_0::id_10);
}
if(  ::id_0::id_10==id_39 )
{
id_39=(((1)==id_37) ?  ::id_0::id_14 :  ::id_0::id_14);
}
if(  ::id_0::id_14==id_39 )
{
 ::id_0::id_43=(99);
id_39= ::id_0::id_2;
}
if(  ::id_0::id_3==id_39 )
{
 ::id_0::id_43=(44);
id_39= ::id_0::id_4;
}
if(  ::id_0::id_4==id_39 )
{
if( ((0)== ::id_0::id_43)||((2)== ::id_0::id_43) )
 ::id_0::id_43=((false) ? (88) : (2));
id_39= ::id_0::id_2;
}
if(  ::id_0::id_2==id_39 )
{
id_38=(2);
id_39=(((2)==id_38) ?  ::id_0::id_5 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_39 )
{
id_39=(((1)==id_38) ?  ::id_0::id_15 :  ::id_0::id_15);
}
if(  ::id_0::id_15==id_39 )
{
 ::id_0::id_43=(99);
id_39= ::id_0::id_17;
}
if(  ::id_0::id_5==id_39 )
{
id_39=((!(((0)== ::id_0::id_43)||((2)== ::id_0::id_43))) ?  ::id_0::id_17 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_39 )
{
id_39=((!(false)) ?  ::id_0::id_16 :  ::id_0::id_13);
}
if(  ::id_0::id_13==id_39 )
{
id_40=(88);
id_39= ::id_0::id_6;
}
if(  ::id_0::id_16==id_39 )
{
 ::id_0::id_44= ::id_0::id_8;
id_39= ::id_0::id_7;
}
if(  ::id_0::id_8==id_39 )
{
id_41= ::id_0::id_45;
id_40=id_41;
id_39= ::id_0::id_6;
}
if(  ::id_0::id_6==id_39 )
{
id_42=id_40;
 ::id_0::id_43=id_42;
id_39= ::id_0::id_17;
}
if(  ::id_0::id_17==id_39 )
{
cease(  ::id_0::id_43 );
return ;
id_39= ::id_0::id_7;
}
if(  ::id_0::id_7==id_39 )
{
 ::id_0::id_46= ::id_0::id_44;
 ::id_0::id_45=(3);
id_19= ::id_0::id_46;
id_39=id_19;
}
goto *(id_20[id_39]);
}
