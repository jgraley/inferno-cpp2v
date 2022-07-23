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
int id_4;
};
id_0 id_9("id_9");
int id_3;
int id_5;

void id_0::id_1()
{
auto void *id_2;
 ::id_3=(1);
 ::id_0::id_4=(5);
 ::id_5=(0);
wait(SC_ZERO_TIME);
{
id_2=((!( ::id_5< ::id_0::id_4)) ? (&&id_6) : (&&id_7));
goto *(id_2);
}
id_7:;
 ::id_3+= ::id_5;
wait(SC_ZERO_TIME);
{
id_2=(&&id_8);
goto *(id_2);
}
id_8:;
 ::id_3*=(2);
 ::id_5++;
{
id_2=(( ::id_5< ::id_0::id_4) ? (&&id_7) : (&&id_6));
goto *(id_2);
}
id_6:;
cease(  ::id_3 );
return ;
}
