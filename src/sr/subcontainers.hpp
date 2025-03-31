#ifndef SUBCONTAINERS
#define SUBCONTAINERS

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "link.hpp"    

#include <list>
#include <set>

namespace SR
{
    
// Internal node classes - NOTE these are not agents, they are local tree nodes

struct SubContainer : Node // TODO #69
{
    NODE_FUNCTIONS
    
    // XLinks to these are often created using CreateDistinct()
    // (putting them in the domain would make the domain too big).
    // These functions operate on links held within the subcontainer,
    // which typically will be in the domain.
    virtual string GetContentsTrace() { return ""; } 
    virtual void AssertMatchingContents( TreePtr<Node> ) { ASSERTFAIL(); }
};


// This kind of subcontainer carries a range (begin/end pair) on the 
// parent X node's container, as well as a pointer to the parent. This
// is sufficient to enable the correct XLinks to be built when needed.
struct SubContainerRange : SubContainer,
                           virtual ContainerInterface // virtual required to allow subclasses to use my impl for interfaces they bring in                        
{    
    NODE_FUNCTIONS
    
    SubContainerRange() {}
    TreePtr<Node> parent_x;
    iterator my_begin;
    iterator my_end;
public:
    SubContainerRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e );
    const iterator &begin() override;
    const iterator &end() override;
    const iterator &insert( const TreePtrInterface & ) final       { ASSERTFAIL("Cannot modify SubContainerRange"); }
    const iterator &insert( const ContainerInterface::iterator_interface &, const TreePtrInterface & ) final       { ASSERTFAIL("Cannot modify SubContainerRange"); }
    const iterator &insert_front( const TreePtrInterface & ) final { ASSERTFAIL("Cannot modify SubContainerRange"); }
    const iterator &erase( const iterator_interface & ) final      { ASSERTFAIL("Cannot modify SubContainerRange"); }
    void clear() final                                  { ASSERTFAIL("Cannot modify SubContainerRange"); }    
    //void push_back( const TreePtrInterface &gx ) final  { ASSERTFAIL("Cannot modify SubContainerRange"); }  
    
    virtual string GetContentsTrace(); 
    virtual void AssertMatchingContents( TreePtr<Node> other );    
    
    TreePtr<Node> GetParentX()
    {
        return parent_x;
    }    
};


struct SubContainerRangeExclusions : SubContainerRange
{
    typedef set<const TreePtrInterface *> ExclusionSet;

    class exclusion_iterator : public ContainerInterface::iterator
    {
    public:    
        typedef forward_iterator_tag iterator_category;
        typedef TreePtrInterface value_type;
        typedef int difference_type;
        typedef const value_type *pointer;
        typedef const value_type &reference;

        exclusion_iterator();
        exclusion_iterator( const exclusion_iterator &i );
        exclusion_iterator( const iterator_interface &ib,
                            weak_ptr<const SubContainerRangeExclusions> container_ );
        exclusion_iterator &operator=( const iterator_interface &ib );
        exclusion_iterator &operator++();
        exclusion_iterator &operator--();
        const value_type &operator*() const;
        const value_type *operator->() const;
        bool operator==( const iterator_interface &ib ) const;
        bool operator==( const exclusion_iterator &i ) const;
        bool operator!=( const iterator_interface &ib ) const;
        bool operator!=( const exclusion_iterator &i ) const;
        void Mutate( const value_type *v ) const;
        bool IsOrdered() const;
        iterator_interface *GetUnderlyingIterator() const;
        virtual unique_ptr<iterator_interface> Clone() const;
        operator string();

    private:
        void NormaliseForward();
        void NormaliseReverse();
        shared_ptr<iterator_interface> pib;
        weak_ptr<const SubContainerRangeExclusions> container;
    };        
    
    SubContainerRangeExclusions() {}
    SubContainerRangeExclusions( TreePtr<Node> parent_x, 
                                 const iterator &b, 
                                 const iterator &e );
    void SetExclusions( const ExclusionSet &exclusions_ );
    
    const iterator &begin() final;
    const iterator &end() final;
    string GetContentsTrace();

private: 
    friend class exclusion_iterator;
    virtual bool IsExcluded( const iterator_interface &ib ) const;  

    shared_ptr<const ExclusionSet> exclusions;
    iterator my_exclusive_begin;
    iterator my_exclusive_end;
    
    NODE_FUNCTIONS 
};



struct SubSequenceRange : SubContainerRange,
                          SequenceInterface
{
    SubSequenceRange() {}
    SubSequenceRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e ) :
        SubContainerRange( parent_x, b, e )
    {
    }
    
    NODE_FUNCTIONS_FINAL 
};


struct SubCollectionRange : SubContainerRangeExclusions,
                            CollectionInterface
{
    SubCollectionRange() {}
    SubCollectionRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e ) :
        SubContainerRangeExclusions( parent_x, b, e )
    {
    }
    
    NODE_FUNCTIONS_FINAL 
};


// This kind of subcontainer stores all the XLinks for retrieval later
// TODO no need for Sequence<Node> any more I believe
struct SubContainerLinks : SubContainer
{    
    NODE_FUNCTIONS
};


struct SubSequence : Sequence<Node>,
                     SubContainerLinks
{
    NODE_FUNCTIONS_FINAL 
    SubSequence &operator=( Sequence<Node> o )
    {
        (void)Sequence<Node>::operator=(o);
        return *this;
    }
    virtual string GetContentsTrace(); 
    virtual void AssertMatchingContents( TreePtr<Node> other );    

    list<XLink> elts;
};


struct SubCollection : Collection<Node>, 
                       SubContainerLinks
{
    NODE_FUNCTIONS_FINAL
    SubCollection &operator=( Collection<Node> o )
    {
        (void)Collection<Node>::operator=(o);
        return *this;
    }
    virtual string GetContentsTrace(); 
    virtual void AssertMatchingContents( TreePtr<Node> other );    
};       

};
#endif
