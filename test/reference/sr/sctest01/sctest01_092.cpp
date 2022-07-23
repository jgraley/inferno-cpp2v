#include "isystemc.h"

class id_0;
class id_11;
class id_20;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_5);
}
enum id_1
{
id_2 = 0,
id_3 = 1,
};
sc_event id_4;
void id_5();
private:
unsigned int id_6;
};
class id_11 : public sc_module
{
public:
SC_CTOR( id_11 )
{
SC_THREAD(id_17);
}
enum id_12
{
id_13 = 0,
id_14 = 1,
id_15 = 2,
};
sc_event id_10;
sc_event id_16;
void id_17();
private:
unsigned int id_18;
};
class id_20 : public sc_module
{
public:
SC_CTOR( id_20 ) :
id_19("id_19"),
id_9("id_9")
{
SC_THREAD(id_22);
}
enum id_21
{
};
void id_22();
 ::id_0 id_19;
 ::id_11 id_9;
};
id_20 id_8("id_8");
int id_7;

void id_0::id_5()
{
do
{
if( (sc_delta_count())==(0) )
{
wait(  ::id_0::id_4 );
 ::id_0::id_6= ::id_0::id_2;
continue;
}
if(  ::id_0::id_2== ::id_0::id_6 )
{
 ::id_7+=(2);
(( ::id_8. ::id_20::id_9). ::id_11::id_10).notify(SC_ZERO_TIME);
wait(  ::id_0::id_4 );
 ::id_0::id_6= ::id_0::id_3;
continue;
}
if(  ::id_0::id_3== ::id_0::id_6 )
{
 ::id_7+=(3);
(( ::id_8. ::id_20::id_9). ::id_11::id_10).notify(SC_ZERO_TIME);
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void id_11::id_17()
{
do
{
if( (sc_delta_count())==(0) )
{
wait(  ::id_11::id_16 );
 ::id_11::id_18= ::id_11::id_13;
continue;
}
if(  ::id_11::id_13== ::id_11::id_18 )
{
 ::id_7*=(5);
(( ::id_8. ::id_20::id_19). ::id_0::id_4).notify(SC_ZERO_TIME);
wait(  ::id_11::id_10 );
 ::id_11::id_18= ::id_11::id_14;
continue;
}
if(  ::id_11::id_14== ::id_11::id_18 )
{
 ::id_7*=(5);
(( ::id_8. ::id_20::id_19). ::id_0::id_4).notify(SC_ZERO_TIME);
wait(  ::id_11::id_10 );
 ::id_11::id_18= ::id_11::id_15;
continue;
}
if(  ::id_11::id_15== ::id_11::id_18 )
{
cease(  ::id_7 );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void id_20::id_22()
{
 ::id_7=(1);
( ::id_20::id_9. ::id_11::id_16).notify(SC_ZERO_TIME);
return ;
}
