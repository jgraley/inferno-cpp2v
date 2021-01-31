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
int id_32;
public:
int id_6;
private:
void *id_33;
public:
/*temp*/ int id_11;
/*temp*/ int id_9;
/*temp*/ void *id_34;
/*temp*/ void *id_7;
};
id_0 id_35("id_35");

void id_0::id_1()
{
 ::id_0::id_2=(4);
({ /*temp*/ bool id_3; id_3=(++ ::id_0::id_2); {
goto *((!id_3) ? (&&id_4) : (&&id_5));
id_5:;
id_3=( ::id_0::id_6=({ {
{
 ::id_0::id_7=(&&id_8);
{
 ::id_0::id_9= ::id_0::id_2;
goto id_10;
}
}
id_8:;
}
 ::id_0::id_11; }));
goto id_12;
id_4:;
;
id_12:;
}
id_3; });
({ /*temp*/ bool id_13; id_13=(!(++ ::id_0::id_2)); {
goto *((!id_13) ? (&&id_14) : (&&id_15));
id_15:;
;
goto id_16;
id_14:;
id_13=( ::id_0::id_6+=({ {
{
 ::id_0::id_7=(&&id_17);
{
 ::id_0::id_9= ::id_0::id_2;
goto id_10;
}
}
id_17:;
}
 ::id_0::id_11; }));
id_16:;
}
id_13; });
{
 ::id_0::id_2=(0);
{
goto *((!( ::id_0::id_2<(2))) ? (&&id_18) : (&&id_19));
id_19:;
{
id_20:;
{
({ /*temp*/ int id_21; {
goto *((!( ::id_0::id_2++)) ? (&&id_22) : (&&id_23));
id_23:;
id_21=( ::id_0::id_6+=({ {
{
 ::id_0::id_7=(&&id_24);
{
 ::id_0::id_9= ::id_0::id_2;
goto id_10;
}
}
id_24:;
}
 ::id_0::id_11; }));
goto id_25;
id_22:;
id_21=( ::id_0::id_6-=({ {
{
 ::id_0::id_7=(&&id_26);
{
 ::id_0::id_9= ::id_0::id_2;
goto id_10;
}
}
id_26:;
}
 ::id_0::id_11; }));
id_25:;
}
id_21; });
id_27:;
;
}
id_28:;
goto *(( ::id_0::id_2<(2)) ? (&&id_20) : (&&id_29));
id_29:;
}
goto id_30;
id_18:;
;
id_30:;
}
}
cease(  ::id_0::id_6 );
return ;
id_10:;
{
/*temp*/ void *id_31;
 ::id_0::id_32= ::id_0::id_9;
 ::id_0::id_33= ::id_0::id_7;
{
 ::id_0::id_11=((100)/ ::id_0::id_32);
{
id_31= ::id_0::id_33;
goto *(id_31);
}
}
}
}
