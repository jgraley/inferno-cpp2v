#include "serial.hpp"

#include "trace.hpp"
#include "progress.hpp"

#include <cxxabi.h>
#include <stdio.h>
#include <iostream>

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

//////////////////////////// SerialNumber ///////////////////////////////

SerialNumber::SerialNumber()
{    
    progress = Progress::GetCurrent();
    
    serial = cache.master_serial_by_progress[progress];
    
    // produce a new construction serial number
    cache.master_serial_by_progress[progress]++;
    
    //printf("%p %p %s\n", __builtin_return_address(2), __builtin_return_address(3), GetSerialString().c_str());
    //std::cout << std::flush;
        
}    


string SerialNumber::GetSerialString() const
{
    string pp = progress.GetPrefix();
    return SSPrintf("#%s-%lu", pp.c_str(), serial);    
}

SerialNumber::Cache::~Cache()
{
    //FTRACE(location_readback)("\n");   
}


SerialNumber::Cache SerialNumber::cache;

//////////////////////////// SatelliteSerial ///////////////////////////////

SatelliteSerial::SatelliteSerial()
{
    serial = -1;
}


SatelliteSerial::SatelliteSerial( const SerialNumber *mother, const void *satellite )
{
    if( !mother )
    {
        serial = 0;
        return; 
    }
    pair<SerialNumber::SNType, SerialNumber::SNType> sn = mother->GetSerialNumber();
    SerialBySatelliteInstance &serial_by_link = serial_by_child_node[sn.first][sn.second];
    if( serial_by_link.count(satellite) == 0 )
    {
        serial = (SatelliteSNType)(serial_by_link.size());
        serial_by_link[satellite] = serial;
    }
    else
    {
        serial = serial_by_link.at(satellite);
    }
}


string SatelliteSerial::GetSerialString() const
{
    if( serial==-1 )
        return "NULL";
    else
        return SSPrintf("#%d", serial);
}


SatelliteSerial::SerialByMotherSerial SatelliteSerial::serial_by_child_node;    

//////////////////////////// free functions ///////////////////////////////

void DumpCounts( int min = 0 ) // for GCC
{
    FTRACEC("Dumping counts...\n");
    LeakCheck::DumpCounts(min);
}
