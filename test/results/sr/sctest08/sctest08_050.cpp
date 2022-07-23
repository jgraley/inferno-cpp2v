#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_18(0)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_20[10]);
void *(id_19[10]);
void *id_32;
unsigned int id_18;
public:
/*temp*/ int id_10;
/*temp*/ void *id_11;
/*temp*/ void *id_29;
/*temp*/ void *id_33;
/*temp*/ void *id_34;
};
id_0 id_35("id_35");
int id_21;
int id_5;
int id_6;

void id_0::id_1()
{
/*temp*/ void *id_2;
/*temp*/ void *id_3;
/*temp*/ int id_4;
 ::id_5=(1);
 ::id_6=(0);
goto *((!( ::id_6<(4))) ? (&&id_7) : (&&id_8));
id_8:;
id_9:;
 ::id_5=( ::id_5+ ::id_6);
id_4=(3);
 ::id_0::id_10=id_4;
 ::id_0::id_11=(&&id_12);
goto id_13;
id_12:;
 ::id_5=((2)* ::id_5);
wait(SC_ZERO_TIME);
id_14:;
 ::id_6=((1)+ ::id_6);
id_15:;
goto *(( ::id_6<(4)) ? (&&id_9) : (&&id_16));
id_16:;
goto id_17;
id_7:;
;
id_17:;
cease(  ::id_5 );
return ;
id_13:;
 ::id_0::id_18++;
( ::id_0::id_19[ ::id_0::id_18])= ::id_0::id_11;
( ::id_0::id_20[ ::id_0::id_18])= ::id_0::id_10;
 ::id_21=(0);
goto *((!( ::id_21<( ::id_0::id_20[ ::id_0::id_18]))) ? (&&id_22) : (&&id_23));
id_23:;
id_24:;
wait(SC_ZERO_TIME);
 ::id_5=((1)+ ::id_5);
id_25:;
 ::id_21=((1)+ ::id_21);
id_26:;
goto *(( ::id_21<( ::id_0::id_20[ ::id_0::id_18])) ? (&&id_24) : (&&id_27));
id_27:;
goto id_28;
id_22:;
;
id_28:;
 ::id_0::id_29=(&&id_30);
goto id_31;
id_30:;
id_2=( ::id_0::id_19[ ::id_0::id_18]);
 ::id_0::id_18--;
goto *(id_2);
id_31:;
 ::id_0::id_32= ::id_0::id_29;
 ::id_5=( ::id_5-(1));
id_3= ::id_0::id_32;
goto *(id_3);
}
