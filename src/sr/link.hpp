#ifndef LINK_HPP
#define LINK_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"

#define LINKS_ENHANCED_TRACE

namespace SR
{ 
class Agent;
class LocatedLink;
class XLink;

class PatternLink : public Traceable
{
public:
    PatternLink();
    PatternLink( shared_ptr<const Node> parent_pattern,
                 const TreePtrInterface *ppattern, 
                 void *whodat=nullptr );
    PatternLink( shared_ptr<const TreePtrInterface> ppattern, 
                 void *whodat=nullptr );
    bool operator<(const PatternLink &other) const;
    bool operator!=(const PatternLink &other) const;
    bool operator==(const PatternLink &other) const;
    bool operator==(const LocatedLink &other) const;
    explicit operator bool() const;
    Agent *GetChildAgent() const;
    TreePtr<Node> GetPattern() const;
    const TreePtrInterface *GetPatternPtr() const;
    string GetTrace() const; // used for debug
    
private: friend class LocatedLink;
    shared_ptr<const TreePtrInterface> asp_pattern;
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
};


class XLink : public Traceable
{
public:
    XLink();
    XLink( const TreePtr<Node> &x_,
           void *whodat=nullptr );
    XLink( const LocatedLink &l );
    bool operator<(const XLink &other) const;
    bool operator!=(const XLink &other) const;
    bool operator==(const XLink &other) const;
    explicit operator bool() const;
    const TreePtr<Node> &GetChildX() const;
    string GetTrace() const; // used for debug

private: friend class LocatedLink;
    TreePtr<Node> x; 
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
};

typedef XLink XLinkMultiplicity;


class LocatedLink : public Traceable
{
public:
    LocatedLink();
    LocatedLink( shared_ptr<const Node> parent_pattern,
                 const TreePtrInterface *ppattern, 
                 const TreePtr<Node> &x_,
                 void *whodat=nullptr );
    LocatedLink( const PatternLink &plink, 
                 const TreePtr<Node> &x_);
    LocatedLink( const pair<PatternLink, TreePtr<Node>> &p ) :
        LocatedLink(p.first, p.second) {}
    LocatedLink( shared_ptr<const Node> parent_pattern,
                 const TreePtrInterface *ppattern, 
                 const XLink &xlink );
    LocatedLink( const PatternLink &plink, 
                 const XLink &xlink);
    bool operator<(const LocatedLink &other) const;
    bool operator!=(const LocatedLink &other) const;
    bool operator==(const LocatedLink &other) const;
    explicit operator bool() const;
    operator pair<const PatternLink, TreePtr<Node>>() const;
    Agent *GetChildAgent() const;
    const TreePtrInterface *GetPatternPtr() const;
    const TreePtr<Node> &GetChildX() const;
    operator PatternLink() const;
    string GetTrace() const; // used for debug

private: friend class PatternLink; friend class XLink;
    PatternLink plink;
    TreePtr<Node> x; 
};

bool operator==( const list<PatternLink> &left, const list<LocatedLink> &right );

list<LocatedLink> LocateLinksFromMap( const list<PatternLink> &plinks, 
                                      const map< PatternLink, TreePtr<Node> > &keys );
};

#endif