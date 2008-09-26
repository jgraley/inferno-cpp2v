
#include "../code/refcount.hpp"
#include <stdio.h>

class Victim : RCTarget<Victim>
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
    return 0;
}
