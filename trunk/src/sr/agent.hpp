#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"
#include <set>

class Conjecture;
class SpecialBase;
class StuffBase;
class StarBase;
class SlaveBase;
class SearchContainerBase;
class SearchReplace;

class Agent
{
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
								  TreePtr<Node> pattern,
								  bool can_key,
							      Conjecture &conj ) const = 0;
    virtual TreePtr<Node> BuildReplace( TreePtr<Node> pattern, 
			 					         TreePtr<Node> keynode=TreePtr<Node>() ) const = 0;
};



class LegacyAgent : public Traceable
{
public:
	LegacyAgent( const CompareReplace &s, CouplingKeys &c );
private:
	const CompareReplace &sr;
	CouplingKeys &coupling_keys;
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
    		                     TreePtr<Node> pattern,
    		                     bool can_key,
    		                     Conjecture &conj ) const;
private:
    bool DecidedCompare( SequenceInterface &x,
    		             SequenceInterface &pattern,
    		             bool can_key,
    		             Conjecture &conj ) const;
    bool DecidedCompare( CollectionInterface &x,
    		             CollectionInterface &pattern,
    		             bool can_key,
    		             Conjecture &conj ) const;
    bool DecidedCompare( const TreePtrInterface &x,
    		             shared_ptr<SearchContainerBase> pattern,
    		             bool can_key,
    		             Conjecture &conj ) const;
public:	
    virtual TreePtr<Node> BuildReplace( TreePtr<Node> pattern, 
								         TreePtr<Node> keynode=TreePtr<Node>() ) const;
private:	
	TreePtr<Node> BuildReplaceOverlay( TreePtr<Node> pattern, 
									    TreePtr<Node> keynode ) const; // under substitution if not NULL
    TreePtr<Node> BuildReplaceSlave( shared_ptr<SlaveBase> pattern, 
									  TreePtr<Node> keynode ) const;    
    TreePtr<Node> BuildReplaceNormal( TreePtr<Node> pattern ) const;
    TreePtr<Node> BuildReplaceKeyed( TreePtr<Node> pattern, 
									  TreePtr<Node> keynode ) const;
    TreePtr<Node> BuildReplaceStar( shared_ptr<StarBase> pattern, 
									 TreePtr<Node> keynode ) const;
									 
private:
    Sequence<Node> WalkContainerPattern( ContainerInterface &pattern,
                                         bool replacing ) const;
	// Internal node classes - NOTE these are not special nodes, and we use them like normal tree nodes
	// Note: only needed for Star pattern
    struct SubContainer : Node
    {
        NODE_FUNCTIONS
    };
    struct SubSequenceRange : SequenceInterface,
                              SubContainer
    {
    	NODE_FUNCTIONS_FINAL 

        SubSequenceRange() {}
    	shared_ptr<iterator_interface> my_begin;
    	shared_ptr<iterator_interface> my_end;
    	operator string() const { return GetName() + SSPrintf("@%p", this); }    
    public:
    	SubSequenceRange( iterator &b, iterator &e ) : my_begin(b.Clone()), my_end(e.Clone())
    	{    	    
    	}
	    virtual const iterator_interface &begin() { return *my_begin; }
        virtual const iterator_interface &end()   { return *my_end; }
        virtual void erase( iterator )  { ASSERTFAIL("Cannot modify SubSequenceRange"); }
        virtual void clear()                                { ASSERTFAIL("Cannot modify SubSequenceRange"); }    
        virtual void insert( const TreePtrInterface & )     { ASSERTFAIL("Cannot modify SubSequenceRange"); }
        virtual TreePtrInterface &operator[]( int i ) { ASSERTFAIL("TODO"); }  
        virtual void push_back( const TreePtrInterface &gx ){ ASSERTFAIL("Cannot modify SubSequenceRange"); }  
    };
    struct SubSequence : Sequence<Node>,
                         SubContainer
    {
    	NODE_FUNCTIONS_FINAL 
    	operator string() const { return GetName() + SSPrintf("@%p", this); }    
    };
    struct SubCollection : Collection<Node>,
                           SubContainer
    {
    	NODE_FUNCTIONS_FINAL
    	operator string() const { return GetName() + SSPrintf("@%p", this); } 
    };

};

#endif
