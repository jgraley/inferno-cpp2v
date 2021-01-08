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
int (id_11[10U]);
void *(id_10[10U]);
public:
int id_2;
private:
unsigned int id_9;
public:
/*temp*/ int id_4;
/*temp*/ void *id_18;
/*temp*/ void *id_5;
};
id_0 id_19("id_19");

void id_0::id_1()
{
 ::id_0::id_2=(0);
({ /*temp*/ int id_3; id_3=(1); {
{
 ::id_0::id_4=id_3;
{
 ::id_0::id_5=(&&id_6);
goto id_7;
}
}
id_6:;
}
});
cease(  ::id_0::id_2 );
return ;
id_7:;
{
{
/*temp*/ void *id_8;
 ::id_0::id_9++;
( ::id_0::id_10[ ::id_0::id_9])= ::id_0::id_5;
( ::id_0::id_11[ ::id_0::id_9])= ::id_0::id_4;
 ::id_0::id_2++;
{
if( !(( ::id_0::id_11[ ::id_0::id_9])<(5)) )
goto id_12;
{
({ /*temp*/ int id_13; id_13=((1)+( ::id_0::id_11[ ::id_0::id_9])); {
{
 ::id_0::id_4=id_13;
{
 ::id_0::id_5=(&&id_14);
goto id_7;
}
}
id_14:;
}
});
({ /*temp*/ int id_15; id_15=((1)+( ::id_0::id_11[ ::id_0::id_9])); {
{
 ::id_0::id_4=id_15;
{
 ::id_0::id_5=(&&id_16);
goto id_7;
}
}
id_16:;
}
});
}
goto id_17;
id_12:;
;
id_17:;
}
{
id_8=( ::id_0::id_10[ ::id_0::id_9]);
{
 ::id_0::id_9--;
goto *(id_8);
}
}
}
}
}
