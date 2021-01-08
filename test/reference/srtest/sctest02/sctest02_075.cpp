#include "isystemc.h"

class id_0;
class id_13;
class id_25;
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
id_3 = 2U,
id_4 = 1U,
id_5 = 3U,
};
void id_6();
bool id_8;
private:
unsigned int id_7;
};
class id_13 : public sc_module
{
public:
SC_CTOR( id_13 )
{
SC_METHOD(id_21);
}
enum id_14
{
id_15 = 0U,
id_16 = 2U,
id_17 = 4U,
id_18 = 1U,
id_19 = 3U,
id_20 = 5U,
};
void id_21();
bool id_12;
bool id_23;
private:
unsigned int id_22;
};
class id_25 : public sc_module
{
public:
SC_CTOR( id_25 ) :
id_24("id_24"),
id_11("id_11")
{
SC_THREAD(id_27);
}
enum id_26
{
};
void id_27();
 ::id_0 id_24;
 ::id_13 id_11;
};
id_25 id_10("id_10");
int id_9;

void id_0::id_6()
{
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_7=((!(! ::id_0::id_8)) ?  ::id_0::id_4 :  ::id_0::id_2);
return ;
}
if(  ::id_0::id_2== ::id_0::id_7 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_7=((! ::id_0::id_8) ?  ::id_0::id_2 :  ::id_0::id_4);
return ;
}
if(  ::id_0::id_4== ::id_0::id_7 )
{
 ::id_0::id_8=(false);
 ::id_9+=(2);
(( ::id_10. ::id_25::id_11). ::id_13::id_12)=(true);
 ::id_0::id_7=((!(! ::id_0::id_8)) ?  ::id_0::id_5 :  ::id_0::id_3);
}
if(  ::id_0::id_3== ::id_0::id_7 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_7=((! ::id_0::id_8) ?  ::id_0::id_3 :  ::id_0::id_5);
return ;
}
if(  ::id_0::id_5== ::id_0::id_7 )
{
 ::id_0::id_8=(false);
 ::id_9+=(3);
(( ::id_10. ::id_25::id_11). ::id_13::id_12)=(true);
return ;
}
next_trigger(SC_ZERO_TIME);
}

void id_13::id_21()
{
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::id_13::id_22=((!(! ::id_13::id_23)) ?  ::id_13::id_18 :  ::id_13::id_15);
return ;
}
if(  ::id_13::id_15== ::id_13::id_22 )
{
next_trigger(SC_ZERO_TIME);
 ::id_13::id_22=((! ::id_13::id_23) ?  ::id_13::id_15 :  ::id_13::id_18);
return ;
}
if(  ::id_13::id_18== ::id_13::id_22 )
{
 ::id_13::id_23=(false);
 ::id_9*=(5);
(( ::id_10. ::id_25::id_24). ::id_0::id_8)=(true);
 ::id_13::id_22=((!(! ::id_13::id_12)) ?  ::id_13::id_19 :  ::id_13::id_16);
}
if(  ::id_13::id_16== ::id_13::id_22 )
{
next_trigger(SC_ZERO_TIME);
 ::id_13::id_22=((! ::id_13::id_12) ?  ::id_13::id_16 :  ::id_13::id_19);
return ;
}
if(  ::id_13::id_19== ::id_13::id_22 )
{
 ::id_13::id_12=(false);
 ::id_9*=(5);
(( ::id_10. ::id_25::id_24). ::id_0::id_8)=(true);
 ::id_13::id_22=((!(! ::id_13::id_12)) ?  ::id_13::id_20 :  ::id_13::id_17);
}
if(  ::id_13::id_17== ::id_13::id_22 )
{
next_trigger(SC_ZERO_TIME);
 ::id_13::id_22=((! ::id_13::id_12) ?  ::id_13::id_17 :  ::id_13::id_20);
return ;
}
if(  ::id_13::id_20== ::id_13::id_22 )
{
 ::id_13::id_12=(false);
cease(  ::id_9 );
return ;
}
next_trigger(SC_ZERO_TIME);
}

void id_25::id_27()
{
 ::id_9=(1);
( ::id_25::id_11. ::id_13::id_23)=(true);
return ;
}
