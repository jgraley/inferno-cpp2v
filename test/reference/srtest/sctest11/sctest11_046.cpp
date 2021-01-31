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
int id_32;
void *id_33;
public:
/*temp*/ int id_18;
/*temp*/ int id_20;
/*temp*/ void *id_16;
/*temp*/ void *id_34;
};
id_0 id_35("id_35");

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
goto id_14;
id_14:;
id_11=id_9;
id_11;
id_10=(!(++ ::id_0::id_13));
goto *((!id_10) ? (&&id_22) : (&&id_23));
id_22:;
 ::id_0::id_16=(&&id_24);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_24:;
id_5= ::id_0::id_20;
id_10=( ::id_0::id_21+=id_5);
goto id_23;
id_23:;
id_12=id_10;
id_12;
 ::id_0::id_13=(0);
goto *((!( ::id_0::id_13<(2))) ? (&&id_25) : (&&id_26));
id_26:;
goto *((!( ::id_0::id_13++)) ? (&&id_27) : (&&id_28));
id_28:;
 ::id_0::id_16=(&&id_29);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_29:;
id_6= ::id_0::id_20;
id_3=( ::id_0::id_21+=id_6);
goto id_30;
id_27:;
 ::id_0::id_16=(&&id_31);
 ::id_0::id_18= ::id_0::id_13;
goto id_19;
id_31:;
id_7= ::id_0::id_20;
id_3=( ::id_0::id_21-=id_7);
goto id_30;
id_30:;
id_8=id_3;
id_8;
goto *(( ::id_0::id_13<(2)) ? (&&id_26) : (&&id_25));
id_25:;
cease(  ::id_0::id_21 );
return ;
goto id_19;
id_19:;
 ::id_0::id_32= ::id_0::id_18;
 ::id_0::id_33= ::id_0::id_16;
 ::id_0::id_20=((100)/ ::id_0::id_32);
id_2= ::id_0::id_33;
goto *(id_2);
}
