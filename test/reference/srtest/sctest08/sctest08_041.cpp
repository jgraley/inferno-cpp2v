#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_13(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_14[10U]);
void *(id_15[10U]);
void *id_22;
unsigned int id_13;
public:
/*temp*/ int id_11;
/*temp*/ void *id_19;
/*temp*/ void *id_23;
/*temp*/ void *id_24;
/*temp*/ void *id_9;
};
id_0 id_25("id_25");
int id_16;
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
 ::id_5=( ::id_5+ ::id_6);
id_4=(3);
 ::id_0::id_9=(&&id_10);
 ::id_0::id_11=id_4;
goto id_12;
id_10:;
 ::id_5=((2)* ::id_5);
wait(SC_ZERO_TIME);
 ::id_6=((1)+ ::id_6);
goto *(( ::id_6<(4)) ? (&&id_8) : (&&id_7));
id_7:;
cease(  ::id_5 );
return ;
id_12:;
 ::id_0::id_13++;
( ::id_0::id_14[ ::id_0::id_13])= ::id_0::id_11;
( ::id_0::id_15[ ::id_0::id_13])= ::id_0::id_9;
 ::id_16=(0);
goto *((!( ::id_16<( ::id_0::id_14[ ::id_0::id_13]))) ? (&&id_17) : (&&id_18));
id_18:;
wait(SC_ZERO_TIME);
 ::id_5=((1)+ ::id_5);
 ::id_16=((1)+ ::id_16);
goto *(( ::id_16<( ::id_0::id_14[ ::id_0::id_13])) ? (&&id_18) : (&&id_17));
id_17:;
 ::id_0::id_19=(&&id_20);
goto id_21;
id_20:;
id_2=( ::id_0::id_15[ ::id_0::id_13]);
 ::id_0::id_13--;
goto *(id_2);
id_21:;
 ::id_0::id_22= ::id_0::id_19;
 ::id_5=( ::id_5-(1));
id_3= ::id_0::id_22;
goto *(id_3);
}
