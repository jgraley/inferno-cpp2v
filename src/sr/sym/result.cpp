#include "result.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- BooleanResult --------------------------

BooleanResult::BooleanResult( Category cat, bool value )
{
    switch( cat )
    {
    case UNDEFINED:
        ASSERT( !value );
        certainty = Certainty::UNDEFINED;
        break;
    case DEFINED:        
        certainty = value ? Certainty::TRUE : Certainty::FALSE;
        break;
    default:
        ASSERTFAIL("Missing case");
    }
}


bool BooleanResult::IsDefinedAndUnique() const
{
    return certainty != Certainty::UNDEFINED;
}


bool BooleanResult::IsDefinedAndTrue() const
{
    return certainty == Certainty::TRUE;
}


bool BooleanResult::IsDefinedAndFalse() const
{
    return certainty == Certainty::FALSE;
}

   
bool BooleanResult::GetAsBool() const
{
    ASSERT( IsDefinedAndUnique() );
    return certainty == Certainty::TRUE;
}


bool BooleanResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const BooleanResult *>(&other);
    return o && certainty == o->certainty;
}


bool BooleanResult::operator<( const BooleanResultInterface &other ) const
{
    auto o = dynamic_cast<const BooleanResult *>(&other);
    ASSERT(o);
    return certainty < o->certainty;
}


string BooleanResult::GetTrace() const
{
    switch( certainty )
    {
    case Certainty::FALSE:
        return "FALSE";
    case Certainty::UNDEFINED:
        return "UNDEFINED";
    case Certainty::TRUE:
        return "TRUE";
    }
    return "CORRUPTED!!";
}


// ------------------------- SymbolResult --------------------------

SymbolResult::SymbolResult( Category cat, SR::XLink xlink_ )
{
    switch( cat )
    {
    case UNDEFINED:
        ASSERT( !xlink_ );
        break;
    case DEFINED:
        ASSERT( xlink_ );
        xlink = xlink_;
        break;
    default:
        ASSERTFAIL("Missing case");
    }
}


bool SymbolResult::IsDefinedAndUnique() const
{
    return (bool)xlink;
}


SR::XLink SymbolResult::GetAsXLink() const
{
    ASSERT( xlink );
    return xlink;
}


bool SymbolResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{
    links = xlink ? set<SR::XLink>{ xlink } : set<SR::XLink>{};
    return true;
}


bool SymbolResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const SymbolResult *>(&other);
    return o && xlink == o->xlink;
}


string SymbolResult::GetTrace() const
{
    if( xlink )
        return "DEFINED:"+Trace(xlink);
    else
        return "UNDEFINED";
}

// ------------------------- SymbolSetResult --------------------------

SymbolSetResult::SymbolSetResult( set<SR::XLink> xlinks_, bool complement_flag_ ) :
    xlinks( xlinks_ ),
    complement_flag( complement_flag_ )
{
}


shared_ptr<SymbolSetResult> SymbolSetResult::Create( shared_ptr<SymbolResultInterface> other )
{
    if( auto ssr = dynamic_pointer_cast<SymbolSetResult>(other) )
    {
        return ssr;
    }
    else if( auto sr = dynamic_pointer_cast<SymbolResult>(other) )
    {
        set<SR::XLink> links;
        bool ok = other->TryGetAsSetOfXLinks( links );
        ASSERTS(ok);
        return make_shared<SymbolSetResult>( links );
    }
    else
    {
        ASSERTFAILS();
    }
}


bool SymbolSetResult::IsDefinedAndUnique() const
{
    return !complement_flag && xlinks.size() == 1;
}


SR::XLink SymbolSetResult::GetAsXLink() const
{
    ASSERT( !complement_flag )("Is complement so not unique");
    return OnlyElementOf(xlinks);
}


bool SymbolSetResult::TryGetAsSetOfXLinks( set<SR::XLink> &links ) const
{
    if( !complement_flag ) // Refusing to extensionalise a complement set
        return false;
    links = xlinks;
    return true;
}


bool SymbolSetResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const SymbolSetResult *>(&other);
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

