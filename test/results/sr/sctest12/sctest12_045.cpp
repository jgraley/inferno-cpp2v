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
void *id_31;
public:
/*temp*/ int id_27;
/*temp*/ void *id_24;
/*temp*/ void *id_32;
};
id_0 id_33("id_33");

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
if( (0)==id_3 )
goto id_4;
goto id_5;
id_5:;
if( (1)==id_3 )
goto id_6;
goto id_7;
id_7:;
if( (4)==id_3 )
goto id_8;
goto id_9;
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
if( (1)==id_12 )
goto id_13;
goto id_14;
id_14:;
if( (2)==id_12 )
goto id_15;
goto id_16;
id_16:;
id_13:;
 ::id_0::id_2=(99);
goto id_17;
id_15:;
{
if( !(((0)== ::id_0::id_2)||((2)== ::id_0::id_2)) )
goto id_18;
goto id_19;
id_19:;
 ::id_0::id_2=({ /*temp*/ int id_20; {
if( !(false) )
goto id_21;
goto id_22;
id_22:;
id_20=(88);
goto id_23;
id_21:;
id_20=({ {
{
 ::id_0::id_24=(&&id_25);
goto id_26;
}
id_25:;
}
 ::id_0::id_27; });
id_23:;
}
id_20; });
goto id_28;
id_18:;
;
id_28:;
}
goto id_29;
}
}
id_17:;
}
id_29:;
}
cease(  ::id_0::id_2 );
return ;
id_26:;
{
/*temp*/ void *id_30;
 ::id_0::id_31= ::id_0::id_24;
{
 ::id_0::id_27=(3);
{
id_30= ::id_0::id_31;
goto *(id_30);
}
}
}
}
