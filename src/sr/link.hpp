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

class PatternLink : public Traceable, 
                    public SatelliteSerial
{
public:
    PatternLink();
    PatternLink( shared_ptr<const Node> parent_pattern,
                 const TreePtrInterface *ppattern, 
                 void *whodat=nullptr );
    PatternLink( const Agent *parent_agent,
                 const TreePtrInterface *ppattern );
    // Make link from walk iterator; if at root (i.e. Walk::begin()) substitute root link
    static PatternLink FromWalkIterator( const Walk::iterator &wit, PatternLink root, int generations = 0 );
    // Make a copy of tp_pattern which acts as a new, distinct value 
    static PatternLink CreateDistinct( const TreePtr<Node> &tp_pattern );
    bool operator<(const PatternLink &other) const;
    bool operator!=(const PatternLink &other) const;
    bool operator==(const PatternLink &other) const;
    bool operator==(const LocatedLink &other) const;
    size_t GetHash() const noexcept;
    explicit operator bool() const;
    Agent *GetChildAgent() const;
    TreePtr<Node> GetPattern() const;
    const TreePtrInterface *GetPatternPtr() const;
    string GetTrace() const; // used for debug
    string GetName() const;
    string GetShortName() const;
    
private: friend class LocatedLink;
    PatternLink( shared_ptr<const TreePtrInterface> ppattern, 
                 void *whodat=nullptr );

    shared_ptr<const TreePtrInterface> asp_pattern;
#ifdef KEEP_WHODAT_INFO
    void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
}; 


class XLink : public Traceable, 
              public SatelliteSerial
{
public:
    XLink();
    XLink( shared_ptr<const Node> parent_x,
           const TreePtrInterface *px,
           void *whodat=nullptr );
    XLink( const LocatedLink &l );
    // Make link from walk iterator; if at root (i.e. Walk::begin()) substitute root link
    static XLink FromWalkIterator( const Walk::iterator &wit, XLink root, int generations = 0 );
    // Make a copy of tp_x which acts as a new, distinct value 
    static XLink CreateDistinct( const TreePtr<Node> &tp_x ); 
    bool operator<(const XLink &other) const;
    bool operator!=(const XLink &other) const;
    bool operator==(const XLink &other) const;
    size_t GetHash() const noexcept;    
    explicit operator bool() const;
    TreePtr<Node> GetChildX() const;
    const TreePtrInterface *GetXPtr() const;
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
    LocatedLink( const pair<const PatternLink, XLink> &p ) :
        LocatedLink(p.first, p.second) {}
    
    bool operator<(const LocatedLink &other) const;
    bool operator!=(const LocatedLink &other) const;
    bool operator==(const LocatedLink &other) const;
    size_t GetHash() const noexcept;    
    explicit operator bool() const;
    operator pair<const PatternLink, XLink>() const;
    Agent *GetChildAgent() const;
    const TreePtrInterface *GetPatternPtr() const;
    TreePtr<Node> GetChildX() const;
    void SetX( const XLink &xlink );
    void SetPattern( const PatternLink &plink );
    explicit operator PatternLink() const;
    string GetTrace() const; // used for debug

private: friend class PatternLink; friend class XLink;
    PatternLink plink;
    XLink xlink; 
};

typedef unordered_map< PatternLink, XLink > SolutionMap;

//bool operator==( const list<PatternLink> &left, const list<LocatedLink> &right );

SolutionMap MapForPattern( const list<PatternLink> &plinks, 
                           const SolutionMap &keys );
void EnsureNotOnStack( const void *p, string trace="" );

};

// The hashing functions we require in order to use links as keys to
// unordered_set, unordered_map
namespace std
{
    template<> struct hash<SR::PatternLink>
    {
        size_t operator()(const SR::PatternLink &plink) const noexcept
        {
            return plink.GetHash();
        }
    };
    template<> struct hash<SR::XLink>
    {
        size_t operator()(const SR::XLink &xlink) const noexcept
        {
            return xlink.GetHash();
        }
    };
}

#endif