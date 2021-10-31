#include "isystemc.h"

class id_0;
class id_7;
class id_11;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_1);
}
void id_1();
bool id_2;
};
class id_7 : public sc_module
{
public:
SC_CTOR( id_7 )
{
SC_THREAD(id_8);
}
void id_8();
bool id_6;
bool id_9;
};
class id_11 : public sc_module
{
public:
SC_CTOR( id_11 ) :
id_10("id_10"),
id_5("id_5")
{
SC_THREAD(id_12);
}
void id_12();
 ::id_0 id_10;
 ::id_7 id_5;
};
id_11 id_4("id_4");
int id_3;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
while( ! ::id_0::id_2 )
wait(SC_ZERO_TIME);
 ::id_0::id_2=(false);
 ::id_3+=(2);
(( ::id_4. ::id_11::id_5). ::id_7::id_6)=(true);
while( ! ::id_0::id_2 )
wait(SC_ZERO_TIME);
 ::id_0::id_2=(false);
 ::id_3+=(3);
(( ::id_4. ::id_11::id_5). ::id_7::id_6)=(true);
}

void id_7::id_8()
{
while( ! ::id_7::id_9 )
wait(SC_ZERO_TIME);
 ::id_7::id_9=(false);
 ::id_3*=(5);
(( ::id_4. ::id_11::id_10). ::id_0::id_2)=(true);
while( ! ::id_7::id_6 )
wait(SC_ZERO_TIME);
 ::id_7::id_6=(false);
 ::id_3*=(5);
(( ::id_4. ::id_11::id_10). ::id_0::id_2)=(true);
while( ! ::id_7::id_6 )
wait(SC_ZERO_TIME);
 ::id_7::id_6=(false);
cease(  ::id_3 );
}

void id_11::id_12()
{
 ::id_3=(1);
( ::id_11::id_5. ::id_7::id_9)=(true);
}
