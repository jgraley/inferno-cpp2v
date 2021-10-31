#include "isystemc.h"

class id_0;
class id_10;
class id_18;
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
class id_10 : public sc_module
{
public:
SC_CTOR( id_10 )
{
SC_THREAD(id_12);
}
sc_event id_11;
sc_event id_8;
void id_12();
};
class id_18 : public sc_module
{
public:
SC_CTOR( id_18 ) :
id_15("id_15"),
id_7("id_7")
{
SC_THREAD(id_19);
}
void id_19();
 ::id_0 id_15;
 ::id_10 id_7;
};
id_18 id_6("id_6");
int id_5;

void id_0::id_2()
{
auto void *id_3;
wait(  ::id_0::id_1 );
{
id_3=(&&id_4);
goto *(id_3);
}
id_4:;
 ::id_5+=(2);
(( ::id_6. ::id_18::id_7). ::id_10::id_8).notify(SC_ZERO_TIME);
wait(  ::id_0::id_1 );
{
id_3=(&&id_9);
goto *(id_3);
}
id_9:;
 ::id_5+=(3);
(( ::id_6. ::id_18::id_7). ::id_10::id_8).notify(SC_ZERO_TIME);
return ;
}

void id_10::id_12()
{
auto void *id_13;
wait(  ::id_10::id_11 );
{
id_13=(&&id_14);
goto *(id_13);
}
id_14:;
 ::id_5*=(5);
(( ::id_6. ::id_18::id_15). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_10::id_8 );
{
id_13=(&&id_16);
goto *(id_13);
}
id_16:;
 ::id_5*=(5);
(( ::id_6. ::id_18::id_15). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_10::id_8 );
{
id_13=(&&id_17);
goto *(id_13);
}
id_17:;
cease(  ::id_5 );
return ;
}

void id_18::id_19()
{
 ::id_5=(1);
( ::id_18::id_7. ::id_10::id_11).notify(SC_ZERO_TIME);
return ;
}
