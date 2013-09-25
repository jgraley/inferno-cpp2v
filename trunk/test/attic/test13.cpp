
void bfunc();

void afunc()
{
    bfunc; // not a call, just a reference that doesn't go anywhere
}    
   
void bfunc()
{
    afunc;
}    

