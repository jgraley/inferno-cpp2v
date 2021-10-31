#include "isystemc.h"

class id_16;
class id_0;
class id_17;
class id_25;
class id_29;
class id_16
{
};
class id_0 : public id_16
{
public:
sc_event id_1;
id_0(auto char (*id_2));
void (id_5)();
};
class id_17 : public id_16
{
public:
sc_event id_14;
sc_event id_18;
id_17(auto char (*id_19));
void (id_21)();
};
class id_25 : public id_16
{
public:
id_25(auto char (*id_26));
 ::id_0 id_22;
 ::id_17 id_13;
void (id_28)();
};
class id_29
{
};
id_25 id_12;
void id_11;
int id_9;
void (id_8)(void id_7);
void (id_31)(void id_30);
void (id_6)(void id_4);
void (id_33)(void id_32);
void (id_35)(void id_34);
void (id_24)(void id_23);
void (id_38)(void id_36, void id_37);

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

id_17::id_17(char (*id_19))
{
 ::id_6( ::id_17::id_21);
}

void (id_17::id_21)()
{
 ::id_8( ::id_17::id_18);
 ::id_9*=(5);
"ERROR: cannot analyse call";
 ::id_8( ::id_17::id_14);
 ::id_9*=(5);
"ERROR: cannot analyse call";
 ::id_8( ::id_17::id_14);
 ::id_24( ::id_9);
}

id_25::id_25(char (*id_26)) : id_22("add_inst"), id_13("mul_inst")
{
 ::id_6( ::id_25::id_28);
}

void (id_25::id_28)()
{
 ::id_9=(1);
"ERROR: cannot analyse call";
}

void (id_8)(void id_7);

void (id_31)(void id_30);

void (id_6)(void id_4);

void (id_33)(void id_32);

void (id_35)(void id_34);

void (id_24)(void id_23);

void (id_38)(void id_36, void id_37);
