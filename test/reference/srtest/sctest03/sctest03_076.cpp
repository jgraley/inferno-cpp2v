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
int id_3;
int id_4;
};
id_0 id_5("id_5");

void id_0::id_1()
{
/*temp*/ bool id_2 = true;
 ::id_0::id_3++;
 ::id_0::id_4+= ::id_0::id_3;
if( (10)== ::id_0::id_3 )
cease(  ::id_0::id_4 );
next_trigger(SC_ZERO_TIME);
id_2=(false);
}
