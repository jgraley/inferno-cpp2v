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
void *id_10;
public:
float id_11;
/*temp*/ int id_8;
/*temp*/ void *id_12;
/*temp*/ void *id_5;
};
id_0 id_13("id_13");

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
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
goto id_4;
id_4:;
}
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
switch( 0 )
{
case 0:;
break;
}
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
continue;
for( ;  ::id_0::id_3<(4);  ::id_0::id_3++ )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(0);  ::id_0::id_3< ::id_0::id_2;  ::id_0::id_3++ )
{
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
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3=(4) )
 ::id_0::id_2+= ::id_0::id_3;
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
 ::id_0::id_3+=(0);
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
 ::id_0::id_2+=({ {
{
 ::id_0::id_5=(&&id_6);
goto id_7;
}
id_6:;
}
 ::id_0::id_8; });
cease(  ::id_0::id_2 );
return ;
id_7:;
{
/*temp*/ void *id_9;
 ::id_0::id_10= ::id_0::id_5;
{
 ::id_0::id_8=(3);
{
id_9= ::id_0::id_10;
goto *(id_9);
}
}
}
}
