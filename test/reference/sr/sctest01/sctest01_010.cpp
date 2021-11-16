#include "isystemc.h"

class id_0;
class id_14;
class id_20;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
}
sc_event id_1;
id_0(auto char (*id_2));
void (id_5)();
};
class id_14 : public sc_module
{
public:
SC_CTOR( id_14 )
{
}
sc_event id_12;
sc_event id_15;
id_14(auto char (*id_16));
void (id_18)();
};
class id_20 : public sc_module
{
public:
SC_CTOR( id_20 ) :
id_19("id_19"),
id_11("id_11")
{
}
id_20(auto char (*id_21));
 ::id_0 id_19;
 ::id_14 id_11;
void (id_23)();
};
id_20 id_10("id_10");
void id_9;
int id_7;
void (id_6)(void id_4);
void (id_25)(void id_24);
void (id_27)(void id_26);
void (id_29)(void id_28);
void (id_31)(void id_30);
void (id_33)(void id_32);
void (id_36)(void id_34, void id_35);

id_0::id_0(char (*id_2))
{
 ::id_6( ::id_0::id_5);
}

void (id_0::id_5)()
{
wait(  ::id_0::id_1 );
 ::id_7+=(2);
"ERROR: cannot analyse call";
wait(  ::id_0::id_1 );
 ::id_7+=(3);
"ERROR: cannot analyse call";
}

id_14::id_14(char (*id_16))
{
 ::id_6( ::id_14::id_18);
}

void (id_14::id_18)()
{
wait(  ::id_14::id_15 );
 ::id_7*=(5);
"ERROR: cannot analyse call";
wait(  ::id_14::id_12 );
 ::id_7*=(5);
"ERROR: cannot analyse call";
wait(  ::id_14::id_12 );
cease(  ::id_7 );
}

id_20::id_20(char (*id_21)) : id_19("add_inst"), id_11("mul_inst")
{
 ::id_6( ::id_20::id_23);
}

void (id_20::id_23)()
{
 ::id_7=(1);
"ERROR: cannot analyse call";
}

void (id_6)(void id_4);

void (id_25)(void id_24);

void (id_27)(void id_26);

void (id_29)(void id_28);

void (id_31)(void id_30);

void (id_33)(void id_32);

void (id_36)(void id_34, void id_35);
