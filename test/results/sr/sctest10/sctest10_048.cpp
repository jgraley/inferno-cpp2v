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
char id_35;
short id_34;
public:
int id_2;
private:
int id_37;
int id_38;
void *id_36;
public:
/*temp*/ char id_15;
/*temp*/ short id_16;
/*temp*/ int id_12;
/*temp*/ int id_18;
/*temp*/ void *id_13;
/*temp*/ void *id_39;
};
id_0 id_40("id_40");

void id_0::id_1()
{
 ::id_0::id_2=(0);
{
/*temp*/ int id_3;
/*temp*/ int id_4;
{
/*temp*/ int id_5;
/*temp*/ int id_6;
id_6=(0);
{
/*temp*/ int id_7;
/*temp*/ int id_8;
id_8=(0);
{
/*temp*/ int id_10;
/*temp*/ int id_9;
id_10=(0);
{
/*temp*/ int id_11;
{
{
 ::id_0::id_12=id_6;
{
 ::id_0::id_13=(&&id_14);
{
 ::id_0::id_15=id_8;
{
 ::id_0::id_16=id_10;
goto id_17;
}
}
}
}
id_14:;
}
id_11= ::id_0::id_18;
id_9=id_11;
}
id_7=id_9;
}
id_5=id_7;
}
id_4=id_5;
}
{
/*temp*/ int id_19;
/*temp*/ int id_20;
id_20=(8);
{
/*temp*/ int id_21;
/*temp*/ int id_22;
id_22=(6);
{
/*temp*/ int id_23;
{
{
 ::id_0::id_12=id_4;
{
 ::id_0::id_13=(&&id_24);
{
 ::id_0::id_15=id_20;
{
 ::id_0::id_16=id_22;
goto id_17;
}
}
}
}
id_24:;
}
id_23= ::id_0::id_18;
id_21=id_23;
}
id_19=id_21;
}
id_3=id_19;
}
 ::id_0::id_2=id_3;
}
{
/*temp*/ int id_25;
/*temp*/ int id_26;
id_26=(1);
{
/*temp*/ int id_27;
/*temp*/ int id_28;
id_28=(3);
{
/*temp*/ int id_29;
/*temp*/ int id_30;
id_30=(2);
{
/*temp*/ int id_31;
{
{
 ::id_0::id_12=id_26;
{
 ::id_0::id_13=(&&id_32);
{
 ::id_0::id_15=id_28;
{
 ::id_0::id_16=id_30;
goto id_17;
}
}
}
}
id_32:;
}
id_31= ::id_0::id_18;
id_29=id_31;
}
id_27=id_29;
}
id_25=id_27;
}
cease(  ::id_0::id_2+((2)*id_25) );
}
return ;
id_17:;
{
/*temp*/ void *id_33;
 ::id_0::id_34= ::id_0::id_16;
 ::id_0::id_35= ::id_0::id_15;
 ::id_0::id_36= ::id_0::id_13;
 ::id_0::id_37= ::id_0::id_12;
 ::id_0::id_38=( ::id_0::id_37+((3)* ::id_0::id_34));
{
 ::id_0::id_18=( ::id_0::id_38+((5)* ::id_0::id_35));
{
id_33= ::id_0::id_36;
goto *(id_33);
}
}
}
}
