

int main()
{
    void *p;
    bool foo;
    goto *(foo ? p : &&LABEL);
    LABEL:;
}
