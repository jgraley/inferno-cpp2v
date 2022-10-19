#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"

using namespace SR;

DeleteCommand::DeleteCommand( const TreeZone &target_ ) :
	target( target_ )
{
}


void DeleteCommand::Execute( const ExecKit &kit ) const
{
	if( kit.monolithic_is_valid ) // Incrementals might depend on monolithics
    {
        kit.x_tree_db->Delete( target );    
    }
    
    if( !kit.monolithic_is_valid ) // Monolithics might depend on tree update
    {
        target.GetBase().ClearXPtr();
    }
}	


InsertCommand::InsertCommand( const TreeZone &target_, const FreeZone &new_zone_ ) :
	target( target_ ),
	new_zone( new_zone_ )
{
}


void InsertCommand::Execute( const ExecKit &kit ) const
{
    XLink base_xlink = target.GetBase();
    if( !kit.monolithic_is_valid ) // Monolithics might depend on tree update
    {
        ASSERT( !base_xlink.GetChildX() );
        base_xlink.SetXPtr( new_zone.GetBase() );
    }
    
    if( kit.monolithic_is_valid ) // Incrementals might depend on monolithics
    {
        TreeZone nt( base_xlink, new_zone );
        kit.x_tree_db->Insert( nt );      
    }
}


void CommandSequence::Add( shared_ptr<UpdateCommand> cmd )
{
	seq.push_back(cmd);
}


void CommandSequence::Execute( const ExecKit &kit ) const
{
	for( shared_ptr<UpdateCommand> cmd : seq )
		cmd->Execute(kit);
}
	
