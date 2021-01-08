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
void (id_2)();
};
id_0 id_3("id_3");

void id_0::id_1()
{
 ::id_0::id_2();
return ;
}

void (id_0::id_2)()
{
return ;
}
