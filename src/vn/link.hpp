#ifndef LINK_HPP
#define LINK_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/walk.hpp"

//#define KEEP_WHODAT_INFO
// Note: you may wish to try subtracting 1 or 8 or something 
// from the address to get a better indication of where the 
// call itself came from.
// The gdb magic you require is eg "info line *b.whodat"

#ifdef KEEP_WHODAT_INFO
#define WHODAT() RETURN_ADDR()
#else
#define WHODAT() nullptr
#endif

namespace VN
{ 
class Agent;
class LocatedLink;
class XLink;

class PatternLink : public Traceable
{
public:
	typedef vector<void *> Whodat;

    PatternLink();
    ~PatternLink();
    PatternLink(const PatternLink &other);
    PatternLink &operator=(const PatternLink &other);
    PatternLink( const TreePtrInterface *ppattern, 
                 Whodat whodat_ = Whodat() );
    bool operator<(const PatternLink &other) const;
    bool operator==(const PatternLink &other) const;
    bool operator==(const LocatedLink &other) const;
    static Orderable::Diff Compare3Way(const PatternLink &l, const PatternLink &r);
    size_t GetHash() const noexcept;
    explicit operator bool() const;
    Agent *GetChildAgent() const;
    TreePtr<Node> GetPattern() const;
    const TreePtrInterface *GetPatternTreePtr() const;
    void Redirect( const TreePtrInterface &new_parent_pattern );
	
    string GetTrace() const; // used for debug
    string GetName() const;
    string GetShortName() const;
    
private:
    const TreePtrInterface *p_tpi;
#ifdef KEEP_WHODAT_INFO
    vector<void *> whodat; 
#endif
}; 


class XLink : public Traceable
{
public:
	typedef vector<void *> Whodat;

    XLink();
    virtual ~XLink();
    XLink(const XLink &other);
    XLink &operator=(const XLink &other);
    XLink( const TreePtrInterface *p_tpi,
           Whodat whodat_ = Whodat() );
    XLink( const LocatedLink &l );
   
    // Make a copy of tp_x which acts as a new, distinct value 
    static XLink CreateFrom( const TreePtrInterface *p_tpi,
                             Whodat whodat_ = Whodat() );
    size_t GetHash() const noexcept;    
    explicit operator bool() const;
    bool HasChildX() const;
    TreePtr<Node> GetChildTreePtr() const;
    const TreePtrInterface *GetTreePtrInterface() const;
    
#ifdef KEEP_WHODAT_INFO
	Whodat GetWhodat() const;
#endif	

    string GetTrace() const; // used for debug
    string GetName() const;
    string GetShortName() const;

    inline bool operator<(const XLink &r) const
    {
        return p_tpi < r.p_tpi;  
    }
        
    inline bool operator==(const XLink &r) const
    {
        return p_tpi == r.p_tpi;    
    }

private:
    const TreePtrInterface *p_tpi;
	
    // So .get() will return const TreePtrInterface*
    
#ifdef KEEP_WHODAT_INFO
    Whodat whodat; 
#endif

    struct MMAXNodeType : virtual Node { NODE_FUNCTIONS_FINAL }; 
    struct OffEndNodeType : virtual Node { NODE_FUNCTIONS_FINAL }; 
    
public:
    // Magic Match Anything Xlink: 
    static const TreePtr<MMAXNodeType> MMAXNode;
    static const XLink MMAX;

    // Gone off the end
    static const TreePtr<OffEndNodeType> OffEndNode;
    static const XLink OffEnd;
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
    bool operator==(const LocatedLink &other) const;
    size_t GetHash() const noexcept;    
    explicit operator bool() const;
    operator pair<const PatternLink, XLink>() const;
    Agent *GetChildAgent() const;
    const TreePtrInterface *GetPatternTreePtr() const;
    TreePtr<Node> GetChildTreePtr() const;
    void SetX( const XLink &xlink );
    void SetPattern( const PatternLink &plink );
    explicit operator PatternLink() const;
    
    string GetTrace() const; // used for debug
    string GetName() const;
    string GetShortName() const;

private: friend class PatternLink; friend class XLink;
    PatternLink plink;
    XLink xlink; 
};

typedef map< PatternLink, XLink > SolutionMap;

//bool operator==( const list<PatternLink> &left, const list<LocatedLink> &right );

SolutionMap MapForPattern( const list<PatternLink> &plinks, 
                           const SolutionMap &keys );
void EnsureNotOnStack( const void *p, string trace="" );

};

// The hashing functions we require in order to use links as keys to
// unordered_set, unordered_map
namespace std
{
    template<> struct hash<VN::PatternLink>
    {
        size_t operator()(const VN::PatternLink &plink) const noexcept
        {
            return plink.GetHash();
        }
    };
    template<> struct hash<VN::XLink>
    {
        size_t operator()(const VN::XLink &xlink) const noexcept
        {
            return xlink.GetHash();
        }
    };
}

#endif
