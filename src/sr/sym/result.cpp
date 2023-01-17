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

// ------------------------- SymbolResultInterface --------------------------

string SymbolResultInterface::GetTrace() const
{
    return Render();
}

// ------------------------- SymbolResult --------------------------

SymbolResult::SymbolResult( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
    ASSERT( xlink_ )("Not allowed to construct with NULL; use other constructor instead");
}


SymbolResult::SymbolResult( Category cat ) :
    xlink()
{
    ASSERT( cat==NOT_A_SYMBOL )("Can only pass in NOT_A_SYMBOL; use other constructor instead");
}


bool SymbolResult::IsDefinedAndUnique() const
{
    return (bool)xlink;
}


SR::XLink SymbolResult::GetOnlyXLink() const
{
    ASSERT( xlink );
    return xlink;
}


bool SymbolResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{
    links = xlink ? set<SR::XLink>{ xlink } : set<SR::XLink>{};
    return true;
}


bool SymbolResult::operator==( const SymbolResultInterface &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return o && xlink == o->xlink;
}


string SymbolResult::Render() const
{
    if( xlink )
        return Trace(xlink);
    else
        return "NOT_A_SYMBOL";
}

// ------------------------- SetResult --------------------------

SetResult::SetResult( set<SR::XLink> xlinks_, bool complement_flag_ ) :
    xlinks( xlinks_ ),
    complement_flag( complement_flag_ )
{
}


SetResult::SetResult( unique_ptr<SymbolResultInterface> other )
{
    if( auto ssr = dynamic_pointer_cast<SetResult>(other) )
    {
        xlinks = ssr->xlinks;
        complement_flag = ssr->complement_flag;
    }
    else // SymbolResultInterface
    {
        set<SR::XLink> links;
        bool ok = other->TryGetAsSetOfXLinks( xlinks );
        ASSERTS(ok);
        complement_flag = false;
    }
}


bool SetResult::IsDefinedAndUnique() const
{
    return !complement_flag && xlinks.size() == 1;
}


SR::XLink SetResult::GetOnlyXLink() const
{
    ASSERT( !complement_flag )("Is complement so not unique");
    return OnlyElementOf(xlinks);
}


bool SetResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{
    if( complement_flag ) // Refusing to extensionalise a complement set
        return false;
    links = xlinks;
    return true;
}


bool SetResult::operator==( const SymbolResultInterface &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return o && xlinks == o->xlinks && complement_flag==o->complement_flag;
}


unique_ptr<SetResult> SetResult::GetComplement() const
{
    return make_unique<SetResult>(xlinks, !complement_flag);
}


unique_ptr<SetResult> SetResult::GetUnion( list<unique_ptr<SetResult>> ops )
{
    int n = ops.size();
    int n_comp = 0;
    for( const unique_ptr<SetResult> &op : ops )
        n_comp += op->complement_flag ? 1 : 0;

    if( n_comp > 0 )
        return DeMorgan( IntersectionCore, move(ops) );
    else
        return UnionCore( move(ops) );
}


unique_ptr<SetResult> SetResult::GetIntersection( list<unique_ptr<SetResult>> ops )
{
    int n = ops.size();
    int n_comp = 0;
    for( const unique_ptr<SetResult> &op : ops )
        n_comp += op->complement_flag ? 1 : 0;

    if( n_comp == n )
        return DeMorgan( UnionCore, move(ops) );
    else
        return IntersectionCore( move(ops) );
}


unique_ptr<SetResult> SetResult::DeMorgan( function<unique_ptr<SetResult>( list<unique_ptr<SetResult>> )> lambda,
                                                                           list<unique_ptr<SetResult>> ops )
{
    list<unique_ptr<SetResult>> cops;
    for( const unique_ptr<SetResult> &op : ops )
        cops.push_back( op->GetComplement() );

    unique_ptr<SetResult> cres = lambda(move(cops));

    return cres->GetComplement();
}                                                       


unique_ptr<SetResult> SetResult::UnionCore( list<unique_ptr<SetResult>> ops )
{
    set<SR::XLink> result_xlinks;
    for( const unique_ptr<SetResult> &op : ops )
    {
        ASSERTS( !op->complement_flag )("UnionCore requires no complements");
        result_xlinks = UnionOf( result_xlinks, op->xlinks );
    }
    return make_unique<SetResult>( result_xlinks );   
}


unique_ptr<SetResult> SetResult::IntersectionCore( list<unique_ptr<SetResult>> ops )
{
    const unique_ptr<SetResult> *non_comp_op = nullptr;
    for( const unique_ptr<SetResult> &op : ops )
        if( !op->complement_flag )
            non_comp_op = &op;
    ASSERTS( non_comp_op )("IntersectionCore requires at least one non-complement");

    // DifferenceOf() is the key to combining complemented with non-complimented
    set<SR::XLink> result_xlinks = (*non_comp_op)->xlinks;
    for( const unique_ptr<SetResult> &op : ops )
    {
        if( &op == non_comp_op )
            continue; // got this one already
        if( op->complement_flag )
            result_xlinks = DifferenceOf( result_xlinks, op->xlinks );
        else
            result_xlinks = IntersectionOf( result_xlinks, op->xlinks );            
    }
    return make_unique<SetResult>( result_xlinks );
}


string SetResult::Render() const
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


bool DepthFirstRangeResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{ 
	const SR::Orderings::DepthFirstOrdering &index = x_tree_db->GetOrderings().depth_first_ordering;
    SR::Orderings::DepthFirstOrderingIterator it_lower, it_upper;
    
    if( lower )
    {
        it_lower = index.find(lower);
        ASSERT( it_lower != index.end() )
              ("lower=")(lower)(" upper=")(upper)("\n")
              (index);
        if( !lower_incl )
            ++it_lower;
    }
    else
    {
        it_lower = index.begin();
    }
    
    if( upper )
    {
        it_upper = index.find(upper);
        ASSERT( it_lower != index.end() );
        if( upper_incl )
            ++it_upper;
    }
    else
    {
        it_upper = index.end();
    }
    
    links = set<SR::XLink>( it_lower, it_upper );
    return true;
}


bool DepthFirstRangeResult::operator==( const SymbolResultInterface &other ) const
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


bool SimpleCompareRangeResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
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


bool SimpleCompareRangeResult::operator==( const SymbolResultInterface &other ) const
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


bool CategoryRangeResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
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


bool CategoryRangeResult::operator==( const SymbolResultInterface &other ) const
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

