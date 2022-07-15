#include "subcontainers.hpp"

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "node/node.hpp"    
    
using namespace SR;    
    
//////////////////////////// SubContainerRange ///////////////////////////////

SubContainerRange::SubContainerRange( TreePtr<Node> parent_x_, const iterator &b, const iterator &e ) : 
    parent_x( parent_x_ ),
    my_begin(b), 
    my_end(e)
{               
}


const ContainerInterface::iterator &SubContainerRange::begin() 
{ 
    return my_begin; 
}


const ContainerInterface::iterator &SubContainerRange::end()
{
    return my_end; 
}


string SubContainerRange::GetContentsTrace()
{
    ContainerInterface *this_ci = dynamic_cast<ContainerInterface *>(this);
    
    bool first = true;
    string s = "SubContainerRange(";
    FOREACH( const TreePtrInterface &e_node, *this_ci )
    {
        if( !first )
            s += ", ";
        first = false;
        XLink e_link = XLink(GetParentX(), &e_node);
        s += Trace(e_link);
    }
    s += ")";
    return s;
}


void SubContainerRange::AssertMatchingContents( TreePtr<Node> other )
{
    auto other_ssr = dynamic_cast<SubContainerRange *>(other.get());
    ASSERT( other_ssr );
    ASSERT( begin() == other_ssr->begin() );
    ASSERT( end() == other_ssr->end() );
}

//////////////////////////// SubContainerRangeExclusions ///////////////////////////////


SubContainerRangeExclusions::exclusion_iterator::exclusion_iterator() :
    pib( shared_ptr<iterator_interface>() ) 
{
}


SubContainerRangeExclusions::exclusion_iterator::exclusion_iterator( const exclusion_iterator &i ) :
    container(i.container)
{
    pib = i.pib->Clone(); // Note we are not clone-on-write, so clone here for in case we write later
}


SubContainerRangeExclusions::exclusion_iterator::exclusion_iterator( const iterator_interface &ib,
                                                                     weak_ptr<const SubContainerRangeExclusions> container_ ) :
    container(container_)
{            
    pib = ib.Clone(); // Note we are not clone-on-write, so clone here for in case we write later
    NormaliseForward();
}


SubContainerRangeExclusions::exclusion_iterator &SubContainerRangeExclusions::exclusion_iterator::operator=( const iterator_interface &ib )
{
    pib = ib.Clone(); // Note we are not clone-on-write, so clone here for in case we write later
    auto i = dynamic_cast<const exclusion_iterator &>(ib);
    container = i.container;
    return *this;
}


SubContainerRangeExclusions::exclusion_iterator &SubContainerRangeExclusions::exclusion_iterator::operator++()
{
    ASSERT(pib)("Attempt to increment uninitialised iterator");
    pib->operator++();
    NormaliseForward();
    return *this;
}


SubContainerRangeExclusions::exclusion_iterator &SubContainerRangeExclusions::exclusion_iterator::operator--()
{
    ASSERT(pib)("Attempt to increment uninitialised iterator");
    pib->operator--();
    NormaliseReverse();
    return *this;
}


const SubContainerRangeExclusions::exclusion_iterator::value_type &SubContainerRangeExclusions::exclusion_iterator::operator*() const 
{
    ASSERT(pib)("Attempt to dereference uninitialised iterator");
    return pib->operator*();
}


const SubContainerRangeExclusions::exclusion_iterator::value_type *SubContainerRangeExclusions::exclusion_iterator::operator->() const
{
    ASSERT(pib)("Attempt to dereference uninitialised iterator");
    return pib->operator->();
}


bool SubContainerRangeExclusions::exclusion_iterator::operator==( const iterator_interface &ib ) const // isovariant param
{
    ASSERT( typeid(*this)==typeid(ib) );
    return operator==(dynamic_cast<const exclusion_iterator &>(ib));
}


bool SubContainerRangeExclusions::exclusion_iterator::operator==( const exclusion_iterator &i ) const // covariant param
{
    ASSERT(pib && i.pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i.pib?"i":"U");
    return pib->operator==( *(i.pib) );
}


bool SubContainerRangeExclusions::exclusion_iterator::operator!=( const iterator_interface &ib ) const // isovariant param
{
    return !operator==( ib );
}


