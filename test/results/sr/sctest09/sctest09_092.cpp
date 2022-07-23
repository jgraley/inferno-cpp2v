#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_14(0)
{
SC_THREAD(id_8);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_3 = 3,
id_4 = 0,
id_5 = 4,
id_6 = 1,
id_7 = 2,
};
void id_8();
private:
int (id_16[10]);
unsigned int (id_15[10]);
public:
int id_9;
private:
unsigned int id_13;
unsigned int id_14;
public:
/*temp*/ int id_10;
/*temp*/ int id_11;
/*temp*/ int id_17;
/*temp*/ int id_18;
/*temp*/ unsigned int id_12;
};
id_0 id_19("id_19");

void id_0::id_8()
{
do
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_9=(0);
 ::id_0::id_10=(1);
 ::id_0::id_11= ::id_0::id_10;
 ::id_0::id_12= ::id_0::id_5;
wait(SC_ZERO_TIME);
 ::id_0::id_13= ::id_0::id_3;
continue;
}
if(  ::id_0::id_5== ::id_0::id_13 )
{
cease(  ::id_0::id_9 );
return ;
 ::id_0::id_13= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_13 )
{
 ::id_0::id_14++;
( ::id_0::id_15[ ::id_0::id_14])= ::id_0::id_12;
( ::id_0::id_16[ ::id_0::id_14])= ::id_0::id_11;
 ::id_0::id_9++;
 ::id_0::id_13=((!(( ::id_0::id_16[ ::id_0::id_14])<(5))) ?  ::id_0::id_6 :  ::id_0::id_7);
}
if(  ::id_0::id_7== ::id_0::id_13 )
{
 ::id_0::id_17=((1)+( ::id_0::id_16[ ::id_0::id_14]));
 ::id_0::id_11= ::id_0::id_17;
 ::id_0::id_12= ::id_0::id_4;
 ::id_0::id_13= ::id_0::id_3;
}
if(  ::id_0::id_4== ::id_0::id_13 )
{
 ::id_0::id_18=((1)+( ::id_0::id_16[ ::id_0::id_14]));
 ::id_0::id_11= ::id_0::id_18;
 ::id_0::id_12= ::id_0::id_6;
 ::id_0::id_13= ::id_0::id_3;
}
if(  ::id_0::id_6== ::id_0::id_13 )
{
 ::id_0::id_1=( ::id_0::id_15[ ::id_0::id_14]);
 ::id_0::id_14--;
 ::id_0::id_13= ::id_0::id_1;
}
wait(SC_ZERO_TIME);
}
while( true );
}
