
#include "sort_decls.hpp"
#include "tree/misc.hpp"
#include "helpers/simple_compare.hpp"

using namespace CPPTree;

/** Walks the tree, avoiding recursing into the body (initialiser) of Callables. */
class UniqueWalkNoBody_iterator : public UniqueWalk::iterator
{
public:
    UniqueWalkNoBody_iterator( TreePtr<Node> &root ) : UniqueWalk::iterator(root) {}        
    UniqueWalkNoBody_iterator() : UniqueWalk::iterator() {}
    virtual unique_ptr<ContainerInterface::iterator_interface> Clone() const
    {
           return make_unique<UniqueWalkNoBody_iterator>(*this);
    }      
protected:
    virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const
    {
        INDENT("G");
        // We need to create a container of elements of the child.
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>( n ) ) // an instance...
            if( dynamic_pointer_cast<Callable>( i->type ) ) // ...of a function
            {
                //TRACE("special behaviour for ")(*n)("\n");
                // it's an instance, so set up a container containing type and identifier only, 
                // not initialiser (others don't matter for deps purposes). We need 
                // the type for params etc
                auto seq = make_shared< Sequence<Node> >();
                seq->push_back( i->type );
                seq->push_back( i->identifier );
                return seq;
            }
        //TRACE("defaulting ")(*n)("\n");
        return UniqueWalk::iterator::GetChildContainer(n);
    }
};

typedef ContainerFromIterator< UniqueWalkNoBody_iterator, TreePtr<Node> > UniqueWalkNoBody;

/** Walks the tree, avoiding recursing into the body (initialiser) of Callables and anything under an indirection. */
class UniqueWalkNoBodyOrIndirection_iterator : public UniqueWalkNoBody::iterator
{
public:
    UniqueWalkNoBodyOrIndirection_iterator( TreePtr<Node> &root ) : UniqueWalkNoBody::iterator(root) {}        
    UniqueWalkNoBodyOrIndirection_iterator() : UniqueWalkNoBody::iterator() {}
    virtual unique_ptr<ContainerInterface::iterator_interface> Clone() const
    {
           return make_unique<UniqueWalkNoBodyOrIndirection_iterator>(*this);
    }      
private:
    virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const
    {
        INDENT("G");
        // We need to create a container of elements of the child.
        if( dynamic_pointer_cast<Indirection>( n ) ) // an instance...
        {
            //TRACE("special behaviour for ")(*n)("\n");
            return make_shared< Sequence<Node> >();
        }
        //TRACE("defaulting ")(*n)("\n");
        return UniqueWalkNoBody::iterator::GetChildContainer(n);
    }
};


typedef ContainerFromIterator< UniqueWalkNoBodyOrIndirection_iterator, TreePtr<Node> > UniqueWalkNoBodyOrIndirection;

// Does a depend on b?
bool IsDependOn( TreePtr<Declaration> a, TreePtr<Declaration> b, bool ignore_indirection_to_record )
{
    if( a == b )
        return false;
    
    const TreePtr<Record> recb = DynamicTreePtrCast<Record>(b); // 2 unrelated uses
    
    // Only ignore pointers and refs if we're checking dependency on a record; typedefs
    // still apply (since you can't forward declare a typedef).
    bool ignore_indirection = ignore_indirection_to_record && recb;

    // Actually, we really want to see whether declaration a depends on the identifier of b
    // since the rest of b is irrelevent (apart from the above).
    TreePtr<Identifier> ib = GetIdentifierOfDeclaration( b ).GetTreePtr();
    ASSERT(ib);
          
      //TRACE("Looking for dependencies on ")(*b)(" (identifier ")(*ib)(") under ")(*a)(ignore_indirection?"":" not")(" ignoring indirection\n");      
          
    // Always ignore function bodies since these are taken outboard by the renderer anyway      
    UniqueWalkNoBody wnb( a );
    UniqueWalkNoBodyOrIndirection wnbi( a );
    ContainerInterface *w = ignore_indirection ? (ContainerInterface *)&wnbi : (ContainerInterface *)&wnb;
        
    ContainerInterface::iterator wa=w->begin();
    while(!(wa == w->end()))
    {
        //TRACE("Seen node ")(**wa)("\n");
    /*    if( ignore_indirection ) // are we to ignore pointers/refs?
        {
            if( TreePtr<Indirection> inda = TreePtr<Indirection>::DynamicCast(*wa) ) // is a a pointer or ref?
            {
                if( dynamic_pointer_cast<Identifier>(inda->destination) == ib ) // does it depend on b?
                {
                    wa.AdvanceOver(); // Then skip it
                    continue;
                }
            }
        }
        */
        if( TreePtr<Node>(*wa) == TreePtr<Node>(ib) ) // If we see b in *any* other context under a's type, there's dep.
        {
            //TRACE("Found dependency\n");
            return true;     
        }           

        ++wa;
    }
    
    // Recurse though members of records since Inferno doesn't require scope to be remembered - so
    // the dependency might be on something buried in the record.
    if( recb )
    {
        for( TreePtr<Declaration> memberb : recb->members )
            if( IsDependOn( a, memberb, ignore_indirection_to_record ) )
                return true;
    }
    //TRACE("Did not find dependency\n");
    return false; 
}


template<typename NodeType>
void ExtractDeclsToBack( Sequence<Declaration> &sorted, Sequence<Declaration> &pre_sorted )
{
	for( ContainerInterface::iterator it = pre_sorted.begin(); it != (ContainerInterface::iterator)pre_sorted.end(); ) 
	{
		if( TreePtr<NodeType>::DynamicCast(*it) )
		{
			sorted.push_back(*it);
			it = pre_sorted.erase(it);
		}
		else
		{
			++it;
		}
	}	
}


