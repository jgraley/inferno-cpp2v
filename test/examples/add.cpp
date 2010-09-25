
int main()
{
	int tot=0;
	{
		int tot;
		tot=10; // let's hope inferno doesn't confuse this with the other top when it tidies away this compound block!!!1lol
	}
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
