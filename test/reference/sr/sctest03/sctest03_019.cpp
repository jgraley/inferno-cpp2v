#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_1);
}
void id_1();
int id_2;
int id_3;
};
id_0 id_4("id_4");
int id_5 = 0;
int id_6 = 0;

void id_0::id_1()
{
 ::id_0::id_2++;
 ::id_0::id_3+= ::id_0::id_2;
if( (10)== ::id_0::id_2 )
cease(  ::id_0::id_3 );
next_trigger(SC_ZERO_TIME);
}
