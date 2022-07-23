#include "transformation.hpp"

#include "common/trace.hpp"
#include "common/read_args.hpp"


TreePtr<Node> Transformation::operator()( TreePtr<Node> root )
{
    return operator()( root, root );
}


void Transformation::operator()( TreePtr<Node> *proot )
{
    operator()( *proot, proot );
}


void Transformation::SetStopAfter( vector<int> ssa, int depth )
{
}


// Implement out-of-place in terms of in-place
TreePtr<Node> InPlaceTransformation::operator()( TreePtr<Node> context,
                                                 TreePtr<Node> root )
{
    ASSERTFAIL("TODO run in-place transformation and then copy back the subtree");
}    		                             


// Implement in-place in terms of out-of-place
void OutOfPlaceTransformation::operator()( TreePtr<Node> context,
                                           TreePtr<Node> *proot )
{
    TreePtr<Node> result_root = operator()( context, *proot );
    *proot = result_root;
}    		                             


TransformationVector::TransformationVector() :
    depth(0)
{
}
    
    
void TransformationVector::operator()( TreePtr<Node> context,     // The whole program, so declarations may be searched for
                                       TreePtr<Node> *proot )     // Root of the subtree we want to modify
{
    INDENT("¬");
    TreePtr<Node> *pcontext;
    if( context == *proot )
        pcontext = proot;
    else 
        pcontext = &context;
    int i = 0;
    for( shared_ptr<Transformation> t : *this )
    {
        TRACE("Transformation vector element %d ", i)(*t)("\n");
        bool stop = depth < stop_after.size() && stop_after[depth]==i;
        if( stop )
            t->SetStopAfter(stop_after, depth+1); // and propagate the remaining ones
        (*t)(*pcontext, proot);
        if( stop )
        {
            TRACE("Stopping after sub-step %d\n", stop_after[depth]);
            break;
        }
        i++;
    }                            
}		                           


void TransformationVector::SetStopAfter( vector<int> ssa, int d )
{
    stop_after = ssa;
    depth = d;
    if( depth < stop_after.size()) 
        ASSERT( stop_after[depth] < size() )("Stop requested after non-existant sub-step, there are only %d", size());
}


bool Filter::IsMatch( TreePtr<Node> root )
{
    return IsMatch( root, root );
}
