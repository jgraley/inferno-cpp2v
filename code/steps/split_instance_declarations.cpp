#include "split_instance_declarations.hpp"
#include "tree/tree.hpp"

void SplitInstanceDeclarations::operator()( SharedPtr<Node> context, SharedPtr<Node> *proot )
{
	{ // Do initialised local variables by leaving an assign behind
		SearchReplace sr1;

		MakeShared<Compound> sc;
		MakeShared<LocalVariable> si;
		si->identifier = MakeShared<InstanceIdentifier>();  // Only acting on initialised Instances
		si->initialiser = MakeShared<Expression>();  // Only acting on initialised Instances
		MakeShared< Star<Declaration> > ss;
		sc->members = ( ss );
		sc->statements = ( MakeShared< Star<Statement> >(), si, MakeShared< Star<Statement> >() );

		MakeShared<Compound> rc;
		MakeShared<LocalVariable> ri;
		ri->initialiser = MakeShared<Uninitialised>();
		MakeShared< Star<Declaration> > rs;
		rc->members = ( ri, rs );
		MakeShared<Assign> ra;
		ra->operands = ( MakeShared<InstanceIdentifier>(), MakeShared<Expression>() );
		rc->statements = ( MakeShared< Star<Statement> >(), ra, MakeShared< Star<Statement> >() );

		CouplingSet sms1((
			Coupling((si, ri)),
			Coupling((ss, rs)),
			Coupling((sc->statements[0], rc->statements[0])),
			Coupling((sc->statements[2], rc->statements[2])),
			Coupling((si->identifier, ra->operands[0])),
			Coupling((si->initialiser, ra->operands[1])) ));

		sr1.Configure(sc, rc, sms1);
		sr1( context, proot );
	}
	{ // Do everything else by just moving to the decls collection
	    SearchReplace sr0;

		MakeShared<Compound> sc;
		MakeShared<Instance> si;
		MakeShared< Star<Declaration> > ss;
		sc->members = ( ss );
		sc->statements = ( MakeShared< Star<Statement> >(), si, MakeShared< Star<Statement> >() );

		MakeShared<Compound> rc;
		MakeShared<Instance> ri;
		MakeShared< Star<Declaration> > rs;
		rc->members = ( ri, rs ); // Instance now in unordered decls part
		rc->statements = ( MakeShared< Star<Statement> >(), MakeShared< Star<Statement> >() );

		CouplingSet sms0((
			Coupling((si, ri)),
			Coupling((ss, rs)),
			Coupling((sc->statements[0], rc->statements[0])),
			Coupling((sc->statements[2], rc->statements[1])) ));

		sr0.Configure(sc, rc, sms0);
		sr0( context, proot );
	}
}


void MergeInstanceDeclarations::operator()( SharedPtr<Node> context, SharedPtr<Node> *proot )
{
	SearchReplace sr1;
	{
		// This is the hard kind of search pattern where Stars exist in two
		// separate containers and have a coupling between them
		MakeShared<Compound> rc;
		MakeShared<LocalVariable> ri;
		ri->identifier = MakeShared<InstanceIdentifier>();
		ri->initialiser = MakeShared<Uninitialised>();
		MakeShared< Star<Declaration> > rs;
		rc->members = ( ri, rs );
		MakeShared<Assign> ra;
		ra->operands = (MakeShared<InstanceIdentifier>(), MakeShared<Expression>() );
		rc->statements = (MakeShared< Star<Statement> >(), ra, MakeShared< Star<Statement> >() );

		MakeShared<Compound> sc;
		MakeShared< Star<Declaration> > ss;
		sc->members = ( ss );
		MakeShared<LocalVariable> si;
		si->identifier = MakeShared<InstanceIdentifier>();
		si->initialiser = MakeShared<Expression>();  // Only acting on initialised Instances
		sc->statements = ( MakeShared< Star<Statement> >(), si, MakeShared< Star<Statement> >() );

		CouplingSet sms1((
			Coupling((si, ri)),
			Coupling((ss, rs)),
			Coupling((sc->statements[0], rc->statements[0])),
			Coupling((sc->statements[2], rc->statements[2])),
			Coupling((si->identifier, ri->identifier, ra->operands[0])),
			Coupling((si->initialiser, ra->operands[1])) ));

		sr1.Configure(rc, sc, sms1);
	}
	sr1( context, proot );
}


void HackUpIfs::operator()( SharedPtr<Node> context, SharedPtr<Node> *proot )
{
	CouplingSet sms1;
	SearchReplace sr1;
	{
		MakeShared<If> sif;
		MakeShared<Expression> stest;
		sif->condition = stest;
		MakeShared< Stuff<Statement> > ssthen;
		sif->body = ssthen;
		ssthen->terminus = MakeShared< Expression >();
		ssthen->restrictor = MakeShared< Expression >();
		MakeShared<Compound> scelse;
		MakeShared< Stuff<Statement> > sselse;
		sif->else_body = sselse;
		sselse->terminus = MakeShared< Statement >();
		sselse->restrictor = MakeShared< Statement >();

		MakeShared< Stuff<Statement> > rs;
		MakeShared<PostIncrement> rpi;
		rs->terminus = rpi;
		rpi->operands.push_back( MakeShared< Expression >() );

		CouplingSet sms1((
			Coupling((ssthen, rs)),
			Coupling((ssthen->terminus, sselse->terminus, rpi->operands[0])) ));

		sr1.Configure(sif, rs, sms1);
	}
	sr1( context, proot );
}


void CrazyNine::operator()( SharedPtr<Node> context, SharedPtr<Node> *proot )
{
	SearchReplace sr1;
	// Replaces entire records with 9 if it has a 9 in it
	{
		MakeShared<Record> s_record;
		MakeShared< Stuff<Declaration> > s_stuff;
		s_record->members = ( s_stuff, MakeShared< Star<Declaration> >() );
		SharedPtr<SpecificInteger> s_nine( new SpecificInteger(9) );
		s_stuff->terminus = s_nine;

		MakeShared<Union> r_union;
		SharedPtr<SpecificTypeIdentifier> r_union_name( new SpecificTypeIdentifier("nine") );
		r_union->identifier = r_union_name;

		sr1.Configure(s_record, r_union);
	}
	sr1( context, proot );
}







/*
 * A rather hacked up and bit-rotted searc/replace turning *(array+i) into array[i]
    // TODO move this out of main()
    {
        SharedPtr<Dereference> sd( new Dereference );
        SharedPtr<Subtract> sa( new Subtract );
        sd->operands.push_back( sa );
        SharedPtr<TypeOf> sseot( new TypeOf );
        sa->operands.push_back( sseot );
        SharedPtr<Array> sar( new Array );
//           sar->element = SharedPtr<Type>();
//           sar->size = SharedPtr<Initialiser>();
        sseot->pattern = sar;
        SharedPtr<Expression> se( new Expression );
        sa->operands.push_back( se );

        SharedPtr<Subscript> rs( new Subscript );
        SharedPtr<Expression> rar( new Expression );
        rs->base = rar;
        SharedPtr<Expression> re( new Expression );
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
