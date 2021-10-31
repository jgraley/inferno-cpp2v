#include "isystemc.h"

class id_0;
class id_14;
class id_22;
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
class id_14 : public sc_module
{
public:
SC_CTOR( id_14 )
{
}
id_14(auto char (*id_15));
bool id_13;
bool id_18;
void (id_17)();
};
class id_22 : public sc_module
{
public:
SC_CTOR( id_22 ) :
id_19("id_19"),
id_12("id_12")
{
}
id_22(auto char (*id_23));
 ::id_0 id_19;
 ::id_14 id_12;
void (id_25)();
};
id_22 id_11("id_11");
void id_8;
int id_10;
void (id_27)(void id_26);
void (id_21)(void id_20);
void (id_29)(void id_28);
void (id_9)(void id_7);
void (id_31)(void id_30);
void (id_5)(void id_3);
void (id_34)(void id_32, void id_33);

id_0::id_0(char (*id_1))
{
 ::id_5( ::id_0::id_4);
}

void (id_0::id_4)()
{
 ::id_9( ::id_8);
while( ! ::id_0::id_6 )
 ::id_9( ::id_8);
 ::id_0::id_6=(false);
 ::id_10+=(2);
(( ::id_11. ::id_22::id_12). ::id_14::id_13)=(true);
while( ! ::id_0::id_6 )
 ::id_9( ::id_8);
 ::id_0::id_6=(false);
 ::id_10+=(3);
(( ::id_11. ::id_22::id_12). ::id_14::id_13)=(true);
}

id_14::id_14(char (*id_15))
{
 ::id_5( ::id_14::id_17);
}

void (id_14::id_17)()
{
while( ! ::id_14::id_18 )
 ::id_9( ::id_8);
 ::id_14::id_18=(false);
 ::id_10*=(5);
(( ::id_11. ::id_22::id_19). ::id_0::id_6)=(true);
while( ! ::id_14::id_13 )
 ::id_9( ::id_8);
 ::id_14::id_13=(false);
 ::id_10*=(5);
(( ::id_11. ::id_22::id_19). ::id_0::id_6)=(true);
while( ! ::id_14::id_13 )
 ::id_9( ::id_8);
 ::id_14::id_13=(false);
 ::id_21( ::id_10);
}

id_22::id_22(char (*id_23)) : id_19("add_inst"), id_12("mul_inst")
{
 ::id_5( ::id_22::id_25);
}

void (id_22::id_25)()
{
 ::id_10=(1);
( ::id_22::id_12. ::id_14::id_18)=(true);
}

void (id_27)(void id_26);

void (id_21)(void id_20);

void (id_29)(void id_28);

void (id_9)(void id_7);

void (id_31)(void id_30);

void (id_5)(void id_3);

void (id_34)(void id_32, void id_33);
