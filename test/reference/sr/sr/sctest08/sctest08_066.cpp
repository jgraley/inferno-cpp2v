#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_15(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_17[10U]);
void *(id_16[10U]);
void *id_25;
unsigned int id_15;
public:
/*temp*/ int id_10;
/*temp*/ void *id_11;
/*temp*/ void *id_22;
/*temp*/ void *id_26;
/*temp*/ void *id_27;
};
id_0 id_28("id_28");
int id_18;
int id_6;
int id_7;

void id_0::id_1()
{
/*temp*/ void *id_2;
/*temp*/ void *id_3;
auto void *id_4;
/*temp*/ int id_5;
 ::id_6=(1);
 ::id_7=(0);
wait(SC_ZERO_TIME);
{
id_4=((!( ::id_7<(4))) ? (&&id_8) : (&&id_9));
goto *(id_4);
}
id_9:;
 ::id_6=( ::id_6+ ::id_7);
id_5=(3);
 ::id_0::id_10=id_5;
 ::id_0::id_11=(&&id_12);
{
id_4=(&&id_13);
goto *(id_4);
}
id_12:;
 ::id_6=((2)* ::id_6);
wait(SC_ZERO_TIME);
{
id_4=(&&id_14);
goto *(id_4);
}
id_14:;
 ::id_7=((1)+ ::id_7);
{
id_4=(( ::id_7<(4)) ? (&&id_9) : (&&id_8));
goto *(id_4);
}
id_8:;
cease(  ::id_6 );
return ;
{
id_4=(&&id_13);
goto *(id_4);
}
id_13:;
 ::id_0::id_15++;
( ::id_0::id_16[ ::id_0::id_15])= ::id_0::id_11;
( ::id_0::id_17[ ::id_0::id_15])= ::id_0::id_10;
 ::id_18=(0);
{
id_4=((!( ::id_18<( ::id_0::id_17[ ::id_0::id_15]))) ? (&&id_19) : (&&id_20));
goto *(id_4);
}
id_20:;
wait(SC_ZERO_TIME);
{
id_4=(&&id_21);
goto *(id_4);
}
id_21:;
 ::id_6=((1)+ ::id_6);
 ::id_18=((1)+ ::id_18);
{
id_4=(( ::id_18<( ::id_0::id_17[ ::id_0::id_15])) ? (&&id_20) : (&&id_19));
goto *(id_4);
}
id_19:;
 ::id_0::id_22=(&&id_23);
{
id_4=(&&id_24);
goto *(id_4);
}
id_23:;
id_2=( ::id_0::id_16[ ::id_0::id_15]);
 ::id_0::id_15--;
{
id_4=id_2;
goto *(id_4);
}
id_24:;
 ::id_0::id_25= ::id_0::id_22;
 ::id_6=( ::id_6-(1));
id_3= ::id_0::id_25;
{
id_4=id_3;
goto *(id_4);
}
}
