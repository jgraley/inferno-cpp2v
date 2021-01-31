#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_16(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_17[10U]);
void *(id_18[10U]);
void *id_28;
unsigned int id_16;
public:
/*temp*/ int id_12;
/*temp*/ void *id_10;
/*temp*/ void *id_25;
/*temp*/ void *id_29;
/*temp*/ void *id_30;
};
id_0 id_31("id_31");
int id_19;
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
 ::id_0::id_10=(&&id_11);
 ::id_0::id_12=id_4;
goto id_13;
id_11:;
 ::id_5=((2)* ::id_5);
wait(SC_ZERO_TIME);
 ::id_6=((1)+ ::id_6);
goto *(( ::id_6<(4)) ? (&&id_9) : (&&id_14));
id_14:;
goto id_15;
id_7:;
id_15:;
cease(  ::id_5 );
return ;
id_13:;
 ::id_0::id_16++;
( ::id_0::id_17[ ::id_0::id_16])= ::id_0::id_12;
( ::id_0::id_18[ ::id_0::id_16])= ::id_0::id_10;
 ::id_19=(0);
goto *((!( ::id_19<( ::id_0::id_17[ ::id_0::id_16]))) ? (&&id_20) : (&&id_21));
id_21:;
id_22:;
wait(SC_ZERO_TIME);
 ::id_5=((1)+ ::id_5);
 ::id_19=((1)+ ::id_19);
goto *(( ::id_19<( ::id_0::id_17[ ::id_0::id_16])) ? (&&id_22) : (&&id_23));
id_23:;
goto id_24;
id_20:;
id_24:;
 ::id_0::id_25=(&&id_26);
goto id_27;
id_26:;
id_2=( ::id_0::id_18[ ::id_0::id_16]);
 ::id_0::id_16--;
goto *(id_2);
id_27:;
 ::id_0::id_28= ::id_0::id_25;
 ::id_5=( ::id_5-(1));
id_3= ::id_0::id_28;
goto *(id_3);
}
