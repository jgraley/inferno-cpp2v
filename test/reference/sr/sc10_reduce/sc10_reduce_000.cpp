class sc_event;
class sc_interface;
class sc_module;
class TopLevel;
class sc_event
{
void (notify)(auto void p1_1);
};
class sc_interface
{
};
class sc_module
{
};
class TopLevel : public sc_module
{
public:
TopLevel(auto char (*name));
int x;
void (T)();
int (f)(auto int i);
};
TopLevel top_level;
void SC_ZERO_TIME;
void (cease)(void p1);
void (exit)(void p1_2);
void (next_trigger)(void p1_3);
void (wait)(void p1_4);
void (SC_METHOD)(void func_1);
void (SC_THREAD)(void func);
void (SC_CTHREAD)(void clock, void func_2);

void (sc_event::notify)(void p1_1);

TopLevel::TopLevel(char (*name))
{
 ::SC_THREAD( ::TopLevel::T);
}

void (TopLevel::T)()
{
 ::cease( ::TopLevel::f(1));
}

int (TopLevel::f)(int i)
{
return i;
}

void (cease)(void p1);

void (exit)(void p1_2);

void (next_trigger)(void p1_3);

void (wait)(void p1_4);

void (SC_METHOD)(void func_1);

void (SC_THREAD)(void func);

void (SC_CTHREAD)(void clock, void func_2);
