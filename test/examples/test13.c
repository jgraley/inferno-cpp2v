
void bfunc();

// References are circular but no actual calls made

void afunc()
{
    if( false )
		bfunc(); 
}    
   
void bfunc()
{
    if( false )
		afunc(); 
}    

