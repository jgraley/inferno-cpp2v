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
id_0 id_14("id_14");
int id_2;
int id_3;

void id_0::id_1()
{
 ::id_2=(1);
{
 ::id_3=(0);
{
if( !( ::id_3<(5)) )
goto id_4;
goto id_5;
id_5:;
{
id_6:;
{
{
 ::id_2+= ::id_3;
{
if( !((0)==( ::id_3%(2))) )
goto id_7;
goto id_8;
id_8:;
{
wait(SC_ZERO_TIME);
 ::id_2^=(1);
}
goto id_9;
id_7:;
;
id_9:;
}
 ::id_2*=(2);
}
id_10:;
 ::id_3++;
}
id_11:;
if(  ::id_3<(5) )
goto id_6;
goto id_12;
id_12:;
}
goto id_13;
id_4:;
;
id_13:;
}
}
cease(  ::id_2 );
return ;
}
