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
void *id_23;
public:
float id_24;
/*temp*/ int id_22;
/*temp*/ void *id_19;
/*temp*/ void *id_25;
};
id_0 id_26("id_26");

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
wait(SC_ZERO_TIME);
goto *((!( ::id_0::id_5<(4))) ? (&&id_6) : (&&id_6));
id_6:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_7) : (&&id_8));
id_8:;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_8) : (&&id_7));
id_7:;
goto *((!( ::id_0::id_5<(4))) ? (&&id_9) : (&&id_10));
id_10:;
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_10) : (&&id_9));
id_9:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5< ::id_0::id_4)) ? (&&id_11) : (&&id_12));
id_12:;
 ::id_0::id_5++;
goto *(( ::id_0::id_5< ::id_0::id_4) ? (&&id_12) : (&&id_11));
id_11:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5=((1)+ ::id_0::id_5) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5+=(1) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5=( ::id_0::id_5-(1)) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5-=(1) )
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_13) : (&&id_14));
id_14:;
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(4);
goto *(( ::id_0::id_5<(4)) ? (&&id_14) : (&&id_13));
id_13:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_15) : (&&id_16));
id_16:;
 ::id_0::id_5+=(0);
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_16) : (&&id_15));
id_15:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_17) : (&&id_18));
id_18:;
 ::id_0::id_19=(&&id_20);
goto id_21;
id_20:;
id_3= ::id_0::id_22;
 ::id_0::id_4+=id_3;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_18) : (&&id_17));
id_17:;
cease(  ::id_0::id_4 );
return ;
goto id_21;
id_21:;
 ::id_0::id_23= ::id_0::id_19;
 ::id_0::id_22=(3);
id_2= ::id_0::id_23;
goto *(id_2);
}
