#include "tree_update.hpp"

#include "x_tree_database.hpp"

using namespace SR;

DeleteCommand::DeleteCommand( XLink target_xlink_ ) :
	target_xlink( target_xlink_ )
{
}


void DeleteCommand::Execute( const ExecKit &kit ) const
{
	// TODO only delete subtree under target_xlink
	kit.x_tree_db->Clear();    
    
    target_xlink.ClearXPtr();
}	


InsertCommand::InsertCommand( XLink target_xlink_, TreePtr<Node> new_x_, PatternLink base_plink_ ) :
	target_xlink( target_xlink_ ),
	new_x( new_x_ ),
	base_plink( base_plink_ )
{
}


void InsertCommand::Execute( const ExecKit &kit ) const
{
	target_xlink.SetXPtr(new_x);
        
	// TODO only build new subtree
    kit.x_tree_db->Build( kit.root_xlink );      
    // Domain extend required on sight of new pattern OR x. This call 
    // is due to the change in X tree.
    kit.x_tree_db->ExtendDomain( base_plink ); 
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
	
