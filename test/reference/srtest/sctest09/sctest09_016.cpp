#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_9(0U)
{
SC_THREAD(id_1);
}
void id_1();
private:
int (id_10[10U]);
void *(id_11[10U]);
public:
int id_2;
private:
unsigned int id_9;
public:
/*temp*/ int id_6;
/*temp*/ void *id_16;
/*temp*/ void *id_4;
};
id_0 id_17("id_17");

void id_0::id_1()
{
 ::id_0::id_2=(0);
({ /*temp*/ int id_3; id_3=(1); {
{
 ::id_0::id_4=(&&id_5);
{
 ::id_0::id_6=id_3;
goto id_7;
}
}
id_5:;
}
});
cease(  ::id_0::id_2 );
return ;
id_7:;
{
{
/*temp*/ void *id_8;
 ::id_0::id_9++;
( ::id_0::id_10[ ::id_0::id_9])= ::id_0::id_6;
( ::id_0::id_11[ ::id_0::id_9])= ::id_0::id_4;
 ::id_0::id_2++;
if( ( ::id_0::id_10[ ::id_0::id_9])<(5) )
{
({ /*temp*/ int id_12; id_12=((1)+( ::id_0::id_10[ ::id_0::id_9])); {
{
 ::id_0::id_4=(&&id_13);
{
 ::id_0::id_6=id_12;
goto id_7;
}
}
id_13:;
}
});
({ /*temp*/ int id_14; id_14=((1)+( ::id_0::id_10[ ::id_0::id_9])); {
{
 ::id_0::id_4=(&&id_15);
{
 ::id_0::id_6=id_14;
goto id_7;
}
}
id_15:;
}
});
}
{
id_8=( ::id_0::id_11[ ::id_0::id_9]);
{
 ::id_0::id_9--;
goto *(id_8);
}
}
}
}
}
