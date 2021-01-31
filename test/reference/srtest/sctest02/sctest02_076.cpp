#include "isystemc.h"

class id_0;
class id_14;
class id_27;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_6);
}
enum id_1
{
id_2 = 0U,
id_3 = 3U,
id_4 = 1U,
id_5 = 2U,
};
void id_6();
bool id_9;
private:
unsigned int id_8;
};
class id_14 : public sc_module
{
public:
SC_CTOR( id_14 )
{
SC_METHOD(id_22);
}
enum id_15
{
id_16 = 0U,
id_17 = 3U,
id_18 = 4U,
id_19 = 1U,
id_20 = 2U,
id_21 = 5U,
};
void id_22();
bool id_13;
bool id_25;
private:
unsigned int id_24;
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

void id_0::id_6()
{
/*temp*/ bool id_7 = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_8=((!(! ::id_0::id_9)) ?  ::id_0::id_4 :  ::id_0::id_2);
id_7=(false);
}
if( id_7 )
{
if(  ::id_0::id_2== ::id_0::id_8 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_8=((! ::id_0::id_9) ?  ::id_0::id_2 :  ::id_0::id_4);
id_7=(false);
}
}
if( id_7 )
{
if(  ::id_0::id_4== ::id_0::id_8 )
{
 ::id_0::id_9=(false);
 ::id_10+=(2);
(( ::id_11. ::id_27::id_12). ::id_14::id_13)=(true);
 ::id_0::id_8=((!(! ::id_0::id_9)) ?  ::id_0::id_5 :  ::id_0::id_3);
}
}
if( id_7 )
{
if(  ::id_0::id_3== ::id_0::id_8 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_8=((! ::id_0::id_9) ?  ::id_0::id_3 :  ::id_0::id_5);
id_7=(false);
}
}
if( id_7 )
{
if(  ::id_0::id_5== ::id_0::id_8 )
{
 ::id_0::id_9=(false);
 ::id_10+=(3);
(( ::id_11. ::id_27::id_12). ::id_14::id_13)=(true);
id_7=(false);
}
}
if( id_7 )
next_trigger(SC_ZERO_TIME);
}

void id_14::id_22()
{
/*temp*/ bool id_23 = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::id_14::id_24=((!(! ::id_14::id_25)) ?  ::id_14::id_19 :  ::id_14::id_16);
id_23=(false);
}
if( id_23 )
{
if(  ::id_14::id_16== ::id_14::id_24 )
{
next_trigger(SC_ZERO_TIME);
 ::id_14::id_24=((! ::id_14::id_25) ?  ::id_14::id_16 :  ::id_14::id_19);
id_23=(false);
}
}
if( id_23 )
{
if(  ::id_14::id_19== ::id_14::id_24 )
{
 ::id_14::id_25=(false);
 ::id_10*=(5);
(( ::id_11. ::id_27::id_26). ::id_0::id_9)=(true);
 ::id_14::id_24=((!(! ::id_14::id_13)) ?  ::id_14::id_21 :  ::id_14::id_17);
}
}
if( id_23 )
{
if(  ::id_14::id_17== ::id_14::id_24 )
{
next_trigger(SC_ZERO_TIME);
 ::id_14::id_24=((! ::id_14::id_13) ?  ::id_14::id_17 :  ::id_14::id_21);
id_23=(false);
}
}
if( id_23 )
{
if(  ::id_14::id_21== ::id_14::id_24 )
{
 ::id_14::id_13=(false);
 ::id_10*=(5);
(( ::id_11. ::id_27::id_26). ::id_0::id_9)=(true);
 ::id_14::id_24=((!(! ::id_14::id_13)) ?  ::id_14::id_20 :  ::id_14::id_18);
}
}
if( id_23 )
{
if(  ::id_14::id_18== ::id_14::id_24 )
{
next_trigger(SC_ZERO_TIME);
 ::id_14::id_24=((! ::id_14::id_13) ?  ::id_14::id_18 :  ::id_14::id_20);
id_23=(false);
}
}
if( id_23 )
{
if(  ::id_14::id_20== ::id_14::id_24 )
{
 ::id_14::id_13=(false);
cease(  ::id_10 );
id_23=(false);
}
}
if( id_23 )
next_trigger(SC_ZERO_TIME);
}

void id_27::id_29()
{
/*temp*/ bool id_30 = true;
 ::id_10=(1);
( ::id_27::id_12. ::id_14::id_25)=(true);
id_30=(false);
}
