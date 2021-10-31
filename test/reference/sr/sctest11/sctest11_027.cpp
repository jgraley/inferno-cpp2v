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
void (id_7)(auto int id_4, auto void *id_5);
/*temp*/ int id_8;
/*temp*/ void *id_13;
};
id_0 id_14("id_14");

void id_0::id_1()
{
 ::id_0::id_2=(4);
(++ ::id_0::id_2)&&( ::id_0::id_3=({ {
 ::id_0::id_7( ::id_0::id_2, &&id_6);
id_6:;
}
 ::id_0::id_8; }));
(!(++ ::id_0::id_2))||( ::id_0::id_3+=({ {
 ::id_0::id_7( ::id_0::id_2, &&id_9);
id_9:;
}
 ::id_0::id_8; }));
for(  ::id_0::id_2=(0);  ::id_0::id_2<(2);  )
( ::id_0::id_2++) ? ( ::id_0::id_3+=({ {
 ::id_0::id_7( ::id_0::id_2, &&id_10);
id_10:;
}
 ::id_0::id_8; })) : ( ::id_0::id_3-=({ {
 ::id_0::id_7( ::id_0::id_2, &&id_11);
id_11:;
}
 ::id_0::id_8; }));
cease(  ::id_0::id_3 );
return ;
}

void (id_0::id_7)(int id_4, void *id_5)
{
/*temp*/ void *id_12;
{
 ::id_0::id_8=((100)/id_4);
{
id_12=id_5;
return ;
}
}
}
