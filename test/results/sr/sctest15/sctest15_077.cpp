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
unsigned int id_11;
public:
/*temp*/ unsigned int id_10;
/*temp*/ unsigned int id_12;
};
id_0 id_13("id_13");

void id_0::id_4()
{
/*temp*/ unsigned int id_5;
static const unsigned int (id_6[]) = { &&id_7, &&id_8 };
auto unsigned int id_9;
 ::id_0::id_10= ::id_0::id_3;
wait(SC_ZERO_TIME);
id_9= ::id_0::id_2;
id_7:;
if(  ::id_0::id_3==id_9 )
{
return ;
id_9= ::id_0::id_2;
}
id_8:;
if(  ::id_0::id_2==id_9 )
{
 ::id_0::id_11= ::id_0::id_10;
id_5= ::id_0::id_11;
id_9=id_5;
}
goto *(id_6[id_9]);
}
