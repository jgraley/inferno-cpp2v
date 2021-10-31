#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_32(0U)
{
SC_THREAD(id_12);
}
enum id_1
{
id_10 = 2U,
id_11 = 6U,
id_2 = 4U,
id_3 = 9U,
id_4 = 1U,
id_5 = 8U,
id_6 = 0U,
id_7 = 5U,
id_8 = 3U,
id_9 = 7U,
};
void id_12();
private:
int (id_34[10U]);
unsigned int (id_33[10U]);
unsigned int id_32;
unsigned int id_37;
public:
/*temp*/ int id_30;
/*temp*/ unsigned int id_31;
/*temp*/ unsigned int id_36;
/*temp*/ unsigned int id_38;
/*temp*/ unsigned int id_39;
};
id_0 id_40("id_40");
int id_28;
int id_29;
int id_35;

void id_0::id_12()
{
/*temp*/ unsigned int id_13;
/*temp*/ unsigned int id_14;
static const unsigned int (id_15[]) = { &&id_16, &&id_17, &&id_18, &&id_19, &&id_20, &&id_21, &&id_22, &&id_23, &&id_24, &&id_25 };
auto unsigned int id_26;
/*temp*/ int id_27;
 ::id_28=(1);
 ::id_29=(0);
wait(SC_ZERO_TIME);
id_26=((!( ::id_29<(4))) ?  ::id_0::id_8 :  ::id_0::id_6);
id_16:;
if(  ::id_0::id_6==id_26 )
{
 ::id_28=( ::id_28+ ::id_29);
id_27=(3);
 ::id_0::id_30=id_27;
 ::id_0::id_31= ::id_0::id_4;
id_26= ::id_0::id_2;
}
goto *(id_15[id_26]);
id_17:;
 ::id_28=((2)* ::id_28);
wait(SC_ZERO_TIME);
id_26= ::id_0::id_10;
goto *(id_15[id_26]);
id_18:;
 ::id_29=((1)+ ::id_29);
id_26=(( ::id_29<(4)) ?  ::id_0::id_6 :  ::id_0::id_8);
goto *(id_15[id_26]);
id_19:;
cease(  ::id_28 );
return ;
id_26= ::id_0::id_2;
goto *(id_15[id_26]);
id_20:;
 ::id_0::id_32++;
( ::id_0::id_33[ ::id_0::id_32])= ::id_0::id_31;
( ::id_0::id_34[ ::id_0::id_32])= ::id_0::id_30;
 ::id_35=(0);
id_26=((!( ::id_35<( ::id_0::id_34[ ::id_0::id_32]))) ?  ::id_0::id_9 :  ::id_0::id_7);
goto *(id_15[id_26]);
id_21:;
wait(SC_ZERO_TIME);
id_26= ::id_0::id_11;
goto *(id_15[id_26]);
id_22:;
 ::id_28=((1)+ ::id_28);
 ::id_35=((1)+ ::id_35);
id_26=(( ::id_35<( ::id_0::id_34[ ::id_0::id_32])) ?  ::id_0::id_7 :  ::id_0::id_9);
goto *(id_15[id_26]);
id_23:;
 ::id_0::id_36= ::id_0::id_5;
id_26= ::id_0::id_3;
goto *(id_15[id_26]);
id_24:;
id_13=( ::id_0::id_33[ ::id_0::id_32]);
 ::id_0::id_32--;
id_26=id_13;
goto *(id_15[id_26]);
id_25:;
 ::id_0::id_37= ::id_0::id_36;
 ::id_28=( ::id_28-(1));
id_14= ::id_0::id_37;
id_26=id_14;
goto *(id_15[id_26]);
}
