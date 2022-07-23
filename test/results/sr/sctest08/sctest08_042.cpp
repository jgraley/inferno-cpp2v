#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_13(0)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_15[10]);
void *(id_14[10]);
void *id_24;
unsigned int id_13;
public:
/*temp*/ int id_6;
/*temp*/ void *id_20;
/*temp*/ void *id_25;
/*temp*/ void *id_26;
/*temp*/ void *id_7;
};
id_0 id_27("id_27");
int id_16;
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
if(  ::id_3<(4) )
{
id_4:;
{
{
 ::id_2=( ::id_2+ ::id_3);
({ /*temp*/ int id_5; id_5=(3); {
{
 ::id_0::id_6=id_5;
{
 ::id_0::id_7=(&&id_8);
goto id_9;
}
}
id_8:;
}
});
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
id_10:;
 ::id_3=((1)+ ::id_3);
}
id_11:;
if(  ::id_3<(4) )
goto id_4;
}
}
cease(  ::id_2 );
return ;
id_9:;
{
{
/*temp*/ void *id_12;
 ::id_0::id_13++;
( ::id_0::id_14[ ::id_0::id_13])= ::id_0::id_7;
( ::id_0::id_15[ ::id_0::id_13])= ::id_0::id_6;
{
 ::id_16=(0);
if(  ::id_16<( ::id_0::id_15[ ::id_0::id_13]) )
{
id_17:;
{
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
id_18:;
 ::id_16=((1)+ ::id_16);
}
id_19:;
if(  ::id_16<( ::id_0::id_15[ ::id_0::id_13]) )
goto id_17;
}
}
{
{
 ::id_0::id_20=(&&id_21);
goto id_22;
}
id_21:;
}
{
id_12=( ::id_0::id_14[ ::id_0::id_13]);
{
 ::id_0::id_13--;
goto *(id_12);
}
}
}
}
id_22:;
{
/*temp*/ void *id_23;
 ::id_0::id_24= ::id_0::id_20;
 ::id_2=( ::id_2-(1));
{
id_23= ::id_0::id_24;
goto *(id_23);
}
}
}
