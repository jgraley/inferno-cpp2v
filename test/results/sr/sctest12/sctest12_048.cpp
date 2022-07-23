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
int id_2;
private:
void *id_33;
public:
/*temp*/ int id_29;
/*temp*/ void *id_26;
/*temp*/ void *id_34;
};
id_0 id_35("id_35");

void id_0::id_1()
{
 ::id_0::id_2=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_2=(99);
break;
case 0:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=((false) ? (88) : (2));
break;
}
{
{
{
auto int id_3;
id_3=(0);
{
goto *(((0)==id_3) ? (&&id_4) : (&&id_5));
id_5:;
goto *(((1)==id_3) ? (&&id_6) : (&&id_7));
id_7:;
goto *(((4)==id_3) ? (&&id_8) : (&&id_9));
id_9:;
id_6:;
 ::id_0::id_2=(99);
goto id_10;
id_8:;
 ::id_0::id_2=(44);
id_4:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=((false) ? (88) : (2));
goto id_11;
}
}
id_11:;
}
id_10:;
}
{
{
{
auto int id_12;
id_12=(2);
{
goto *(((1)==id_12) ? (&&id_13) : (&&id_14));
id_14:;
goto *(((2)==id_12) ? (&&id_15) : (&&id_16));
id_16:;
id_13:;
 ::id_0::id_2=(99);
goto id_17;
id_15:;
{
goto *((!(((0)== ::id_0::id_2)||((2)== ::id_0::id_2))) ? (&&id_18) : (&&id_19));
id_19:;
{
/*temp*/ int id_20;
/*temp*/ int id_21;
{
goto *((!(false)) ? (&&id_22) : (&&id_23));
id_23:;
id_20=(88);
goto id_24;
id_22:;
{
/*temp*/ int id_25;
{
{
 ::id_0::id_26=(&&id_27);
goto id_28;
}
id_27:;
}
id_25= ::id_0::id_29;
id_20=id_25;
}
id_24:;
}
id_21=id_20;
 ::id_0::id_2=id_21;
}
goto id_30;
id_18:;
;
id_30:;
}
goto id_31;
}
}
id_17:;
}
id_31:;
}
cease(  ::id_0::id_2 );
return ;
id_28:;
{
/*temp*/ void *id_32;
 ::id_0::id_33= ::id_0::id_26;
{
 ::id_0::id_29=(3);
{
id_32= ::id_0::id_33;
goto *(id_32);
}
}
}
}
