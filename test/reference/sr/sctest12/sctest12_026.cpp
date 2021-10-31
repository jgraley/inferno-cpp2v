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
void (id_3)();
/*temp*/ int id_4;
};
id_0 id_5("id_5");

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
switch( 0 )
{
case 1:;
 ::id_0::id_2=(99);
break;
case 4:;
 ::id_0::id_2=(44);
case 0:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=((false) ? (88) : (2));
break;
}
switch( 2 )
{
case 1:;
 ::id_0::id_2=(99);
break;
case 2:;
if( ((0)== ::id_0::id_2)||((2)== ::id_0::id_2) )
 ::id_0::id_2=((false) ? (88) : ({  ::id_0::id_3();  ::id_0::id_4; }));
break;
}
cease(  ::id_0::id_2 );
return ;
}

void (id_0::id_3)()
{
{
 ::id_0::id_4=(3);
return ;
}
}
