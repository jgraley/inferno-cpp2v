#ifndef VALUE_SELECTOR_HPP
#define VALUE_SELECTOR_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <functional>

namespace VN
{
    class XTreeDatabase;
};

namespace CSP
{ 
    
class Agent;
class SolverHolder;
typedef set<shared_ptr<Constraint>> ConstraintSet;

class ValueSelector : public Traceable
{
public:
    ValueSelector( const ConstraintSet &constraints_to_query,
                   const VN::XTreeDatabase *x_tree_db_,
                   Assignments &assignments_,
                   VariableId var );
    ~ValueSelector();
    void SetupDefaultGenerator();
    void SetupSuggestionGenerator( shared_ptr<set<Value>> s );
    Value GetNextValue();
    
private:
    const VN::XTreeDatabase * const x_tree_db;
    Assignments &assignments;
    const VariableId my_var;
    const ConstraintSet &constraints_to_query;
    
    function<Value()> values_generator;  

public:
    static void DumpGSV();
    
private:
    static uint64_t gsv_n;
    static uint64_t gsv_nfail;
    static uint64_t gsv_nempty;
    static uint64_t gsv_tot;    
};
    
};

#endif
