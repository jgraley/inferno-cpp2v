#include "subcontainers.hpp"

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "node/node.hpp"    
    
using namespace SR;    
    
//////////////////////////// SubSequenceRange ///////////////////////////////

SubSequenceRange::SubSequenceRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e ) : 
    SubContainerRange( parent_x ),
    my_begin(b.Clone()), 
    my_end(e.Clone())
{           
    if( !(*my_begin == *my_end) )
        ASSERT_NOT_ON_STACK( &*(*my_begin) )( *this ); 
}


const SubSequenceRange::iterator_interface &SubSequenceRange::begin() 
{ 
    return *my_begin; 
}


const SubSequenceRange::iterator_interface &SubSequenceRange::end()
{
    return *my_end; 
}


string SubSequenceRange::GetContentsTrace()
{
    ContainerInterface *this_ci = dynamic_cast<ContainerInterface *>(this);
    
    bool first = true;
    string s = "SubContainerRange[";
    FOREACH( const TreePtrInterface &e_node, *this_ci )
    {
        if( !first )
            s += ", ";
        first = false;
        XLink e_link = XLink(GetParentX(), &e_node);
        s += Trace(e_link);
    }
    s += "]";
    return s;
}


void SubSequenceRange::AssertMatchingContents( TreePtr<Node> other )
{
    auto other_ssr = dynamic_cast<SubSequenceRange *>(other.get());
    ASSERT( other_ssr );
    ASSERT( begin() == other_ssr->begin() );
    ASSERT( end() == other_ssr->end() );
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
    bool first = true;
    string s = "SubCollection{";
    for( XLink e_link : elts )
    {
        if( !first )
            s += ", ";
        first = false;
        s += Trace(e_link);
    }
    s += "}";
    return s;
}


void SubCollection::AssertMatchingContents( TreePtr<Node> other )
{
    auto other_scl = dynamic_cast<SubCollection *>(other.get());
    ASSERT( other_scl );
    ASSERT( elts == other_scl->elts )
          (elts)(" != ")(other_scl->elts);
}

