#include "split_instance_declarations.hpp"
#include "tree/tree.hpp"

void SplitInstanceDeclarations::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	{ // Do initialised local variables by leaving an assign behind
		MakeTreePtr<Compound> sc;
		MakeTreePtr<LocalVariable> si;
		si->identifier = MakeTreePtr<InstanceIdentifier>();  // Only acting on initialised Instances
		si->initialiser = MakeTreePtr<Expression>();  // Only acting on initialised Instances
		MakeTreePtr< Star<Declaration> > decls;
		sc->members = ( decls );
		MakeTreePtr< Star<Statement> > pre, post;
		sc->statements = ( pre, si, post );

		MakeTreePtr<Compound> rc;
		MakeTreePtr<LocalVariable> ri;
		ri->initialiser = MakeTreePtr<Uninitialised>();
		rc->members = ( ri, decls );
		MakeTreePtr<Assign> ra;
		ra->operands = ( si->identifier, si->initialiser );
		rc->statements = ( pre, ra, post );

		CouplingSet sms1((
			Coupling((si, ri)) ));

		SearchReplace(sc, rc, sms1)( context, proot );
	}
	{ // Do everything else by just moving to the decls collection
		MakeTreePtr<Compound> sc;
		MakeTreePtr<Instance> si;
		MakeTreePtr< Star<Declaration> > decls;
		sc->members = ( decls );
		MakeTreePtr< Star<Statement> > pre, post;
		sc->statements = ( pre, si, post );

		MakeTreePtr<Compound> rc;
		MakeTreePtr<Instance> ri;
		rc->members = ( ri, decls ); // Instance now in unordered decls part
		rc->statements = ( pre, post );

		CouplingSet sms0((
			Coupling((si, ri)) ));

		SearchReplace(sc, rc, sms0)( context, proot );
	}
}

