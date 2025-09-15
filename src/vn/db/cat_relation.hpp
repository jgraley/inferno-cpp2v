#ifndef CAT_RELATION_HPP
#define CAT_RELATION_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"


namespace SYM
{
    class BooleanExpression;
};
    
    
namespace VN 
{
class Lacing;
class XTreeDatabase;
    
class CategoryRelation
{
public:
    typedef TreePtr<Node> KeyType;    

    //CategoryRelation();
    CategoryRelation( shared_ptr<Lacing> lacing );
    CategoryRelation& operator=(const CategoryRelation &other);
    
    bool operator() (KeyType l_key, KeyType r_key) const;
    Orderable::Diff Compare3Way(KeyType l_key, KeyType r_key) const;
    
    void Test( const vector<KeyType> &keys );

    // Create a node here so that a regular XLink can be used and passed
    // through the sym stuff by value. Is minimus because intervals on this
    // ordering are half-open.
    class MinimusNode : public Node
    {
    public:
        NODE_FUNCTIONS_FINAL
        MinimusNode( int lacing_ordinal );
        MinimusNode(); ///< default constructor, for making archetypes 
        int GetMinimusOrdinal() const;
        BoundingRole GetRole() const { return BoundingRole::MINIMUS; }
        string GetName() const final;
    private:
        int lacing_ordinal;
    };

private:    
    shared_ptr<Lacing> lacing;
};
    
}

#endif
