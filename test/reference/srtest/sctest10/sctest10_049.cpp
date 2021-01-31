#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_1);
}
void id_1();
private:
char id_37;
short id_36;
public:
int id_25;
private:
int id_35;
int id_39;
void *id_38;
public:
/*temp*/ char id_28;
/*temp*/ short id_29;
/*temp*/ int id_30;
/*temp*/ int id_32;
/*temp*/ void *id_26;
/*temp*/ void *id_40;
};
id_0 id_41("id_41");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto void *id_3;
/*temp*/ int id_10;
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
/*temp*/ int id_4;
/*temp*/ int id_5;
/*temp*/ int id_6;
/*temp*/ int id_7;
/*temp*/ int id_8;
/*temp*/ int id_9;
 ::id_0::id_25=(0);
id_22=(8);
id_19=(6);
id_23=(0);
id_20=(0);
id_16=(0);
 ::id_0::id_26=(&&id_27);
 ::id_0::id_28=id_23;
 ::id_0::id_29=id_20;
 ::id_0::id_30=id_16;
wait(SC_ZERO_TIME);
{
id_3=(&&id_31);
goto *(id_3);
}
id_27:;
id_4= ::id_0::id_32;
id_5=id_4;
id_6=id_5;
id_7=id_6;
id_17=id_7;
 ::id_0::id_26=(&&id_33);
 ::id_0::id_28=id_22;
 ::id_0::id_29=id_19;
 ::id_0::id_30=id_17;
{
id_3=(&&id_31);
goto *(id_3);
}
id_33:;
id_8= ::id_0::id_32;
id_9=id_8;
id_10=id_9;
id_11=id_10;
 ::id_0::id_25=id_11;
id_24=(3);
id_21=(2);
id_18=(1);
 ::id_0::id_26=(&&id_34);
 ::id_0::id_28=id_24;
 ::id_0::id_29=id_21;
 ::id_0::id_30=id_18;
{
id_3=(&&id_31);
goto *(id_3);
}
id_34:;
id_12= ::id_0::id_32;
id_13=id_12;
id_14=id_13;
id_15=id_14;
cease(  ::id_0::id_25+((2)*id_15) );
return ;
{
id_3=(&&id_31);
goto *(id_3);
}
id_31:;
 ::id_0::id_35= ::id_0::id_30;
 ::id_0::id_36= ::id_0::id_29;
 ::id_0::id_37= ::id_0::id_28;
 ::id_0::id_38= ::id_0::id_26;
 ::id_0::id_39=( ::id_0::id_35+((3)* ::id_0::id_36));
 ::id_0::id_32=( ::id_0::id_39+((5)* ::id_0::id_37));
id_2= ::id_0::id_38;
{
id_3=id_2;
goto *(id_3);
}
}
