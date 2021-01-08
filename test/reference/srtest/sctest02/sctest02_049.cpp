#include "isystemc.h"

class id_0;
class id_12;
class id_23;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_1);
}
void id_1();
bool id_3;
};
class id_12 : public sc_module
{
public:
SC_CTOR( id_12 )
{
SC_THREAD(id_13);
}
void id_13();
bool id_15;
bool id_9;
};
class id_23 : public sc_module
{
public:
SC_CTOR( id_23 ) :
id_18("id_18"),
id_8("id_8")
{
SC_THREAD(id_24);
}
void id_24();
 ::id_0 id_18;
 ::id_12 id_8;
};
id_23 id_7("id_7");
int id_6;

void id_0::id_1()
{
auto void *id_2;
wait(SC_ZERO_TIME);
{
id_2=((!(! ::id_0::id_3)) ? (&&id_4) : (&&id_5));
goto *(id_2);
}
id_5:;
wait(SC_ZERO_TIME);
{
id_2=((! ::id_0::id_3) ? (&&id_5) : (&&id_4));
goto *(id_2);
}
id_4:;
 ::id_0::id_3=(false);
 ::id_6+=(2);
(( ::id_7. ::id_23::id_8). ::id_12::id_9)=(true);
{
id_2=((!(! ::id_0::id_3)) ? (&&id_10) : (&&id_11));
goto *(id_2);
}
id_11:;
wait(SC_ZERO_TIME);
{
id_2=((! ::id_0::id_3) ? (&&id_11) : (&&id_10));
goto *(id_2);
}
id_10:;
 ::id_0::id_3=(false);
 ::id_6+=(3);
(( ::id_7. ::id_23::id_8). ::id_12::id_9)=(true);
return ;
}

void id_12::id_13()
{
auto void *id_14;
wait(SC_ZERO_TIME);
{
id_14=((!(! ::id_12::id_15)) ? (&&id_16) : (&&id_17));
goto *(id_14);
}
id_17:;
wait(SC_ZERO_TIME);
{
id_14=((! ::id_12::id_15) ? (&&id_17) : (&&id_16));
goto *(id_14);
}
id_16:;
 ::id_12::id_15=(false);
 ::id_6*=(5);
(( ::id_7. ::id_23::id_18). ::id_0::id_3)=(true);
{
id_14=((!(! ::id_12::id_9)) ? (&&id_19) : (&&id_20));
goto *(id_14);
}
id_20:;
wait(SC_ZERO_TIME);
{
id_14=((! ::id_12::id_9) ? (&&id_20) : (&&id_19));
goto *(id_14);
}
id_19:;
 ::id_12::id_9=(false);
 ::id_6*=(5);
(( ::id_7. ::id_23::id_18). ::id_0::id_3)=(true);
{
id_14=((!(! ::id_12::id_9)) ? (&&id_21) : (&&id_22));
goto *(id_14);
}
id_22:;
wait(SC_ZERO_TIME);
{
id_14=((! ::id_12::id_9) ? (&&id_22) : (&&id_21));
goto *(id_14);
}
id_21:;
 ::id_12::id_9=(false);
cease(  ::id_6 );
return ;
}

void id_23::id_24()
{
 ::id_6=(1);
( ::id_23::id_8. ::id_12::id_15)=(true);
return ;
}
