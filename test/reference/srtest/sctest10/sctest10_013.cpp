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
void (id_14)();
/*temp*/ char id_11;
/*temp*/ short id_12;
/*temp*/ int id_13;
/*temp*/ int id_15;
/*temp*/ void *id_27;
/*temp*/ void *id_9;
};
id_0 id_28("id_28");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=({ /*temp*/ int id_3; id_3=(8); ({ /*temp*/ int id_4; id_4=(6); ({ /*temp*/ int id_5; id_5=({ /*temp*/ int id_6; id_6=(0); ({ /*temp*/ int id_7; id_7=(0); ({ /*temp*/ int id_8; id_8=(0); ({ {
{
 ::id_0::id_9=(&&id_10);
{
 ::id_0::id_11=id_6;
{
 ::id_0::id_12=id_7;
{
 ::id_0::id_13=id_8;
 ::id_0::id_14();
}
}
}
}
id_10:;
}
 ::id_0::id_15; }); }); }); }); ({ {
{
 ::id_0::id_9=(&&id_16);
{
 ::id_0::id_11=id_3;
{
 ::id_0::id_12=id_4;
{
 ::id_0::id_13=id_5;
 ::id_0::id_14();
}
}
}
}
id_16:;
}
 ::id_0::id_15; }); }); }); });
cease(  ::id_0::id_2+((2)*({ /*temp*/ int id_17; id_17=(3); ({ /*temp*/ int id_18; id_18=(2); ({ /*temp*/ int id_19; id_19=(1); ({ {
{
 ::id_0::id_9=(&&id_20);
{
 ::id_0::id_11=id_17;
{
 ::id_0::id_12=id_18;
{
 ::id_0::id_13=id_19;
 ::id_0::id_14();
}
}
}
}
id_20:;
}
 ::id_0::id_15; }); }); }); })) );
return ;
}

void (id_0::id_14)()
{
/*temp*/ void *id_21;
auto char id_22;
auto short id_23;
auto int id_24;
auto int id_25;
auto void *id_26;
id_24= ::id_0::id_13;
id_23= ::id_0::id_12;
id_22= ::id_0::id_11;
id_26= ::id_0::id_9;
id_25=(id_24+((3)*id_23));
{
 ::id_0::id_15=(id_25+((5)*id_22));
{
id_21=id_26;
return ;
}
}
}
