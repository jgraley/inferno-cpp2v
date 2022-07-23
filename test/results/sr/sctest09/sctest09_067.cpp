#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_12(0)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_14[10]);
void *(id_13[10]);
public:
int id_7;
private:
unsigned int id_12;
public:
/*temp*/ int id_8;
/*temp*/ void *id_18;
/*temp*/ void *id_9;
};
id_0 id_19("id_19");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto void *id_3;
/*temp*/ int id_4;
/*temp*/ int id_5;
/*temp*/ int id_6;
 ::id_0::id_7=(0);
id_4=(1);
 ::id_0::id_8=id_4;
 ::id_0::id_9=(&&id_10);
wait(SC_ZERO_TIME);
{
id_3=(&&id_11);
goto *(id_3);
}
id_10:;
cease(  ::id_0::id_7 );
return ;
{
id_3=(&&id_11);
goto *(id_3);
}
id_11:;
 ::id_0::id_12++;
( ::id_0::id_13[ ::id_0::id_12])= ::id_0::id_9;
( ::id_0::id_14[ ::id_0::id_12])= ::id_0::id_8;
 ::id_0::id_7++;
{
id_3=((!(( ::id_0::id_14[ ::id_0::id_12])<(5))) ? (&&id_15) : (&&id_16));
goto *(id_3);
}
id_16:;
id_5=((1)+( ::id_0::id_14[ ::id_0::id_12]));
 ::id_0::id_8=id_5;
 ::id_0::id_9=(&&id_17);
{
id_3=(&&id_11);
goto *(id_3);
}
id_17:;
id_6=((1)+( ::id_0::id_14[ ::id_0::id_12]));
 ::id_0::id_8=id_6;
 ::id_0::id_9=(&&id_15);
{
id_3=(&&id_11);
goto *(id_3);
}
id_15:;
id_2=( ::id_0::id_13[ ::id_0::id_12]);
 ::id_0::id_12--;
{
id_3=id_2;
goto *(id_3);
}
}
