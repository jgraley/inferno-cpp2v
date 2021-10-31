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
id_0 id_19("id_19");
int id_2;
int id_3;
int id_7;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
{
goto *((!( ::id_3<(4))) ? (&&id_4) : (&&id_5));
id_5:;
{
id_6:;
{
{
 ::id_2+= ::id_3;
{
 ::id_7=(0);
{
goto *((!( ::id_7<(3))) ? (&&id_8) : (&&id_9));
id_9:;
{
id_10:;
{
{
wait(SC_ZERO_TIME);
 ::id_2++;
}
id_11:;
 ::id_7++;
}
id_12:;
goto *(( ::id_7<(3)) ? (&&id_10) : (&&id_13));
id_13:;
}
goto id_14;
id_8:;
;
id_14:;
}
}
 ::id_2*=(2);
}
id_15:;
 ::id_3++;
}
id_16:;
goto *(( ::id_3<(4)) ? (&&id_6) : (&&id_17));
id_17:;
}
goto id_18;
id_4:;
;
id_18:;
}
}
cease(  ::id_2 );
return ;
}
