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
void *id_47;
public:
float id_48;
/*temp*/ int id_44;
/*temp*/ void *id_41;
/*temp*/ void *id_49;
};
id_0 id_50("id_50");

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
id_8:;
goto id_9;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_8) : (&&id_10));
id_10:;
goto id_11;
id_6:;
id_11:;
id_9:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_12) : (&&id_13));
id_13:;
id_14:;
goto id_15;
id_15:;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_14) : (&&id_16));
id_16:;
goto id_17;
id_12:;
id_17:;
goto *((!( ::id_0::id_5<(4))) ? (&&id_18) : (&&id_19));
id_19:;
id_20:;
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_20) : (&&id_21));
id_21:;
goto id_22;
id_18:;
id_22:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5< ::id_0::id_4)) ? (&&id_23) : (&&id_24));
id_24:;
id_25:;
 ::id_0::id_5++;
goto *(( ::id_0::id_5< ::id_0::id_4) ? (&&id_25) : (&&id_26));
id_26:;
goto id_27;
id_23:;
id_27:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5=((1)+ ::id_0::id_5) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5+=(1) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5=( ::id_0::id_5-(1)) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5-=(1) )
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_28) : (&&id_29));
id_29:;
id_30:;
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(4);
goto *(( ::id_0::id_5<(4)) ? (&&id_30) : (&&id_31));
id_31:;
goto id_32;
id_28:;
id_32:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_33) : (&&id_34));
id_34:;
id_35:;
 ::id_0::id_5+=(0);
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_35) : (&&id_36));
id_36:;
goto id_37;
id_33:;
id_37:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_38) : (&&id_39));
id_39:;
id_40:;
 ::id_0::id_41=(&&id_42);
goto id_43;
id_42:;
id_3= ::id_0::id_44;
 ::id_0::id_4+=id_3;
 ::id_0::id_5++;
goto *(( ::id_0::id_5<(4)) ? (&&id_40) : (&&id_45));
id_45:;
goto id_46;
id_38:;
id_46:;
cease(  ::id_0::id_4 );
return ;
id_43:;
 ::id_0::id_47= ::id_0::id_41;
 ::id_0::id_44=(3);
id_2= ::id_0::id_47;
goto *(id_2);
}
