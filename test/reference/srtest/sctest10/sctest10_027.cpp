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
char id_23;
short id_24;
public:
int id_2;
private:
int id_25;
int id_26;
void *id_22;
public:
/*temp*/ char id_9;
/*temp*/ short id_8;
/*temp*/ int id_13;
/*temp*/ int id_7;
/*temp*/ void *id_10;
/*temp*/ void *id_27;
};
id_0 id_28("id_28");

void id_0::id_1()
{
 ::id_0::id_2=(0);
 ::id_0::id_2=({ /*temp*/ int id_3; id_3=({ /*temp*/ int id_4; id_4=(0); ({ /*temp*/ int id_5; id_5=(0); ({ /*temp*/ int id_6; id_6=(0); ({ {
{
 ::id_0::id_7=id_4;
{
 ::id_0::id_8=id_5;
{
 ::id_0::id_9=id_6;
{
 ::id_0::id_10=(&&id_11);
goto id_12;
}
}
}
}
id_11:;
}
 ::id_0::id_13; }); }); }); }); ({ /*temp*/ int id_14; id_14=(6); ({ /*temp*/ int id_15; id_15=(8); ({ {
{
 ::id_0::id_7=id_3;
{
 ::id_0::id_8=id_14;
{
 ::id_0::id_9=id_15;
{
 ::id_0::id_10=(&&id_16);
goto id_12;
}
}
}
}
id_16:;
}
 ::id_0::id_13; }); }); }); });
cease(  ::id_0::id_2+((2)*({ /*temp*/ int id_17; id_17=(1); ({ /*temp*/ int id_18; id_18=(2); ({ /*temp*/ int id_19; id_19=(3); ({ {
{
 ::id_0::id_7=id_17;
{
 ::id_0::id_8=id_18;
{
 ::id_0::id_9=id_19;
{
 ::id_0::id_10=(&&id_20);
goto id_12;
}
}
}
}
id_20:;
}
 ::id_0::id_13; }); }); }); })) );
return ;
id_12:;
{
/*temp*/ void *id_21;
 ::id_0::id_22= ::id_0::id_10;
 ::id_0::id_23= ::id_0::id_9;
 ::id_0::id_24= ::id_0::id_8;
 ::id_0::id_25= ::id_0::id_7;
 ::id_0::id_26=( ::id_0::id_25+((3)* ::id_0::id_24));
{
 ::id_0::id_13=( ::id_0::id_26+((5)* ::id_0::id_23));
{
id_21= ::id_0::id_22;
goto *(id_21);
}
}
}
}
