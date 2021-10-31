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
int id_5;
int id_6;
private:
void *id_24;
public:
float id_25;
/*temp*/ int id_23;
/*temp*/ void *id_20;
/*temp*/ void *id_26;
};
id_0 id_27("id_27");

void id_0::id_1()
{
/*temp*/ void *id_2;
auto void *id_3;
/*temp*/ int id_4;
 ::id_0::id_5=(0);
for(  ::id_0::id_6=(0);  ::id_0::id_6<(4);  ::id_0::id_6++ )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(0);  ::id_0::id_6<=(4);  ::id_0::id_6++ )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(0); (4)!= ::id_0::id_6;  ::id_0::id_6++ )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(4);  ::id_0::id_6>(0);  ::id_0::id_6-- )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(4);  ::id_0::id_6>=(0);  ::id_0::id_6-- )
 ::id_0::id_5+= ::id_0::id_6;
 ::id_0::id_6=(0);
wait(SC_ZERO_TIME);
{
id_3=((!( ::id_0::id_6<(4))) ? (&&id_7) : (&&id_7));
goto *(id_3);
}
id_7:;
for(  ::id_0::id_6=(0);  ::id_0::id_6<(4);  ::id_0::id_6++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_6=(0);
{
id_3=((!( ::id_0::id_6<(4))) ? (&&id_8) : (&&id_9));
goto *(id_3);
}
id_9:;
 ::id_0::id_6++;
{
id_3=(( ::id_0::id_6<(4)) ? (&&id_9) : (&&id_8));
goto *(id_3);
}
id_8:;
{
id_3=((!( ::id_0::id_6<(4))) ? (&&id_10) : (&&id_11));
goto *(id_3);
}
id_11:;
 ::id_0::id_5+= ::id_0::id_6;
 ::id_0::id_6++;
{
id_3=(( ::id_0::id_6<(4)) ? (&&id_11) : (&&id_10));
goto *(id_3);
}
id_10:;
 ::id_0::id_6=(0);
{
id_3=((!( ::id_0::id_6< ::id_0::id_5)) ? (&&id_12) : (&&id_13));
goto *(id_3);
}
id_13:;
 ::id_0::id_6++;
{
id_3=(( ::id_0::id_6< ::id_0::id_5) ? (&&id_13) : (&&id_12));
goto *(id_3);
}
id_12:;
for(  ::id_0::id_6=(0);  ::id_0::id_6<(4);  ::id_0::id_6=((1)+ ::id_0::id_6) )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(0);  ::id_0::id_6<(4);  ::id_0::id_6+=(1) )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(4);  ::id_0::id_6>(0);  ::id_0::id_6=( ::id_0::id_6-(1)) )
 ::id_0::id_5+= ::id_0::id_6;
for(  ::id_0::id_6=(4);  ::id_0::id_6>(0);  ::id_0::id_6-=(1) )
 ::id_0::id_5+= ::id_0::id_6;
 ::id_0::id_6=(0);
{
id_3=((!( ::id_0::id_6<(4))) ? (&&id_14) : (&&id_15));
goto *(id_3);
}
id_15:;
 ::id_0::id_5+= ::id_0::id_6;
 ::id_0::id_6=(4);
{
id_3=(( ::id_0::id_6<(4)) ? (&&id_15) : (&&id_14));
goto *(id_3);
}
id_14:;
 ::id_0::id_6=(0);
{
id_3=((!( ::id_0::id_6<(4))) ? (&&id_16) : (&&id_17));
goto *(id_3);
}
id_17:;
 ::id_0::id_6+=(0);
 ::id_0::id_6++;
{
id_3=(( ::id_0::id_6<(4)) ? (&&id_17) : (&&id_16));
goto *(id_3);
}
id_16:;
 ::id_0::id_6=(0);
{
id_3=((!( ::id_0::id_6<(4))) ? (&&id_18) : (&&id_19));
goto *(id_3);
}
id_19:;
 ::id_0::id_20=(&&id_21);
{
id_3=(&&id_22);
goto *(id_3);
}
id_21:;
id_4= ::id_0::id_23;
 ::id_0::id_5+=id_4;
 ::id_0::id_6++;
{
id_3=(( ::id_0::id_6<(4)) ? (&&id_19) : (&&id_18));
goto *(id_3);
}
id_18:;
cease(  ::id_0::id_5 );
return ;
{
id_3=(&&id_22);
goto *(id_3);
}
id_22:;
 ::id_0::id_24= ::id_0::id_20;
 ::id_0::id_23=(3);
id_2= ::id_0::id_24;
{
id_3=id_2;
goto *(id_3);
}
}
