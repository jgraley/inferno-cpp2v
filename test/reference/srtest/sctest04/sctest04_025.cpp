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

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
if(  ::id_3<(5) )
{
id_4:;
{
{
 ::id_2+= ::id_3;
wait(SC_ZERO_TIME);
 ::id_2*=(2);
}
id_5:;
 ::id_3++;
}
id_6:;
if(  ::id_3<(5) )
goto id_4;
}
}
cease(  ::id_2 );
return ;
}
