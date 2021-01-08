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
};
id_0 id_5("id_5");
int id_2;
int id_3;
int id_4;

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(4);  ::id_3++ )
{
 ::id_2+= ::id_3;
for(  ::id_4=(0);  ::id_4<(3);  ::id_4++ )
{
wait(SC_ZERO_TIME);
 ::id_2++;
}
 ::id_2*=(2);
wait(SC_ZERO_TIME);
}
cease(  ::id_2 );
}
