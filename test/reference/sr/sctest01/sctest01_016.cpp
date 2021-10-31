#include "isystemc.h"

class id_0;
class id_7;
class id_11;
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
class id_7 : public sc_module
{
public:
SC_CTOR( id_7 )
{
SC_THREAD(id_9);
}
sc_event id_6;
sc_event id_8;
void id_9();
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
void id_13;
int id_3;
void (id_15)(void id_14);
void (id_17)(void id_16);
void (id_19)(void id_18);
void (id_21)(void id_20);

void id_0::id_2()
{
wait(  ::id_0::id_1 );
 ::id_3+=(2);
(( ::id_4. ::id_11::id_5). ::id_7::id_6).notify(SC_ZERO_TIME);
wait(  ::id_0::id_1 );
 ::id_3+=(3);
(( ::id_4. ::id_11::id_5). ::id_7::id_6).notify(SC_ZERO_TIME);
}

void id_7::id_9()
{
wait(  ::id_7::id_8 );
 ::id_3*=(5);
(( ::id_4. ::id_11::id_10). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_7::id_6 );
 ::id_3*=(5);
(( ::id_4. ::id_11::id_10). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_7::id_6 );
cease(  ::id_3 );
}

void id_11::id_12()
{
 ::id_3=(1);
( ::id_11::id_5. ::id_7::id_8).notify(SC_ZERO_TIME);
}

void (id_15)(void id_14);

void (id_17)(void id_16);

void (id_19)(void id_18);

void (id_21)(void id_20);
