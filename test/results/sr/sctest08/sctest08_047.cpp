#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_17(0)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_19[10]);
void *(id_18[10]);
void *id_32;
unsigned int id_17;
public:
/*temp*/ int id_8;
/*temp*/ void *id_28;
/*temp*/ void *id_33;
/*temp*/ void *id_34;
/*temp*/ void *id_9;
};
id_0 id_35("id_35");
int id_2;
int id_20;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
{
goto *((!( ::id_3<(4))) ? (&&id_4) : (&&id_5));
id_5:;
{
id_6:;
{
{
 ::id_2=( ::id_2+ ::id_3);
{
/*temp*/ int id_7;
id_7=(3);
{
{
 ::id_0::id_8=id_7;
{
 ::id_0::id_9=(&&id_10);
goto id_11;
}
}
id_10:;
}
}
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
id_12:;
 ::id_3=((1)+ ::id_3);
}
id_13:;
goto *(( ::id_3<(4)) ? (&&id_6) : (&&id_14));
id_14:;
}
goto id_15;
id_4:;
;
id_15:;
}
}
cease(  ::id_2 );
return ;
id_11:;
{
{
/*temp*/ void *id_16;
 ::id_0::id_17++;
( ::id_0::id_18[ ::id_0::id_17])= ::id_0::id_9;
( ::id_0::id_19[ ::id_0::id_17])= ::id_0::id_8;
{
 ::id_20=(0);
{
goto *((!( ::id_20<( ::id_0::id_19[ ::id_0::id_17]))) ? (&&id_21) : (&&id_22));
id_22:;
{
id_23:;
{
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
id_24:;
 ::id_20=((1)+ ::id_20);
}
id_25:;
goto *(( ::id_20<( ::id_0::id_19[ ::id_0::id_17])) ? (&&id_23) : (&&id_26));
id_26:;
}
goto id_27;
id_21:;
;
id_27:;
}
}
{
{
 ::id_0::id_28=(&&id_29);
goto id_30;
}
id_29:;
}
{
id_16=( ::id_0::id_18[ ::id_0::id_17]);
{
 ::id_0::id_17--;
goto *(id_16);
}
}
}
}
id_30:;
{
/*temp*/ void *id_31;
 ::id_0::id_32= ::id_0::id_28;
 ::id_2=( ::id_2-(1));
{
id_31= ::id_0::id_32;
goto *(id_31);
}
}
}
