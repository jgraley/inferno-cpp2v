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
id_13 = 8U,
id_14 = 10U,
id_15 = 11U,
id_16 = 13U,
id_2 = 1U,
id_3 = 5U,
id_4 = 6U,
id_5 = 7U,
id_6 = 9U,
id_7 = 12U,
id_8 = 14U,
id_9 = 3U,
};
void id_17();
int id_37;
int id_38;
private:
void *id_41;
public:
float id_42;
/*temp*/ int id_40;
/*temp*/ void *id_39;
/*temp*/ void *id_43;
};
id_0 id_44("id_44");

void id_0::id_17()
{
/*temp*/ void *id_18;
static const void *(id_19[]) = { &&id_20, &&id_21, &&id_22, &&id_23, &&id_24, &&id_25, &&id_26, &&id_27, &&id_28, &&id_29, &&id_30, &&id_31, &&id_32, &&id_33, &&id_34 };
auto void *id_35;
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
{
id_35=((!( ::id_0::id_38<(4))) ? (id_19[ ::id_0::id_10]) : (id_19[ ::id_0::id_10]));
goto *(id_35);
}
id_20:;
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_38=(0);
{
id_35=((!( ::id_0::id_38<(4))) ? (id_19[ ::id_0::id_12]) : (id_19[ ::id_0::id_9]));
goto *(id_35);
}
id_23:;
 ::id_0::id_38++;
{
id_35=(( ::id_0::id_38<(4)) ? (id_19[ ::id_0::id_9]) : (id_19[ ::id_0::id_12]));
goto *(id_35);
}
id_24:;
{
id_35=((!( ::id_0::id_38<(4))) ? (id_19[ ::id_0::id_13]) : (id_19[ ::id_0::id_4]));
goto *(id_35);
}
id_26:;
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38++;
{
id_35=(( ::id_0::id_38<(4)) ? (id_19[ ::id_0::id_4]) : (id_19[ ::id_0::id_13]));
goto *(id_35);
}
id_28:;
 ::id_0::id_38=(0);
{
id_35=((!( ::id_0::id_38< ::id_0::id_37)) ? (id_19[ ::id_0::id_15]) : (id_19[ ::id_0::id_6]));
goto *(id_35);
}
id_29:;
 ::id_0::id_38++;
{
id_35=(( ::id_0::id_38< ::id_0::id_37) ? (id_19[ ::id_0::id_6]) : (id_19[ ::id_0::id_15]));
goto *(id_35);
}
id_31:;
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38=((1)+ ::id_0::id_38) )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(0);  ::id_0::id_38<(4);  ::id_0::id_38+=(1) )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(4);  ::id_0::id_38>(0);  ::id_0::id_38=( ::id_0::id_38-(1)) )
 ::id_0::id_37+= ::id_0::id_38;
for(  ::id_0::id_38=(4);  ::id_0::id_38>(0);  ::id_0::id_38-=(1) )
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38=(0);
{
id_35=((!( ::id_0::id_38<(4))) ? (id_19[ ::id_0::id_16]) : (id_19[ ::id_0::id_8]));
goto *(id_35);
}
id_34:;
 ::id_0::id_37+= ::id_0::id_38;
 ::id_0::id_38=(4);
{
id_35=(( ::id_0::id_38<(4)) ? (id_19[ ::id_0::id_8]) : (id_19[ ::id_0::id_16]));
goto *(id_35);
}
id_33:;
 ::id_0::id_38=(0);
{
id_35=((!( ::id_0::id_38<(4))) ? (id_19[ ::id_0::id_14]) : (id_19[ ::id_0::id_7]));
goto *(id_35);
}
id_32:;
 ::id_0::id_38+=(0);
 ::id_0::id_38++;
{
id_35=(( ::id_0::id_38<(4)) ? (id_19[ ::id_0::id_7]) : (id_19[ ::id_0::id_14]));
goto *(id_35);
}
id_30:;
 ::id_0::id_38=(0);
{
id_35=((!( ::id_0::id_38<(4))) ? (id_19[ ::id_0::id_11]) : (id_19[ ::id_0::id_5]));
goto *(id_35);
}
id_27:;
 ::id_0::id_39=(id_19[ ::id_0::id_3]);
{
id_35=(id_19[ ::id_0::id_2]);
goto *(id_35);
}
id_25:;
id_36= ::id_0::id_40;
 ::id_0::id_37+=id_36;
 ::id_0::id_38++;
{
id_35=(( ::id_0::id_38<(4)) ? (id_19[ ::id_0::id_5]) : (id_19[ ::id_0::id_11]));
goto *(id_35);
}
id_22:;
cease(  ::id_0::id_37 );
return ;
{
id_35=(id_19[ ::id_0::id_2]);
goto *(id_35);
}
id_21:;
 ::id_0::id_41= ::id_0::id_39;
 ::id_0::id_40=(3);
id_18= ::id_0::id_41;
{
id_35=id_18;
goto *(id_35);
}
}
