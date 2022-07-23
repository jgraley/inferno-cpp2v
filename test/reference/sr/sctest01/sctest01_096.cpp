#include "isystemc.h"

class id_0;
class id_12;
class id_22;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_5);
}
enum id_1
{
id_2 = 0,
id_3 = 1,
};
sc_event id_4;
void id_5();
private:
unsigned int id_7;
};
class id_12 : public sc_module
{
public:
SC_CTOR( id_12 )
{
SC_METHOD(id_18);
}
enum id_13
{
id_14 = 0,
id_15 = 1,
id_16 = 2,
};
sc_event id_11;
sc_event id_17;
void id_18();
private:
unsigned int id_20;
};
class id_22 : public sc_module
{
public:
SC_CTOR( id_22 ) :
id_21("id_21"),
id_10("id_10")
{
SC_THREAD(id_24);
}
enum id_23
{
};
void id_24();
 ::id_0 id_21;
 ::id_12 id_10;
};
id_22 id_9("id_9");
int id_8;

void id_0::id_5()
{
/*temp*/ bool id_6 = true;
if( (sc_delta_count())==(0) )
{
next_trigger(  ::id_0::id_4 );
 ::id_0::id_7= ::id_0::id_2;
id_6=(false);
}
if( id_6&&( ::id_0::id_2== ::id_0::id_7) )
{
 ::id_8+=(2);
(( ::id_9. ::id_22::id_10). ::id_12::id_11).notify(SC_ZERO_TIME);
next_trigger(  ::id_0::id_4 );
 ::id_0::id_7= ::id_0::id_3;
id_6=(false);
}
if( id_6&&( ::id_0::id_3== ::id_0::id_7) )
{
 ::id_8+=(3);
(( ::id_9. ::id_22::id_10). ::id_12::id_11).notify(SC_ZERO_TIME);
id_6=(false);
}
if( id_6 )
next_trigger(SC_ZERO_TIME);
}

void id_12::id_18()
{
/*temp*/ bool id_19 = true;
if( (sc_delta_count())==(0) )
{
next_trigger(  ::id_12::id_17 );
 ::id_12::id_20= ::id_12::id_14;
id_19=(false);
}
if( id_19&&( ::id_12::id_14== ::id_12::id_20) )
{
 ::id_8*=(5);
(( ::id_9. ::id_22::id_21). ::id_0::id_4).notify(SC_ZERO_TIME);
next_trigger(  ::id_12::id_11 );
 ::id_12::id_20= ::id_12::id_15;
id_19=(false);
}
if( id_19&&( ::id_12::id_15== ::id_12::id_20) )
{
 ::id_8*=(5);
(( ::id_9. ::id_22::id_21). ::id_0::id_4).notify(SC_ZERO_TIME);
next_trigger(  ::id_12::id_11 );
 ::id_12::id_20= ::id_12::id_16;
id_19=(false);
}
if( id_19&&( ::id_12::id_16== ::id_12::id_20) )
{
cease(  ::id_8 );
id_19=(false);
}
if( id_19 )
next_trigger(SC_ZERO_TIME);
}

void id_22::id_24()
{
/*temp*/ bool id_25 = true;
 ::id_8=(1);
( ::id_22::id_10. ::id_12::id_17).notify(SC_ZERO_TIME);
id_25=(false);
}
