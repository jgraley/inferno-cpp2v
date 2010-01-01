#include "split_instance_declarations.hpp"
#include "tree/tree.hpp"

void SplitInstanceDeclarations::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
    // TODO only split for Auto variables - for others, take the whole lot including init into the members colelction
	{ // Do uninitialised ones
		SearchReplace sr0;

		SharedNew<Compound> sc;
		SharedNew<Instance> si;
		si->initialiser = SharedNew<Uninitialised>();  // Only acting on uninitialised Instances
		SharedNew< Star<Declaration> > ss;
		sc->members = ( ss );
		sc->statements = ( SharedNew< Star<Statement> >(), si, SharedNew< Star<Statement> >() );

		SharedNew<Compound> rc;
		SharedNew<Instance> ri;
		// ri->initialiser = SharedNew<Uninitialised>();
		SharedNew< Star<Declaration> > rs;
		rc->members = ( ri, rs ); // Instance now in unordered decls part
		rc->statements = ( SharedNew< Star<Statement> >(), SharedNew< Star<Statement> >() );

		CouplingSet sms0((
			Coupling((si, ri)),
			Coupling((ss, rs)),
			Coupling((sc->statements[0], rc->statements[0])),
			Coupling((sc->statements[2], rc->statements[1])) ));

		sr0.Configure(sc, rc, sms0);
		sr0( context, proot );
	}
	{ // Do initialised ones by leaving an assign behind
		SearchReplace sr1;

		SharedNew<Compound> sc;
		SharedNew<Instance> si;
		si->identifier = SharedNew<InstanceIdentifier>();  // Only acting on initialised Instances
		si->initialiser = SharedNew<Expression>();  // Only acting on initialised Instances
		SharedNew< Star<Declaration> > ss;
		sc->members = ( ss );
		sc->statements = ( SharedNew< Star<Statement> >(), si, SharedNew< Star<Statement> >() );

		SharedNew<Compound> rc;
		SharedNew<Instance> ri;
		ri->initialiser = SharedNew<Uninitialised>();
		SharedNew< Star<Declaration> > rs;
		rc->members = ( ri, rs );
		SharedNew<Assign> ra;
		ra->operands = ( SharedNew<InstanceIdentifier>(), SharedNew<Expression>() );
		rc->statements = ( SharedNew< Star<Statement> >(), ra, SharedNew< Star<Statement> >() );

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
}


void MergeInstanceDeclarations::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace sr1;
	{
		// This is the hard kind of search pattern where Stars exist in two
		// separate containers and have a match set linking them together
		SharedNew<Compound> rc;
		SharedNew<Instance> ri;
		ri->identifier = SharedNew<InstanceIdentifier>();
		ri->initialiser = SharedNew<Uninitialised>();
		SharedNew< Star<Declaration> > rs;
		rc->members = ( ri, rs );
		SharedNew<Assign> ra;
		ra->operands = (SharedNew<InstanceIdentifier>(), SharedNew<Expression>() );
		rc->statements = (SharedNew< Star<Statement> >(), ra, SharedNew< Star<Statement> >() );

		SharedNew<Compound> sc;
		SharedNew< Star<Declaration> > ss;
		sc->members = ( ss );
		SharedNew<Instance> si;
		si->identifier = SharedNew<InstanceIdentifier>();
		si->initialiser = SharedNew<Expression>();  // Only acting on initialised Instances
		sc->statements = ( SharedNew< Star<Statement> >(), si, SharedNew< Star<Statement> >() );

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


void HackUpIfs::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	CouplingSet sms1;
	SearchReplace sr1;
	{
		SharedNew<If> sif;
		SharedNew<Expression> stest;
		sif->condition = stest;
		SharedNew< Stuff<Statement> > ssthen;
		sif->body = ssthen;
		ssthen->terminus = SharedNew< Expression >();
		ssthen->restrictor = SharedNew< Expression >();
		SharedNew<Compound> scelse;
		SharedNew< Stuff<Statement> > sselse;
		sif->else_body = sselse;
		sselse->terminus = SharedNew< Statement >();
		sselse->restrictor = SharedNew< Statement >();

		SharedNew< Stuff<Statement> > rs;
		SharedNew<PostIncrement> rpi;
		rs->terminus = rpi;
		rpi->operands.push_back( SharedNew< Expression >() );

		CouplingSet sms1((
			Coupling((ssthen, rs)),
			Coupling((ssthen->terminus, sselse->terminus, rpi->operands[0])) ));

		sr1.Configure(sif, rs, sms1);
	}
	sr1( context, proot );
}


void CrazyNine::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace sr1;
	// Replaces entire records with 9 if it has a 9 in it
	{
		SharedNew<Record> s_record;
		SharedNew< Stuff<Declaration> > s_stuff;
		s_record->members = ( s_stuff, SharedNew< Star<Declaration> >() );
		shared_ptr<SpecificInteger> s_nine( new SpecificInteger(9) );
		s_stuff->terminus = s_nine;

		SharedNew<Union> r_union;
		shared_ptr<SpecificTypeIdentifier> r_union_name( new SpecificTypeIdentifier("nine") );
		r_union->identifier = r_union_name;

		sr1.Configure(s_record, r_union);
	}
	sr1( context, proot );
}







/*
 * A rather hacked up and bit-rotted searc/replace turning *(array+i) into array[i]
    // TODO move this out of main()
    {
        shared_ptr<Dereference> sd( new Dereference );
        shared_ptr<Subtract> sa( new Subtract );
        sd->operands.push_back( sa );
        shared_ptr<TypeOf> sseot( new TypeOf );
        sa->operands.push_back( sseot );
        shared_ptr<Array> sar( new Array );
//           sar->element = SharedPtr<Type>();
//           sar->size = SharedPtr<Initialiser>();
        sseot->pattern = sar;
        shared_ptr<Expression> se( new Expression );
        sa->operands.push_back( se );

        shared_ptr<Subscript> rs( new Subscript );
        shared_ptr<Expression> rar( new Expression );
        rs->base = rar;
        shared_ptr<Expression> re( new Expression );
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
