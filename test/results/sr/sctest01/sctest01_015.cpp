#include "isystemc.h"

class id_0;
class id_12;
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
id_0(auto char (*id_10));
};
class id_12 : public sc_module
{
public:
SC_CTOR( id_12 )
{
SC_THREAD(id_14);
}
sc_event id_13;
sc_event id_8;
void id_14();
id_12(auto char (*id_16));
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
id_18(auto char (*id_20));
 ::id_0 id_15;
 ::id_12 id_7;
};
id_18 id_6("id_6");
void id_5;
int id_3;
void (id_23)(void id_22);
void (id_25)(void id_24);
void (id_27)(void id_26);
void (id_29)(void id_28);

void id_0::id_2()
{
wait(  ::id_0::id_1 );
 ::id_3+=(2);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
wait(  ::id_0::id_1 );
 ::id_3+=(3);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
}

id_0::id_0(char (*id_10))
{
}

void id_12::id_14()
{
wait(  ::id_12::id_13 );
 ::id_3*=(5);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
wait(  ::id_12::id_8 );
 ::id_3*=(5);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
wait(  ::id_12::id_8 );
cease(  ::id_3 );
}

id_12::id_12(char (*id_16))
{
}

void id_18::id_19()
{
 ::id_3=(1);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
}

id_18::id_18(char (*id_20)) : id_15("add_inst"), id_7("mul_inst")
{
}

void (id_23)(void id_22);

void (id_25)(void id_24);

void (id_27)(void id_26);

void (id_29)(void id_28);
