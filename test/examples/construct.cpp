class ClassA 
{
public:
    ClassA( char *str )
    {
	}
};

ClassA instance_a("foo");

class ClassB
{
	ClassB() : 
	    field_a("bar")
	{
	}

	ClassA field_a;	
};
