// Test the ExplicitiseReturn step, with different categories of function
class C
{
    ~C() // Subroutine, need return
    {
    }

    C() // Procedure, need return
    {
    }

    C(int i) // Procedure already has return, no action
    {
        return;
    }
                
    void f() // void Function, need return
    {
    }

    void g() // void Function already has return, no action
    {
        return;
    }
    
    int h() // int Function already has return, no action
    {
        return 0;
    }

    int i() // int Function with hard-to-find return, no action
    {
        if( 0 )
            return 0;
        else
            return 0;
    }
};

