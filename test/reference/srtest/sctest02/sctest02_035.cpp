#include "isystemc.h"

class id_0;
class id_13;
class id_26;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_1);
}
void id_1();
bool id_2;
};
class id_13 : public sc_module
{
public:
SC_CTOR( id_13 )
{
SC_THREAD(id_14);
}
void id_14();
bool id_15;
bool id_9;
};
class id_26 : public sc_module
{
public:
SC_CTOR( id_26 ) :
id_19("id_19"),
id_8("id_8")
{
SC_THREAD(id_27);
}
void id_27();
 ::id_0 id_19;
 ::id_13 id_8;
};
id_26 id_7("id_7");
int id_6;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
goto *((!(! ::id_0::id_2)) ? (&&id_3) : (&&id_4));
id_4:;
wait(SC_ZERO_TIME);
goto *((! ::id_0::id_2) ? (&&id_4) : (&&id_5));
id_5:;
goto id_3;
id_3:;
 ::id_0::id_2=(false);
 ::id_6+=(2);
(( ::id_7. ::id_26::id_8). ::id_13::id_9)=(true);
goto *((!(! ::id_0::id_2)) ? (&&id_10) : (&&id_11));
id_11:;
wait(SC_ZERO_TIME);
goto *((! ::id_0::id_2) ? (&&id_11) : (&&id_12));
id_12:;
goto id_10;
id_10:;
 ::id_0::id_2=(false);
 ::id_6+=(3);
(( ::id_7. ::id_26::id_8). ::id_13::id_9)=(true);
return ;
}

void id_13::id_14()
{
goto *((!(! ::id_13::id_15)) ? (&&id_16) : (&&id_17));
id_17:;
wait(SC_ZERO_TIME);
goto *((! ::id_13::id_15) ? (&&id_17) : (&&id_18));
id_18:;
goto id_16;
id_16:;
 ::id_13::id_15=(false);
 ::id_6*=(5);
(( ::id_7. ::id_26::id_19). ::id_0::id_2)=(true);
goto *((!(! ::id_13::id_9)) ? (&&id_20) : (&&id_21));
id_21:;
wait(SC_ZERO_TIME);
goto *((! ::id_13::id_9) ? (&&id_21) : (&&id_22));
id_22:;
goto id_20;
id_20:;
 ::id_13::id_9=(false);
 ::id_6*=(5);
(( ::id_7. ::id_26::id_19). ::id_0::id_2)=(true);
goto *((!(! ::id_13::id_9)) ? (&&id_23) : (&&id_24));
id_24:;
wait(SC_ZERO_TIME);
goto *((! ::id_13::id_9) ? (&&id_24) : (&&id_25));
id_25:;
goto id_23;
id_23:;
 ::id_13::id_9=(false);
cease(  ::id_6 );
return ;
}

void id_26::id_27()
{
 ::id_6=(1);
( ::id_26::id_8. ::id_13::id_15)=(true);
return ;
}
