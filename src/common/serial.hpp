#ifndef SERIAL_HPP
#define SERIAL_HPP

#include "progress.hpp"

#include <map>
#include <bits/stdint-uintn.h>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>

using namespace std;


struct PairHash
{
    template<class T1, class T2>
    size_t operator()( const pair<T1,T2> &p ) const 
    {
        auto h1 = hash<T1>{}(p.first);
        auto h2 = hash<T2>{}(p.second);
        return h1 ^ h2;  
    }
};


class LeakCheck
{
public:
    inline LeakCheck() :
        origin( GetOrigin() )
    { 
        Construct();       
    }
    inline LeakCheck( const LeakCheck &other ) :
        origin( GetOrigin() )
    {
        // Identity semantics: ignore "other"
        Construct();        
    }
    ~LeakCheck();
    inline LeakCheck &operator=( const LeakCheck &other )
    {
        // Identity semantics: ignore "other"
        return *this;
    }
    void Construct();
    static void DumpCounts( int min );
    
private:    
    typedef vector<void *> Origin;
    struct Block
    {
        int count;
        set<Origin> destructs;
    };
    static Origin GetOrigin();
    const Origin origin;
    static map<Origin, Block> instance_counts;
};

//
// The idea is to provide an alternative to raw pointers for ordering our sets, miltsets etc.
// This should be more repeatable - i.e. a slight disturbance to the dynamic memory allocator
// should not cause everything to come out in a different order.
//
// We construct the new ordering as follows:
// 1st: the step the object was constructed during (as specified by the top level)
// 2nd: the location in the code where the object was constructed, foced down into 
//      a sequential ordering to protect against changes in the location of code
// 3rd: the count of objects constructed at that point.
//
// Note the 2nd criterion should differentiate between different object types, so 
// no further action needed there.
//
class SerialNumber
{
public:
    typedef uint64_t SNType;
    
protected:
    SerialNumber();
    inline SerialNumber( const SerialNumber &other ) :
        SerialNumber() // Identity semantics: ignore "other"
    {      
    }
    inline SerialNumber &operator=( const SerialNumber &other )
    {
        // Identity semantics: ignore "other"
        return *this;
    }

public:
    bool operator<( const SerialNumber &o ) const
    {
        if( progress != o.progress )
            return progress < o.progress;
        else
            return serial < o.serial;
    }
    
    inline pair<SNType, SNType> GetSerialNumber() const 
    {
        return make_pair(0, serial); // This is enough for uniqueness
    }
    string GetSerialString() const; 
    
private:    
    SNType serial;
    Progress progress;

    static struct Cache
    {
        ~Cache();

        map<Progress, SNType> master_serial_by_progress;
    } cache;
};


class SatelliteSerial
{
public:
    typedef int SatelliteSNType;
    
    SatelliteSerial();
    explicit SatelliteSerial( const SerialNumber *mother, const void *satellite );

    string GetSerialString() const;
    bool operator<( const SatelliteSerial &other ) const;
    void Redetermine( const SerialNumber *mother, const void *satellite );
    
private:
    static SatelliteSNType DetermineSerialNumber( const SerialNumber *mother, const void *satellite );

    struct MotherBlock
    {
        map<const void *, SatelliteSNType> serial_by_satellite;
        int next_serial=0;
    };
    // Index by location, then serial. Serial scales with input tree size so
    // use unordered_map for that.
    typedef unordered_map< pair<SerialNumber::SNType, SerialNumber::SNType>, 
                           MotherBlock, 
                           PairHash > BlocksByMotherSerial;
    static BlocksByMotherSerial blocks_by_mother_serial;    

    SatelliteSNType serial;
};

#endif
