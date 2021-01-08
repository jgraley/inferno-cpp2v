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
void *id_60;
public:
float id_61;
/*temp*/ int id_54;
/*temp*/ void *id_51;
/*temp*/ void *id_62;
};
id_0 id_63("id_63");

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
goto *((!( ::id_0::id_3<(4))) ? (&&id_4) : (&&id_5));
id_5:;
{
id_6:;
{
goto id_7;
id_8:;
 ::id_0::id_3++;
}
id_9:;
goto *(( ::id_0::id_3<(4)) ? (&&id_6) : (&&id_10));
id_10:;
}
goto id_11;
id_4:;
;
id_11:;
}
}
id_7:;
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
goto *((!( ::id_0::id_3<(4))) ? (&&id_12) : (&&id_13));
id_13:;
{
id_14:;
{
goto id_15;
id_15:;
 ::id_0::id_3++;
}
id_16:;
goto *(( ::id_0::id_3<(4)) ? (&&id_14) : (&&id_17));
id_17:;
}
goto id_18;
id_12:;
;
id_18:;
}
}
{
;
{
goto *((!( ::id_0::id_3<(4))) ? (&&id_19) : (&&id_20));
id_20:;
{
id_21:;
{
 ::id_0::id_2+= ::id_0::id_3;
id_22:;
 ::id_0::id_3++;
}
id_23:;
goto *(( ::id_0::id_3<(4)) ? (&&id_21) : (&&id_24));
id_24:;
}
goto id_25;
id_19:;
;
id_25:;
}
}
{
 ::id_0::id_3=(0);
{
goto *((!( ::id_0::id_3< ::id_0::id_2)) ? (&&id_26) : (&&id_27));
id_27:;
{
id_28:;
{
{
}
id_29:;
 ::id_0::id_3++;
}
id_30:;
goto *(( ::id_0::id_3< ::id_0::id_2) ? (&&id_28) : (&&id_31));
id_31:;
}
goto id_32;
id_26:;
;
id_32:;
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
goto *((!( ::id_0::id_3<(4))) ? (&&id_33) : (&&id_34));
id_34:;
{
id_35:;
{
 ::id_0::id_2+= ::id_0::id_3;
id_36:;
 ::id_0::id_3=(4);
}
id_37:;
goto *(( ::id_0::id_3<(4)) ? (&&id_35) : (&&id_38));
id_38:;
}
goto id_39;
id_33:;
;
id_39:;
}
}
{
 ::id_0::id_3=(0);
{
goto *((!( ::id_0::id_3<(4))) ? (&&id_40) : (&&id_41));
id_41:;
{
id_42:;
{
 ::id_0::id_3+=(0);
id_43:;
 ::id_0::id_3++;
}
id_44:;
goto *(( ::id_0::id_3<(4)) ? (&&id_42) : (&&id_45));
id_45:;
}
goto id_46;
id_40:;
;
id_46:;
}
}
{
 ::id_0::id_3=(0);
{
goto *((!( ::id_0::id_3<(4))) ? (&&id_47) : (&&id_48));
id_48:;
{
id_49:;
{
{
/*temp*/ int id_50;
{
{
 ::id_0::id_51=(&&id_52);
goto id_53;
}
id_52:;
}
id_50= ::id_0::id_54;
 ::id_0::id_2+=id_50;
}
id_55:;
 ::id_0::id_3++;
}
id_56:;
goto *(( ::id_0::id_3<(4)) ? (&&id_49) : (&&id_57));
id_57:;
}
goto id_58;
id_47:;
;
id_58:;
}
}
cease(  ::id_0::id_2 );
return ;
id_53:;
{
/*temp*/ void *id_59;
 ::id_0::id_60= ::id_0::id_51;
{
 ::id_0::id_54=(3);
{
id_59= ::id_0::id_60;
goto *(id_59);
}
}
}
}
