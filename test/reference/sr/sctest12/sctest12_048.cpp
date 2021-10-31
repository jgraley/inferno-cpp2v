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
int id_8;
private:
void *id_33;
public:
/*temp*/ int id_30;
/*temp*/ void *id_27;
/*temp*/ void *id_34;
};
id_0 id_35("id_35");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto int id_3;
auto int id_4;
/*temp*/ int id_5;
/*temp*/ int id_6;
/*temp*/ int id_7;
 ::id_0::id_8=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_8=(99);
break;
case 0:;
if( ((0)== ::id_0::id_8)||((2)== ::id_0::id_8) )
 ::id_0::id_8=((false) ? (88) : (2));
break;
}
id_3=(0);
goto *(((0)==id_3) ? (&&id_9) : (&&id_10));
id_10:;
goto *(((4)==id_3) ? (&&id_11) : (&&id_12));
id_12:;
goto *(((1)==id_3) ? (&&id_13) : (&&id_14));
id_14:;
id_13:;
 ::id_0::id_8=(99);
goto id_15;
id_11:;
 ::id_0::id_8=(44);
id_9:;
if( ((0)== ::id_0::id_8)||((2)== ::id_0::id_8) )
 ::id_0::id_8=((false) ? (88) : (2));
goto id_16;
id_16:;
id_15:;
id_4=(2);
goto *(((2)==id_4) ? (&&id_17) : (&&id_18));
id_18:;
goto *(((1)==id_4) ? (&&id_19) : (&&id_20));
id_20:;
id_19:;
 ::id_0::id_8=(99);
goto id_21;
id_17:;
goto *((!(((0)== ::id_0::id_8)||((2)== ::id_0::id_8))) ? (&&id_22) : (&&id_23));
id_23:;
goto *((!(false)) ? (&&id_24) : (&&id_25));
id_25:;
id_5=(88);
goto id_26;
id_24:;
 ::id_0::id_27=(&&id_28);
goto id_29;
id_28:;
id_6= ::id_0::id_30;
id_5=id_6;
id_26:;
id_7=id_5;
 ::id_0::id_8=id_7;
goto id_31;
id_22:;
;
id_31:;
goto id_32;
id_32:;
id_21:;
cease(  ::id_0::id_8 );
return ;
id_29:;
 ::id_0::id_33= ::id_0::id_27;
 ::id_0::id_30=(3);
id_2= ::id_0::id_33;
goto *(id_2);
}
