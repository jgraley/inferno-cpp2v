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
int id_18;
public:
int id_2;
int id_3;
private:
void *id_17;
public:
/*temp*/ int id_4;
/*temp*/ int id_8;
/*temp*/ void *id_19;
/*temp*/ void *id_5;
};
id_0 id_20("id_20");

void id_0::id_1()
{
 ::id_0::id_2=(4);
(++ ::id_0::id_2)&&( ::id_0::id_3=({ {
{
 ::id_0::id_4= ::id_0::id_2;
{
 ::id_0::id_5=(&&id_6);
goto id_7;
}
}
id_6:;
}
 ::id_0::id_8; }));
({ /*temp*/ bool id_9; id_9=(!(++ ::id_0::id_2)); if( id_9 )
;
else
id_9=( ::id_0::id_3+=({ {
{
 ::id_0::id_4= ::id_0::id_2;
{
 ::id_0::id_5=(&&id_10);
goto id_7;
}
}
id_10:;
}
 ::id_0::id_8; }));
id_9; });
{
 ::id_0::id_2=(0);
if(  ::id_0::id_2<(2) )
{
id_11:;
{
( ::id_0::id_2++) ? ( ::id_0::id_3+=({ {
{
 ::id_0::id_4= ::id_0::id_2;
{
 ::id_0::id_5=(&&id_12);
goto id_7;
}
}
id_12:;
}
 ::id_0::id_8; })) : ( ::id_0::id_3-=({ {
{
 ::id_0::id_4= ::id_0::id_2;
{
 ::id_0::id_5=(&&id_13);
goto id_7;
}
}
id_13:;
}
 ::id_0::id_8; }));
id_14:;
;
}
id_15:;
if(  ::id_0::id_2<(2) )
goto id_11;
}
}
cease(  ::id_0::id_3 );
return ;
id_7:;
{
/*temp*/ void *id_16;
 ::id_0::id_17= ::id_0::id_5;
 ::id_0::id_18= ::id_0::id_4;
{
 ::id_0::id_8=((100)/ ::id_0::id_18);
{
id_16= ::id_0::id_17;
goto *(id_16);
}
}
}
}
