#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_22(0U)
{
SC_THREAD(id_7);
}
enum id_1
{
id_2 = 1U,
id_3 = 0U,
id_4 = 3U,
id_5 = 4U,
id_6 = 2U,
};
void id_7();
private:
int (id_23[10U]);
unsigned int (id_24[10U]);
public:
int id_19;
private:
unsigned int id_22;
public:
/*temp*/ int id_21;
/*temp*/ unsigned int id_20;
/*temp*/ unsigned int id_25;
};
id_0 id_26("id_26");

void id_0::id_7()
{
/*temp*/ unsigned int id_8;
static const unsigned int (id_9[]) = { &&id_10, &&id_11, &&id_12, &&id_13, &&id_14 };
auto unsigned int id_15;
/*temp*/ int id_16;
/*temp*/ int id_17;
/*temp*/ int id_18;
 ::id_0::id_19=(0);
id_16=(1);
 ::id_0::id_20= ::id_0::id_3;
 ::id_0::id_21=id_16;
wait(SC_ZERO_TIME);
id_15= ::id_0::id_2;
id_14:;
id_13:;
id_12:;
id_11:;
id_10:;
if(  ::id_0::id_3==id_15 )
{
cease(  ::id_0::id_19 );
return ;
id_15= ::id_0::id_2;
}
if(  ::id_0::id_2==id_15 )
{
 ::id_0::id_22++;
( ::id_0::id_23[ ::id_0::id_22])= ::id_0::id_21;
( ::id_0::id_24[ ::id_0::id_22])= ::id_0::id_20;
 ::id_0::id_19++;
id_15=((!(( ::id_0::id_23[ ::id_0::id_22])<(5))) ?  ::id_0::id_5 :  ::id_0::id_6);
}
if(  ::id_0::id_6==id_15 )
{
id_17=((1)+( ::id_0::id_23[ ::id_0::id_22]));
 ::id_0::id_20= ::id_0::id_4;
 ::id_0::id_21=id_17;
id_15= ::id_0::id_2;
}
if(  ::id_0::id_4==id_15 )
{
id_18=((1)+( ::id_0::id_23[ ::id_0::id_22]));
 ::id_0::id_20= ::id_0::id_5;
 ::id_0::id_21=id_18;
id_15= ::id_0::id_2;
}
if(  ::id_0::id_5==id_15 )
{
id_8=( ::id_0::id_24[ ::id_0::id_22]);
 ::id_0::id_22--;
id_15=id_8;
}
goto *(id_9[id_15]);
}
