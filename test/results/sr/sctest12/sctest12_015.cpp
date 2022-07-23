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
id_0(auto char (*id_4));
int id_2;
int (id_3)();
};
id_0 id_6("id_6");
void id_7;
void (id_11)(void id_10);
void (id_13)(void id_12);
void (id_15)(void id_14);
void (id_9)(void id_8);

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
 ::id_0::id_2=((false) ? (88) : ( ::id_0::id_3()));
break;
}
cease(  ::id_0::id_2 );
}

id_0::id_0(char (*id_4))
{
}

int (id_0::id_3)()
{
return 3;
}

void (id_11)(void id_10);

void (id_13)(void id_12);

void (id_15)(void id_14);

void (id_9)(void id_8);
