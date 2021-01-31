#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_18);
}
enum id_1
{
id_10 = 1U,
id_11 = 8U,
id_12 = 14U,
id_13 = 15U,
id_14 = 2U,
id_15 = 10U,
id_16 = 13U,
id_17 = 6U,
id_2 = 7U,
id_3 = 3U,
id_4 = 4U,
id_5 = 12U,
id_6 = 9U,
id_7 = 5U,
id_8 = 11U,
id_9 = 0U,
};
void id_18();
int id_43;
private:
void *id_46;
public:
/*temp*/ int id_45;
/*temp*/ void *id_44;
/*temp*/ void *id_47;
};
id_0 id_48("id_48");

void id_0::id_18()
{
/*temp*/ void *id_19;
static const void *(id_20[]) = { &&id_21, &&id_22, &&id_23, &&id_24, &&id_25, &&id_26, &&id_27, &&id_28, &&id_29, &&id_30, &&id_31, &&id_32, &&id_33, &&id_34, &&id_35, &&id_36 };
auto int id_37;
auto int id_38;
auto void *id_39;
/*temp*/ int id_40;
/*temp*/ int id_41;
/*temp*/ int id_42;
 ::id_0::id_43=(0);
switch( 0 )
{
case 1:;
 ::id_0::id_43=(99);
break;
case 0:;
if( ((0)== ::id_0::id_43)||((2)== ::id_0::id_43) )
 ::id_0::id_43=((false) ? (88) : (2));
break;
}
id_37=(0);
wait(SC_ZERO_TIME);
{
id_39=(((0)==id_37) ? (id_20[ ::id_0::id_4]) : (id_20[ ::id_0::id_9]));
goto *(id_39);
}
id_21:;
{
id_39=(((4)==id_37) ? (id_20[ ::id_0::id_3]) : (id_20[ ::id_0::id_10]));
goto *(id_39);
}
id_22:;
{
id_39=(((1)==id_37) ? (id_20[ ::id_0::id_14]) : (id_20[ ::id_0::id_14]));
goto *(id_39);
}
id_23:;
 ::id_0::id_43=(99);
{
id_39=(id_20[ ::id_0::id_2]);
goto *(id_39);
}
id_24:;
 ::id_0::id_43=(44);
{
id_39=(id_20[ ::id_0::id_4]);
goto *(id_39);
}
id_25:;
if( ((0)== ::id_0::id_43)||((2)== ::id_0::id_43) )
 ::id_0::id_43=((false) ? (88) : (2));
{
id_39=(id_20[ ::id_0::id_2]);
goto *(id_39);
}
id_28:;
id_38=(2);
{
id_39=(((2)==id_38) ? (id_20[ ::id_0::id_5]) : (id_20[ ::id_0::id_11]));
goto *(id_39);
}
id_29:;
{
id_39=(((1)==id_38) ? (id_20[ ::id_0::id_15]) : (id_20[ ::id_0::id_15]));
goto *(id_39);
}
id_31:;
 ::id_0::id_43=(99);
{
id_39=(id_20[ ::id_0::id_17]);
goto *(id_39);
}
id_33:;
{
id_39=((!(((0)== ::id_0::id_43)||((2)== ::id_0::id_43))) ? (id_20[ ::id_0::id_17]) : (id_20[ ::id_0::id_12]));
goto *(id_39);
}
id_35:;
{
id_39=((!(false)) ? (id_20[ ::id_0::id_16]) : (id_20[ ::id_0::id_13]));
goto *(id_39);
}
id_36:;
id_40=(88);
{
id_39=(id_20[ ::id_0::id_6]);
goto *(id_39);
}
id_34:;
 ::id_0::id_44=(id_20[ ::id_0::id_8]);
{
id_39=(id_20[ ::id_0::id_7]);
goto *(id_39);
}
id_32:;
id_41= ::id_0::id_45;
id_40=id_41;
{
id_39=(id_20[ ::id_0::id_6]);
goto *(id_39);
}
id_30:;
id_42=id_40;
 ::id_0::id_43=id_42;
{
id_39=(id_20[ ::id_0::id_17]);
goto *(id_39);
}
id_27:;
cease(  ::id_0::id_43 );
return ;
{
id_39=(id_20[ ::id_0::id_7]);
goto *(id_39);
}
id_26:;
 ::id_0::id_46= ::id_0::id_44;
 ::id_0::id_45=(3);
id_19= ::id_0::id_46;
{
id_39=id_19;
goto *(id_39);
}
}
