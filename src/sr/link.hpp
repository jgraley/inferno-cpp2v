#ifndef LINK_HPP
#define LINK_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"

namespace SR
{ 
class Agent;

class PatternLink 
{
public:
    PatternLink();
    PatternLink( const TreePtrInterface *ppattern, 
                 void *whodat=nullptr );
    bool operator<(const PatternLink &other) const;
    bool operator!=(const PatternLink &other) const;
    bool operator==(const PatternLink &other) const;
    explicit operator bool() const;
    Agent *GetChildAgent() const;

private:
    const TreePtrInterface *ppattern;
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
};


class LocatedLink 
{
public:
    LocatedLink();
    LocatedLink( const TreePtrInterface *ppattern_, 
                 const TreePtr<Node> &x_,
                 void *whodat=nullptr );
    bool operator<(const LocatedLink &other) const;
    explicit operator bool() const;
    Agent *GetChildAgent() const;
    const TreePtr<Node> &GetChildX() const;
    operator PatternLink() const;

private:
    const TreePtrInterface *ppattern;
    TreePtr<Node> x; 
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
};

};

#endif