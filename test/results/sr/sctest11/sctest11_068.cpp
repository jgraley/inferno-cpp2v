#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_16);
}
enum id_1
{
id_10 = 2,
id_11 = 5,
id_12 = 12,
id_13 = 6,
id_14 = 10,
id_15 = 4,
id_2 = 1,
id_3 = 13,
id_4 = 3,
id_5 = 7,
id_6 = 9,
id_7 = 11,
id_8 = 0,
id_9 = 8,
};
void id_16();
int id_44;
int id_48;
private:
int id_50;
void *id_49;
public:
/*temp*/ int id_45;
/*temp*/ int id_47;
/*temp*/ void *id_46;
/*temp*/ void *id_51;
};
id_0 id_52("id_52");

void id_0::id_16()
{
/*temp*/ void *id_17;
static const void *(id_18[]) = { &&id_19, &&id_20, &&id_21, &&id_22, &&id_23, &&id_24, &&id_25, &&id_26, &&id_27, &&id_28, &&id_29, &&id_30, &&id_31, &&id_32 };
auto void *id_33;
/*temp*/ int id_34;
/*temp*/ int id_35;
/*temp*/ int id_36;
/*temp*/ int id_37;
/*temp*/ int id_38;
/*temp*/ int id_39;
/*temp*/ bool id_40;
/*temp*/ bool id_41;
/*temp*/ bool id_42;
/*temp*/ bool id_43;
 ::id_0::id_44=(4);
id_40=(++ ::id_0::id_44);
wait(SC_ZERO_TIME);
{
id_33=((!id_40) ? (id_18[ ::id_0::id_15]) : (id_18[ ::id_0::id_8]));
goto *(id_33);
}
id_19:;
 ::id_0::id_45= ::id_0::id_44;
 ::id_0::id_46=(id_18[ ::id_0::id_4]);
{
id_33=(id_18[ ::id_0::id_3]);
goto *(id_33);
}
id_22:;
id_35= ::id_0::id_47;
id_40=( ::id_0::id_48=id_35);
{
id_33=(id_18[ ::id_0::id_15]);
goto *(id_33);
}
id_23:;
id_42=id_40;
id_42;
id_41=(!(++ ::id_0::id_44));
{
id_33=((!id_41) ? (id_18[ ::id_0::id_13]) : (id_18[ ::id_0::id_10]));
goto *(id_33);
}
id_25:;
 ::id_0::id_45= ::id_0::id_44;
 ::id_0::id_46=(id_18[ ::id_0::id_5]);
{
id_33=(id_18[ ::id_0::id_3]);
goto *(id_33);
}
id_26:;
id_36= ::id_0::id_47;
id_41=( ::id_0::id_48+=id_36);
{
id_33=(id_18[ ::id_0::id_10]);
goto *(id_33);
}
id_21:;
id_43=id_41;
id_43;
 ::id_0::id_44=(0);
{
id_33=((!( ::id_0::id_44<(2))) ? (id_18[ ::id_0::id_12]) : (id_18[ ::id_0::id_11]));
goto *(id_33);
}
id_24:;
{
id_33=((!( ::id_0::id_44++)) ? (id_18[ ::id_0::id_14]) : (id_18[ ::id_0::id_9]));
goto *(id_33);
}
id_27:;
 ::id_0::id_45= ::id_0::id_44;
 ::id_0::id_46=(id_18[ ::id_0::id_6]);
{
id_33=(id_18[ ::id_0::id_3]);
goto *(id_33);
}
id_28:;
id_37= ::id_0::id_47;
id_34=( ::id_0::id_48+=id_37);
{
id_33=(id_18[ ::id_0::id_2]);
goto *(id_33);
}
id_29:;
 ::id_0::id_45= ::id_0::id_44;
 ::id_0::id_46=(id_18[ ::id_0::id_7]);
{
id_33=(id_18[ ::id_0::id_3]);
goto *(id_33);
}
id_30:;
id_38= ::id_0::id_47;
id_34=( ::id_0::id_48-=id_38);
{
id_33=(id_18[ ::id_0::id_2]);
goto *(id_33);
}
id_20:;
id_39=id_34;
id_39;
{
id_33=(( ::id_0::id_44<(2)) ? (id_18[ ::id_0::id_11]) : (id_18[ ::id_0::id_12]));
goto *(id_33);
}
id_31:;
cease(  ::id_0::id_48 );
return ;
{
id_33=(id_18[ ::id_0::id_3]);
goto *(id_33);
}
id_32:;
 ::id_0::id_49= ::id_0::id_46;
 ::id_0::id_50= ::id_0::id_45;
 ::id_0::id_47=((100)/ ::id_0::id_50);
id_17= ::id_0::id_49;
{
id_33=id_17;
goto *(id_33);
}
}
