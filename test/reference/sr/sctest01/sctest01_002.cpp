#include "isystemc.h"

class id_0;
class id_16;
class id_24;
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
class id_16 : public sc_module
{
public:
SC_CTOR( id_16 )
{
}
sc_event id_14;
sc_event id_17;
id_16(auto char (*id_18));
void (id_20)();
};
class id_24 : public sc_module
{
public:
SC_CTOR( id_24 ) :
id_21("id_21"),
id_13("id_13")
{
}
id_24(auto char (*id_25));
 ::id_0 id_21;
 ::id_16 id_13;
void (id_27)();
};
id_24 id_12("id_12");
void id_11;
int id_9;
void (id_29)(void id_28);
void (id_23)(void id_22);
void (id_8)(void id_7);
void (id_31)(void id_30);
void (id_6)(void id_4);
void (id_33)(void id_32);
void (id_36)(void id_34, void id_35);

id_0::id_0(char (*id_2))
{
 ::id_6( ::id_0::id_5);
}

void (id_0::id_5)()
{
 ::id_8( ::id_0::id_1);
 ::id_9+=(2);
"ERROR: cannot analyse call";
 ::id_8( ::id_0::id_1);
 ::id_9+=(3);
"ERROR: cannot analyse call";
}

id_16::id_16(char (*id_18))
{
 ::id_6( ::id_16::id_20);
}

void (id_16::id_20)()
{
 ::id_8( ::id_16::id_17);
 ::id_9*=(5);
"ERROR: cannot analyse call";
 ::id_8( ::id_16::id_14);
 ::id_9*=(5);
"ERROR: cannot analyse call";
 ::id_8( ::id_16::id_14);
 ::id_23( ::id_9);
}

id_24::id_24(char (*id_25)) : id_21("add_inst"), id_13("mul_inst")
{
 ::id_6( ::id_24::id_27);
}

void (id_24::id_27)()
{
 ::id_9=(1);
"ERROR: cannot analyse call";
}

void (id_29)(void id_28);

void (id_23)(void id_22);

void (id_8)(void id_7);

void (id_31)(void id_30);

void (id_6)(void id_4);

void (id_33)(void id_32);

void (id_36)(void id_34, void id_35);
