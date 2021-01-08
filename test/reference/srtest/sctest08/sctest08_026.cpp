#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_15(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_17[10U]);
void *(id_16[10U]);
void *id_28;
unsigned int id_15;
public:
/*temp*/ int id_7;
/*temp*/ void *id_24;
/*temp*/ void *id_29;
/*temp*/ void *id_30;
/*temp*/ void *id_8;
};
id_0 id_31("id_31");
int id_18;
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
{
if( !( ::id_3<(4)) )
goto id_4;
{
id_5:;
{
{
 ::id_2=( ::id_2+ ::id_3);
({ /*temp*/ int id_6; id_6=(3); {
{
 ::id_0::id_7=id_6;
{
 ::id_0::id_8=(&&id_9);
goto id_10;
}
}
id_9:;
}
});
 ::id_2=((2)* ::id_2);
wait(SC_ZERO_TIME);
}
id_11:;
 ::id_3=((1)+ ::id_3);
}
id_12:;
if(  ::id_3<(4) )
goto id_5;
}
goto id_13;
id_4:;
;
id_13:;
}
}
cease(  ::id_2 );
return ;
id_10:;
{
{
/*temp*/ void *id_14;
 ::id_0::id_15++;
( ::id_0::id_16[ ::id_0::id_15])= ::id_0::id_8;
( ::id_0::id_17[ ::id_0::id_15])= ::id_0::id_7;
{
 ::id_18=(0);
{
if( !( ::id_18<( ::id_0::id_17[ ::id_0::id_15])) )
goto id_19;
{
id_20:;
{
{
wait(SC_ZERO_TIME);
 ::id_2=((1)+ ::id_2);
}
id_21:;
 ::id_18=((1)+ ::id_18);
}
id_22:;
if(  ::id_18<( ::id_0::id_17[ ::id_0::id_15]) )
goto id_20;
}
goto id_23;
id_19:;
;
id_23:;
}
}
{
{
 ::id_0::id_24=(&&id_25);
goto id_26;
}
id_25:;
}
{
id_14=( ::id_0::id_16[ ::id_0::id_15]);
{
 ::id_0::id_15--;
goto *(id_14);
}
}
}
}
id_26:;
{
/*temp*/ void *id_27;
 ::id_0::id_28= ::id_0::id_24;
 ::id_2=( ::id_2-(1));
{
id_27= ::id_0::id_28;
goto *(id_27);
}
}
}
