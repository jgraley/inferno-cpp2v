#include "serial.hpp"

#include "trace.hpp"
#include "progress.hpp"

#include <cxxabi.h>
#include <stdio.h>
#include <iostream>

#define USE_HOOK

//////////////////////////// SerialNumber ///////////////////////////////

SerialNumber::SerialNumber()
{    
    progress = Progress::GetCurrent();    
    serial = cache.master_serial_by_progress[progress];
    
    // produce a new construction serial number
    cache.master_serial_by_progress[progress]++;      
}    


SerialNumber::SerialNumber( const SerialNumber &other ) :
	SerialNumber() // Identity semantics: ignore "other"
{      
}


inline SerialNumber &SerialNumber::operator=( const SerialNumber &other )
{
	// Identity semantics: ignore "other"
	return *this;
}


bool SerialNumber::operator<( const SerialNumber &o ) const
{
	if( progress != o.progress )
		return progress < o.progress;
	else
		return serial < o.serial;
}


SerialNumber::SNType SerialNumber::GetSerialNumber() const 
{
	return serial; // This is enough for uniqueness
}


string SerialNumber::GetSerialString() const
{
    string pp = progress.GetPrefix();
    return SSPrintf("#%s-%lu", pp.c_str(), serial);    
}


void SerialNumber::SetHook(shared_ptr<Hook> h) const
{
	hook = h;
}


bool SerialNumber::HasHook() const
{
	return (bool)hook;
}


shared_ptr<SerialNumber::Hook> SerialNumber::GetHook() const
{
	return hook;
}


SerialNumber::Cache::~Cache()
{
    //FTRACE(location_readback)("\n");   
}


SerialNumber::Cache SerialNumber::cache;

//////////////////////////// SatelliteSerial ///////////////////////////////

SatelliteSerial::SatelliteSerial() :
    serial( -1 )
{
}


SatelliteSerial::SatelliteSerial( const SerialNumber *mother, const void *satellite ) :
    serial( DetermineSerialNumber( mother, satellite ) )
{
}


string SatelliteSerial::GetSerialString() const
{
    if( serial==-1 )
        return "NULL";
    else
        return SSPrintf("#%d", serial);
}


bool SatelliteSerial::operator<( const SatelliteSerial &other ) const
{
    return serial < other.serial;
}


void SatelliteSerial::Redetermine( const SerialNumber *mother, const void *satellite )
{
    serial = DetermineSerialNumber( mother, satellite );
}


SatelliteSerial::SatelliteSNType SatelliteSerial::DetermineSerialNumber( const SerialNumber *mother, const void *satellite )
{
    if( !mother )
        return 0;
    
	shared_ptr<MotherBlock> mother_block;
	if( mother->HasHook() )
	{
		mother_block = dynamic_pointer_cast<MotherBlock>( mother->GetHook() );
		ASSERT( mother_block ); // Will be NULL if wrong type of block on hook
	}
	else
	{
		mother_block = make_shared<MotherBlock>();
		mother->SetHook(mother_block); 
	}

    if( mother_block->serial_by_satellite.count(satellite) == 0 )
    {
        int ns = mother_block->next_serial;
        mother_block->next_serial++;
        mother_block->serial_by_satellite[satellite] = ns;
        return ns;
    }
    else
    {
        return mother_block->serial_by_satellite.at(satellite);
    }
}

//////////////////////////// LeakCheck ///////////////////////////////

LeakCheck::~LeakCheck()
{
    instance_counts.at(origin).count--;
    instance_counts.at(origin).destructs.insert( GetOrigin() );
}


void LeakCheck::Construct()
{
    if( instance_counts.count(origin)==0 )
        instance_counts[origin] = {0};
    instance_counts.at(origin).count++;
}


void LeakCheck::DumpCounts( int min )
{
    FTRACEC("LeakCheck:\n");
    for( auto p : instance_counts )  
        if( p.second.count >= min )
            FTRACEC(p.first)
                   (" %llu instances\n", p.second.count)
                   ("destructs: ")(p.second.destructs)("\n");
}

#define TRY(SEQ, LEVEL) \
    if( !ON_STACK( __builtin_frame_address(LEVEL) ) ) \
        break; \
    SEQ.push_back( __builtin_extract_return_addr(__builtin_return_address(LEVEL)) ); 
 

LeakCheck::Origin LeakCheck::GetOrigin()
{
    Origin o;
    do 
    {
        TRY(o, 1)
        TRY(o, 2)
        TRY(o, 3)
        TRY(o, 4)
        TRY(o, 5)
    } while(0);
    return o;
}

#undef TRY


map<LeakCheck::Origin, LeakCheck::Block> LeakCheck::instance_counts;


void DumpCounts( int min = 0 ) // for GCC
{
    FTRACEC("Dumping counts...\n");
    LeakCheck::DumpCounts(min);
}


