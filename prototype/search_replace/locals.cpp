SEARCH
{
    CODE
    {
        $STUFF;   
        auto $TYPE1 $OBJECT1;
        EXPAND_EXPRESSION
        {
            $STUFF;
            $OBJECT1; 
            $STUFF; 
        }    
    }
}

REPLACE
{
    CODE
    {
        $STUFF;
        static $TYPE1 $OBJECT1[10];
        static int sp_$OBJECT1=0;
        sp_$OBJECT1++;        
        EXPAND_EXPRESSION
        {
            $STUFF;
            $OBJECT1[sp_$OBJECT1];
            $STUFF; 
        }    
        sp_$OBJECT1--;
    }
}

