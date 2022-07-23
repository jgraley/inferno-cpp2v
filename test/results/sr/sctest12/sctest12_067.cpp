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
int id_9;
private:
void *id_28;
public:
/*temp*/ int id_27;
/*temp*/ void *id_24;
/*temp*/ void *id_29;
};
id_0 id_30("id_30");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto int id_3;
auto int id_4;
auto void *id_5;
/*temp*/ int id_6;
/*temp*/ int id_7;
/*temp*/ int id_8;
 ::id_0::id_9=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_9=(99);
break;
case 0:;
if( ((0)== ::id_0::id_9)||((2)== ::id_0::id_9) )
 ::id_0::id_9=((false) ? (88) : (2));
break;
}
id_3=(0);
wait(SC_ZERO_TIME);
{
id_5=(((0)==id_3) ? (&&id_10) : (&&id_11));
goto *(id_5);
}
id_11:;
{
id_5=(((1)==id_3) ? (&&id_12) : (&&id_13));
goto *(id_5);
}
id_13:;
{
id_5=(((4)==id_3) ? (&&id_14) : (&&id_12));
goto *(id_5);
}
id_12:;
 ::id_0::id_9=(99);
{
id_5=(&&id_15);
goto *(id_5);
}
id_14:;
 ::id_0::id_9=(44);
{
id_5=(&&id_10);
goto *(id_5);
}
id_10:;
if( ((0)== ::id_0::id_9)||((2)== ::id_0::id_9) )
 ::id_0::id_9=((false) ? (88) : (2));
{
id_5=(&&id_15);
goto *(id_5);
}
id_15:;
id_4=(2);
{
id_5=(((1)==id_4) ? (&&id_16) : (&&id_17));
goto *(id_5);
}
id_17:;
{
id_5=(((2)==id_4) ? (&&id_18) : (&&id_16));
goto *(id_5);
}
id_16:;
 ::id_0::id_9=(99);
{
id_5=(&&id_19);
goto *(id_5);
}
id_18:;
{
id_5=((!(((0)== ::id_0::id_9)||((2)== ::id_0::id_9))) ? (&&id_19) : (&&id_20));
goto *(id_5);
}
id_20:;
{
id_5=((!(false)) ? (&&id_21) : (&&id_22));
goto *(id_5);
}
id_22:;
id_6=(88);
{
id_5=(&&id_23);
goto *(id_5);
}
id_21:;
 ::id_0::id_24=(&&id_25);
{
id_5=(&&id_26);
goto *(id_5);
}
id_25:;
id_7= ::id_0::id_27;
id_6=id_7;
{
id_5=(&&id_23);
goto *(id_5);
}
id_23:;
id_8=id_6;
 ::id_0::id_9=id_8;
{
id_5=(&&id_19);
goto *(id_5);
}
id_19:;
cease(  ::id_0::id_9 );
return ;
{
id_5=(&&id_26);
goto *(id_5);
}
id_26:;
 ::id_0::id_28= ::id_0::id_24;
 ::id_0::id_27=(3);
id_2= ::id_0::id_28;
{
id_5=id_2;
goto *(id_5);
}
}
