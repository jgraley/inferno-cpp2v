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
int id_13;
int id_2;
private:
int id_40;
void *id_39;
public:
/*temp*/ int id_12;
/*temp*/ int id_8;
/*temp*/ void *id_41;
/*temp*/ void *id_9;
};
id_0 id_42("id_42");

void id_0::id_1()
{
 ::id_0::id_2=(4);
{
/*temp*/ bool id_3;
/*temp*/ bool id_4;
id_3=(++ ::id_0::id_2);
{
goto *((!id_3) ? (&&id_5) : (&&id_6));
id_6:;
{
/*temp*/ int id_7;
{
{
 ::id_0::id_8= ::id_0::id_2;
{
 ::id_0::id_9=(&&id_10);
goto id_11;
}
}
id_10:;
}
id_7= ::id_0::id_12;
id_3=( ::id_0::id_13=id_7);
}
goto id_14;
id_5:;
;
id_14:;
}
id_4=id_3;
id_4;
}
{
/*temp*/ bool id_15;
/*temp*/ bool id_16;
id_15=(!(++ ::id_0::id_2));
{
goto *((!id_15) ? (&&id_17) : (&&id_18));
id_18:;
;
goto id_19;
id_17:;
{
/*temp*/ int id_20;
{
{
 ::id_0::id_8= ::id_0::id_2;
{
 ::id_0::id_9=(&&id_21);
goto id_11;
}
}
id_21:;
}
id_20= ::id_0::id_12;
id_15=( ::id_0::id_13+=id_20);
}
id_19:;
}
id_16=id_15;
id_16;
}
{
 ::id_0::id_2=(0);
{
goto *((!( ::id_0::id_2<(2))) ? (&&id_22) : (&&id_23));
id_23:;
{
id_24:;
{
{
/*temp*/ int id_25;
/*temp*/ int id_26;
{
goto *((!( ::id_0::id_2++)) ? (&&id_27) : (&&id_28));
id_28:;
{
/*temp*/ int id_29;
{
{
 ::id_0::id_8= ::id_0::id_2;
{
 ::id_0::id_9=(&&id_30);
goto id_11;
}
}
id_30:;
}
id_29= ::id_0::id_12;
id_25=( ::id_0::id_13+=id_29);
}
goto id_31;
id_27:;
{
/*temp*/ int id_32;
{
{
 ::id_0::id_8= ::id_0::id_2;
{
 ::id_0::id_9=(&&id_33);
goto id_11;
}
}
id_33:;
}
id_32= ::id_0::id_12;
id_25=( ::id_0::id_13-=id_32);
}
id_31:;
}
id_26=id_25;
id_26;
}
id_34:;
;
}
id_35:;
goto *(( ::id_0::id_2<(2)) ? (&&id_24) : (&&id_36));
id_36:;
}
goto id_37;
id_22:;
;
id_37:;
}
}
cease(  ::id_0::id_13 );
return ;
id_11:;
{
/*temp*/ void *id_38;
 ::id_0::id_39= ::id_0::id_9;
 ::id_0::id_40= ::id_0::id_8;
{
 ::id_0::id_12=((100)/ ::id_0::id_40);
{
id_38= ::id_0::id_39;
goto *(id_38);
}
}
}
}
