#include "isystemc.h"

class TopLevel;
class TopLevel:public sc_module
{
  public:
    SC_CTOR(TopLevel)
    {
        SC_THREAD(U);
        SC_THREAD(T);
    }
    void U();
  private:
    void (*HelperU_return_address);
  public:
    void (HelperT) ();
    void (HelperU) ();
    void T();
  private:
    void (*HelperT_return_address);
};

void TopLevel::U()
{
    auto void (*state_u);
    state_u = (&&BOOTSTRAP_U);
    do
    {
        goto *(state_u);
      BOOTSTRAP_U:;
        ::TopLevel::HelperU_return_address = (&&RETURN_U);
        ::TopLevel::HelperU();
        state_u = (&&RETURN_U);
        goto *(state_u);
      RETURN_U:;
        return;
    }
    while(true);
}

void (TopLevel::HelperT) ()
{
    auto void (*state_ht);
    auto void (*return_address_ht);
    state_ht = (&&BOOTSTRAP_HT);
    do
    {
        goto *(state_ht);
      BOOTSTRAP_HT:;
        return_address_ht =::TopLevel::HelperT_return_address;
        ::TopLevel::HelperT_return_address = return_address_ht;
        return;
    }
    while(true);
}

void (TopLevel::HelperU) ()
{
    auto void (*state_hu);
    auto void (*return_address_hu);
    state_hu = (&&BOOTSTRAP_HU);
    do
    {
        goto *(state_hu);
      BOOTSTRAP_HU:;
        return_address_hu =::TopLevel::HelperU_return_address;
        ::TopLevel::HelperU_return_address = return_address_hu;
        return;
    }
    while(true);
}

void TopLevel::T()
{
    auto void (*state_t);
    state_t = (&&BOOTSTRAP_T);
    do
    {
        goto *(state_t);
      BOOTSTRAP_T:;
        ::TopLevel::HelperU_return_address = (&&RETURN_T);
        ::TopLevel::HelperT();
        state_t = (&&RETURN_T);
        goto *(state_t);
      RETURN_T:;
        return;
    }
    while(true);
}

