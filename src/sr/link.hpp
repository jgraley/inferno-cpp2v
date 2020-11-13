#ifndef LINK_HPP
#define LINK_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/walk.hpp"

#define KEEP_WHODAT_INFO


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
    PatternLink( const Agent *parent_agent,
                 const TreePtrInterface *ppattern );
    // Make link from walk iterator; if at root (i.e. Walk::begin()) substitute root link
    static PatternLink FromWalkIterator( const Walk::iterator &wit, PatternLink root = PatternLink() );
    // Make a copy of tp_pattern which acts as a new, distinct value 
    static PatternLink CreateDistinct( const TreePtr<Node> &tp_pattern );
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
    PatternLink( shared_ptr<const TreePtrInterface> ppattern, 
                 void *whodat=nullptr );

    shared_ptr<const TreePtrInterface> asp_pattern;
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
}; 


class XLink : public Traceable
{
public:
    XLink();
    XLink( shared_ptr<const Node> parent_x,
           const TreePtrInterface *px,
           void *whodat=nullptr );
    XLink( const LocatedLink &l );
    // Make link from walk iterator; if at root (i.e. Walk::begin()) substitute root link
    static XLink FromWalkIterator( const Walk::iterator &wit, XLink root = XLink() );
    // Make a copy of tp_x which acts as a new, distinct value 
    static XLink CreateDistinct( const TreePtr<Node> &tp_x ); 
    bool operator<(const XLink &other) const;
    bool operator!=(const XLink &other) const;
    bool operator==(const XLink &other) const;
    explicit operator bool() const;
    TreePtr<Node> GetChildX() const;
    string GetTrace() const; // used for debug

private: friend class LocatedLink;
    XLink( shared_ptr<const TreePtrInterface> px,
           void *whodat=nullptr );
           
    shared_ptr<const TreePtrInterface> asp_x;
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif

    struct MMAX : virtual Node { NODE_FUNCTIONS_FINAL }; 
    
public:
    // Magic Match Anything X node: 
    static const XLink MMAX_Link;
};


class LocatedLink : public Traceable
{
public:
    LocatedLink();
    explicit LocatedLink( const PatternLink &plink, 
                          const XLink &xlink);
    explicit LocatedLink( const pair<PatternLink, XLink> &p ) :
        LocatedLink(p.first, p.second) {}
    
    bool operator<(const LocatedLink &other) const;
    bool operator!=(const LocatedLink &other) const;
    bool operator==(const LocatedLink &other) const;
    explicit operator bool() const;
    operator pair<const PatternLink, XLink>() const;
    Agent *GetChildAgent() const;
    const TreePtrInterface *GetPatternPtr() const;
    TreePtr<Node> GetChildX() const;
    void SetX( const XLink &x );
    void SetPattern( const PatternLink &pattern );
    explicit operator PatternLink() const;
    string GetTrace() const; // used for debug

private: friend class PatternLink; friend class XLink;
    PatternLink plink;
    XLink xlink; 
};

//bool operator==( const list<PatternLink> &left, const list<LocatedLink> &right );

list<LocatedLink> LocateLinksFromMap( const list<PatternLink> &plinks, 
                                      const map< PatternLink, XLink > &keys );
void EnsureNotOnStack( const void *p, string trace="" );

typedef XLink CouplingKey;
typedef map< Agent *, CouplingKey > CouplingKeysMap;

};

#endif