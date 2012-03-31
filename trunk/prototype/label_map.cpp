


int main()
{
    enum { SA, SB } States;
    void *(labels_map[]) = { &&LA, &&LB };
    LA:;
    goto LB;
    return 1;
    LB:;
    return 2;
}
