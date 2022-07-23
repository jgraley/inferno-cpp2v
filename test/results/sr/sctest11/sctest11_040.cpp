#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_1);
}
void id_1();
private:
int id_19;
public:
int id_2;
int id_4;
private:
void *id_18;
public:
/*temp*/ int id_5;
/*temp*/ int id_9;
/*temp*/ void *id_20;
/*temp*/ void *id_6;
};
id_0 id_21("id_21");

void id_0::id_1()
{
 ::id_0::id_2=(4);
({ /*temp*/ bool id_3; id_3=(++ ::id_0::id_2); if( id_3 )
id_3=( ::id_0::id_4=({ {
{
 ::id_0::id_5= ::id_0::id_2;
{
 ::id_0::id_6=(&&id_7);
goto id_8;
}
}
id_7:;
}
 ::id_0::id_9; }));
id_3; });
({ /*temp*/ bool id_10; id_10=(!(++ ::id_0::id_2)); if( id_10 )
;
else
id_10=( ::id_0::id_4+=({ {
{
 ::id_0::id_5= ::id_0::id_2;
{
 ::id_0::id_6=(&&id_11);
goto id_8;
}
}
id_11:;
}
 ::id_0::id_9; }));
id_10; });
{
 ::id_0::id_2=(0);
if(  ::id_0::id_2<(2) )
{
id_12:;
{
( ::id_0::id_2++) ? ( ::id_0::id_4+=({ {
{
 ::id_0::id_5= ::id_0::id_2;
{
 ::id_0::id_6=(&&id_13);
goto id_8;
}
}
id_13:;
}
 ::id_0::id_9; })) : ( ::id_0::id_4-=({ {
{
 ::id_0::id_5= ::id_0::id_2;
{
 ::id_0::id_6=(&&id_14);
goto id_8;
}
}
id_14:;
}
 ::id_0::id_9; }));
id_15:;
;
}
id_16:;
if(  ::id_0::id_2<(2) )
goto id_12;
}
}
cease(  ::id_0::id_4 );
return ;
id_8:;
{
/*temp*/ void *id_17;
 ::id_0::id_18= ::id_0::id_6;
 ::id_0::id_19= ::id_0::id_5;
{
 ::id_0::id_9=((100)/ ::id_0::id_19);
{
id_17= ::id_0::id_18;
goto *(id_17);
}
}
}
}
