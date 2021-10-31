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
id_0 id_8("id_8");
int id_3;
int id_4;

void id_0::id_1()
{
auto void *id_2;
 ::id_3=(1);
 ::id_4=(0);
wait(SC_ZERO_TIME);
{
id_2=((!( ::id_4<(5))) ? (&&id_5) : (&&id_6));
goto *(id_2);
}
id_6:;
 ::id_3+= ::id_4;
wait(SC_ZERO_TIME);
{
id_2=(&&id_7);
goto *(id_2);
}
id_7:;
 ::id_3*=(2);
 ::id_4++;
{
id_2=(( ::id_4<(5)) ? (&&id_6) : (&&id_5));
goto *(id_2);
}
id_5:;
cease(  ::id_3 );
return ;
}
