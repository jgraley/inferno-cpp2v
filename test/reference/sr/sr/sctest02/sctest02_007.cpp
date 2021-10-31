#include "isystemc.h"

class id_0;
class id_11;
class id_19;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
}
id_0(auto char (*id_1));
bool id_6;
void (id_4)();
};
class id_11 : public sc_module
{
public:
SC_CTOR( id_11 )
{
}
id_11(auto char (*id_12));
bool id_10;
bool id_15;
void (id_14)();
};
class id_19 : public sc_module
{
public:
SC_CTOR( id_19 ) :
id_16("id_16"),
id_9("id_9")
{
}
id_19(auto char (*id_20));
 ::id_0 id_16;
 ::id_11 id_9;
void (id_22)();
};
id_19 id_8("id_8");
void id_23;
int id_7;
void (id_18)(void id_17);
void (id_25)(void id_24);
void (id_27)(void id_26);
void (id_29)(void id_28);
void (id_5)(void id_3);
void (id_31)(void id_30);
void (id_34)(void id_32, void id_33);

id_0::id_0(char (*id_1))
{
 ::id_5( ::id_0::id_4);
}

void (id_0::id_4)()
{
wait(SC_ZERO_TIME);
while( ! ::id_0::id_6 )
wait(SC_ZERO_TIME);
 ::id_0::id_6=(false);
 ::id_7+=(2);
(( ::id_8. ::id_19::id_9). ::id_11::id_10)=(true);
while( ! ::id_0::id_6 )
wait(SC_ZERO_TIME);
 ::id_0::id_6=(false);
 ::id_7+=(3);
(( ::id_8. ::id_19::id_9). ::id_11::id_10)=(true);
}

id_11::id_11(char (*id_12))
{
 ::id_5( ::id_11::id_14);
}

void (id_11::id_14)()
{
while( ! ::id_11::id_15 )
wait(SC_ZERO_TIME);
 ::id_11::id_15=(false);
 ::id_7*=(5);
(( ::id_8. ::id_19::id_16). ::id_0::id_6)=(true);
while( ! ::id_11::id_10 )
wait(SC_ZERO_TIME);
 ::id_11::id_10=(false);
 ::id_7*=(5);
(( ::id_8. ::id_19::id_16). ::id_0::id_6)=(true);
while( ! ::id_11::id_10 )
wait(SC_ZERO_TIME);
 ::id_11::id_10=(false);
 ::id_18( ::id_7);
}

id_19::id_19(char (*id_20)) : id_16("add_inst"), id_9("mul_inst")
{
 ::id_5( ::id_19::id_22);
}

void (id_19::id_22)()
{
 ::id_7=(1);
( ::id_19::id_9. ::id_11::id_15)=(true);
}

void (id_18)(void id_17);

void (id_25)(void id_24);

void (id_27)(void id_26);

void (id_29)(void id_28);

void (id_5)(void id_3);

void (id_31)(void id_30);

void (id_34)(void id_32, void id_33);
