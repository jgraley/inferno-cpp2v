#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_4);
}
enum id_1
{
id_2 = 1,
id_3 = 0,
};
void id_4();
private:
void *id_11;
public:
/*temp*/ void *id_10;
/*temp*/ void *id_12;
};
id_0 id_13("id_13");

void id_0::id_4()
{
/*temp*/ void *id_5;
static const void *(id_6[]) = { &&id_7, &&id_8 };
auto void *id_9;
 ::id_0::id_10=(id_6[ ::id_0::id_3]);
wait(SC_ZERO_TIME);
{
id_9=(id_6[ ::id_0::id_2]);
goto *(id_9);
}
id_7:;
return ;
{
id_9=(id_6[ ::id_0::id_2]);
goto *(id_9);
}
id_8:;
 ::id_0::id_11= ::id_0::id_10;
id_5= ::id_0::id_11;
{
id_9=id_5;
goto *(id_9);
}
}
