
#include "flatten.hpp"
#include "transformation.hpp"

////////////////////////// FlattenNode //////////////////////////

bool FlattenNode_iterator::IsAtEnd() const
{
    return empty || mit == m_end;
}


void FlattenNode_iterator::NormaliseNewMember()
{
	if( !IsAtEnd() )
	{
		Itemiser::Element *ie = GetCurrentMember();
		if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(ie) )
		{
			cit = con->begin();
			c_end = con->end();
			BypassEndOfContainer();
		}
	}
}


void FlattenNode_iterator::BypassEndOfContainer()
{
	ASSERT( !IsAtEnd() && dynamic_cast<ContainerInterface *>(GetCurrentMember()) ); // this fn requires us to be on a container
	if( cit == c_end )
	{
		++mit;
		NormaliseNewMember();
	}
}


FlattenNode_iterator::FlattenNode_iterator( TreePtr<Node> r ) :
	root( r.get() ),
	ref_holder( r ),
	empty( false )
{
    //members = shared_ptr< vector< int > >( new vector< int >( root->BasicItemise() ) );

    mit = 0;
    m_end = root->ItemiseSize();
	//TRACE("Flattening node ")(*root)(" size %d\n", m_end);
    NormaliseNewMember();
}


FlattenNode_iterator::FlattenNode_iterator( const Node *r ) :
	root( r ),
	empty( false )
{
    //members = shared_ptr< vector< int > >( new vector< int >( root->BasicItemise() ) );

    mit = 0;
    m_end = root->ItemiseSize();
	//TRACE("Flattening node ")(*root)(" size %d\n", m_end);
    NormaliseNewMember();
}


FlattenNode_iterator::FlattenNode_iterator() :
	empty( true )
{
}


FlattenNode_iterator::FlattenNode_iterator( const FlattenNode_iterator & other ) :
    mit( other.mit ),
    m_end( other.m_end ),
    cit( other.cit ),
    c_end( other.c_end ),
    root( other.root ),
	empty( other.empty )
{
}


string FlattenNode_iterator::GetName() const
{
    string s = root->GetTrace();
    if (IsAtEnd())
    	s += string("end()");
    else if( dynamic_cast<CollectionInterface *>(GetCurrentMember()) )
        s += string("{}.") + cit->GetTrace() + SSPrintf("@%p", cit->get());
	else if( dynamic_cast<SequenceInterface *>(GetCurrentMember()) )
        s += string("[].") + cit->GetTrace() + SSPrintf("@%p", cit->get());
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
       	s += string(".") + ptr->GetTrace() + SSPrintf("@%p", ptr->get());
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
        
    return s;
}


shared_ptr<ContainerInterface::iterator_interface> FlattenNode_iterator::Clone() const
{
	shared_ptr<FlattenNode_iterator> ni( new FlattenNode_iterator(*this) );
	return ni;
}


FlattenNode_iterator &FlattenNode_iterator::operator++()
{
	ASSERT( !IsAtEnd() );
    if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
    {
    	++cit;
    	BypassEndOfContainer();
    }
    else if( dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
    {
        ++mit;
        NormaliseNewMember();
    }
    else
    {
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
    }
	return *this;
}


FlattenNode_iterator::reference FlattenNode_iterator::operator*() const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        return *cit;
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        return *ptr;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}


FlattenNode_iterator::pointer FlattenNode_iterator::operator->() const
{
	return &operator*();
}


bool FlattenNode_iterator::operator==( const ContainerInterface::iterator_interface &ciii_o ) const
{ 
	const FlattenNode_iterator &o = *dynamic_cast<const FlattenNode_iterator *>(&ciii_o);
	ASSERT(&o)("Comparing flattern iterator with something else ")(ciii_o);

	if( IsAtEnd() || o.IsAtEnd() )
		return IsAtEnd() && o.IsAtEnd();
    if( root != o.root )
        return false;
	if( mit != o.mit )
        return false;
    ASSERT( GetCurrentMember() == o.GetCurrentMember() ); // because the mits match
    if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        if( cit != o.cit )
            return false;
    return true;
}


/*bool FlattenNode_iterator::operator<( const FlattenNode_iterator &o ) const
{
	if( IsAtEnd() || o.IsAtEnd() )
		return !IsAtEnd() && o.IsAtEnd();
    if( root != o.root )
        return root < o.root;
	if( mit != o.mit )
        return mit < o.mit;
    ASSERT( GetCurrentMember() == o.GetCurrentMember() ); // because the mits match
    if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        return cit < o.cit;            
    return false;
}*/


void FlattenNode_iterator::Overwrite( FlattenNode_iterator::pointer v ) const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        cit.Overwrite( v );
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        *ptr = *v;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}


const bool FlattenNode_iterator::IsOrdered() const
{
	return true; // traverse walks tree in order generally
}
