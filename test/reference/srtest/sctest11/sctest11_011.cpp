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
int id_3;
void (id_7)();
/*temp*/ int id_6;
/*temp*/ int id_8;
/*temp*/ void *id_15;
/*temp*/ void *id_4;
};
id_0 id_16("id_16");

void id_0::id_1()
{
 ::id_0::id_2=(4);
(++ ::id_0::id_2)&&( ::id_0::id_3=({ {
{
 ::id_0::id_4=(&&id_5);
{
 ::id_0::id_6= ::id_0::id_2;
 ::id_0::id_7();
}
}
id_5:;
}
 ::id_0::id_8; }));
(!(++ ::id_0::id_2))||( ::id_0::id_3+=({ {
{
 ::id_0::id_4=(&&id_9);
{
 ::id_0::id_6= ::id_0::id_2;
 ::id_0::id_7();
}
}
id_9:;
}
 ::id_0::id_8; }));
for(  ::id_0::id_2=(0);  ::id_0::id_2<(2);  )
( ::id_0::id_2++) ? ( ::id_0::id_3+=({ {
{
 ::id_0::id_4=(&&id_10);
{
 ::id_0::id_6= ::id_0::id_2;
 ::id_0::id_7();
}
}
id_10:;
}
 ::id_0::id_8; })) : ( ::id_0::id_3-=({ {
{
 ::id_0::id_4=(&&id_11);
{
 ::id_0::id_6= ::id_0::id_2;
 ::id_0::id_7();
}
}
id_11:;
}
 ::id_0::id_8; }));
cease(  ::id_0::id_3 );
return ;
}

void (id_0::id_7)()
{
/*temp*/ void *id_12;
auto int id_13 =  ::id_0::id_6;
auto void *id_14 =  ::id_0::id_4;
{
 ::id_0::id_8=((100)/id_13);
{
id_12=id_14;
return ;
}
}
}
