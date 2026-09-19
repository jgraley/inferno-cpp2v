#include "common/common.hpp"

#include "clone.hpp"

shared_ptr<Cloner> Cloner::Duplicate( shared_ptr<Cloner> p )
{
    ASSERT( p.get() == this ); // unfortunate wrinkle: must always call as PX->Duplicate(PX) TODO does shared_from_this help?
    return Clone(); // default duplication is to clone, but can be over-ridden
}
