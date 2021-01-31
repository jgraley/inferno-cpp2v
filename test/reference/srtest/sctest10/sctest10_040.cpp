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
char id_36;
short id_35;
public:
int id_24;
private:
int id_34;
int id_38;
void *id_37;
public:
/*temp*/ char id_27;
/*temp*/ short id_28;
/*temp*/ int id_29;
/*temp*/ int id_31;
/*temp*/ void *id_25;
/*temp*/ void *id_39;
};
id_0 id_40("id_40");

void id_0::id_1()
{
/*temp*/ void *id_2;
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
/*temp*/ int id_3;
/*temp*/ int id_4;
/*temp*/ int id_5;
/*temp*/ int id_6;
/*temp*/ int id_7;
/*temp*/ int id_8;
/*temp*/ int id_9;
 ::id_0::id_24=(0);
id_21=(8);
id_18=(6);
id_22=(0);
id_19=(0);
id_15=(0);
 ::id_0::id_25=(&&id_26);
 ::id_0::id_27=id_22;
 ::id_0::id_28=id_19;
 ::id_0::id_29=id_15;
goto id_30;
id_26:;
id_3= ::id_0::id_31;
id_4=id_3;
id_5=id_4;
id_6=id_5;
id_16=id_6;
 ::id_0::id_25=(&&id_32);
 ::id_0::id_27=id_21;
 ::id_0::id_28=id_18;
 ::id_0::id_29=id_16;
goto id_30;
id_32:;
id_7= ::id_0::id_31;
id_8=id_7;
id_9=id_8;
id_10=id_9;
 ::id_0::id_24=id_10;
id_23=(3);
id_20=(2);
id_17=(1);
 ::id_0::id_25=(&&id_33);
 ::id_0::id_27=id_23;
 ::id_0::id_28=id_20;
 ::id_0::id_29=id_17;
goto id_30;
id_33:;
id_11= ::id_0::id_31;
id_12=id_11;
id_13=id_12;
id_14=id_13;
cease(  ::id_0::id_24+((2)*id_14) );
return ;
id_30:;
 ::id_0::id_34= ::id_0::id_29;
 ::id_0::id_35= ::id_0::id_28;
 ::id_0::id_36= ::id_0::id_27;
 ::id_0::id_37= ::id_0::id_25;
 ::id_0::id_38=( ::id_0::id_34+((3)* ::id_0::id_35));
 ::id_0::id_31=( ::id_0::id_38+((5)* ::id_0::id_36));
id_2= ::id_0::id_37;
goto *(id_2);
}
