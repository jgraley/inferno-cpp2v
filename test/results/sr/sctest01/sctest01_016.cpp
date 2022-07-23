#include "isystemc.h"

class id_0;
class id_9;
class id_15;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_2);
}
sc_event id_1;
void id_2();
id_0(auto char (*id_7));
};
class id_9 : public sc_module
{
public:
SC_CTOR( id_9 )
{
SC_THREAD(id_11);
}
sc_event id_10;
sc_event id_6;
void id_11();
id_9(auto char (*id_13));
};
class id_15 : public sc_module
{
public:
SC_CTOR( id_15 ) :
id_12("id_12"),
id_5("id_5")
{
SC_THREAD(id_16);
}
void id_16();
id_15(auto char (*id_17));
 ::id_0 id_12;
 ::id_9 id_5;
};
id_15 id_4("id_4");
void id_19;
int id_3;
void (id_21)(void id_20);
void (id_23)(void id_22);
void (id_25)(void id_24);
void (id_27)(void id_26);

void id_0::id_2()
{
wait(  ::id_0::id_1 );
 ::id_3+=(2);
(( ::id_4. ::id_15::id_5). ::id_9::id_6).notify(SC_ZERO_TIME);
wait(  ::id_0::id_1 );
 ::id_3+=(3);
(( ::id_4. ::id_15::id_5). ::id_9::id_6).notify(SC_ZERO_TIME);
}

id_0::id_0(char (*id_7))
{
}

void id_9::id_11()
{
wait(  ::id_9::id_10 );
 ::id_3*=(5);
(( ::id_4. ::id_15::id_12). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_9::id_6 );
 ::id_3*=(5);
(( ::id_4. ::id_15::id_12). ::id_0::id_1).notify(SC_ZERO_TIME);
wait(  ::id_9::id_6 );
cease(  ::id_3 );
}

id_9::id_9(char (*id_13))
{
}

void id_15::id_16()
{
 ::id_3=(1);
( ::id_15::id_5. ::id_9::id_10).notify(SC_ZERO_TIME);
}

id_15::id_15(char (*id_17)) : id_12("add_inst"), id_5("mul_inst")
{
}

void (id_21)(void id_20);

void (id_23)(void id_22);

void (id_25)(void id_24);

void (id_27)(void id_26);
