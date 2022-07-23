#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_10(0)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_12[10]);
void *(id_11[10]);
void *id_18;
unsigned int id_10;
public:
/*temp*/ int id_5;
/*temp*/ void *id_14;
/*temp*/ void *id_19;
/*temp*/ void *id_20;
/*temp*/ void *id_6;
};
id_0 id_21("id_21");
int id_13;
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(4);  ::id_3=((1)+ ::id_3) )
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
cease(  ::id_2 );
return ;
id_8:;
{
{
/*temp*/ void *id_9;
 ::id_0::id_10++;
( ::id_0::id_11[ ::id_0::id_10])= ::id_0::id_6;
( ::id_0::id_12[ ::id_0::id_10])= ::id_0::id_5;
for(  ::id_13=(0);  ::id_13<( ::id_0::id_12[ ::id_0::id_10]);  ::id_13=((1)+ ::id_13) )
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
{
{
 ::id_0::id_14=(&&id_15);
goto id_16;
}
id_15:;
}
{
id_9=( ::id_0::id_11[ ::id_0::id_10]);
{
 ::id_0::id_10--;
goto *(id_9);
}
}
}
}
id_16:;
{
/*temp*/ void *id_17;
 ::id_0::id_18= ::id_0::id_14;
 ::id_2=( ::id_2-(1));
{
id_17= ::id_0::id_18;
goto *(id_17);
}
}
}
