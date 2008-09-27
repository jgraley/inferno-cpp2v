
#include "../code/refcount.hpp"
#include "../code/tree_pointers.hpp"
#include <stdio.h>

class Victim : public RCTarget
{
public:
    Victim()
    {
        printf("I are constructed\n");
    }
    Victim(const Victim &a)
    {
        printf("I are copied\n");
    }
    ~Victim()
    {
        printf("I are destructed\n");
    }
    int member;
};

class Victim2 : public Victim
{
public:
    Victim2()
    {
        printf("I are constructed 2\n");
    }
    Victim2(const Victim &a)
    {
        printf("I are copied 2\n");
    }
    ~Victim2()
    {
        printf("I are destructed 2\n");
    }
    int member;
};

int main()
{
    {
        RCPtr<Victim> p;
        printf("A\n");
        {
            printf("B\n");
            Victim *v = new Victim();
            printf("C\n");
            p = v;
            printf("D\n");
        }
        p->member = 9;
        printf("E\n");
    }
    printf("F\n");
    {
        RCPtr<Victim> p;
        RCPtr<Victim> p2(p);
        printf("A\n");
        {
            printf("B\n");
            Victim2 *v = new Victim2();
            printf("C\n");
            p = v;
            printf("D\n");
            OwnerPtr<Victim> op(v, p);
            printf("D1\n");
            ShortcutPtr<Victim> sp(p);
            printf("D2\n");
            sp = p;
            printf("D3\n");
        }
        p->member = 9;
        printf("E\n");
        
    }

    return 0;
}
