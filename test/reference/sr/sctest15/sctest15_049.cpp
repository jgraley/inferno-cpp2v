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
void *id_6;
public:
/*temp*/ void *id_3;
/*temp*/ void *id_7;
};
id_0 id_8("id_8");

void id_0::id_1()
{
/*temp*/ void *id_2;
 ::id_0::id_3=(&&id_4);
goto id_5;
id_4:;
return ;
id_5:;
 ::id_0::id_6= ::id_0::id_3;
id_2= ::id_0::id_6;
goto *(id_2);
}
