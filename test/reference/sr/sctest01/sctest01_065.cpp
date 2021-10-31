#include "isystemc.h"

class id_0;
class id_9;
class id_16;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_2);
}
sc_event id_1;
void id_2();
};
class id_9 : public sc_module
{
public:
SC_CTOR( id_9 )
{
SC_THREAD(id_11);
}
sc_event id_10;
sc_event id_7;
void id_11();
};
class id_16 : public sc_module
{
public:
SC_CTOR( id_16 ) :
id_13("id_13"),
id_6("id_6")
{
SC_THREAD(id_17);
}
void id_17();
 ::id_0 id_13;
 ::id_9 id_6;
};
id_16 id_5("id_5");
int id_4;

void id_0::id_2()
{
wait(  ::id_0::id_1 );
goto id_3;
id_3:;
 ::id_4+=(2);
(( ::id_5. ::id_16::id_6). ::id_9::id_7).notify(SC_ZERO_TIME);
wait(  ::id_0::id_1 );
goto id_8;
id_8:;
 ::id_4+=(3);
(( ::id_5. ::id_16::id_6). ::id_9::id_7).notify(SC_ZERO_TIME);
return ;
}

void id_9::id_11()
{
wait(  ::id_9::id_10 );
goto id_12;
id_12:;
 ::id_4*=(5);
(( ::id_5. ::id_16::id_13). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_9::id_7 );
goto id_14;
id_14:;
 ::id_4*=(5);
(( ::id_5. ::id_16::id_13). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_9::id_7 );
goto id_15;
id_15:;
cease(  ::id_4 );
return ;
}

void id_16::id_17()
{
 ::id_4=(1);
( ::id_16::id_6. ::id_9::id_10).notify(SC_ZERO_TIME);
return ;
}
