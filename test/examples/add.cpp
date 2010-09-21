
int main()
{
	static int sss=999;
    int tot=0;
    int i;
    for( i=0; i<5; i++ )
    {
    	do
    	{
    		continue; // applies to inner loop
    	} while(0);
		tot = tot + i;
        if(1)
    	{
            continue; // applies to outer loop
        }
        tot *= 2;
    }
    return tot;
}
