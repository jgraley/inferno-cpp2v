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
int id_3;
};
id_0 id_12("id_12");
int id_2;
int id_4;

void id_0::id_1()
{
 ::id_2=(1);
 ::id_0::id_3=(5);
{
 ::id_4=(0);
{
if( !( ::id_4< ::id_0::id_3) )
goto id_5;
goto id_6;
id_6:;
{
id_7:;
{
{
 ::id_2+= ::id_4;
wait(SC_ZERO_TIME);
 ::id_2*=(2);
}
id_8:;
 ::id_4++;
}
id_9:;
if(  ::id_4< ::id_0::id_3 )
goto id_7;
goto id_10;
id_10:;
}
goto id_11;
id_5:;
;
id_11:;
}
}
cease(  ::id_2 );
return ;
}
