#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_17);
}
enum id_1
{
id_10 = 0U,
id_11 = 2U,
id_12 = 4U,
id_13 = 6U,
id_14 = 8U,
id_15 = 10U,
id_16 = 13U,
id_2 = 14U,
id_3 = 12U,
id_4 = 3U,
id_5 = 5U,
id_6 = 7U,
id_7 = 9U,
id_8 = 11U,
id_9 = 1U,
};
void id_17();
int id_37;
int id_38;
float id_42;
private:
unsigned int id_41;
public:
/*temp*/ int id_40;
/*temp*/ unsigned int id_39;
/*temp*/ unsigned int id_43;
};
id_0 id_44("id_44");

void id_0::id_17()
{
/*temp*/ unsigned int id_18;
static const unsigned int (id_19[]) = { &&id_20, &&id_21, &&id_22, &&id_23, &&id_24, &&id_25, &&id_26, &&id_27, &&id_28, &&id_29, &&id_30, &&id_31, &&id_32, &&id_33, &&id_34 };
auto unsigned int id_35;
/*temp*/ int id_36;
 ::id_0::id_37=(0);
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38++ )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(0);  ::id_0::id_38<=(4);  ::id_0::id_38++ )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(0); (4)!= ::id_0::id_38;  ::id_0::id_38++ )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(4);  ::id_0::id_38>(0);  ::id_0::id_38-- )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(4);  ::id_0::id_38>=(0);  ::id_0::id_38-- )
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38=(0);
wait(SC_ZERO_TIME);
id_35=((!( ::id_0::id_38<(4))) ?  ::id_0::id_10 :  ::id_0::id_10);
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
id_20:;
if(  ::id_0::id_10==id_35 )
{
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_38=(0);
id_35=((!( ::id_0::id_38<(4))) ?  ::id_0::id_11 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_35 )
{
 ::id_0::id_38++;
id_35=(( ::id_0::id_38<(4)) ?  ::id_0::id_9 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_35 )
{
id_35=((!( ::id_0::id_38<(4))) ?  ::id_0::id_12 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_35 )
{
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38++;
id_35=(( ::id_0::id_38<(4)) ?  ::id_0::id_4 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_35 )
{
 ::id_0::id_38=(0);
id_35=((!( ::id_0::id_38< ::id_0::id_37)) ?  ::id_0::id_13 :  ::id_0::id_5);
}
if(  ::id_0::id_5==id_35 )
{
 ::id_0::id_38++;
id_35=(( ::id_0::id_38< ::id_0::id_37) ?  ::id_0::id_5 :  ::id_0::id_13);
}
if(  ::id_0::id_13==id_35 )
{
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38=((1)+ ::id_0::id_38) )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38+=(1) )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(4);  ::id_0::id_38>(0);  ::id_0::id_38=( ::id_0::id_38-(1)) )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(4);  ::id_0::id_38>(0);  ::id_0::id_38-=(1) )
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38=(0);
id_35=((!( ::id_0::id_38<(4))) ?  ::id_0::id_14 :  ::id_0::id_6);
}
if(  ::id_0::id_6==id_35 )
{
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38=(4);
id_35=(( ::id_0::id_38<(4)) ?  ::id_0::id_6 :  ::id_0::id_14);
}
if(  ::id_0::id_14==id_35 )
{
 ::id_0::id_38=(0);
id_35=((!( ::id_0::id_38<(4))) ?  ::id_0::id_15 :  ::id_0::id_7);
}
if(  ::id_0::id_7==id_35 )
{
 ::id_0::id_38+=(0);
 ::id_0::id_38++;
id_35=(( ::id_0::id_38<(4)) ?  ::id_0::id_7 :  ::id_0::id_15);
}
if(  ::id_0::id_15==id_35 )
{
 ::id_0::id_38=(0);
id_35=((!( ::id_0::id_38<(4))) ?  ::id_0::id_16 :  ::id_0::id_8);
}
if(  ::id_0::id_8==id_35 )
{
 ::id_0::id_39= ::id_0::id_3;
id_35= ::id_0::id_2;
}
if(  ::id_0::id_3==id_35 )
{
id_36= ::id_0::id_40;
 ::id_0::id_37+=id_36;
 ::id_0::id_38++;
id_35=(( ::id_0::id_38<(4)) ?  ::id_0::id_8 :  ::id_0::id_16);
}
if(  ::id_0::id_16==id_35 )
{
cease(  ::id_0::id_37 );
return ;
id_35= ::id_0::id_2;
}
if(  ::id_0::id_2==id_35 )
{
 ::id_0::id_41= ::id_0::id_39;
 ::id_0::id_40=(3);
id_18= ::id_0::id_41;
id_35=id_18;
}
goto *(id_19[id_35]);
}
