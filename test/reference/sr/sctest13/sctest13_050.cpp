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
void *id_60;
public:
float id_61;
/*temp*/ int id_55;
/*temp*/ void *id_52;
/*temp*/ void *id_62;
};
id_0 id_63("id_63");

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
id_10:;
 ::id_0::id_5++;
id_11:;
goto *(( ::id_0::id_5<(4)) ? (&&id_8) : (&&id_12));
id_12:;
goto id_13;
id_6:;
id_13:;
id_9:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_14) : (&&id_15));
id_15:;
id_16:;
goto id_17;
id_17:;
 ::id_0::id_5++;
id_18:;
goto *(( ::id_0::id_5<(4)) ? (&&id_16) : (&&id_19));
id_19:;
goto id_20;
id_14:;
id_20:;
goto *((!( ::id_0::id_5<(4))) ? (&&id_21) : (&&id_22));
id_22:;
id_23:;
 ::id_0::id_4+= ::id_0::id_5;
id_24:;
 ::id_0::id_5++;
id_25:;
goto *(( ::id_0::id_5<(4)) ? (&&id_23) : (&&id_26));
id_26:;
goto id_27;
id_21:;
id_27:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5< ::id_0::id_4)) ? (&&id_28) : (&&id_29));
id_29:;
id_30:;
id_31:;
 ::id_0::id_5++;
id_32:;
goto *(( ::id_0::id_5< ::id_0::id_4) ? (&&id_30) : (&&id_33));
id_33:;
goto id_34;
id_28:;
id_34:;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5=((1)+ ::id_0::id_5) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(0);  ::id_0::id_5<(4);  ::id_0::id_5+=(1) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5=( ::id_0::id_5-(1)) )
 ::id_0::id_4+= ::id_0::id_5;
for(  ::id_0::id_5=(4);  ::id_0::id_5>(0);  ::id_0::id_5-=(1) )
 ::id_0::id_4+= ::id_0::id_5;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_35) : (&&id_36));
id_36:;
id_37:;
 ::id_0::id_4+= ::id_0::id_5;
id_38:;
 ::id_0::id_5=(4);
id_39:;
goto *(( ::id_0::id_5<(4)) ? (&&id_37) : (&&id_40));
id_40:;
goto id_41;
id_35:;
id_41:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_42) : (&&id_43));
id_43:;
id_44:;
 ::id_0::id_5+=(0);
id_45:;
 ::id_0::id_5++;
id_46:;
goto *(( ::id_0::id_5<(4)) ? (&&id_44) : (&&id_47));
id_47:;
goto id_48;
id_42:;
id_48:;
 ::id_0::id_5=(0);
goto *((!( ::id_0::id_5<(4))) ? (&&id_49) : (&&id_50));
id_50:;
id_51:;
 ::id_0::id_52=(&&id_53);
goto id_54;
id_53:;
id_3= ::id_0::id_55;
 ::id_0::id_4+=id_3;
id_56:;
 ::id_0::id_5++;
id_57:;
goto *(( ::id_0::id_5<(4)) ? (&&id_51) : (&&id_58));
id_58:;
goto id_59;
id_49:;
id_59:;
cease(  ::id_0::id_4 );
return ;
id_54:;
 ::id_0::id_60= ::id_0::id_52;
 ::id_0::id_55=(3);
id_2= ::id_0::id_60;
goto *(id_2);
}
