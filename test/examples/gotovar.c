int main()
{
	void * dest;
	int x = 0;
	x += 1;
	
	switch(x)
	{
	case 0:
		dest = &&FOO;
		break;
	case 1:
		dest = &&BAR;
		break;
	case 2:
		dest = &&BAZ;
		break;
	}
	
    goto *dest;
    return 0;
    
FOO:
	return 45;
BAR:
	return 67;        
BAZ:
	return 89;        
} 

