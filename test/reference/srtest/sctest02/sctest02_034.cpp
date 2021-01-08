#include "isystemc.h"

class id_0;
class id_17;
class id_36;
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
class id_17 : public sc_module
{
public:
SC_CTOR( id_17 )
{
SC_THREAD(id_18);
}
void id_18();
bool id_11;
bool id_19;
};
class id_36 : public sc_module
{
public:
SC_CTOR( id_36 ) :
id_25("id_25"),
id_10("id_10")
{
SC_THREAD(id_37);
}
void id_37();
 ::id_0 id_25;
 ::id_17 id_10;
};
id_36 id_9("id_9");
int id_8;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
goto *((!(! ::id_0::id_2)) ? (&&id_3) : (&&id_4));
id_4:;
id_5:;
wait(SC_ZERO_TIME);
goto *((! ::id_0::id_2) ? (&&id_5) : (&&id_6));
id_6:;
goto id_7;
id_3:;
id_7:;
 ::id_0::id_2=(false);
 ::id_8+=(2);
(( ::id_9. ::id_36::id_10). ::id_17::id_11)=(true);
goto *((!(! ::id_0::id_2)) ? (&&id_12) : (&&id_13));
id_13:;
id_14:;
wait(SC_ZERO_TIME);
goto *((! ::id_0::id_2) ? (&&id_14) : (&&id_15));
id_15:;
goto id_16;
id_12:;
id_16:;
 ::id_0::id_2=(false);
 ::id_8+=(3);
(( ::id_9. ::id_36::id_10). ::id_17::id_11)=(true);
return ;
}

void id_17::id_18()
{
goto *((!(! ::id_17::id_19)) ? (&&id_20) : (&&id_21));
id_21:;
id_22:;
wait(SC_ZERO_TIME);
goto *((! ::id_17::id_19) ? (&&id_22) : (&&id_23));
id_23:;
goto id_24;
id_20:;
id_24:;
 ::id_17::id_19=(false);
 ::id_8*=(5);
(( ::id_9. ::id_36::id_25). ::id_0::id_2)=(true);
goto *((!(! ::id_17::id_11)) ? (&&id_26) : (&&id_27));
id_27:;
id_28:;
wait(SC_ZERO_TIME);
goto *((! ::id_17::id_11) ? (&&id_28) : (&&id_29));
id_29:;
goto id_30;
id_26:;
id_30:;
 ::id_17::id_11=(false);
 ::id_8*=(5);
(( ::id_9. ::id_36::id_25). ::id_0::id_2)=(true);
goto *((!(! ::id_17::id_11)) ? (&&id_31) : (&&id_32));
id_32:;
id_33:;
wait(SC_ZERO_TIME);
goto *((! ::id_17::id_11) ? (&&id_33) : (&&id_34));
id_34:;
goto id_35;
id_31:;
id_35:;
 ::id_17::id_11=(false);
cease(  ::id_8 );
return ;
}

void id_36::id_37()
{
 ::id_8=(1);
( ::id_36::id_10. ::id_17::id_19)=(true);
return ;
}
