#ifndef LINK_HPP
#define LINK_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"

#define LINKS_ENHANCED_TRACE

namespace SR
{ 
class Agent;
class LocatedLink;

class PatternLink : public Traceable
{
public:
    PatternLink();
    PatternLink( const Agent *parent_pattern,
                 const TreePtrInterface *ppattern, 
                 void *whodat=nullptr );
    bool operator<(const PatternLink &other) const;
    bool operator!=(const PatternLink &other) const;
    bool operator==(const PatternLink &other) const;
    bool operator==(const LocatedLink &other) const;
    explicit operator bool() const;
    Agent *GetChildAgent() const;
    const TreePtrInterface *GetPatternPtr() const;
    string GetTrace() const; // used for debug
    
private: friend class LocatedLink;
#ifdef LINKS_ENHANCED_TRACE
    const Agent *parent_pattern;
#endif    
    const TreePtrInterface *ppattern;
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
};


class LocatedLink : public Traceable
{
public:
    LocatedLink();
    LocatedLink( const Agent *parent_pattern,
                 const TreePtrInterface *ppattern_, 
                 const TreePtr<Node> &x_,
                 void *whodat=nullptr );
    LocatedLink( const PatternLink &plink, 
                 const TreePtr<Node> &x_);
    bool operator<(const LocatedLink &other) const;
    bool operator!=(const LocatedLink &other) const;
    bool operator==(const LocatedLink &other) const;
    explicit operator bool() const;
    Agent *GetChildAgent() const;
    const TreePtrInterface *GetPatternPtr() const;
    const TreePtr<Node> &GetChildX() const;
    operator PatternLink() const;
    string GetTrace() const; // used for debug

private: friend class PatternLink;
#ifdef LINKS_ENHANCED_TRACE
    const Agent *parent_pattern;
#endif
    const TreePtrInterface *ppattern;
    TreePtr<Node> x; 
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
};

bool operator==( const list<PatternLink> &left, const list<LocatedLink> &right );

list<LocatedLink> LocateLinksFromMap( const list<PatternLink> &plinks, 
                                      const map< Agent *, TreePtr<Node> > &mappy );
};

#endif