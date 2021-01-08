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
int (id_6)(auto char id_5, auto short id_4, auto int id_3);
};
id_0 id_8("id_8");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=( ::id_0::id_6(8, 6,  ::id_0::id_6(0, 0, 0)));
cease(  ::id_0::id_2+((2)*( ::id_0::id_6(3, 2, 1))) );
}

int (id_0::id_6)(char id_5, short id_4, int id_3)
{
auto int id_7 = id_3+((3)*id_4);
return id_7+((5)*id_5);
}
