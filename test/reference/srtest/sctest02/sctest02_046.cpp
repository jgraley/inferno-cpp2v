#include "isystemc.h"

class id_0;
class id_11;
class id_21;
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
class id_11 : public sc_module
{
public:
SC_CTOR( id_11 )
{
SC_THREAD(id_12);
}
void id_12();
bool id_13;
bool id_8;
};
class id_21 : public sc_module
{
public:
SC_CTOR( id_21 ) :
id_16("id_16"),
id_7("id_7")
{
SC_THREAD(id_22);
}
void id_22();
 ::id_0 id_16;
 ::id_11 id_7;
};
id_21 id_6("id_6");
int id_5;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
goto *((!(! ::id_0::id_2)) ? (&&id_3) : (&&id_4));
id_4:;
wait(SC_ZERO_TIME);
goto *((! ::id_0::id_2) ? (&&id_4) : (&&id_3));
id_3:;
 ::id_0::id_2=(false);
 ::id_5+=(2);
(( ::id_6. ::id_21::id_7). ::id_11::id_8)=(true);
goto *((!(! ::id_0::id_2)) ? (&&id_9) : (&&id_10));
id_10:;
wait(SC_ZERO_TIME);
goto *((! ::id_0::id_2) ? (&&id_10) : (&&id_9));
id_9:;
 ::id_0::id_2=(false);
 ::id_5+=(3);
(( ::id_6. ::id_21::id_7). ::id_11::id_8)=(true);
return ;
}

void id_11::id_12()
{
goto *((!(! ::id_11::id_13)) ? (&&id_14) : (&&id_15));
id_15:;
wait(SC_ZERO_TIME);
goto *((! ::id_11::id_13) ? (&&id_15) : (&&id_14));
id_14:;
 ::id_11::id_13=(false);
 ::id_5*=(5);
(( ::id_6. ::id_21::id_16). ::id_0::id_2)=(true);
goto *((!(! ::id_11::id_8)) ? (&&id_17) : (&&id_18));
id_18:;
wait(SC_ZERO_TIME);
goto *((! ::id_11::id_8) ? (&&id_18) : (&&id_17));
id_17:;
 ::id_11::id_8=(false);
 ::id_5*=(5);
(( ::id_6. ::id_21::id_16). ::id_0::id_2)=(true);
goto *((!(! ::id_11::id_8)) ? (&&id_19) : (&&id_20));
id_20:;
wait(SC_ZERO_TIME);
goto *((! ::id_11::id_8) ? (&&id_20) : (&&id_19));
id_19:;
 ::id_11::id_8=(false);
cease(  ::id_5 );
return ;
}

void id_21::id_22()
{
 ::id_5=(1);
( ::id_21::id_7. ::id_11::id_13)=(true);
return ;
}
