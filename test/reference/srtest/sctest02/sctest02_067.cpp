#include "isystemc.h"

class id_0;
class id_15;
class id_29;
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
bool id_10;
};
class id_15 : public sc_module
{
public:
SC_CTOR( id_15 )
{
SC_THREAD(id_23);
}
enum id_16
{
id_17 = 0U,
id_18 = 3U,
id_19 = 4U,
id_20 = 1U,
id_21 = 2U,
id_22 = 5U,
};
void id_23();
bool id_14;
bool id_27;
};
class id_29 : public sc_module
{
public:
SC_CTOR( id_29 ) :
id_28("id_28"),
id_13("id_13")
{
SC_THREAD(id_31);
}
enum id_30
{
};
void id_31();
 ::id_0 id_28;
 ::id_15 id_13;
};
id_29 id_12("id_12");
int id_11;

void id_0::id_6()
{
static const unsigned int (id_7[]) = { &&id_8, &&id_8, &&id_8, &&id_8 };
auto unsigned int id_9;
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
id_9=((!(! ::id_0::id_10)) ?  ::id_0::id_4 :  ::id_0::id_2);
continue;
}
if(  ::id_0::id_2==id_9 )
{
wait(SC_ZERO_TIME);
id_9=((! ::id_0::id_10) ?  ::id_0::id_2 :  ::id_0::id_4);
continue;
}
if(  ::id_0::id_4==id_9 )
{
 ::id_0::id_10=(false);
 ::id_11+=(2);
(( ::id_12. ::id_29::id_13). ::id_15::id_14)=(true);
id_9=((!(! ::id_0::id_10)) ?  ::id_0::id_5 :  ::id_0::id_3);
}
if(  ::id_0::id_3==id_9 )
{
wait(SC_ZERO_TIME);
id_9=((! ::id_0::id_10) ?  ::id_0::id_3 :  ::id_0::id_5);
continue;
}
if(  ::id_0::id_5==id_9 )
{
 ::id_0::id_10=(false);
 ::id_11+=(3);
(( ::id_12. ::id_29::id_13). ::id_15::id_14)=(true);
return ;
}
}
while( true );
}

void id_15::id_23()
{
static const unsigned int (id_24[]) = { &&id_25, &&id_25, &&id_25, &&id_25, &&id_25, &&id_25 };
auto unsigned int id_26;
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
id_26=((!(! ::id_15::id_27)) ?  ::id_15::id_20 :  ::id_15::id_17);
continue;
}
if(  ::id_15::id_17==id_26 )
{
wait(SC_ZERO_TIME);
id_26=((! ::id_15::id_27) ?  ::id_15::id_17 :  ::id_15::id_20);
continue;
}
if(  ::id_15::id_20==id_26 )
{
 ::id_15::id_27=(false);
 ::id_11*=(5);
(( ::id_12. ::id_29::id_28). ::id_0::id_10)=(true);
id_26=((!(! ::id_15::id_14)) ?  ::id_15::id_22 :  ::id_15::id_18);
}
if(  ::id_15::id_18==id_26 )
{
wait(SC_ZERO_TIME);
id_26=((! ::id_15::id_14) ?  ::id_15::id_18 :  ::id_15::id_22);
continue;
}
if(  ::id_15::id_22==id_26 )
{
 ::id_15::id_14=(false);
 ::id_11*=(5);
(( ::id_12. ::id_29::id_28). ::id_0::id_10)=(true);
id_26=((!(! ::id_15::id_14)) ?  ::id_15::id_21 :  ::id_15::id_19);
}
if(  ::id_15::id_19==id_26 )
{
wait(SC_ZERO_TIME);
id_26=((! ::id_15::id_14) ?  ::id_15::id_19 :  ::id_15::id_21);
continue;
}
if(  ::id_15::id_21==id_26 )
{
 ::id_15::id_14=(false);
cease(  ::id_11 );
return ;
}
}
while( true );
}

void id_29::id_31()
{
static const unsigned int (id_32[]) = {  };
 ::id_11=(1);
( ::id_29::id_13. ::id_15::id_27)=(true);
return ;
}
