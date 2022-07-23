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
void *id_27;
public:
/*temp*/ int id_26;
/*temp*/ void *id_23;
/*temp*/ void *id_28;
};
id_0 id_29("id_29");

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
wait(SC_ZERO_TIME);
goto *(((0)==id_3) ? (&&id_9) : (&&id_10));
id_10:;
goto *(((1)==id_3) ? (&&id_11) : (&&id_12));
id_12:;
goto *(((4)==id_3) ? (&&id_13) : (&&id_11));
id_11:;
 ::id_0::id_8=(99);
goto id_14;
id_13:;
 ::id_0::id_8=(44);
goto id_9;
id_9:;
if( ((0)== ::id_0::id_8)||((2)== ::id_0::id_8) )
 ::id_0::id_8=((false) ? (88) : (2));
goto id_14;
id_14:;
id_4=(2);
goto *(((1)==id_4) ? (&&id_15) : (&&id_16));
id_16:;
goto *(((2)==id_4) ? (&&id_17) : (&&id_15));
id_15:;
 ::id_0::id_8=(99);
goto id_18;
id_17:;
goto *((!(((0)== ::id_0::id_8)||((2)== ::id_0::id_8))) ? (&&id_18) : (&&id_19));
id_19:;
goto *((!(false)) ? (&&id_20) : (&&id_21));
id_21:;
id_5=(88);
goto id_22;
id_20:;
 ::id_0::id_23=(&&id_24);
goto id_25;
id_24:;
id_6= ::id_0::id_26;
id_5=id_6;
goto id_22;
id_22:;
id_7=id_5;
 ::id_0::id_8=id_7;
goto id_18;
id_18:;
cease(  ::id_0::id_8 );
return ;
goto id_25;
id_25:;
 ::id_0::id_27= ::id_0::id_23;
 ::id_0::id_26=(3);
id_2= ::id_0::id_27;
goto *(id_2);
}
