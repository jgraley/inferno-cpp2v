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
void *id_20;
public:
/*temp*/ int id_17;
/*temp*/ void *id_14;
/*temp*/ void *id_21;
};
id_0 id_22("id_22");

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
auto int id_3 = 0;
{
if( (0)==id_3 )
goto id_4;
if( (4)==id_3 )
goto id_5;
if( (1)==id_3 )
goto id_6;
id_6:;
 ::id_0::id_2=(99);
goto id_7;
id_5:;
 ::id_0::id_2=(44);
id_4:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=((false) ? (88) : (2));
goto id_8;
}
}
id_8:;
}
id_7:;
}
{
{
{
auto int id_9 = 2;
{
if( (2)==id_9 )
goto id_10;
if( (1)==id_9 )
goto id_11;
id_11:;
 ::id_0::id_2=(99);
goto id_12;
id_10:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=({ /*temp*/ int id_13; if( false )
id_13=(88);
else
id_13=({ {
{
 ::id_0::id_14=(&&id_15);
goto id_16;
}
id_15:;
}
 ::id_0::id_17; });
id_13; });
goto id_18;
}
}
id_18:;
}
id_12:;
}
cease(  ::id_0::id_2 );
return ;
id_16:;
{
/*temp*/ void *id_19;
 ::id_0::id_20= ::id_0::id_14;
{
 ::id_0::id_17=(3);
{
id_19= ::id_0::id_20;
goto *(id_19);
}
}
}
}
