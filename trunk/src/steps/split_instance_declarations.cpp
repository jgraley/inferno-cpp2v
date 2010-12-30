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


void MergeInstanceDeclarations::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	{
		// This is the hard kind of search pattern where Stars exist in two
		// separate containers and have a coupling between them
		MakeTreePtr<Compound> rc;
		MakeTreePtr<LocalVariable> ri;
		ri->identifier = MakeTreePtr<InstanceIdentifier>();
		ri->initialiser = MakeTreePtr<Uninitialised>();
		MakeTreePtr< Star<Declaration> > rs;
		rc->members = ( ri, rs );
		MakeTreePtr<Assign> ra;
		ra->operands = (MakeTreePtr<InstanceIdentifier>(), MakeTreePtr<Expression>() );
		rc->statements = (MakeTreePtr< Star<Statement> >(), ra, MakeTreePtr< Star<Statement> >() );

		MakeTreePtr<Compound> sc;
		MakeTreePtr< Star<Declaration> > ss;
		sc->members = ( ss );
		MakeTreePtr<LocalVariable> si;
		si->identifier = MakeTreePtr<InstanceIdentifier>();
		si->initialiser = MakeTreePtr<Expression>();  // Only acting on initialised Instances
		sc->statements = ( MakeTreePtr< Star<Statement> >(), si, MakeTreePtr< Star<Statement> >() );

		CouplingSet sms1((
			Coupling((si, ri)),
			Coupling((ss, rs)),
			Coupling((sc->statements[0], rc->statements[0])),
			Coupling((sc->statements[2], rc->statements[2])),
			Coupling((si->identifier, ri->identifier, ra->operands[0])),
			Coupling((si->initialiser, ra->operands[1])) ));

		SearchReplace(rc, sc, sms1)( context, proot );
	}
}


void HackUpIfs::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	{
		MakeTreePtr<If> sif;
		MakeTreePtr<Expression> stest;
		sif->condition = stest;
		MakeTreePtr< Stuff<Statement> > ssthen;
		sif->body = ssthen;
		ssthen->terminus = MakeTreePtr< Expression >();
		ssthen->recurse_restriction = MakeTreePtr< Expression >();
		MakeTreePtr<Compound> scelse;
		MakeTreePtr< Stuff<Statement> > sselse;
		sif->else_body = sselse;
		sselse->terminus = MakeTreePtr< Statement >();
		sselse->recurse_restriction = MakeTreePtr< Statement >();

		MakeTreePtr< Stuff<Statement> > rs;
		MakeTreePtr<PostIncrement> rpi;
		rs->terminus = rpi;
		rpi->operands.push_back( MakeTreePtr< Expression >() );

		CouplingSet sms1((
			Coupling((ssthen, rs)),
			Coupling((ssthen->terminus, sselse->terminus, rpi->operands[0])) ));

		SearchReplace(sif, rs, sms1)( context, proot );
	}
}


void CrazyNine::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	// Replaces entire records with 9 if it has a 9 in it
	{
		MakeTreePtr<Record> s_record;
		MakeTreePtr< Stuff<Declaration> > s_stuff;
		s_record->members = ( s_stuff, MakeTreePtr< Star<Declaration> >() );
		TreePtr<SpecificInteger> s_nine( new SpecificInteger(9) );
		s_stuff->terminus = s_nine;

		MakeTreePtr<Union> r_union;
		TreePtr<SpecificTypeIdentifier> r_union_name( new SpecificTypeIdentifier("nine") );
		r_union->identifier = r_union_name;

		SearchReplace(s_record, r_union)( context, proot );
	}
}







/*
 * A rather hacked up and bit-rotted searc/replace turning *(array+i) into array[i]
    // TODO move this out of main()
    {
        TreePtr<Dereference> sd( new Dereference );
        TreePtr<Subtract> sa( new Subtract );
        sd->operands.push_back( sa );
        TreePtr<TypeOf> sseot( new TypeOf );
        sa->operands.push_back( sseot );
        TreePtr<Array> sar( new Array );
//           sar->element = TreePtr<Type>();
//           sar->size = TreePtr<Initialiser>();
        sseot->pattern = sar;
        TreePtr<Expression> se( new Expression );
        sa->operands.push_back( se );

        TreePtr<Subscript> rs( new Subscript );
        TreePtr<Expression> rar( new Expression );
        rs->base = rar;
        TreePtr<Expression> re( new Expression );
        rs->index = re;

        Coupling mar;
        mar = ( sseot );
        mar = ( rar );
        Coupling me;
        me = ( se );
        me = ( re );
        set<Coupling> sm;
        sm = ( mar );
        sm = ( me );

        SearchReplace(sd, rs, &sm)(program);
    }






Call
 callee=SEOT
  proc
   members
    instance
     ident1
     type2
    *
 operands
  operand
   ident1
   expr3
  *

Call
 callee=SEOT
  proc
   members
    instance
     ident1
     type2
    *
 operands
  operand
   ident1
   cast
    type2
    expr3
  *







                 */
