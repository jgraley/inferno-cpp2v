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
int id_4;
int id_5;
private:
void *id_32;
public:
float id_33;
/*temp*/ int id_30;
/*temp*/ void *id_27;
/*temp*/ void *id_34;
};
id_0 id_35("id_35");

void id_0::id_1()
{
/*temp*/ void *id_2;
/*temp*/ int id_3;
 ::id_0::id_4=(0);
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5++ )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(0);  ::id_0::id_5<=(4);  ::id_0::id_5++ )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(0); (4)!= ::id_0::id_5;  ::id_0::id_5++ )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5-- )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>=(0);  ::id_0::id_5-- )
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_6) : (&&id_7));
id_7:;
goto id_6;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_7) : (&&id_8));
id_8:;
goto id_6;
id_6:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_9) : (&&id_10));
id_10:;
goto id_11;
id_11:;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_10) : (&&id_12));
id_12:;
goto id_9;
id_9:;
goto *((!( ::id_0::id_5<(4))) ? (&&id_13) : (&&id_14));
id_14:;
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_14) : (&&id_15));
id_15:;
goto id_13;
id_13:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5< ::id_0::id_4)) ? (&&id_16) : (&&id_17));
id_17:;
 ::id_0::id_5++;
goto *(( ::id_0::id_5< ::id_0::id_4) ? (&&id_17) : (&&id_18));
id_18:;
goto id_16;
id_16:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5=((1)+ ::id_0::id_5) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5+=(1) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5=( ::id_0::id_5-(1)) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5-=(1) )
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_19) : (&&id_20));
id_20:;
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(4);
goto *(( ::id_0::id_5<(4)) ? (&&id_20) : (&&id_21));
id_21:;
goto id_19;
id_19:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_22) : (&&id_23));
id_23:;
 ::id_0::id_5+=(0);
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_23) : (&&id_24));
id_24:;
goto id_22;
id_22:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_25) : (&&id_26));
id_26:;
 ::id_0::id_27=(&&id_28);
goto id_29;
id_28:;
id_3= ::id_0::id_30;
 ::id_0::id_4+=id_3;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_26) : (&&id_31));
id_31:;
goto id_25;
id_25:;
cease(  ::id_0::id_4 );
return ;
id_29:;
 ::id_0::id_32= ::id_0::id_27;
 ::id_0::id_30=(3);
id_2= ::id_0::id_32;
goto *(id_2);
}
