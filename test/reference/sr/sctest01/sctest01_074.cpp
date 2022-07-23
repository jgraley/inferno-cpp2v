#include "isystemc.h"

class id_0;
class id_14;
class id_27;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_5);
}
enum id_1
{
id_2 = 0,
id_3 = 1,
};
sc_event id_4;
void id_5();
};
class id_14 : public sc_module
{
public:
SC_CTOR( id_14 )
{
SC_THREAD(id_20);
}
enum id_15
{
id_16 = 0,
id_17 = 1,
id_18 = 2,
};
sc_event id_13;
sc_event id_19;
void id_20();
};
class id_27 : public sc_module
{
public:
SC_CTOR( id_27 ) :
id_26("id_26"),
id_12("id_12")
{
SC_THREAD(id_29);
}
enum id_28
{
};
void id_29();
 ::id_0 id_26;
 ::id_14 id_12;
};
id_27 id_11("id_11");
int id_10;

void id_0::id_5()
{
static const unsigned int (id_6[]) = { &&id_7, &&id_8 };
auto unsigned int id_9;
wait(  ::id_0::id_4 );
id_9= ::id_0::id_2;
id_7:;
if(  ::id_0::id_2==id_9 )
{
 ::id_10+=(2);
(( ::id_11. ::id_27::id_12). ::id_14::id_13).notify(SC_ZERO_TIME);
wait(  ::id_0::id_4 );
id_9= ::id_0::id_3;
goto *(id_6[id_9]);
}
goto *(id_6[id_9]);
id_8:;
 ::id_10+=(3);
(( ::id_11. ::id_27::id_12). ::id_14::id_13).notify(SC_ZERO_TIME);
return ;
}

void id_14::id_20()
{
static const unsigned int (id_21[]) = { &&id_22, &&id_23, &&id_24 };
auto unsigned int id_25;
wait(  ::id_14::id_19 );
id_25= ::id_14::id_16;
id_22:;
if(  ::id_14::id_16==id_25 )
{
 ::id_10*=(5);
(( ::id_11. ::id_27::id_26). ::id_0::id_4).notify(SC_ZERO_TIME);
wait(  ::id_14::id_13 );
id_25= ::id_14::id_17;
goto *(id_21[id_25]);
}
id_23:;
if(  ::id_14::id_17==id_25 )
{
 ::id_10*=(5);
(( ::id_11. ::id_27::id_26). ::id_0::id_4).notify(SC_ZERO_TIME);
wait(  ::id_14::id_13 );
id_25= ::id_14::id_18;
goto *(id_21[id_25]);
}
goto *(id_21[id_25]);
id_24:;
cease(  ::id_10 );
return ;
}

void id_27::id_29()
{
static const unsigned int (id_30[]) = {  };
 ::id_10=(1);
( ::id_27::id_12. ::id_14::id_19).notify(SC_ZERO_TIME);
return ;
}
