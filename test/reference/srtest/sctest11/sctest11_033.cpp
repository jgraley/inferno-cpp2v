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
int id_13;
int id_21;
private:
int id_39;
void *id_40;
public:
/*temp*/ int id_18;
/*temp*/ int id_20;
/*temp*/ void *id_16;
/*temp*/ void *id_41;
};
id_0 id_42("id_42");

void id_0::id_1()
{
/*temp*/ void *id_2;
/*temp*/ int id_3;
/*temp*/ int id_4;
/*temp*/ int id_5;
/*temp*/ int id_6;
/*temp*/ int id_7;
/*temp*/ int id_8;
/*temp*/ bool id_10;
/*temp*/ bool id_11;
/*temp*/ bool id_12;
/*temp*/ bool id_9;
 ::id_0::id_13=(4);
id_9=(++ ::id_0::id_13);
goto *((!id_9) ? (&&id_14) : (&&id_15));
id_15:;
 ::id_0::id_16=(&&id_17);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_17:;
id_4= ::id_0::id_20;
id_9=( ::id_0::id_21=id_4);
goto id_22;
id_14:;
id_22:;
id_11=id_9;
id_11;
id_10=(!(++ ::id_0::id_13));
goto *((!id_10) ? (&&id_23) : (&&id_24));
id_24:;
goto id_25;
id_23:;
 ::id_0::id_16=(&&id_26);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_26:;
id_5= ::id_0::id_20;
id_10=( ::id_0::id_21+=id_5);
id_25:;
id_12=id_10;
id_12;
 ::id_0::id_13=(0);
goto *((!( ::id_0::id_13<(2))) ? (&&id_27) : (&&id_28));
id_28:;
id_29:;
goto *((!( ::id_0::id_13++)) ? (&&id_30) : (&&id_31));
id_31:;
 ::id_0::id_16=(&&id_32);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_32:;
id_6= ::id_0::id_20;
id_3=( ::id_0::id_21+=id_6);
goto id_33;
id_30:;
 ::id_0::id_16=(&&id_34);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_34:;
id_7= ::id_0::id_20;
id_3=( ::id_0::id_21-=id_7);
id_33:;
id_8=id_3;
id_8;
id_35:;
id_36:;
goto *(( ::id_0::id_13<(2)) ? (&&id_29) : (&&id_37));
id_37:;
goto id_38;
id_27:;
id_38:;
cease(  ::id_0::id_21 );
return ;
id_19:;
 ::id_0::id_39= ::id_0::id_18;
 ::id_0::id_40= ::id_0::id_16;
 ::id_0::id_20=((100)/ ::id_0::id_39);
id_2= ::id_0::id_40;
goto *(id_2);
}
