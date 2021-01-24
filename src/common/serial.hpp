#ifndef SERIAL_HPP
#define SERIAL_HPP

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
    
private:    
    static SNType master_location_serial;
    static int current_step;
    static map<void *, SNType> location_serial;
    static map<SNType, void *> location_readback;
    static map<void *, SNType> master_serial;
    SNType serial;
    SNType location;
    SNType step;

protected:
    void Construct();
    inline SerialNumber() 
    { 
        Construct();       
    }
    inline SerialNumber( const SerialNumber &other ) 
    {
        // Identity semantics: ignore "other"
        Construct();        
    }
    inline SerialNumber &operator=( const SerialNumber &other )
    {
        // Identity semantics: ignore "other"
        return *this;
    }
    inline bool operator<( const SerialNumber &o )
    {
        if( step != o.step )
            return step < o.step;
        else if( location < o.location )
            return location < o.location;
        else
            return serial < o.serial;
    }
public:
    static void SetStep( int s );
    static void *GetLocation( SNType location );

    inline pair<SNType, SNType> GetSerialNumber() const 
    {
        return make_pair(location, serial); // This is enough for uniqueness
    }
    string GetSerialString() const; 
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
