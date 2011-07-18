/** Intermediate for anything that may be bound to.
    The instance must not be initialised. */
struct Bindable : Type
{
    NODE_FUNCTIONS
};

/** SystemC Channel
    Can only inherit from Interface 
    TODO do I mean channel or signal????
    */
struct Channel : Bindable
{
    NODE_FUNCTIONS
};

/** SystemC Channel
    Can only inherit from Interface 
    TODO do I mean channel or signal????
    */
struct PrimativeChannel : Channel, 
                          InheritanceRecord                     
{
    NODE_FUNCTIONS
};

/** SystemC interface
    All member functions to be pure virtual.
    No other kinds of members allowed */    
struct Interface : InheritanceRecord
{
    NODE_FUNCTIONS_FINAL
};

/** SystemC Port */
struct Port : TypedBindable
{
    NODE_FUNCTIONS    
    Type type;
};

/** SystemC input port */
struct InPort : Port
{
    NODE_FUNCTIONS_FINAL    
};

/** SystemC output port */
struct OutPort : Port
{
    NODE_FUNCTIONS_FINAL
};

/** SystemC in/out port */
struct InOutPort : Port
{
    NODE_FUNCTIONS_FINAL
};

/** SystemC clock input port */
struct ClockInPort : Port
{
    NODE_FUNCTIONS_FINAL    
};

/** SystemC signal */
struct Signal : Channel
{
    NODE_FUNCTIONS    
    Type type;
};

/** SystemC export
    Instance must not be initialised. 
    The type of the instance must be an Interface
    The channel to which the export is bound must be specified */        
struct Export : TypedBindable
{
    NODE_FUNCTIONS_FINAL    
    Type type;
};

/** Indicate a single binding 
    Bindings are placed in the declarations of whichever module
    has visibility of the things being bound. The binding has to
    be the right way around: the process end should be nearest
    to the process and the channel end should be nearest the channel
    even when going though ports and exports. 
    Both expressions are lvalues. For the child end, use Lookup (a.b)
    */    
struct Binding : virtual Declaration
{
	NODE_FUNCTIONS_FINAL
	TreePtr<Expression> process;
	TreePtr<Expression> channel;
};

/** SystemC Module
    No inheritance (all interaction through ports and exports)
    Processes, ports, sensitivities etc are all attributes of the members, 
    so nothing extra needed here */
struct Module : Record
{
    NODE_FUNCTIONS_FINAL
};

/** Intermediate for sensitivity lists
    Like Binding above, except process must really be 
    a process. Also the edge choices are derived from here */
struct Sensitivity : Declaration
{
    NODE_FUNCTIONS
    TreePtr<InstanceIdentifier> process;
	TreePtr<Expression> channel;
};

/** positive edge sensitivity */
struct PosEdge : Sensitivity
{
    NODE_FUNCTIONS_FINAL
};

/** negative edge sensitivity */
struct NegEdge : Sensitivity
{
    NODE_FUNCTIONS_FINAL
};

/** positive or negative edge sensitivity */
struct AnyEdge : Sensitivity
{
    NODE_FUNCTIONS_FINAL
};

/** SystemC process
    Does not have params or return value
    sensitivity list must all be ports */
struct Process : Subroutine
{
    NODE_FUNCTIONS
};

struct EventProcess : Process
{
    NODE_FUNCTIONS
    Collection<Sensitivity> sensitivity;
};

/** SystemC method */
struct Method : EventProcess
{
    NODE_FUNCTIONS_FINAL
};

/** SystemC thread */
struct Thread : EventProcess
{
    NODE_FUNCTIONS_FINAL
}

/** SystemC clocked thread 
    Sensitivity must be a ClockInPort */
struct ClockedThread : Process
{
    NODE_FUNCTIONS_FINAL
    TreePtr<Sensitivity> clock;
}

/** SystemC wait() 
    Note in clocked thread, this awaits next clock */
struct Wait : Statement
{
    NODE_FUNCTIONS_FINAL
}

/** SystemC wait_until() */
struct WaitUntil : Statement
{
    NODE_FUNCTIONS_FINAL
    TreePtr<Expression> condition;
}

