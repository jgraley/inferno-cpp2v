#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
short j;
void *link;
int t;
public:
/*temp*/ int f_return;
void T();
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ char f_k;
int x;
private:
char k;
int i;
public:
/*temp*/ void *f_link1;
/*temp*/ short f_j;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_k; temp_k=(8); ({ /*temp*/ int temp_j; temp_j=(6); ({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_j1; temp_j1=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k1; temp_k1=(0); ({ {
{
 ::TopLevel::f_i=temp_i1;
{
 ::TopLevel::f_j=temp_j1;
{
 ::TopLevel::f_k=temp_k1;
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
}
}
}
}
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ {
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_j=temp_j;
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
}
}
}
}
LINK1:;
}
 ::TopLevel::f_return; }); }); }); });
cease( ((2)*({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_j2; temp_j2=(2); ({ {
{
 ::TopLevel::f_i=temp_i2;
{
 ::TopLevel::f_j=temp_j2;
{
 ::TopLevel::f_k=temp_k2;
{
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
}
}
}
}
LINK2:;
}
 ::TopLevel::f_return; }); }); }); }))+ ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
{
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
