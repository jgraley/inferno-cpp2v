
class Test 
{
public:
    virtual char f()
    {
        return 12;
    }
};


int main()
{
    Test t;
    return t.f();
}

