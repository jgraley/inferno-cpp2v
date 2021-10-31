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
private:
void *id_7;
public:
/*temp*/ void *id_4;
/*temp*/ void *id_8;
};
id_0 id_9("id_9");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto void *id_3;
 ::id_0::id_4=(&&id_5);
wait(SC_ZERO_TIME);
{
id_3=(&&id_6);
goto *(id_3);
}
id_5:;
return ;
{
id_3=(&&id_6);
goto *(id_3);
}
id_6:;
 ::id_0::id_7= ::id_0::id_4;
id_2= ::id_0::id_7;
{
id_3=id_2;
goto *(id_3);
}
}
