#include "isystemc.h"

class id_0;
class id_15;
class id_31;
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
class id_15 : public sc_module
{
public:
SC_CTOR( id_15 )
{
SC_THREAD(id_16);
}
void id_16();
bool id_10;
bool id_17;
};
class id_31 : public sc_module
{
public:
SC_CTOR( id_31 ) :
id_22("id_22"),
id_9("id_9")
{
SC_THREAD(id_32);
}
void id_32();
 ::id_0 id_22;
 ::id_15 id_9;
};
id_31 id_8("id_8");
int id_7;

void id_0::id_1()
{
wait(SC_ZERO_TIME);
{
if( !(! ::id_0::id_2) )
goto id_3;
{
id_4:;
wait(SC_ZERO_TIME);
id_5:;
if( ! ::id_0::id_2 )
goto id_4;
}
goto id_6;
id_3:;
;
id_6:;
}
 ::id_0::id_2=(false);
 ::id_7+=(2);
(( ::id_8. ::id_31::id_9). ::id_15::id_10)=(true);
{
if( !(! ::id_0::id_2) )
goto id_11;
{
id_12:;
wait(SC_ZERO_TIME);
id_13:;
if( ! ::id_0::id_2 )
goto id_12;
}
goto id_14;
id_11:;
;
id_14:;
}
 ::id_0::id_2=(false);
 ::id_7+=(3);
(( ::id_8. ::id_31::id_9). ::id_15::id_10)=(true);
return ;
}

void id_15::id_16()
{
{
if( !(! ::id_15::id_17) )
goto id_18;
{
id_19:;
wait(SC_ZERO_TIME);
id_20:;
if( ! ::id_15::id_17 )
goto id_19;
}
goto id_21;
id_18:;
;
id_21:;
}
 ::id_15::id_17=(false);
 ::id_7*=(5);
(( ::id_8. ::id_31::id_22). ::id_0::id_2)=(true);
{
if( !(! ::id_15::id_10) )
goto id_23;
{
id_24:;
wait(SC_ZERO_TIME);
id_25:;
if( ! ::id_15::id_10 )
goto id_24;
}
goto id_26;
id_23:;
;
id_26:;
}
 ::id_15::id_10=(false);
 ::id_7*=(5);
(( ::id_8. ::id_31::id_22). ::id_0::id_2)=(true);
{
if( !(! ::id_15::id_10) )
goto id_27;
{
id_28:;
wait(SC_ZERO_TIME);
id_29:;
if( ! ::id_15::id_10 )
goto id_28;
}
goto id_30;
id_27:;
;
id_30:;
}
 ::id_15::id_10=(false);
cease(  ::id_7 );
return ;
}

void id_31::id_32()
{
 ::id_7=(1);
( ::id_31::id_9. ::id_15::id_17)=(true);
return ;
}
