#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_11(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_13[10U]);
void *(id_12[10U]);
void *id_20;
unsigned int id_11;
public:
/*temp*/ int id_5;
/*temp*/ void *id_16;
/*temp*/ void *id_21;
/*temp*/ void *id_22;
/*temp*/ void *id_6;
};
id_0 id_23("id_23");
int id_14;
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
if(  ::id_3<(4) )
do
{
{
 ::id_2=( ::id_2+ ::id_3);
({ /*temp*/ int id_4; id_4=(3); {
{
 ::id_0::id_5=id_4;
{
 ::id_0::id_6=(&&id_7);
goto id_8;
}
}
id_7:;
}
});
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
id_9:;
 ::id_3=((1)+ ::id_3);
}
while(  ::id_3<(4) );
}
cease(  ::id_2 );
return ;
id_8:;
{
{
/*temp*/ void *id_10;
 ::id_0::id_11++;
( ::id_0::id_12[ ::id_0::id_11])= ::id_0::id_6;
( ::id_0::id_13[ ::id_0::id_11])= ::id_0::id_5;
{
 ::id_14=(0);
if(  ::id_14<( ::id_0::id_13[ ::id_0::id_11]) )
do
{
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
id_15:;
 ::id_14=((1)+ ::id_14);
}
while(  ::id_14<( ::id_0::id_13[ ::id_0::id_11]) );
}
{
{
 ::id_0::id_16=(&&id_17);
goto id_18;
}
id_17:;
}
{
id_10=( ::id_0::id_12[ ::id_0::id_11]);
{
 ::id_0::id_11--;
goto *(id_10);
}
}
}
}
id_18:;
{
/*temp*/ void *id_19;
 ::id_0::id_20= ::id_0::id_16;
 ::id_2=( ::id_2-(1));
{
id_19= ::id_0::id_20;
goto *(id_19);
}
}
}
