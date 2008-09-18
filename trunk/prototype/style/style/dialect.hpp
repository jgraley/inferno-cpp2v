
#ifndef DIALECT_HPP
#define DIALECT_HPP

#include <vector>
#include <string>
#include "mybitset.hpp"

// Constructions within the universal language that may or may not appear in the program.
enum Construct
{
	LOCAL_VARIABLE, 
	NESTED_IF, // c-style IF with bracketed blocks
	COND_BRANCH, // conditional branch (ie terminator)
	CPP_CLASS, // C++-style class (ie not a module)
	
	EXPRESSION_UNOPT_GCSE, // Expressions exist upon which global subexpression elimination has not been attempted
    EXPRESSION_UNOPT_LOOP, // no loop induction optimisation
    
    NUM_CONSTRUCTS // not a construct!
};

// A set of constructs for general purposes
typedef std::bitset<NUM_CONSTRUCTS> ConstructSet; 

ConstructSet MakeSet( Construct a[], int l );
#define MAKE_SET(A) MakeSet( (A), sizeof(A)/sizeof((A)[0]) )

// A set of constructs that is available for a program to use at a given stage in compilation.
// Constructs in the set may be used (but are not required to be used); constructs absent from 
// the set may not be used.
class Dialect : public ConstructSet
{
public:
    // Holds the dialect behaviour of a pass. Passes have limitations on the dialects they support
    // as inputs. Passes also update the dialect by adding and/or removing constructs.
    class Transformation
    {
    public:
        // The default values represent an analysis pass or an optimisation that doesn't
        // change constructs. The allwoed set must always be supplied.
        Transformation( const ConstructSet &a, 
                        const ConstructSet &i, 
                        const ConstructSet &e,
                        const std::string n=std::string("<unnamed>") );
        const ConstructSet &GetAllowed() const;
        const ConstructSet &GetIntroduces() const;
        const ConstructSet &GetEliminates() const;
       
    private: friend Dialect;    
        // The set of constructs we can accept as input. 
        ConstructSet allowed;     
        
        // The constructs we add to a program and take away. 
        ConstructSet introduces;
        ConstructSet eliminates;
        
        // A moniker for the transformation
        std::string name; 
    };

    Dialect()
    {
    }
    
    Dialect( ConstructSet &c )
       : ConstructSet( c )
    {
    }
    
    static Dialect Universal(); // All constructs allowed
    static Dialect LegacyLLVM(); // The original LLVC IR allowed these constructs
    static Dialect ANSIC(); // As per ANSI C
    static Dialect CPP(); // As per C++
    bool CanApply( const Transformation &trans );
    void Apply( const Transformation &trans );    
};

extern void Demo();

#endif // include guard
