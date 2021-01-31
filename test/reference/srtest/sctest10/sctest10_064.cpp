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
char id_43;
short id_42;
public:
int id_35;
private:
int id_41;
int id_45;
unsigned int id_44;
public:
/*temp*/ char id_37;
/*temp*/ short id_38;
/*temp*/ int id_39;
/*temp*/ int id_40;
/*temp*/ unsigned int id_36;
/*temp*/ unsigned int id_46;
};
id_0 id_47("id_47");

void id_0::id_6()
{
/*temp*/ unsigned int id_7;
static const unsigned int (id_8[]) = { &&id_9, &&id_10, &&id_11, &&id_12 };
auto unsigned int id_13;
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
/*temp*/ int id_32;
/*temp*/ int id_33;
/*temp*/ int id_34;
 ::id_0::id_35=(0);
id_32=(8);
id_29=(6);
id_33=(0);
id_30=(0);
id_26=(0);
 ::id_0::id_36= ::id_0::id_3;
 ::id_0::id_37=id_33;
 ::id_0::id_38=id_30;
 ::id_0::id_39=id_26;
wait(SC_ZERO_TIME);
id_13= ::id_0::id_2;
id_12:;
id_11:;
id_10:;
id_9:;
if(  ::id_0::id_3==id_13 )
{
id_14= ::id_0::id_40;
id_15=id_14;
id_16=id_15;
id_17=id_16;
id_27=id_17;
 ::id_0::id_36= ::id_0::id_4;
 ::id_0::id_37=id_32;
 ::id_0::id_38=id_29;
 ::id_0::id_39=id_27;
id_13= ::id_0::id_2;
}
if(  ::id_0::id_4==id_13 )
{
id_18= ::id_0::id_40;
id_19=id_18;
id_20=id_19;
id_21=id_20;
 ::id_0::id_35=id_21;
id_34=(3);
id_31=(2);
id_28=(1);
 ::id_0::id_36= ::id_0::id_5;
 ::id_0::id_37=id_34;
 ::id_0::id_38=id_31;
 ::id_0::id_39=id_28;
id_13= ::id_0::id_2;
}
if(  ::id_0::id_5==id_13 )
{
id_22= ::id_0::id_40;
id_23=id_22;
id_24=id_23;
id_25=id_24;
cease(  ::id_0::id_35+((2)*id_25) );
return ;
id_13= ::id_0::id_2;
}
if(  ::id_0::id_2==id_13 )
{
 ::id_0::id_41= ::id_0::id_39;
 ::id_0::id_42= ::id_0::id_38;
 ::id_0::id_43= ::id_0::id_37;
 ::id_0::id_44= ::id_0::id_36;
 ::id_0::id_45=( ::id_0::id_41+((3)* ::id_0::id_42));
 ::id_0::id_40=( ::id_0::id_45+((5)* ::id_0::id_43));
id_7= ::id_0::id_44;
id_13=id_7;
}
goto *(id_8[id_13]);
}
