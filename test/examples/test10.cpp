void f()
{
// Excite the multiple choices bug with sequences. Use with hacked 
// MergeInstanceDeclarations 
    if(0)
    {
        continue; // First sequence match finds this, it gets keyed
        break; // This is never found...
    }
    else
    {
        break; // ...even though it would match this
    }    
}



int main()
{
// Try and excite the multiple choice bug with collections - somewhat
// random, you may need to vary the commented-out assigns. Use with
// SplitInstanceDeclarations -> MergeInstanceDeclarations
    int a;
    a = 1;
    int b;
    //b = 2;
    int c;
    c = 3;
    int d;
    d = 4;
    int e;
    e = 5;
    int f;
    //f = 6;
    int g;
    g = 7;
    return 0;
}

