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
void *id_13;
public:
/*temp*/ int id_10;
/*temp*/ void *id_14;
/*temp*/ void *id_7;
};
id_0 id_15("id_15");

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
switch( 0 )
{
case 1:;
 ::id_0::id_2=(99);
goto id_3;
case 4:;
 ::id_0::id_2=(44);
case 0:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=((false) ? (88) : (2));
goto id_4;
}
id_4:;
}
id_3:;
}
{
{
switch( 2 )
{
case 1:;
 ::id_0::id_2=(99);
goto id_5;
case 2:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=({ /*temp*/ int id_6; if( false )
id_6=(88);
else
id_6=({ {
{
 ::id_0::id_7=(&&id_8);
goto id_9;
}
id_8:;
}
 ::id_0::id_10; });
id_6; });
goto id_11;
}
id_5:;
}
id_11:;
}
cease(  ::id_0::id_2 );
return ;
id_9:;
{
/*temp*/ void *id_12;
 ::id_0::id_13= ::id_0::id_7;
{
 ::id_0::id_10=(3);
{
id_12= ::id_0::id_13;
goto *(id_12);
}
}
}
}