bool SubContainerRangeExclusions::exclusion_iterator::operator!=( const exclusion_iterator &i ) const // covariant param
{
    ASSERT(pib && i.pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i.pib?"i":"U");
    return !operator==( i );
}


void SubContainerRangeExclusions::exclusion_iterator::Overwrite( const value_type *v ) const
{
    ASSERT(pib)("Attempt to Overwrite through uninitialised iterator");
    pib->Overwrite( v );
}

        
const bool SubContainerRangeExclusions::exclusion_iterator::IsOrdered() const
{
    return pib->IsOrdered();
}


ContainerInterface::iterator_interface *SubContainerRangeExclusions::exclusion_iterator::GetUnderlyingIterator() const
{
    if( pib )
        return pib.get();
    else
        return nullptr;
}


shared_ptr<ContainerInterface::iterator_interface> SubContainerRangeExclusions::exclusion_iterator::Clone() const 
{
    return make_shared<exclusion_iterator>(*pib, container);
}


SubContainerRangeExclusions::exclusion_iterator::operator string()
{   
    if( pib )
        return Traceable::TypeIdName( *pib );
    else 
        return string("UNINITIALISED");
}


void SubContainerRangeExclusions::exclusion_iterator::NormaliseForward()
{
    shared_ptr<const SubContainerRangeExclusions> lc = container.lock();
    ASSERT( lc )("Container went away\n");
    while( lc->IsExcluded(*pib) )
        pib->operator++();
}


void SubContainerRangeExclusions::exclusion_iterator::NormaliseReverse()
{
    shared_ptr<const SubContainerRangeExclusions> lc = container.lock();
    ASSERT( lc )("Container went away\n");
    while( lc->IsExcluded(*pib) )
        pib->operator--();
}


SubContainerRangeExclusions::SubContainerRangeExclusions( TreePtr<Node> parent_x, 
                                                          const iterator &b, 
                                                          const iterator &e ) :
    SubContainerRange( parent_x, b, e )
{
}


void SubContainerRangeExclusions::SetExclusions( const ExclusionSet &exclusions_ )
{    
    exclusions = make_shared<const ExclusionSet>( exclusions_ );
    
    weak_ptr<const SubContainerRangeExclusions> sp_this = 
        dynamic_pointer_cast<SubContainerRangeExclusions>(shared_from_this());
    my_exclusive_begin = exclusion_iterator( my_begin, sp_this );
    my_exclusive_end = exclusion_iterator( my_end, sp_this );
}


const ContainerInterface::iterator &SubContainerRangeExclusions::begin() 
{ 
    return my_exclusive_begin; 
}


const ContainerInterface::iterator &SubContainerRangeExclusions::end()
{
    return my_exclusive_end; 
}


bool SubContainerRangeExclusions::IsExcluded( const iterator_interface &ib ) const
{
    if(ib == my_end) 
    {
        return false; // can't dereference end iterator - but can't exclude it either
    }
    else
    {
        bool exc = exclusions->count( &*ib );
        return exc > 0;
    }
}


string SubContainerRangeExclusions::GetContentsTrace()
{
    ContainerInterface *this_ci = dynamic_cast<ContainerInterface *>(this);
    
    string s = "SubContainerRangeExclusions";

    s += "(";
    bool first = true;
    FOREACH( const TreePtrInterface &e_node, *this_ci )
    {
        if( !first )
            s += ", ";
        first = false;
        XLink e_link = XLink(GetParentX(), &e_node);
        s += Trace(e_link);
    }
    s += ")";
    return s;
}

//////////////////////////// SubSequence ///////////////////////////////

string SubSequence::GetContentsTrace()
{
    bool first = true;
    string s = "SubSequence[";
    for( XLink e_link : elts )
    {
        if( !first )
            s += ", ";
        first = false;
        s += Trace(e_link);
    }
    s += "]";
    return s;
}


void SubSequence::AssertMatchingContents( TreePtr<Node> other )
{
    auto other_ssl = dynamic_cast<SubSequence *>(other.get());
    ASSERT( other_ssl );
    ASSERT( elts == other_ssl->elts )
          (elts)(" != ")(other_ssl->elts);    
}


//////////////////////////// SubCollection ///////////////////////////////

string SubCollection::GetContentsTrace()
{
    string s = "SubCollection{DEPRACATED}";
    return s;
}


void SubCollection::AssertMatchingContents( TreePtr<Node> other )
{
    ASSERT(!"DEPRACATED");
}

