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
id_0 id_4("id_4");
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
for(  ::id_3=(0);  ::id_3<(5);  ::id_3++ )
{
 ::id_2+= ::id_3;
wait(SC_ZERO_TIME);
 ::id_2*=(2);
}
cease(  ::id_2 );
return ;
}
