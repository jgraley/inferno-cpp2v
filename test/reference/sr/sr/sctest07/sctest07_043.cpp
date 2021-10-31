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
id_0 id_15("id_15");
int id_2;
int id_3;
int id_6;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
{
if( !( ::id_3<(4)) )
goto id_4;
{
id_5:;
{
{
 ::id_2+= ::id_3;
{
 ::id_6=(0);
{
if( !( ::id_6<(3)) )
goto id_7;
{
id_8:;
{
{
wait(SC_ZERO_TIME);
 ::id_2++;
}
id_9:;
 ::id_6++;
}
id_10:;
if(  ::id_6<(3) )
goto id_8;
}
goto id_11;
id_7:;
;
id_11:;
}
}
 ::id_2*=(2);
wait(SC_ZERO_TIME);
}
id_12:;
 ::id_3++;
}
id_13:;
if(  ::id_3<(4) )
goto id_5;
}
goto id_14;
id_4:;
;
id_14:;
}
}
cease(  ::id_2 );
return ;
}
