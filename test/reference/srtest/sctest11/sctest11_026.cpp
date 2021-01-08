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
int id_2;
private:
int id_28;
public:
int id_5;
private:
void *id_27;
public:
/*temp*/ int id_10;
/*temp*/ int id_6;
/*temp*/ void *id_29;
/*temp*/ void *id_7;
};
id_0 id_30("id_30");

void id_0::id_1()
{
 ::id_0::id_2=(4);
({ /*temp*/ bool id_3; id_3=(++ ::id_0::id_2); {
if( !id_3 )
goto id_4;
id_3=( ::id_0::id_5=({ {
{
 ::id_0::id_6= ::id_0::id_2;
{
 ::id_0::id_7=(&&id_8);
goto id_9;
}
}
id_8:;
}
 ::id_0::id_10; }));
goto id_11;
id_4:;
;
id_11:;
}
id_3; });
({ /*temp*/ bool id_12; id_12=(!(++ ::id_0::id_2)); {
if( !id_12 )
goto id_13;
;
goto id_14;
id_13:;
id_12=( ::id_0::id_5+=({ {
{
 ::id_0::id_6= ::id_0::id_2;
{
 ::id_0::id_7=(&&id_15);
goto id_9;
}
}
id_15:;
}
 ::id_0::id_10; }));
id_14:;
}
id_12; });
{
 ::id_0::id_2=(0);
{
if( !( ::id_0::id_2<(2)) )
goto id_16;
{
id_17:;
{
({ /*temp*/ int id_18; {
if( !( ::id_0::id_2++) )
goto id_19;
id_18=( ::id_0::id_5+=({ {
{
 ::id_0::id_6= ::id_0::id_2;
{
 ::id_0::id_7=(&&id_20);
goto id_9;
}
}
id_20:;
}
 ::id_0::id_10; }));
goto id_21;
id_19:;
id_18=( ::id_0::id_5-=({ {
{
 ::id_0::id_6= ::id_0::id_2;
{
 ::id_0::id_7=(&&id_22);
goto id_9;
}
}
id_22:;
}
 ::id_0::id_10; }));
id_21:;
}
id_18; });
id_23:;
;
}
id_24:;
if(  ::id_0::id_2<(2) )
goto id_17;
}
goto id_25;
id_16:;
;
id_25:;
}
}
cease(  ::id_0::id_5 );
return ;
id_9:;
{
/*temp*/ void *id_26;
 ::id_0::id_27= ::id_0::id_7;
 ::id_0::id_28= ::id_0::id_6;
{
 ::id_0::id_10=((100)/ ::id_0::id_28);
{
id_26= ::id_0::id_27;
goto *(id_26);
}
}
}
}
