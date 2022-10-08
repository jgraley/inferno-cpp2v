#ifndef SERIAL_HPP
#define SERIAL_HPP

#include "progress.hpp"
#include "orderable.hpp"

#include <map>
#include <bits/stdint-uintn.h>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>
#include <memory>

using namespace std;

//////////////////////////// SerialNumber ///////////////////////////////
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
    struct Hook
    {
		virtual ~Hook() {};
	};
    
protected:
    SerialNumber();
    SerialNumber( const SerialNumber &other );
    SerialNumber &operator=( const SerialNumber &other );

public:
    static Orderable::Diff Compare3WayIdentity(const SerialNumber &l, const SerialNumber &r);

    inline SNType GetSerialNumber() const;
    string GetSerialString() const; 
	void SetHook(shared_ptr<Hook> h) const;
	bool HasHook() const;
	shared_ptr<Hook> GetHook() const;
	
private:    
    SNType serial;
    Progress progress;

    static struct Cache
    {
        ~Cache();
        map<Progress, SNType> main_serial_by_progress;        
    } cache;
    
    mutable shared_ptr<Hook> hook;
};

//////////////////////////// SatelliteSerial ///////////////////////////////

class SatelliteSerial
{
public:
    typedef int SatelliteSNType;
    
    SatelliteSerial();
    SatelliteSerial( const SatelliteSerial &other );
    SatelliteSerial &operator=( const SatelliteSerial &other );
    explicit SatelliteSerial( const SerialNumber *mother, const void *satellite );

    string GetSerialString() const;
    static Orderable::Diff Compare3WayIdentity(const SatelliteSerial &l, const SatelliteSerial &r);
        
private:
	// These are hooked to the mother SerialNumber instance
    struct MotherBlock : SerialNumber::Hook
    {
        int next_serial=0;
        int AssignSerial(const SatelliteSerial *ss_for_trace);
    };  

    static shared_ptr<MotherBlock> GetMotherBlock( const SerialNumber *mother, const void *satellite );

	// Ordinary pointer is OK as long as we keep mother object alive, which
	// we do if used with TreePtr
	MotherBlock *p_mother_block;
    SatelliteSNType serial;
};

//////////////////////////// LeakCheck ///////////////////////////////

class LeakCheck
{
public:
    LeakCheck();
    LeakCheck( const LeakCheck &other );
    ~LeakCheck();
    LeakCheck &operator=( const LeakCheck &other );
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


#endif
