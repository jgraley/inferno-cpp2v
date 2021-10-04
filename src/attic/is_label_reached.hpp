/*
 * JSG moved to attic because was out of test coverage (only used in state-out,
 * not in fall-out) and is substantially different to other agents.
 */


//---------------------------------- IsLabelReachedAgent ------------------------------------    

/// `IsLabelReachedAgent` matches a `LabelIdentifier` if that label is used
/// anywhere in the expression pointed to by `pattern`.
/// TODO generalise to more than just labels.
struct IsLabelReachedAgent : public virtual SearchLeafAgent, 
                             Special<CPPTree::LabelIdentifier>
{
	SPECIAL_NODE_FUNCTIONS	

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
	virtual void FlushCache() const;
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
    TreePtr<CPPTree::Expression> pattern;           
           
private:
    bool CanReachExpr( set< TreePtr<CPPTree::InstanceIdentifier> > *f,
                       TreePtr<CPPTree::LabelIdentifier> x, 
                       TreePtr<CPPTree::Expression> y ) const; // y is expression. Can it yield label x?
    
    bool CanReachVar( set< TreePtr<CPPTree::InstanceIdentifier> > *f,
                      TreePtr<CPPTree::LabelIdentifier> x, 
                      TreePtr<CPPTree::InstanceIdentifier> y ) const; // y is instance identifier. Can expression x be assigned to it?
    
    struct Reaching
    {
        Reaching( TreePtr<CPPTree::LabelIdentifier> f, TreePtr<CPPTree::InstanceIdentifier> t ) : from(f), to(t) {}
        const TreePtr<CPPTree::LabelIdentifier> from;
        const TreePtr<CPPTree::InstanceIdentifier> to;
        bool operator<( const Reaching &other ) const 
        {
            return from==other.from ? to<other.to : from<other.from;
        }
    };
    mutable map<Reaching, bool> cache; // it's a cache, so sue me
};

