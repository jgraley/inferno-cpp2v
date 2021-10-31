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
int id_3;
private:
void *id_31;
public:
float id_32;
/*temp*/ int id_27;
/*temp*/ void *id_24;
/*temp*/ void *id_33;
};
id_0 id_34("id_34");

void id_0::id_1()
{
 ::id_0::id_2=(0);
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(0);  ::id_0::id_3<=(4);  ::id_0::id_3++ )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(0); (4)!= ::id_0::id_3;  ::id_0::id_3++ )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(4);  ::id_0::id_3>(0);  ::id_0::id_3-- )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(4);  ::id_0::id_3>=(0);  ::id_0::id_3-- )
 ::id_0::id_2+= ::id_0::id_3;
{
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
{
id_4:;
{
goto id_5;
id_6:;
 ::id_0::id_3++;
}
id_7:;
if(  ::id_0::id_3<(4) )
goto id_4;
}
}
id_5:;
}
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
switch( 0 )
{
case 0:;
break;
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
{
id_8:;
{
goto id_9;
id_9:;
 ::id_0::id_3++;
}
id_10:;
if(  ::id_0::id_3<(4) )
goto id_8;
}
}
{
;
if(  ::id_0::id_3<(4) )
{
id_11:;
{
 ::id_0::id_2+= ::id_0::id_3;
id_12:;
 ::id_0::id_3++;
}
id_13:;
if(  ::id_0::id_3<(4) )
goto id_11;
}
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3< ::id_0::id_2 )
{
id_14:;
{
{
}
id_15:;
 ::id_0::id_3++;
}
id_16:;
if(  ::id_0::id_3< ::id_0::id_2 )
goto id_14;
}
}
;
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3=((1)+ ::id_0::id_3) )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3+=(1) )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(4);  ::id_0::id_3>(0);  ::id_0::id_3=( ::id_0::id_3-(1)) )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(4);  ::id_0::id_3>(0);  ::id_0::id_3-=(1) )
 ::id_0::id_2+= ::id_0::id_3;
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
{
id_17:;
{
 ::id_0::id_2+= ::id_0::id_3;
id_18:;
 ::id_0::id_3=(4);
}
id_19:;
if(  ::id_0::id_3<(4) )
goto id_17;
}
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
{
id_20:;
{
 ::id_0::id_3+=(0);
id_21:;
 ::id_0::id_3++;
}
id_22:;
if(  ::id_0::id_3<(4) )
goto id_20;
}
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
{
id_23:;
{
 ::id_0::id_2+=({ {
{
 ::id_0::id_24=(&&id_25);
goto id_26;
}
id_25:;
}
 ::id_0::id_27; });
id_28:;
 ::id_0::id_3++;
}
id_29:;
if(  ::id_0::id_3<(4) )
goto id_23;
}
}
cease(  ::id_0::id_2 );
return ;
id_26:;
{
/*temp*/ void *id_30;
 ::id_0::id_31= ::id_0::id_24;
{
 ::id_0::id_27=(3);
{
id_30= ::id_0::id_31;
goto *(id_30);
}
}
}
}
