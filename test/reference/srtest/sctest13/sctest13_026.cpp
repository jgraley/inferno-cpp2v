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
void *id_45;
public:
float id_46;
/*temp*/ int id_40;
/*temp*/ void *id_37;
/*temp*/ void *id_47;
};
id_0 id_48("id_48");

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
{
if( !( ::id_0::id_3<(4)) )
goto id_4;
{
id_5:;
{
goto id_6;
id_7:;
 ::id_0::id_3++;
}
id_8:;
if(  ::id_0::id_3<(4) )
goto id_5;
}
goto id_9;
id_4:;
;
id_9:;
}
}
id_6:;
}
for(  ::id_0::id_3=(0);  ::id_0::id_3<(4);  ::id_0::id_3++ )
switch( 0 )
{
case 0:;
break;
}
{
 ::id_0::id_3=(0);
{
if( !( ::id_0::id_3<(4)) )
goto id_10;
{
id_11:;
{
goto id_12;
id_12:;
 ::id_0::id_3++;
}
id_13:;
if(  ::id_0::id_3<(4) )
goto id_11;
}
goto id_14;
id_10:;
;
id_14:;
}
}
{
;
{
if( !( ::id_0::id_3<(4)) )
goto id_15;
{
id_16:;
{
 ::id_0::id_2+= ::id_0::id_3;
id_17:;
 ::id_0::id_3++;
}
id_18:;
if(  ::id_0::id_3<(4) )
goto id_16;
}
goto id_19;
id_15:;
;
id_19:;
}
}
{
 ::id_0::id_3=(0);
{
if( !( ::id_0::id_3< ::id_0::id_2) )
goto id_20;
{
id_21:;
{
{
}
id_22:;
 ::id_0::id_3++;
}
id_23:;
if(  ::id_0::id_3< ::id_0::id_2 )
goto id_21;
}
goto id_24;
id_20:;
;
id_24:;
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
{
if( !( ::id_0::id_3<(4)) )
goto id_25;
{
id_26:;
{
 ::id_0::id_2+= ::id_0::id_3;
id_27:;
 ::id_0::id_3=(4);
}
id_28:;
if(  ::id_0::id_3<(4) )
goto id_26;
}
goto id_29;
id_25:;
;
id_29:;
}
}
{
 ::id_0::id_3=(0);
{
if( !( ::id_0::id_3<(4)) )
goto id_30;
{
id_31:;
{
 ::id_0::id_3+=(0);
id_32:;
 ::id_0::id_3++;
}
id_33:;
if(  ::id_0::id_3<(4) )
goto id_31;
}
goto id_34;
id_30:;
;
id_34:;
}
}
{
 ::id_0::id_3=(0);
{
if( !( ::id_0::id_3<(4)) )
goto id_35;
{
id_36:;
{
 ::id_0::id_2+=({ {
{
 ::id_0::id_37=(&&id_38);
goto id_39;
}
id_38:;
}
 ::id_0::id_40; });
id_41:;
 ::id_0::id_3++;
}
id_42:;
if(  ::id_0::id_3<(4) )
goto id_36;
}
goto id_43;
id_35:;
;
id_43:;
}
}
cease(  ::id_0::id_2 );
return ;
id_39:;
{
/*temp*/ void *id_44;
 ::id_0::id_45= ::id_0::id_37;
{
 ::id_0::id_40=(3);
{
id_44= ::id_0::id_45;
goto *(id_44);
}
}
}
}
