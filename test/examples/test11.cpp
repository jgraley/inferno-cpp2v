
// basic trigger for crazynine
struct S { static int b; };
int S::b = 9;

// try to trigger multiple choice bug for stuff nodes when used with HackUpIfs
int main()
{
    if(0)
    {   
        {
            8;
            9;
        }
    }
    else
    {
        9;
    }
}

