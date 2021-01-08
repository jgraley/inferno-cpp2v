#include "isystemc.h"

class id_0;
class id_18;
class id_37;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_6);
}
enum id_1
{
id_2 = 0U,
id_3 = 2U,
id_4 = 1U,
id_5 = 3U,
};
void id_6();
bool id_13;
};
class id_18 : public sc_module
{
public:
SC_CTOR( id_18 )
{
SC_THREAD(id_26);
}
enum id_19
{
id_20 = 0U,
id_21 = 2U,
id_22 = 4U,
id_23 = 1U,
id_24 = 3U,
id_25 = 5U,
};
void id_26();
bool id_17;
bool id_35;
};
class id_37 : public sc_module
{
public:
SC_CTOR( id_37 ) :
id_36("id_36"),
id_16("id_16")
{
SC_THREAD(id_39);
}
enum id_38
{
};
void id_39();
 ::id_0 id_36;
 ::id_18 id_16;
};
id_37 id_15("id_15");
int id_14;

void id_0::id_6()
{
static const unsigned int (id_7[]) = { &&id_8, &&id_9, &&id_10, &&id_11 };
auto unsigned int id_12;
wait(SC_ZERO_TIME);
id_12=((!(! ::id_0::id_13)) ?  ::id_0::id_4 :  ::id_0::id_2);
id_8:;
if(  ::id_0::id_2==id_12 )
{
wait(SC_ZERO_TIME);
id_12=((! ::id_0::id_13) ?  ::id_0::id_2 :  ::id_0::id_4);
goto *(id_7[id_12]);
}
id_9:;
if(  ::id_0::id_4==id_12 )
{
 ::id_0::id_13=(false);
 ::id_14+=(2);
(( ::id_15. ::id_37::id_16). ::id_18::id_17)=(true);
id_12=((!(! ::id_0::id_13)) ?  ::id_0::id_5 :  ::id_0::id_3);
}
id_10:;
if(  ::id_0::id_3==id_12 )
{
wait(SC_ZERO_TIME);
id_12=((! ::id_0::id_13) ?  ::id_0::id_3 :  ::id_0::id_5);
goto *(id_7[id_12]);
}
goto *(id_7[id_12]);
id_11:;
 ::id_0::id_13=(false);
 ::id_14+=(3);
(( ::id_15. ::id_37::id_16). ::id_18::id_17)=(true);
return ;
}

void id_18::id_26()
{
static const unsigned int (id_27[]) = { &&id_28, &&id_29, &&id_30, &&id_31, &&id_32, &&id_33 };
auto unsigned int id_34;
wait(SC_ZERO_TIME);
id_34=((!(! ::id_18::id_35)) ?  ::id_18::id_23 :  ::id_18::id_20);
id_28:;
if(  ::id_18::id_20==id_34 )
{
wait(SC_ZERO_TIME);
id_34=((! ::id_18::id_35) ?  ::id_18::id_20 :  ::id_18::id_23);
goto *(id_27[id_34]);
}
id_29:;
if(  ::id_18::id_23==id_34 )
{
 ::id_18::id_35=(false);
 ::id_14*=(5);
(( ::id_15. ::id_37::id_36). ::id_0::id_13)=(true);
id_34=((!(! ::id_18::id_17)) ?  ::id_18::id_24 :  ::id_18::id_21);
}
id_30:;
if(  ::id_18::id_21==id_34 )
{
wait(SC_ZERO_TIME);
id_34=((! ::id_18::id_17) ?  ::id_18::id_21 :  ::id_18::id_24);
goto *(id_27[id_34]);
}
goto *(id_27[id_34]);
id_31:;
 ::id_18::id_17=(false);
 ::id_14*=(5);
(( ::id_15. ::id_37::id_36). ::id_0::id_13)=(true);
id_34=((!(! ::id_18::id_17)) ?  ::id_18::id_25 :  ::id_18::id_22);
goto *(id_27[id_34]);
id_32:;
wait(SC_ZERO_TIME);
id_34=((! ::id_18::id_17) ?  ::id_18::id_22 :  ::id_18::id_25);
goto *(id_27[id_34]);
id_33:;
 ::id_18::id_17=(false);
cease(  ::id_14 );
return ;
}

void id_37::id_39()
{
static const unsigned int (id_40[]) = {  };
 ::id_14=(1);
( ::id_37::id_16. ::id_18::id_35)=(true);
return ;
}
