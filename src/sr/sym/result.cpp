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

string SymbolicResult::GetTrace() const
{
    return Render();
}

// ------------------------- UniqueResult --------------------------

UniqueResult::UniqueResult( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
    ASSERT( xlink_ )("Not allowed to construct with NULL; use EmptyResult instead");
}


bool UniqueResult::IsDefinedAndUnique() const
{
    return true;
}


SR::XLink UniqueResult::GetOnlyXLink() const
{
    ASSERT( xlink );
    return xlink;
}


bool UniqueResult::TryExtensionalise( set<SR::XLink> &links ) const
{
    links = set<SR::XLink>{ xlink };
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


SR::XLink EmptyResult::GetOnlyXLink() const
{
    ASSERTFAIL();
}


bool EmptyResult::TryExtensionalise( set<SR::XLink> &links ) const
{
    links = set<SR::XLink>{};
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

SubsetResult::SubsetResult( set<SR::XLink> xlinks_, bool complement_flag_ ) :
    xlinks( xlinks_ ),
    complement_flag( complement_flag_ )
{
}


SubsetResult::SubsetResult( unique_ptr<SymbolicResult> other )
{
    if( auto ssr = dynamic_pointer_cast<SubsetResult>(other) )
    {
        xlinks = ssr->xlinks;
        complement_flag = ssr->complement_flag;
    }
    else // SymbolicResult
    {
        set<SR::XLink> links;
        bool ok = other->TryExtensionalise( xlinks );
        ASSERTS(ok);
        complement_flag = false;
    }
}


bool SubsetResult::IsDefinedAndUnique() const
{
    return !complement_flag && xlinks.size() == 1;
}


SR::XLink SubsetResult::GetOnlyXLink() const
{
    ASSERT( !complement_flag )("Is complement so not unique");
    return OnlyElementOf(xlinks);
}


bool SubsetResult::TryExtensionalise( set<SR::XLink> &links ) const
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
    int n = ops.size();
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
    set<SR::XLink> result_xlinks;
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
    set<SR::XLink> result_xlinks = (*non_comp_op)->xlinks;
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

DepthFirstRangeResult::DepthFirstRangeResult( const SR::XTreeDatabase *x_tree_db_, SR::XLink lower_, bool lower_incl_, SR::XLink upper_, bool upper_incl_ ) :
    x_tree_db( x_tree_db_ ),
    lower( lower_ ),
    lower_incl( lower_incl_ ),
    upper( upper_ ),
    upper_incl( upper_incl_ )
{
}


bool DepthFirstRangeResult::IsDefinedAndUnique() const
{
    ASSERTFAIL("TODO");
}


SR::XLink DepthFirstRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool DepthFirstRangeResult::TryExtensionalise( set<SR::XLink> &links ) const
{ 
	const SR::Orderings::DepthFirstOrdering &ordering = x_tree_db->GetOrderings().depth_first_ordering;
    SR::Orderings::DepthFirstOrderingIterator it_lower, it_upper;
    
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
    
    links = set<SR::XLink>( it_lower, it_upper );
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

SimpleCompareRangeResult::SimpleCompareRangeResult( const SR::XTreeDatabase *x_tree_db_, SR::XLink lower_, bool lower_incl_, SR::XLink upper_, bool upper_incl_ ) :
    x_tree_db( x_tree_db_ ),
    lower( lower_ ),
    lower_incl( lower_incl_ ),
    upper( upper_ ),
    upper_incl( upper_incl_ )
{
}


SimpleCompareRangeResult::SimpleCompareRangeResult( const SR::XTreeDatabase *x_tree_db_, TreePtr<Node> lower_, bool lower_incl_, TreePtr<Node> upper_, bool upper_incl_ ) :
    x_tree_db( x_tree_db_ ),
    lower( SR::XLink::CreateDistinct( lower_ ) ),
    lower_incl( lower_incl_ ),
    upper( SR::XLink::CreateDistinct( upper_ ) ),
    upper_incl( upper_incl_ )
{
}


bool SimpleCompareRangeResult::IsDefinedAndUnique() const
{
    ASSERTFAIL("TODO");
}


SR::XLink SimpleCompareRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool SimpleCompareRangeResult::TryExtensionalise( set<SR::XLink> &links ) const
{        
    SR::Orderings::SimpleCompareOrderingIterator it_lower, it_upper;

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
        it_upper = x_tree_db->GetOrderings().simple_compare_ordering.begin();
    }

    links = set<SR::XLink>( it_lower, it_upper );
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
        restrictions.push_back( string(lower_incl?"∈":"∉") + lower.GetTrace() );
    if( upper )
        restrictions.push_back( upper.GetTrace() + string(upper_incl?"∋":"∌") );
        
    return Join(restrictions, ", ", "{SC ", "}");
}

// ------------------------- CategoryRangeResult --------------------------

CategoryRangeResult::CategoryRangeResult( const SR::XTreeDatabase *x_tree_db_, XLinkBoundsList &&bounds_list_, bool lower_incl_, bool upper_incl_ ) :
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


SR::XLink CategoryRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool CategoryRangeResult::TryExtensionalise( set<SR::XLink> &links ) const
{        
    links.clear();
    for( const XLinkBounds &bounds : bounds_list )
    {
        SR::Orderings::SimpleCompareOrderingIterator it_lower, it_upper;

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

        links = UnionOf( links, set<SR::XLink>( it_lower, it_upper ) );
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
    for( const XLinkBounds &bounds : bounds_list )
    {
        list<string> restrictions;
        restrictions.push_back( string(lower_incl?"∈":"∉") + bounds.first->GetTrace() );
        restrictions.push_back( bounds.second->GetTrace() + string(upper_incl?"∋":"∌") );
        terms.push_back( Join(restrictions, ", ") );
    }
    return Join(terms, " ∪ ", "{CAT ", "}");
}

