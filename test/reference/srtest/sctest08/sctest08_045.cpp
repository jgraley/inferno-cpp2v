#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_14(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_16[10U]);
void *(id_15[10U]);
void *id_24;
unsigned int id_14;
public:
/*temp*/ int id_9;
/*temp*/ void *id_10;
/*temp*/ void *id_21;
/*temp*/ void *id_25;
/*temp*/ void *id_26;
};
id_0 id_27("id_27");
int id_17;
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
 ::id_0::id_9=id_4;
 ::id_0::id_10=(&&id_11);
goto id_12;
id_11:;
 ::id_5=((2)* ::id_5);
{
wait(SC_ZERO_TIME);
goto id_13;
id_13:;
}
 ::id_6=((1)+ ::id_6);
goto *(( ::id_6<(4)) ? (&&id_8) : (&&id_7));
id_7:;
cease(  ::id_5 );
return ;
id_12:;
 ::id_0::id_14++;
( ::id_0::id_15[ ::id_0::id_14])= ::id_0::id_10;
( ::id_0::id_16[ ::id_0::id_14])= ::id_0::id_9;
 ::id_17=(0);
goto *((!( ::id_17<( ::id_0::id_16[ ::id_0::id_14]))) ? (&&id_18) : (&&id_19));
id_19:;
{
wait(SC_ZERO_TIME);
goto id_20;
id_20:;
}
 ::id_5=((1)+ ::id_5);
 ::id_17=((1)+ ::id_17);
goto *(( ::id_17<( ::id_0::id_16[ ::id_0::id_14])) ? (&&id_19) : (&&id_18));
id_18:;
 ::id_0::id_21=(&&id_22);
goto id_23;
id_22:;
id_2=( ::id_0::id_15[ ::id_0::id_14]);
 ::id_0::id_14--;
goto *(id_2);
id_23:;
 ::id_0::id_24= ::id_0::id_21;
 ::id_5=( ::id_5-(1));
id_3= ::id_0::id_24;
goto *(id_3);
}
