#include "result.hpp"

#include "common/orderable.hpp"
#include "db/x_tree_database.hpp"

using namespace SYM;

// ------------------------- BooleanResult --------------------------

BooleanResult::BooleanResult( bool value_ ) :
    value( value_ )
{
}


bool BooleanResult::IsDefinedAndUnique() const
{
    return true;
}


bool BooleanResult::IsDefinedAndTrue() const
{
    return value;
}


bool BooleanResult::IsDefinedAndFalse() const
{
    return !value;
}

   
bool BooleanResult::GetAsBool() const
{
    return value;
}


bool BooleanResult::operator==( const BooleanResult &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return value == o->value;
}


bool BooleanResult::operator<( const BooleanResult &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return (int)value < (int)(o->value);
}


string BooleanResult::Render() const
{
    return Trace(value);
}


string BooleanResult::GetTrace() const
{
    return Render();
}

// ------------------------- SymbolicResult --------------------------

SymbolicResult::~SymbolicResult()
{
}


string SymbolicResult::GetTrace() const
{
    return SSPrintf("@%p ", this) + Render();
}

// ------------------------- UniqueResult --------------------------

UniqueResult::UniqueResult( XValue xlink_ )    
{
	INDENT("U");
    ASSERT( xlink_ )("Not allowed to construct with NULL; use EmptyResult instead");
	//FTRACE("Result at %p sets xlink to: ", this)(xlink_)("\n"); // crashes when xlink copy constructor nulls the sp_tp_
	xlink = xlink_;
	//FTRACE("Done\n"); 
}


UniqueResult::~UniqueResult()
{
	//FTRACE("Destructo at %p\n", this); 	
}


bool UniqueResult::IsDefinedAndUnique() const
{
    return true;
}


XValue UniqueResult::GetOnlyXLink() const
{
    ASSERT( xlink );
    return xlink;
}


bool UniqueResult::TryExtensionalise( set<XValue> &links ) const
{
    links = set<XValue>{ xlink };
    return true;
}


bool UniqueResult::operator==( const SymbolicResult &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return o && xlink == o->xlink;
}


string UniqueResult::Render() const
{
    return Trace(xlink);
}

// ------------------------- EmptyResult --------------------------

bool EmptyResult::IsDefinedAndUnique() const
{
    return false;
}


XValue EmptyResult::GetOnlyXLink() const
{
    ASSERTFAIL();
}


bool EmptyResult::TryExtensionalise( set<XValue> &links ) const
{
    links = set<XValue>{};
    return true;
}


bool EmptyResult::operator==( const SymbolicResult &other ) const
{
    return !!GET_THAT_POINTER(&other);
}


string EmptyResult::Render() const
{
    return "{}";
}

// ------------------------- SubsetResult --------------------------

SubsetResult::SubsetResult( set<XValue> xlinks_, bool complement_flag_ ) :
    xlinks( xlinks_ ),
    complement_flag( complement_flag_ )
{
}


SubsetResult::SubsetResult( unique_ptr<SymbolicResult> other )
{
    if( auto ssr = dynamic_pointer_cast<SubsetResult>(move(other)) )
    {
        xlinks = ssr->xlinks;
        complement_flag = ssr->complement_flag;
    }
    else // SymbolicResult
    {
        set<XValue> links;
        bool ok = other->TryExtensionalise( xlinks );
        ASSERTS(ok);
        complement_flag = false;
    }
}


bool SubsetResult::IsDefinedAndUnique() const
{
    return !complement_flag && xlinks.size() == 1;
}


XValue SubsetResult::GetOnlyXLink() const
{
    ASSERT( !complement_flag )("Is complement so not unique");
    return SoloElementOf(xlinks);
}


bool SubsetResult::TryExtensionalise( set<XValue> &links ) const
{
    if( complement_flag ) // Refusing to extensionalise a complement set
        return false;
    links = xlinks;
    return true;
}


bool SubsetResult::operator==( const SymbolicResult &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return o && xlinks == o->xlinks && complement_flag==o->complement_flag;
}


unique_ptr<SubsetResult> SubsetResult::GetComplement() const
{
    return make_unique<SubsetResult>(xlinks, !complement_flag);
}


unique_ptr<SubsetResult> SubsetResult::GetUnion( list<unique_ptr<SubsetResult>> ops )
{
    int n_comp = 0;
    for( const unique_ptr<SubsetResult> &op : ops )
        n_comp += op->complement_flag ? 1 : 0;

    if( n_comp > 0 )
        return DeMorgan( IntersectionCore, move(ops) );
    else
        return UnionCore( move(ops) );
}


