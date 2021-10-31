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
id_0 id_7("id_7");
int id_2;
int id_3;
int id_4;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
if(  ::id_3<(4) )
do
{
{
 ::id_2+= ::id_3;
{
 ::id_4=(0);
if(  ::id_4<(3) )
do
{
{
wait(SC_ZERO_TIME);
 ::id_2++;
}
id_5:;
 ::id_4++;
}
while(  ::id_4<(3) );
}
 ::id_2*=(2);
}
id_6:;
 ::id_3++;
}
while(  ::id_3<(4) );
}
cease(  ::id_2 );
return ;
}
