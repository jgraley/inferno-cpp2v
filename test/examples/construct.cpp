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
	// Put this before the member init that references it, because
	// parser is on-pass on class bodies.
	ClassA field_a;	

	ClassB() : 
	    field_a("bar")
	{
	}

};
