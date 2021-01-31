#include "isystemc.h"

class id_0;
class id_13;
class id_25;
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
id_3 = 3U,
id_4 = 1U,
id_5 = 2U,
};
void id_6();
bool id_8;
};
class id_13 : public sc_module
{
public:
SC_CTOR( id_13 )
{
SC_THREAD(id_21);
}
enum id_14
{
id_15 = 0U,
id_16 = 3U,
id_17 = 4U,
id_18 = 1U,
id_19 = 2U,
id_20 = 5U,
};
void id_21();
bool id_12;
bool id_23;
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
auto unsigned int id_7;
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
id_7=((!(! ::id_0::id_8)) ?  ::id_0::id_4 :  ::id_0::id_2);
continue;
}
if(  ::id_0::id_2==id_7 )
{
wait(SC_ZERO_TIME);
id_7=((! ::id_0::id_8) ?  ::id_0::id_2 :  ::id_0::id_4);
continue;
}
if(  ::id_0::id_4==id_7 )
{
 ::id_0::id_8=(false);
 ::id_9+=(2);
(( ::id_10. ::id_25::id_11). ::id_13::id_12)=(true);
id_7=((!(! ::id_0::id_8)) ?  ::id_0::id_5 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_7 )
{
wait(SC_ZERO_TIME);
id_7=((! ::id_0::id_8) ?  ::id_0::id_3 :  ::id_0::id_5);
continue;
}
if(  ::id_0::id_5==id_7 )
{
 ::id_0::id_8=(false);
 ::id_9+=(3);
(( ::id_10. ::id_25::id_11). ::id_13::id_12)=(true);
return ;
}
}
while( true );
}

void id_13::id_21()
{
auto unsigned int id_22;
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
id_22=((!(! ::id_13::id_23)) ?  ::id_13::id_18 :  ::id_13::id_15);
continue;
}
if(  ::id_13::id_15==id_22 )
{
wait(SC_ZERO_TIME);
id_22=((! ::id_13::id_23) ?  ::id_13::id_15 :  ::id_13::id_18);
continue;
}
if(  ::id_13::id_18==id_22 )
{
 ::id_13::id_23=(false);
 ::id_9*=(5);
(( ::id_10. ::id_25::id_24). ::id_0::id_8)=(true);
id_22=((!(! ::id_13::id_12)) ?  ::id_13::id_20 :  ::id_13::id_16);
}
if(  ::id_13::id_16==id_22 )
{
wait(SC_ZERO_TIME);
id_22=((! ::id_13::id_12) ?  ::id_13::id_16 :  ::id_13::id_20);
continue;
}
if(  ::id_13::id_20==id_22 )
{
 ::id_13::id_12=(false);
 ::id_9*=(5);
(( ::id_10. ::id_25::id_24). ::id_0::id_8)=(true);
id_22=((!(! ::id_13::id_12)) ?  ::id_13::id_19 :  ::id_13::id_17);
}
if(  ::id_13::id_17==id_22 )
{
wait(SC_ZERO_TIME);
id_22=((! ::id_13::id_12) ?  ::id_13::id_17 :  ::id_13::id_19);
continue;
}
if(  ::id_13::id_19==id_22 )
{
 ::id_13::id_12=(false);
cease(  ::id_9 );
return ;
}
}
while( true );
}

void id_25::id_27()
{
 ::id_9=(1);
( ::id_25::id_11. ::id_13::id_23)=(true);
return ;
}
