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
int id_2;
void (id_7)();
/*temp*/ int id_6;
/*temp*/ void *id_15;
/*temp*/ void *id_4;
};
id_0 id_16("id_16");

void id_0::id_1()
{
 ::id_0::id_2=(0);
({ /*temp*/ int id_3; id_3=(1); {
{
 ::id_0::id_4=(&&id_5);
{
 ::id_0::id_6=id_3;
 ::id_0::id_7();
}
}
id_5:;
}
});
cease(  ::id_0::id_2 );
return ;
}

void (id_0::id_7)()
{
/*temp*/ void *id_8;
auto int id_9;
auto void *id_10;
id_9= ::id_0::id_6;
id_10= ::id_0::id_4;
 ::id_0::id_2++;
if( id_9<(5) )
{
({ /*temp*/ int id_11; id_11=((1)+id_9); {
{
 ::id_0::id_4=(&&id_12);
{
 ::id_0::id_6=id_11;
 ::id_0::id_7();
}
}
id_12:;
}
});
({ /*temp*/ int id_13; id_13=((1)+id_9); {
{
 ::id_0::id_4=(&&id_14);
{
 ::id_0::id_6=id_13;
 ::id_0::id_7();
}
}
id_14:;
}
});
}
{
id_8=id_10;
return ;
}
}
