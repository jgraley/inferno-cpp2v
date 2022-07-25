class sc_event;
class sc_interface;
class sc_module;
class TopLevel;
class sc_event
{
void (notify)(auto void p1_2);
};
class sc_interface
{
};
class sc_module
{
};
class TopLevel : private sc_module
{
public:
TopLevel(auto char (*name));
int gvar;
int tot;
void (method)();
};
TopLevel top_level;
void SC_ZERO_TIME;
int gvar_1 = 0;
int tot_1 = 0;
void (cease)(void p1);
void (exit)(void p1_3);
void (next_trigger)(void p1_1);
void (wait)(void p1_4);
void (SC_METHOD)(void func);
void (SC_THREAD)(void func_1);
void (SC_CTHREAD)(void clock, void func_2);

void (sc_event::notify)(void p1_2);

TopLevel::TopLevel(char (*name))
{
 ::SC_METHOD( ::TopLevel::method);
}

void (TopLevel::method)()
{
 ::TopLevel::gvar++;
 ::TopLevel::tot+= ::TopLevel::gvar;
if( (10)== ::TopLevel::gvar )
 ::cease( ::TopLevel::tot);
 ::next_trigger( ::SC_ZERO_TIME);
}

void (cease)(void p1);

void (exit)(void p1_3);

void (next_trigger)(void p1_1);

void (wait)(void p1_4);

void (SC_METHOD)(void func);

void (SC_THREAD)(void func_1);

void (SC_CTHREAD)(void clock, void func_2);