unique_ptr<SubsetResult> SubsetResult::GetIntersection( list<unique_ptr<SubsetResult>> ops )
{
    int n = ops.size();
    int n_comp = 0;
    for( const unique_ptr<SubsetResult> &op : ops )
        n_comp += op->complement_flag ? 1 : 0;

    if( n_comp == n )
        return DeMorgan( UnionCore, move(ops) );
    else
        return IntersectionCore( move(ops) );
}


unique_ptr<SubsetResult> SubsetResult::DeMorgan( function<unique_ptr<SubsetResult>( list<unique_ptr<SubsetResult>> )> lambda,
                                                                           list<unique_ptr<SubsetResult>> ops )
{
    list<unique_ptr<SubsetResult>> cops;
    for( const unique_ptr<SubsetResult> &op : ops )
        cops.push_back( op->GetComplement() );

    unique_ptr<SubsetResult> cres = lambda(move(cops));

    return cres->GetComplement();
}                                                       


unique_ptr<SubsetResult> SubsetResult::UnionCore( list<unique_ptr<SubsetResult>> ops )
{
    set<XValue> result_xlinks;
    for( const unique_ptr<SubsetResult> &op : ops )
    {
        ASSERTS( !op->complement_flag )("UnionCore requires no complements");
        result_xlinks = UnionOf( result_xlinks, op->xlinks );
    }
    return make_unique<SubsetResult>( result_xlinks );   
}


unique_ptr<SubsetResult> SubsetResult::IntersectionCore( list<unique_ptr<SubsetResult>> ops )
{
    const unique_ptr<SubsetResult> *non_comp_op = nullptr;
    for( const unique_ptr<SubsetResult> &op : ops )
        if( !op->complement_flag )
            non_comp_op = &op;
    ASSERTS( non_comp_op )("IntersectionCore requires at least one non-complement");

    // DifferenceOf() is the key to combining complemented with non-complimented
    set<XValue> result_xlinks = (*non_comp_op)->xlinks;
    for( const unique_ptr<SubsetResult> &op : ops )
    {
        if( &op == non_comp_op )
            continue; // got this one already
        if( op->complement_flag )
            result_xlinks = DifferenceOf( result_xlinks, op->xlinks );
        else
            result_xlinks = IntersectionOf( result_xlinks, op->xlinks );            
    }
    return make_unique<SubsetResult>( result_xlinks );
}


string SubsetResult::Render() const
{
    // ç is being used to mean complement of, since existing symbols not that great
    string s;
    if( complement_flag )
        s += "ç";
    s += Trace(xlinks);
    return s;
}

// ------------------------- DepthFirstRangeResult --------------------------

