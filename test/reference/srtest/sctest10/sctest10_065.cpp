#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_6);
}
enum id_1
{
id_2 = 3U,
id_3 = 0U,
id_4 = 1U,
id_5 = 2U,
};
void id_6();
private:
char id_40;
short id_39;
public:
int id_32;
private:
int id_38;
int id_42;
unsigned int id_41;
public:
/*temp*/ char id_34;
/*temp*/ short id_35;
/*temp*/ int id_36;
/*temp*/ int id_37;
/*temp*/ unsigned int id_33;
/*temp*/ unsigned int id_43;
};
id_0 id_44("id_44");

void id_0::id_6()
{
/*temp*/ unsigned int id_7;
static const unsigned int (id_8[]) = { &&id_9, &&id_9, &&id_9, &&id_9 };
auto unsigned int id_10;
/*temp*/ int id_11;
/*temp*/ int id_12;
/*temp*/ int id_13;
/*temp*/ int id_14;
/*temp*/ int id_15;
/*temp*/ int id_16;
/*temp*/ int id_17;
/*temp*/ int id_18;
/*temp*/ int id_19;
/*temp*/ int id_20;
/*temp*/ int id_21;
/*temp*/ int id_22;
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ int id_25;
/*temp*/ int id_26;
/*temp*/ int id_27;
/*temp*/ int id_28;
/*temp*/ int id_29;
/*temp*/ int id_30;
/*temp*/ int id_31;
 ::id_0::id_32=(0);
id_29=(8);
id_26=(6);
id_30=(0);
id_27=(0);
id_23=(0);
 ::id_0::id_33= ::id_0::id_3;
 ::id_0::id_34=id_30;
 ::id_0::id_35=id_27;
 ::id_0::id_36=id_23;
wait(SC_ZERO_TIME);
id_10= ::id_0::id_2;
id_9:;
if(  ::id_0::id_3==id_10 )
{
id_11= ::id_0::id_37;
id_12=id_11;
id_13=id_12;
id_14=id_13;
id_24=id_14;
 ::id_0::id_33= ::id_0::id_4;
 ::id_0::id_34=id_29;
 ::id_0::id_35=id_26;
 ::id_0::id_36=id_24;
id_10= ::id_0::id_2;
}
if(  ::id_0::id_4==id_10 )
{
id_15= ::id_0::id_37;
id_16=id_15;
id_17=id_16;
id_18=id_17;
 ::id_0::id_32=id_18;
id_31=(3);
id_28=(2);
id_25=(1);
 ::id_0::id_33= ::id_0::id_5;
 ::id_0::id_34=id_31;
 ::id_0::id_35=id_28;
 ::id_0::id_36=id_25;
id_10= ::id_0::id_2;
}
if(  ::id_0::id_5==id_10 )
{
id_19= ::id_0::id_37;
id_20=id_19;
id_21=id_20;
id_22=id_21;
cease(  ::id_0::id_32+((2)*id_22) );
return ;
id_10= ::id_0::id_2;
}
if(  ::id_0::id_2==id_10 )
{
 ::id_0::id_38= ::id_0::id_36;
 ::id_0::id_39= ::id_0::id_35;
 ::id_0::id_40= ::id_0::id_34;
 ::id_0::id_41= ::id_0::id_33;
 ::id_0::id_42=( ::id_0::id_38+((3)* ::id_0::id_39));
 ::id_0::id_37=( ::id_0::id_42+((5)* ::id_0::id_40));
id_7= ::id_0::id_41;
id_10=id_7;
}
goto *(id_8[id_10]);
}
