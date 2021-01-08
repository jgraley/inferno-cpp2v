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
void *id_17;
public:
float id_18;
/*temp*/ int id_14;
/*temp*/ void *id_11;
/*temp*/ void *id_19;
};
id_0 id_20("id_20");

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
do
{
goto id_4;
id_5:;
 ::id_0::id_3++;
}
while(  ::id_0::id_3<(4) );
}
id_4:;
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
do
{
goto id_6;
id_6:;
 ::id_0::id_3++;
}
while(  ::id_0::id_3<(4) );
}
{
;
if(  ::id_0::id_3<(4) )
do
{
 ::id_0::id_2+= ::id_0::id_3;
id_7:;
 ::id_0::id_3++;
}
while(  ::id_0::id_3<(4) );
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3< ::id_0::id_2 )
do
{
{
}
id_8:;
 ::id_0::id_3++;
}
while(  ::id_0::id_3< ::id_0::id_2 );
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
do
{
 ::id_0::id_2+= ::id_0::id_3;
id_9:;
 ::id_0::id_3=(4);
}
while(  ::id_0::id_3<(4) );
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
do
{
 ::id_0::id_3+=(0);
id_10:;
 ::id_0::id_3++;
}
while(  ::id_0::id_3<(4) );
}
{
 ::id_0::id_3=(0);
if(  ::id_0::id_3<(4) )
do
{
 ::id_0::id_2+=({ {
{
 ::id_0::id_11=(&&id_12);
goto id_13;
}
id_12:;
}
 ::id_0::id_14; });
id_15:;
 ::id_0::id_3++;
}
while(  ::id_0::id_3<(4) );
}
cease(  ::id_0::id_2 );
return ;
id_13:;
{
/*temp*/ void *id_16;
 ::id_0::id_17= ::id_0::id_11;
{
 ::id_0::id_14=(3);
{
id_16= ::id_0::id_17;
goto *(id_16);
}
}
}
}
