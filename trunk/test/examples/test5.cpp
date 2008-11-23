
int main()
{
    int a=9;
    
    goto L1;
    
    a=0;
    
    goto L2;
    
    a=0;
    
    L1:
    
    a++;
    
    goto L3;
    
    L2:
    
    a=0;
    
    L3:
    L4:
    
    a++;
    
    return (unsigned char)a;
}

