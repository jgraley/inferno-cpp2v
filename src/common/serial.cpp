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
    serial = cache.main_serial_by_progress[progress];
    
    // produce a new construction serial number
    cache.main_serial_by_progress[progress]++;      
}    


SerialNumber::SerialNumber( const SerialNumber & ) :
    SerialNumber() // Identity semantics: ignore "other"
{      
}


inline SerialNumber &SerialNumber::operator=( const SerialNumber & )
{
    // Identity semantics: ignore "other"
    return *this;
}

/*
Orderable::Diff SerialNumber::Compare3WayIdentity(const SerialNumber &l, const SerialNumber &r)
{
    if( l.progress != r.progress )
        return Progress::Compare3Way(l.progress, r.progress);
        
    return l.serial - r.serial;
}
*/

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
    p_mother_block( nullptr ),
    serial( -1 )
{
}


SatelliteSerial::SatelliteSerial( const SerialNumber *mother, const void *satellite ) :
    p_mother_block( GetMotherBlock(mother, satellite).get() ),
    serial( p_mother_block ? p_mother_block->AssignSerial(this) : -1 )
{
}


SatelliteSerial::SatelliteSerial( const SatelliteSerial &other ) :
    p_mother_block( other.p_mother_block ),
    serial( p_mother_block ? p_mother_block->AssignSerial(this) : -1 )
{            
}


SatelliteSerial &SatelliteSerial::operator=( const SatelliteSerial &other )
{
    p_mother_block = other.p_mother_block;
    serial = p_mother_block ? p_mother_block->AssignSerial(this) : -1;
    return *this;
}
    
    
string SatelliteSerial::GetSerialString() const
{
    if( serial==-1 )
        return "#?";
    else
        return SSPrintf("#%d", serial);
}

/*
Orderable::Diff SatelliteSerial::Compare3WayIdentity(const SatelliteSerial &l, const SatelliteSerial &r)
{
    Orderable::Diff d = l.serial - r.serial;

    // Check that we're really getting an identity relation
    if( d==0 )
        ASSERTS( &l == &r )
               ("l=")(l.GetSerialString())(" at %p mb=%p\n", &l, l.p_mother_block)
               ("r=")(r.GetSerialString())(" at %p mb=%p\n", &r, r.p_mother_block);
    else
        ASSERTS( &l != &r )
               ("l=")(l.GetSerialString())(" at %p mb=%p\n", &l, l.p_mother_block)
               ("r=")(r.GetSerialString())(" at %p mb=%p\n", &r, r.p_mother_block);
    return d;
}
*/

int SatelliteSerial::MotherBlock::AssignSerial(const SatelliteSerial *)
{
    int ns = next_serial;
    next_serial++;
    return ns;
}


shared_ptr<SatelliteSerial::MotherBlock> SatelliteSerial::GetMotherBlock( const SerialNumber *mother, const void * )
{
    if( !mother )
    {
        return nullptr;
    }
    
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
    
    return mother_block;
}


//////////////////////////// LeakCheck ///////////////////////////////

LeakCheck::LeakCheck() :
    origin( GetOrigin() )
{ 
    Construct();       
}


LeakCheck::LeakCheck( const LeakCheck & ) :
    origin( GetOrigin() )
{
    // Identity semantics: ignore "other"
    Construct();        
}


LeakCheck::~LeakCheck()
{
    instance_counts.at(origin).count--;
    instance_counts.at(origin).destructs.insert( GetOrigin() );
}


LeakCheck &LeakCheck::operator=( const LeakCheck &)
{
    // Identity semantics: ignore "other"
    return *this;
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
 

LeakCheck::Origin LeakCheck::GetOrigin()
{
    Origin o;
    // This was doing __builtin_frame_address() with non-zero argument, but this is not now allowed under -Wall
    return o;
}


map<LeakCheck::Origin, LeakCheck::Block> LeakCheck::instance_counts;


void DumpCounts( int min = 0 ) // for GCC
{
    FTRACEC("Dumping counts...\n");
    LeakCheck::DumpCounts(min);
}


