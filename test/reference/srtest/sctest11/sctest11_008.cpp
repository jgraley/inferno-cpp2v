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
int (id_5)(auto int id_4);
};
id_0 id_6("id_6");

void id_0::id_1()
{
 ::id_0::id_2=(4);
(++ ::id_0::id_2)&&( ::id_0::id_3=( ::id_0::id_5( ::id_0::id_2)));
(!(++ ::id_0::id_2))||( ::id_0::id_3+=( ::id_0::id_5( ::id_0::id_2)));
for(  ::id_0::id_2=(0);  ::id_0::id_2<(2);  )
( ::id_0::id_2++) ? ( ::id_0::id_3+=( ::id_0::id_5( ::id_0::id_2))) : ( ::id_0::id_3-=( ::id_0::id_5( ::id_0::id_2)));
cease(  ::id_0::id_3 );
return ;
}

int (id_0::id_5)(int id_4)
{
return (100)/id_4;
}