DepthFirstRangeResult::DepthFirstRangeResult( const SR::XTreeDatabase *x_tree_db_, XValue lower_, bool lower_incl_, XValue upper_, bool upper_incl_ ) :
    x_tree_db( x_tree_db_ ),
    lower( lower_ ),
    upper( upper_ ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


bool DepthFirstRangeResult::IsDefinedAndUnique() const
{
    ASSERTFAIL("TODO");
}


XValue DepthFirstRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool DepthFirstRangeResult::TryExtensionalise( set<XValue> &links ) const
{ 
    const SR::Orderings::DepthFirstOrdering &ordering = x_tree_db->GetOrderings().depth_first_ordering;
    SR::Orderings::DepthFirstOrdering::iterator it_lower, it_upper;
    
    if( lower )
    {
        it_lower = ordering.find(lower);
        ASSERT( it_lower != ordering.end() )
              ("lower=")(lower)(" upper=")(upper)("\n")
              (ordering);
        if( !lower_incl )
            ++it_lower;
    }
    else
    {
        it_lower = ordering.begin();
    }
    
    if( upper )
    {
        it_upper = ordering.find(upper);
        ASSERT( it_lower != ordering.end() );
        if( upper_incl )
            ++it_upper;
    }
    else
    {
        it_upper = ordering.end();
    }
    
    links = set<XValue>( it_lower, it_upper );
    return true;
}


bool DepthFirstRangeResult::operator==( const SymbolicResult &other ) const
{
    ASSERTFAIL("TODO");
}


string DepthFirstRangeResult::Render() const
{
    list<string> restrictions;
    
    if( lower )
        restrictions.push_back( string(lower_incl?">=":">") + lower.GetTrace() );
    if( upper )
        restrictions.push_back( string(upper_incl?"<=":"<") + upper.GetTrace() );
        
    return Join(restrictions, " & ", "{DF", "}");
}

// ------------------------- SimpleCompareRangeResult --------------------------

SimpleCompareRangeResult::SimpleCompareRangeResult( const SR::XTreeDatabase *x_tree_db_, KeyType lower_, bool lower_incl_, KeyType upper_, bool upper_incl_ ) :
    x_tree_db( x_tree_db_ ),
    lower( lower_ ),
    upper( upper_ ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


bool SimpleCompareRangeResult::IsDefinedAndUnique() const
{
    ASSERTFAIL("TODO");
}


XValue SimpleCompareRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool SimpleCompareRangeResult::TryExtensionalise( set<XValue> &links ) const
{        
    links.clear();
    SR::Orderings::SimpleCompareOrdering::const_iterator it_lower, it_upper;

    if( lower )
    {
        if( lower_incl )
            it_lower = x_tree_db->GetOrderings().simple_compare_ordering.lower_bound(lower);
        else
            it_lower = x_tree_db->GetOrderings().simple_compare_ordering.upper_bound(lower);
    }
    else
    {
        it_lower = x_tree_db->GetOrderings().simple_compare_ordering.begin();
    }
    
    if( upper )
    {
        if( upper_incl )
            it_upper = x_tree_db->GetOrderings().simple_compare_ordering.upper_bound(upper);
        else
            it_upper = x_tree_db->GetOrderings().simple_compare_ordering.lower_bound(upper);
    }
    else
    {
        it_upper = x_tree_db->GetOrderings().simple_compare_ordering.end();
    }
    
    for( SR::Orderings::SimpleCompareOrdering::const_iterator it = it_lower;
         it != it_upper;
         ++it )
	{
		const set<XValue> &new_links = x_tree_db->GetNodeRow(*it).incoming_xlinks;
		for( XValue l : new_links )
			links.insert(l);
	}	    
	return true;
}


bool SimpleCompareRangeResult::operator==( const SymbolicResult &other ) const
{
    ASSERTFAIL("TODO");
}


string SimpleCompareRangeResult::Render() const
{
    // ∈ etc means elt included (closed bound); ∉ means elt not included (open bound)
    list<string> restrictions;
    
    if( lower )
        restrictions.push_back( string(lower_incl?"∈ ":"∉ ") + lower.GetTrace() );
    if( upper )
        restrictions.push_back( upper.GetTrace() + string(upper_incl?"∋ ":"∌ ") );
        
    return Join(restrictions, ", ", "{SC ", "}");
}

// ------------------------- CategoryRangeResult --------------------------

CategoryRangeResult::CategoryRangeResult( const SR::XTreeDatabase *x_tree_db_, CatBoundsList &&bounds_list_, bool lower_incl_, bool upper_incl_ ) :
    x_tree_db( x_tree_db_ ),
    bounds_list( move(bounds_list_) ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


bool CategoryRangeResult::IsDefinedAndUnique() const
{
    ASSERTFAIL("TODO");
}


XValue CategoryRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool CategoryRangeResult::TryExtensionalise( set<XValue> &links ) const
{        
    links.clear();
    for( const CatBounds &bounds : bounds_list )
    {
        SR::Orderings::CategoryOrdering::const_iterator it_lower, it_upper; 

        ASSERT( &bounds );
        ASSERT( bounds.first );
        if( lower_incl )
            it_lower = x_tree_db->GetOrderings().category_ordering.lower_bound(*bounds.first);
        else
            it_lower = x_tree_db->GetOrderings().category_ordering.upper_bound(*bounds.first);

        ASSERT( bounds.second );
        if( upper_incl )
            it_upper = x_tree_db->GetOrderings().category_ordering.upper_bound(*bounds.second);
        else
            it_upper = x_tree_db->GetOrderings().category_ordering.lower_bound(*bounds.second);

        for( SR::Orderings::CategoryOrdering::const_iterator it = it_lower;
             it != it_upper;
             ++it )
        {
            const set<XValue> &new_links = x_tree_db->GetNodeRow(*it).incoming_xlinks;
            for( XValue l : new_links )
				links.insert(l);
		}		
    }
    return true;
}


bool CategoryRangeResult::operator==( const SymbolicResult &other ) const
{
    ASSERTFAIL("TODO");
}


string CategoryRangeResult::Render() const
{
    // ∈ etc means elt included (closed bound); ∉ means elt not included (open bound)
    list<string> terms;
    for( const CatBounds &bounds : bounds_list )
    {
        list<string> restrictions;
        restrictions.push_back( string(lower_incl?"∈ ":"∉ ") + bounds.first->GetTrace() );
        restrictions.push_back( bounds.second->GetTrace() + string(upper_incl?"∋ ":"∌ ") );
        terms.push_back( Join(restrictions, ", ") );
    }
    return Join(terms, " ∪ ", "{CAT ", "}");
}

