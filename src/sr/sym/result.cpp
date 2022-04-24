#include "result.hpp"
#include "the_knowledge.hpp"

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


string BooleanResult::GetTrace() const
{
    return Trace(value);
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


string SymbolResult::GetTrace() const
{
    if( xlink )
        return Trace(xlink);
    else
        return "NOT_A_SYMBOL";
}

// ------------------------- SymbolSetResult --------------------------

SymbolSetResult::SymbolSetResult( set<SR::XLink> xlinks_, bool complement_flag_ ) :
    xlinks( xlinks_ ),
    complement_flag( complement_flag_ )
{
}


SymbolSetResult::SymbolSetResult( shared_ptr<SymbolResultInterface> other )
{
    if( auto ssr = dynamic_pointer_cast<SymbolSetResult>(other) )
    {
        xlinks = ssr->xlinks;
        complement_flag = ssr->complement_flag;
    }
    else if( auto sr = dynamic_pointer_cast<SymbolResultInterface>(other) )
    {
        set<SR::XLink> links;
        bool ok = other->TryGetAsSetOfXLinks( xlinks );
        ASSERTS(ok);
        complement_flag = false;
    }
    else
    {
        ASSERTS(false)("Don't know how to make a SymbolSetResult out of ")(*other);
    }
}


bool SymbolSetResult::IsDefinedAndUnique() const
{
    return !complement_flag && xlinks.size() == 1;
}


SR::XLink SymbolSetResult::GetOnlyXLink() const
{
    ASSERT( !complement_flag )("Is complement so not unique");
    return OnlyElementOf(xlinks);
}


bool SymbolSetResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{
    if( complement_flag ) // Refusing to extensionalise a complement set
        return false;
    links = xlinks;
    return true;
}


bool SymbolSetResult::operator==( const SymbolResultInterface &other ) const
{
    auto o = GET_THAT_POINTER(&other);
    return o && xlinks == o->xlinks && complement_flag==o->complement_flag;
}


shared_ptr<SymbolSetResult> SymbolSetResult::GetComplement() const
{
    return make_shared<SymbolSetResult>(xlinks, !complement_flag);
}


shared_ptr<SymbolSetResult> SymbolSetResult::GetUnion( list<shared_ptr<SymbolSetResult>> ops )
{
    int n = ops.size();
    int n_comp = 0;
    for( shared_ptr<SymbolSetResult> op : ops )
        n_comp += op->complement_flag ? 1 : 0;

    if( n_comp > 0 )
        return DeMorgan( IntersectionCore, ops );
    else
        return UnionCore( ops );
}


shared_ptr<SymbolSetResult> SymbolSetResult::GetIntersection( list<shared_ptr<SymbolSetResult>> ops )
{
    int n = ops.size();
    int n_comp = 0;
    for( shared_ptr<SymbolSetResult> op : ops )
        n_comp += op->complement_flag ? 1 : 0;

    if( n_comp == n )
        return DeMorgan( UnionCore, ops );
    else
        return IntersectionCore( ops );
}


shared_ptr<SymbolSetResult> SymbolSetResult::DeMorgan( function<shared_ptr<SymbolSetResult>( list<shared_ptr<SymbolSetResult>> )> lambda,
                                                       list<shared_ptr<SymbolSetResult>> ops )
{
    list<shared_ptr<SymbolSetResult>> cops;
    for( shared_ptr<SymbolSetResult> op : ops )
        cops.push_back( op->GetComplement() );

    shared_ptr<SymbolSetResult> cres = lambda(cops);

    return cres->GetComplement();
}                                                       


shared_ptr<SymbolSetResult> SymbolSetResult::UnionCore( list<shared_ptr<SymbolSetResult>> ops )
{
    set<SR::XLink> result_xlinks;
    for( shared_ptr<SymbolSetResult> op : ops )
    {
        ASSERTS( !op->complement_flag )("UnionCore requires no complements");
        result_xlinks = UnionOf( result_xlinks, op->xlinks );
    }
    return make_shared<SymbolSetResult>( result_xlinks );   
}


shared_ptr<SymbolSetResult> SymbolSetResult::IntersectionCore( list<shared_ptr<SymbolSetResult>> ops )
{
    shared_ptr<SymbolSetResult> non_comp_op;
    for( shared_ptr<SymbolSetResult> op : ops )
        if( !op->complement_flag )
            non_comp_op = op;
    ASSERTS( non_comp_op )("IntersectionCore requires at least one non-complement");

    // DifferenceOf() is the key to combining complemented with non-complimented
    set<SR::XLink> result_xlinks = non_comp_op->xlinks;
    for( shared_ptr<SymbolSetResult> op : ops )
    {
        if( op == non_comp_op )
            continue; // got this one already
        if( op->complement_flag )
            result_xlinks = DifferenceOf( result_xlinks, op->xlinks );
        else
            result_xlinks = IntersectionOf( result_xlinks, op->xlinks );            
    }
    return make_shared<SymbolSetResult>( result_xlinks );
}


string SymbolSetResult::GetTrace() const
{
    string s;
    if( complement_flag )
        s += "ç";
    s += Trace(xlinks);
    return s;
}

// ------------------------- SymbolRangeResult --------------------------

SymbolRangeResult::SymbolRangeResult( const SR::TheKnowledge *knowledge_, SR::XLink lower_, bool lower_incl_, SR::XLink upper_, bool upper_incl_ ) :
    knowledge( knowledge_ ),
    lower( lower_ ),
    lower_incl( lower_incl_ ),
    upper( upper_ ),
    upper_incl( upper_incl_ )
{
}


bool SymbolRangeResult::IsDefinedAndUnique() const
{
    ASSERTFAIL("TODO");
}


SR::XLink SymbolRangeResult::GetOnlyXLink() const
{
    ASSERTFAIL("TODO");
}


bool SymbolRangeResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{ 
    SR::TheKnowledge::DepthFirstOrderedIt it_lower, it_upper;
    
    if( lower )
    {
        it_lower = knowledge->GetNugget(lower).depth_first_ordered_it;
        if( !lower_incl )
            ++it_lower;
    }
    else
    {
        it_lower = knowledge->depth_first_ordered_domain.begin();
    }
    
    if( upper )
    {
        it_upper = knowledge->GetNugget(upper).depth_first_ordered_it;
        if( upper_incl && it_upper != knowledge->depth_first_ordered_domain.end() )
            ++it_upper;
    }
    else
    {
        it_upper = knowledge->depth_first_ordered_domain.end();
    }
    
    links = set<SR::XLink>( it_lower, it_upper );
    return true;
}


bool SymbolRangeResult::operator==( const SymbolResultInterface &other ) const
{
    ASSERTFAIL("TODO");
}


string SymbolRangeResult::GetTrace() const
{
    list<string> restrictions;
    
    if( lower )
        restrictions.push_back( string(lower_incl?">=":">") + lower.GetTrace() );
    if( upper )
        restrictions.push_back( string(upper_incl?"<=":"<") + upper.GetTrace() );
        
    return Join(restrictions, " & ", "{", "}");
}
