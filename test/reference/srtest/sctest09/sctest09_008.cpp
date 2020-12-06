#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
int x;
void (recurser)(auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1);  ::TopLevel::recurser(temp_i); });
cease(  ::TopLevel::x );
return ;
}

void (TopLevel::recurser)(int i)
{
 ::TopLevel::x++;
if( i<(5) )
{
({ /*temp*/ int temp_i1; temp_i1=((1)+i);  ::TopLevel::recurser(temp_i1); });
({ /*temp*/ int temp_i2; temp_i2=((1)+i);  ::TopLevel::recurser(temp_i2); });
}
return ;
}
