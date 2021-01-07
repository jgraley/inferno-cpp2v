#include "serial.hpp"
#include "trace.hpp"
#include <cxxabi.h>
#include <stdio.h>

//////////////////////////// SerialNumber ///////////////////////////////

SerialNumber::SNType SerialNumber::master_location_serial;
int SerialNumber::current_step;
map<void *, SerialNumber::SNType> SerialNumber::location_serial;
map<SerialNumber::SNType, void *> SerialNumber::location_readback;
map<void *, SerialNumber::SNType> SerialNumber::master_serial;


void SerialNumber::Construct()
{
    // Get the point in the code where we were constructed 
    void *lp = __builtin_return_address(1); 
    
    // See if we know about this location
    map<void *, SNType>::iterator it = location_serial.find(lp);
    if( it == location_serial.end() )
    {
        // We don't know about this location, so produce a new location serial number and start the construction count 
        location_serial.insert( pair<void *, SerialNumber::SNType>(lp, master_location_serial) );
        location_readback[master_location_serial] = lp;
        master_serial.insert( pair<void *, SerialNumber::SNType>(lp, 0) );
        master_location_serial++;
    }
        
    // Remember values for this object
    serial = master_serial[lp];
    location = location_serial[lp];
    step = current_step;
    
    // produce a new construction serial number
    master_serial[lp]++;
}    


void SerialNumber::SetStep( int s )
{
    current_step = s;
    // Just bin the structures we built up - this forces step to be primary ordering
    location_serial = map<void *, SNType>();
    master_serial = map<void *, SNType>();
}


void *SerialNumber::GetLocation( SNType location )
{
    return location_readback.at(location);
}


string SerialNumber::GetSerialString() const
{
    string ss;
    switch( step )
    {
        case -3: // inputting
            ss = SSPrintf("#I-%lu-%lu", location, serial);  
            break;
        case -2: // outputting
            ss = SSPrintf("#O-%lu-%lu", location, serial);  
            break;
        case -1: // planning
            ss = SSPrintf("#P-%lu-%lu", location, serial);  
            break;
        default: // during a step
            ss = SSPrintf("#%d-%lu-%lu", step, location, serial);  
            break;
    }
    return ss;
}


void *location(SerialNumber::SNType location) // for GCC
{
    return SerialNumber::GetLocation(location);
}

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
