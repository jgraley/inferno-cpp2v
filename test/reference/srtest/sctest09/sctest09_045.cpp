#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_11(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_12[10U]);
void *(id_13[10U]);
public:
int id_6;
private:
unsigned int id_11;
public:
/*temp*/ int id_9;
/*temp*/ void *id_17;
/*temp*/ void *id_7;
};
id_0 id_18("id_18");

void id_0::id_1()
{
/*temp*/ void *id_2;
/*temp*/ int id_3;
/*temp*/ int id_4;
/*temp*/ int id_5;
 ::id_0::id_6=(0);
id_3=(1);
 ::id_0::id_7=(&&id_8);
 ::id_0::id_9=id_3;
goto id_10;
id_8:;
cease(  ::id_0::id_6 );
return ;
id_10:;
 ::id_0::id_11++;
( ::id_0::id_12[ ::id_0::id_11])= ::id_0::id_9;
( ::id_0::id_13[ ::id_0::id_11])= ::id_0::id_7;
 ::id_0::id_6++;
goto *((!(( ::id_0::id_12[ ::id_0::id_11])<(5))) ? (&&id_14) : (&&id_15));
id_15:;
id_4=((1)+( ::id_0::id_12[ ::id_0::id_11]));
 ::id_0::id_7=(&&id_16);
 ::id_0::id_9=id_4;
goto id_10;
id_16:;
id_5=((1)+( ::id_0::id_12[ ::id_0::id_11]));
 ::id_0::id_7=(&&id_14);
 ::id_0::id_9=id_5;
goto id_10;
id_14:;
id_2=( ::id_0::id_13[ ::id_0::id_11]);
 ::id_0::id_11--;
goto *(id_2);
}
