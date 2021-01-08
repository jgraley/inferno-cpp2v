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
void (id_5)(auto int id_4);
};
id_0 id_8("id_8");

void id_0::id_1()
{
 ::id_0::id_2=(0);
({ /*temp*/ int id_3; id_3=(1);  ::id_0::id_5(id_3); });
cease(  ::id_0::id_2 );
return ;
}

void (id_0::id_5)(int id_4)
{
 ::id_0::id_2++;
if( id_4<(5) )
{
({ /*temp*/ int id_6; id_6=((1)+id_4);  ::id_0::id_5(id_6); });
({ /*temp*/ int id_7; id_7=((1)+id_4);  ::id_0::id_5(id_7); });
}
return ;
}
