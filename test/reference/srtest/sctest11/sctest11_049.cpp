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
int id_14;
int id_22;
private:
int id_34;
void *id_33;
public:
/*temp*/ int id_17;
/*temp*/ int id_21;
/*temp*/ void *id_18;
/*temp*/ void *id_35;
};
id_0 id_36("id_36");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto void *id_3;
/*temp*/ int id_4;
/*temp*/ int id_5;
/*temp*/ int id_6;
/*temp*/ int id_7;
/*temp*/ int id_8;
/*temp*/ int id_9;
/*temp*/ bool id_10;
/*temp*/ bool id_11;
/*temp*/ bool id_12;
/*temp*/ bool id_13;
 ::id_0::id_14=(4);
id_10=(++ ::id_0::id_14);
wait(SC_ZERO_TIME);
{
id_3=((!id_10) ? (&&id_15) : (&&id_16));
goto *(id_3);
}
id_16:;
 ::id_0::id_17= ::id_0::id_14;
 ::id_0::id_18=(&&id_19);
{
id_3=(&&id_20);
goto *(id_3);
}
id_19:;
id_5= ::id_0::id_21;
id_10=( ::id_0::id_22=id_5);
{
id_3=(&&id_15);
goto *(id_3);
}
id_15:;
id_12=id_10;
id_12;
id_11=(!(++ ::id_0::id_14));
{
id_3=((!id_11) ? (&&id_23) : (&&id_24));
goto *(id_3);
}
id_23:;
 ::id_0::id_17= ::id_0::id_14;
 ::id_0::id_18=(&&id_25);
{
id_3=(&&id_20);
goto *(id_3);
}
id_25:;
id_6= ::id_0::id_21;
id_11=( ::id_0::id_22+=id_6);
{
id_3=(&&id_24);
goto *(id_3);
}
id_24:;
id_13=id_11;
id_13;
 ::id_0::id_14=(0);
{
id_3=((!( ::id_0::id_14<(2))) ? (&&id_26) : (&&id_27));
goto *(id_3);
}
id_27:;
{
id_3=((!( ::id_0::id_14++)) ? (&&id_28) : (&&id_29));
goto *(id_3);
}
id_29:;
 ::id_0::id_17= ::id_0::id_14;
 ::id_0::id_18=(&&id_30);
{
id_3=(&&id_20);
goto *(id_3);
}
id_30:;
id_7= ::id_0::id_21;
id_4=( ::id_0::id_22+=id_7);
{
id_3=(&&id_31);
goto *(id_3);
}
id_28:;
 ::id_0::id_17= ::id_0::id_14;
 ::id_0::id_18=(&&id_32);
{
id_3=(&&id_20);
goto *(id_3);
}
id_32:;
id_8= ::id_0::id_21;
id_4=( ::id_0::id_22-=id_8);
{
id_3=(&&id_31);
goto *(id_3);
}
id_31:;
id_9=id_4;
id_9;
{
id_3=(( ::id_0::id_14<(2)) ? (&&id_27) : (&&id_26));
goto *(id_3);
}
id_26:;
cease(  ::id_0::id_22 );
return ;
{
id_3=(&&id_20);
goto *(id_3);
}
id_20:;
 ::id_0::id_33= ::id_0::id_18;
 ::id_0::id_34= ::id_0::id_17;
 ::id_0::id_21=((100)/ ::id_0::id_34);
id_2= ::id_0::id_33;
{
id_3=id_2;
goto *(id_3);
}
}
