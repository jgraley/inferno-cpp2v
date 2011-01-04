#include "split_instance_declarations.hpp"
#include "tree/tree.hpp"

void SplitInstanceDeclarations::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	{ // Do initialised local variables by leaving an assign behind
		MakeTreePtr<Compound> sc;
		MakeTreePtr<LocalVariable> si;
        MakeTreePtr< Overlay<LocalVariable> > over;
		si->identifier = MakeTreePtr<InstanceIdentifier>();  // Only acting on initialised Instances
		si->initialiser = MakeTreePtr<Expression>();  // Only acting on initialised Instances
		MakeTreePtr< Star<Declaration> > decls;
		sc->members = ( decls );
		MakeTreePtr< Star<Statement> > pre, post;
		sc->statements = ( pre, over, post );

		MakeTreePtr<Compound> rc;
		MakeTreePtr<LocalVariable> ri;
        over->base = si;
        over->overlay = ri;
		ri->initialiser = MakeTreePtr<Uninitialised>();
		rc->members = ( over, decls );
		MakeTreePtr<Assign> ra;
		ra->operands = ( si->identifier, si->initialiser );
		rc->statements = ( pre, ra, post );

		SearchReplace(sc, rc)( context, proot );
	}
	{ // Do everything else by just moving to the decls collection
		MakeTreePtr<Compound> sc;
		MakeTreePtr<Instance> si;
        MakeTreePtr< Overlay<LocalVariable> > over;
		MakeTreePtr< Star<Declaration> > decls;
		sc->members = ( decls );
		MakeTreePtr< Star<Statement> > pre, post;
		sc->statements = ( pre, over, post );

		MakeTreePtr<Compound> rc;
		MakeTreePtr<Instance> ri;
        over->base = si;
        over->overlay = ri;
		rc->members = ( over, decls ); // Instance now in unordered decls part
		rc->statements = ( pre, post );

		SearchReplace(sc, rc)( context, proot );
	}
}

