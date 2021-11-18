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
    SerialNumber( const SerialNumber *serial_to_use = nullptr );
    inline SerialNumber( const SerialNumber &other ) :
        SerialNumber() // Identity semantics: ignore "other"
    {      
    }
    inline SerialNumber &operator=( const SerialNumber &other )
    {
        // Identity semantics: ignore "other"
        return *this;
    }
    inline bool operator<( const SerialNumber &o )
    {
        if( progress != o.progress )
            return progress < o.progress;
        else
            return serial < o.serial;
    }
    
public:
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
    
private:
    typedef map<const void *, SatelliteSNType> SerialBySatelliteInstance;
    // Index by location, then serial. Serial scales with input tree size so
    // use unordered_map for that.
    typedef map< SerialNumber::SNType, unordered_map< SerialNumber::SNType, SerialBySatelliteInstance > > SerialByMotherSerial;
    static SerialByMotherSerial serial_by_child_node;    

    SatelliteSNType serial;
};

#endif
