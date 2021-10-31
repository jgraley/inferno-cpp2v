#include "isystemc.h"

class id_0;
class id_11;
class id_21;
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
class id_11 : public sc_module
{
public:
SC_CTOR( id_11 )
{
SC_THREAD(id_12);
}
void id_12();
bool id_13;
bool id_8;
};
class id_21 : public sc_module
{
public:
SC_CTOR( id_21 ) :
id_16("id_16"),
id_7("id_7")
{
SC_THREAD(id_22);
}
void id_22();
 ::id_0 id_16;
 ::id_11 id_7;
};
id_21 id_6("id_6");
int id_5;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
if( ! ::id_0::id_2 )
{
id_3:;
wait(SC_ZERO_TIME);
id_4:;
if( ! ::id_0::id_2 )
goto id_3;
}
 ::id_0::id_2=(false);
 ::id_5+=(2);
(( ::id_6. ::id_21::id_7). ::id_11::id_8)=(true);
if( ! ::id_0::id_2 )
{
id_9:;
wait(SC_ZERO_TIME);
id_10:;
if( ! ::id_0::id_2 )
goto id_9;
}
 ::id_0::id_2=(false);
 ::id_5+=(3);
(( ::id_6. ::id_21::id_7). ::id_11::id_8)=(true);
return ;
}

void id_11::id_12()
{
if( ! ::id_11::id_13 )
{
id_14:;
wait(SC_ZERO_TIME);
id_15:;
if( ! ::id_11::id_13 )
goto id_14;
}
 ::id_11::id_13=(false);
 ::id_5*=(5);
(( ::id_6. ::id_21::id_16). ::id_0::id_2)=(true);
if( ! ::id_11::id_8 )
{
id_17:;
wait(SC_ZERO_TIME);
id_18:;
if( ! ::id_11::id_8 )
goto id_17;
}
 ::id_11::id_8=(false);
 ::id_5*=(5);
(( ::id_6. ::id_21::id_16). ::id_0::id_2)=(true);
if( ! ::id_11::id_8 )
{
id_19:;
wait(SC_ZERO_TIME);
id_20:;
if( ! ::id_11::id_8 )
goto id_19;
}
 ::id_11::id_8=(false);
cease(  ::id_5 );
return ;
}

void id_21::id_22()
{
 ::id_5=(1);
( ::id_21::id_7. ::id_11::id_13)=(true);
return ;
}
