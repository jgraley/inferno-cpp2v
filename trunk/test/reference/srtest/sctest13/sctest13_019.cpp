#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
int i;
float fi;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
private:
void *link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<=(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i!=(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>=(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
{
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
do
{
goto BREAK;
CONTINUE:;
 ::TopLevel::i++;
}
while(  ::TopLevel::i<(4) );
}
BREAK:;
}
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
do
{
goto CONTINUE1;
CONTINUE1:;
 ::TopLevel::i++;
}
while(  ::TopLevel::i<(4) );
}
{
;
if(  ::TopLevel::i<(4) )
do
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE2:;
 ::TopLevel::i++;
}
while(  ::TopLevel::i<(4) );
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i< ::TopLevel::x )
do
{
{
}
CONTINUE3:;
 ::TopLevel::i++;
}
while(  ::TopLevel::i< ::TopLevel::x );
}
;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=( ::TopLevel::i+(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
do
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE4:;
 ::TopLevel::i=(4);
}
while(  ::TopLevel::i<(4) );
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
do
{
 ::TopLevel::i+=(0);
CONTINUE5:;
 ::TopLevel::i++;
}
while(  ::TopLevel::i<(4) );
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
do
{
 ::TopLevel::x+=({ {
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
CONTINUE6:;
 ::TopLevel::i++;
}
while(  ::TopLevel::i<(4) );
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}