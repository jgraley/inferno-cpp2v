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
void *id_12;
public:
/*temp*/ int id_9;
/*temp*/ void *id_13;
/*temp*/ void *id_6;
};
id_0 id_14("id_14");

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
 ::id_0::id_2=((false) ? (88) : ({ {
{
 ::id_0::id_6=(&&id_7);
goto id_8;
}
id_7:;
}
 ::id_0::id_9; }));
goto id_10;
}
id_10:;
}
id_5:;
}
cease(  ::id_0::id_2 );
return ;
id_8:;
{
/*temp*/ void *id_11;
 ::id_0::id_12= ::id_0::id_6;
{
 ::id_0::id_9=(3);
{
id_11= ::id_0::id_12;
goto *(id_11);
}
}
}
}
