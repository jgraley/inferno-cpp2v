#include "serial.hpp"

#include "trace.hpp"
#include "progress.hpp"

#include <cxxabi.h>
#include <stdio.h>

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

SerialNumber::SerialNumber( bool use_location_, const SerialNumber *serial_to_use ) :
    use_location( use_location_ )
{    
    progress = Progress::GetCurrent();
    
    if( serial_to_use )
    {
        // Serial number supplied to constructor (explicitly), take
        // location and serial but not progress.
        serial = serial_to_use->serial;
        location = serial_to_use->location;
        return;
    }
    
    if( use_location )
    {
        // Get the point in the code where we were constructed 
        void *lp = __builtin_return_address(1); 
        
        // See if we know about this location
        if( location_serial.count(lp) == 0 )
        {        
            // We don't know about this location, so produce a new location 
            // serial number and start the construction count 
            location = master_location_serial;
            master_location_serial++;
            
            location_serial[lp] = location;
            location_readback[location] = lp;
            //TRACE("%p assigned serial -%lu-\n", lp, location);

            master_serial_by_location[location] = 0;
        }
        else
        {
            location = location_serial.at(lp);
        }
        
        serial = master_serial_by_location.at(location);
        
        // produce a new construction serial number
        master_serial_by_location[location]++;
    }
    else
    {
        location = 0;
        serial = master_serial_by_step[progress.GetStep()];
        
        // produce a new construction serial number
        master_serial_by_step[progress.GetStep()]++;
    }
        
}    


void *SerialNumber::GetLocation( SNType location )
{
    return location_readback.at(location);
}


string SerialNumber::GetSerialString() const
{
    string pp = progress.GetPrefix();
    if( use_location )
        return SSPrintf("#%s-%lu-%lu", pp.c_str(), location, serial);
    else
        return SSPrintf("#%s-%lu", pp.c_str(), serial);    
}


SerialNumber::SNType SerialNumber::master_location_serial = 1;
map<void *, SerialNumber::SNType> SerialNumber::location_serial;
map<SerialNumber::SNType, void *> SerialNumber::location_readback;
map<SerialNumber::SNType, SerialNumber::SNType> SerialNumber::master_serial_by_location;
map<int, SerialNumber::SNType> SerialNumber::master_serial_by_step;

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

void *GetLocation(SerialNumber::SNType location) // for GCC
{
    return SerialNumber::GetLocation(location);
}

void DumpCounts( int min = 0 ) // for GCC
{
    FTRACEC("Dumping counts...\n");
    LeakCheck::DumpCounts(min);
}
