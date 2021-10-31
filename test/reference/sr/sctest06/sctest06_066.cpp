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
id_0 id_11("id_11");
int id_3;
int id_4;
int id_7;

void id_0::id_1()
{
auto void *id_2;
 ::id_3=(1);
 ::id_4=(0);
wait(SC_ZERO_TIME);
{
id_2=((!( ::id_4<(4))) ? (&&id_5) : (&&id_6));
goto *(id_2);
}
id_6:;
 ::id_3+= ::id_4;
 ::id_7=(0);
{
id_2=((!( ::id_7<(3))) ? (&&id_8) : (&&id_9));
goto *(id_2);
}
id_9:;
wait(SC_ZERO_TIME);
{
id_2=(&&id_10);
goto *(id_2);
}
id_10:;
 ::id_3++;
 ::id_7++;
{
id_2=(( ::id_7<(3)) ? (&&id_9) : (&&id_8));
goto *(id_2);
}
id_8:;
 ::id_3*=(2);
 ::id_4++;
{
id_2=(( ::id_4<(4)) ? (&&id_6) : (&&id_5));
goto *(id_2);
}
id_5:;
cease(  ::id_3 );
return ;
}
