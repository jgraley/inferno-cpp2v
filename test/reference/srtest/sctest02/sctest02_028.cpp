#include "isystemc.h"

class id_0;
class id_19;
class id_41;
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
class id_19 : public sc_module
{
public:
SC_CTOR( id_19 )
{
SC_THREAD(id_20);
}
void id_20();
bool id_12;
bool id_21;
};
class id_41 : public sc_module
{
public:
SC_CTOR( id_41 ) :
id_28("id_28"),
id_11("id_11")
{
SC_THREAD(id_42);
}
void id_42();
 ::id_0 id_28;
 ::id_19 id_11;
};
id_41 id_10("id_10");
int id_9;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
{
goto *((!(! ::id_0::id_2)) ? (&&id_3) : (&&id_4));
id_4:;
{
id_5:;
wait(SC_ZERO_TIME);
id_6:;
goto *((! ::id_0::id_2) ? (&&id_5) : (&&id_7));
id_7:;
}
goto id_8;
id_3:;
;
id_8:;
}
 ::id_0::id_2=(false);
 ::id_9+=(2);
(( ::id_10. ::id_41::id_11). ::id_19::id_12)=(true);
{
goto *((!(! ::id_0::id_2)) ? (&&id_13) : (&&id_14));
id_14:;
{
id_15:;
wait(SC_ZERO_TIME);
id_16:;
goto *((! ::id_0::id_2) ? (&&id_15) : (&&id_17));
id_17:;
}
goto id_18;
id_13:;
;
id_18:;
}
 ::id_0::id_2=(false);
 ::id_9+=(3);
(( ::id_10. ::id_41::id_11). ::id_19::id_12)=(true);
return ;
}

void id_19::id_20()
{
{
goto *((!(! ::id_19::id_21)) ? (&&id_22) : (&&id_23));
id_23:;
{
id_24:;
wait(SC_ZERO_TIME);
id_25:;
goto *((! ::id_19::id_21) ? (&&id_24) : (&&id_26));
id_26:;
}
goto id_27;
id_22:;
;
id_27:;
}
 ::id_19::id_21=(false);
 ::id_9*=(5);
(( ::id_10. ::id_41::id_28). ::id_0::id_2)=(true);
{
goto *((!(! ::id_19::id_12)) ? (&&id_29) : (&&id_30));
id_30:;
{
id_31:;
wait(SC_ZERO_TIME);
id_32:;
goto *((! ::id_19::id_12) ? (&&id_31) : (&&id_33));
id_33:;
}
goto id_34;
id_29:;
;
id_34:;
}
 ::id_19::id_12=(false);
 ::id_9*=(5);
(( ::id_10. ::id_41::id_28). ::id_0::id_2)=(true);
{
goto *((!(! ::id_19::id_12)) ? (&&id_35) : (&&id_36));
id_36:;
{
id_37:;
wait(SC_ZERO_TIME);
id_38:;
goto *((! ::id_19::id_12) ? (&&id_37) : (&&id_39));
id_39:;
}
goto id_40;
id_35:;
;
id_40:;
}
 ::id_19::id_12=(false);
cease(  ::id_9 );
return ;
}

void id_41::id_42()
{
 ::id_9=(1);
( ::id_41::id_11. ::id_19::id_21)=(true);
return ;
}
