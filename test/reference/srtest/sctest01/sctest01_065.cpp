#include "isystemc.h"

class id_0;
class id_13;
class id_24;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_5);
}
enum id_1
{
id_2 = 0U,
id_3 = 1U,
};
sc_event id_4;
void id_5();
};
class id_13 : public sc_module
{
public:
SC_CTOR( id_13 )
{
SC_THREAD(id_19);
}
enum id_14
{
id_15 = 0U,
id_16 = 1U,
id_17 = 2U,
};
sc_event id_12;
sc_event id_18;
void id_19();
};
class id_24 : public sc_module
{
public:
SC_CTOR( id_24 ) :
id_23("id_23"),
id_11("id_11")
{
SC_THREAD(id_26);
}
enum id_25
{
};
void id_26();
 ::id_0 id_23;
 ::id_13 id_11;
};
id_24 id_10("id_10");
int id_9;

void id_0::id_5()
{
static const unsigned int (id_6[]) = { &&id_7, &&id_7 };
auto unsigned int id_8;
wait(  ::id_0::id_4 );
id_8= ::id_0::id_2;
id_7:;
if(  ::id_0::id_2==id_8 )
{
 ::id_9+=(2);
(( ::id_10. ::id_24::id_11). ::id_13::id_12).notify(SC_ZERO_TIME);
wait(  ::id_0::id_4 );
id_8= ::id_0::id_3;
goto *(id_6[id_8]);
}
if(  ::id_0::id_3==id_8 )
{
 ::id_9+=(3);
(( ::id_10. ::id_24::id_11). ::id_13::id_12).notify(SC_ZERO_TIME);
return ;
}
goto *(id_6[id_8]);
}

void id_13::id_19()
{
static const unsigned int (id_20[]) = { &&id_21, &&id_21, &&id_21 };
auto unsigned int id_22;
wait(  ::id_13::id_18 );
id_22= ::id_13::id_15;
id_21:;
if(  ::id_13::id_15==id_22 )
{
 ::id_9*=(5);
(( ::id_10. ::id_24::id_23). ::id_0::id_4).notify(SC_ZERO_TIME);
wait(  ::id_13::id_12 );
id_22= ::id_13::id_16;
goto *(id_20[id_22]);
}
if(  ::id_13::id_16==id_22 )
{
 ::id_9*=(5);
(( ::id_10. ::id_24::id_23). ::id_0::id_4).notify(SC_ZERO_TIME);
wait(  ::id_13::id_12 );
id_22= ::id_13::id_17;
goto *(id_20[id_22]);
}
if(  ::id_13::id_17==id_22 )
{
cease(  ::id_9 );
return ;
}
goto *(id_20[id_22]);
}

void id_24::id_26()
{
static const unsigned int (id_27[]) = {  };
 ::id_9=(1);
( ::id_24::id_11. ::id_13::id_18).notify(SC_ZERO_TIME);
return ;
}