Sequence<Declaration> SortDecls( ContainerInterface &c, bool ignore_indirection_to_record, const UniquifyIdentifiers::IdentifierNameMap &unique_ids )
{
    int ocs = c.size();
    
    // Our algorithm will modify the source container, so make a copy of it
    Sequence<Declaration> unsorted;
    for( const TreePtrInterface &a : c )
        unsorted.push_back( a );

     // Sort using SimpleCompare first: this should improve reproducibility
    Sequence<Declaration> pre_sorted = PreSortDecls( unsorted, unique_ids );

    Sequence<Declaration> sorted;

	// Move pre-processor declarations into the sorted list first
	ExtractDeclsToBack<LocalInclude>(sorted, pre_sorted);
	ExtractDeclsToBack<SystemInclude>(sorted, pre_sorted);
	ExtractDeclsToBack<PreProcDecl>(sorted, pre_sorted);

    // Uncomment one of these to stress the sorter
    //pre_sorted = ReverseDecls( pre_sorted );
    //pre_sorted = JumbleDecls( pre_sorted );

   // Keep searching our local container of decls (cc) for decls that do not depend
    // on anything else in the container. Such a decl may be safely rendered before the
    // rest of the decls, so place it at the end of the sequence we are building up
    // (s) and remove from the container cc since cc only holds the ones we have still to
    // place in s. Repeat until we've done all the decls at which point cc is empty.
    // If no non-dependent decl may be found in cc then it's irredemably circular and
    // we fail. This looks like O(N^3).
    TRACE("Adding decls in dep order: ");
    while( !pre_sorted.empty() )
    {
        bool found = false; // just for ASSERT check
        for( const TreePtr<Declaration> &a : pre_sorted )
        {
            bool a_has_deps=false;
            for( const TreePtr<Declaration> &b : pre_sorted )
            {
                TreePtr<Declaration> aid = dynamic_cast< const TreePtr<Declaration> & >(a);
                a_has_deps |= IsDependOn( aid, b, ignore_indirection_to_record );
            }
            if( !a_has_deps )
            {
                TRACE(*a)(" ");
                sorted.push_back(a);
                pre_sorted.erase(a);
                found = true;
                break;
            }
        }

        if( !found )
        {   
            TRACE("\nRemaining unsequenceable decls: ");
            for( const TreePtr<Declaration> &a : pre_sorted )
                   TRACE(*a)(" ");
        }
        ASSERT( found )("\nfailed to find a decl to add without dependencies, maybe circular\n");
        //TRACE("%d %d\n", s.size(), c.size() );
    }
    TRACE("\n");
    ASSERT( sorted.size() == ocs );
    return sorted;
}


Sequence<Declaration> PreSortDecls( Sequence<Declaration> c, const UniquifyIdentifiers::IdentifierNameMap &unique_ids )
{
    //FTRACE("PreSortDecls()\n");

    // Make a SimpleCompare-ordered set and fill it with the decls
    auto comparer = UniquifyCompare(unique_ids);
    SimpleCompare::TreePtrOrdering sco = comparer.GetTreePtrOrdering(c);
    //SimpleCompare(Orderable::REPEATABLE).GetTreePtrOrdering(c);

    // Extract the decls from the set, now in SimpleCompare order
    Sequence<Declaration> s;
    for( TreePtr<Node> e : sco )
    {
        auto d = TreePtr<Declaration>::DynamicCast(e);
        s.push_back(d);
        //if( auto inst = TreePtr<Instance>::DynamicCast(d) )
        //{
        //    auto sid = TreePtr<SpecificIdentifier>::DynamicCast(inst->identifier);
        //    string ustr = unique->at(sid);
        //    FTRACEC(inst)("(")(ustr)(", ")(inst->type)(", ")(inst->initialiser)(")\n");
        //}
    }
    
    return s;
}


Sequence<Declaration> JumbleDecls( Sequence<Declaration> c )
{
    srand(99);
    
    Sequence<Declaration> s;
    for( TreePtr<Declaration> to_insert : c ) // we will insert each element from the collection
    {
        // Idea is to insert each new element just before the first exiting element that
        // depends on it. This is the latest position we can insert the new element.
        Sequence<Declaration>::iterator i;
        int n = rand() % (s.size()+1);
        for( i = s.begin(); i != s.end(); ++i )
           {
               if( n-- == 0 )
               {
                   // Found element that depends on the one we want to insert. So insert just 
                   // before the found element.
                   break;            
               }
           }
           
           // Insert the element. If we didn't find a dependency, we'll be off the end of
           // the sequence and hopefully insert() will actually push_back()
          s.insert( i, (const TreePtrInterface &)to_insert ); 
    }
    
    ASSERT( s.size() == c.size() );
    return s;
}

Sequence<Declaration> ReverseDecls( Sequence<Declaration> c )
{
    Sequence<Declaration> s;
    for( TreePtr<Declaration> to_insert : c ) // we will insert each element from the collection
    {
           // Insert the element. If we didn't find a dependency, we'll be off the end of
           // the sequence and hopefully insert() will actually push_back()
          s.insert( s.begin(), (const TreePtrInterface &)to_insert ); 
    }
    
    ASSERT( s.size() == c.size() );
    return s;
}

// TODO
// add ifelse.c to the tests
// Every way of getting to anythign should be via RenderIntoProduction()
// Add a ticket to make Record a true Type not a UserType. Generate TypeDef during parse and just leave them in.
//     For renderer, Record can just be handled like another type with declarator support, and
//     PROTOTYPE production triggers prototype declarator. Consider eliding Typedef(Record...).
