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
void (id_4)(auto int id_3);
};
id_0 id_5("id_5");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_4(1);
cease(  ::id_0::id_2 );
}

void (id_0::id_4)(int id_3)
{
 ::id_0::id_2++;
if( id_3<(5) )
{
 ::id_0::id_4((1)+id_3);
 ::id_0::id_4((1)+id_3);
}
}
