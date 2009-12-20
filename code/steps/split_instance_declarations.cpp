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
		SharedNew< SearchReplace::Star<Declaration> > ss;
		sc->members.insert( ss );
		sc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );
		sc->statements.push_back( si ); // Instance is in the ordered statements part
		sc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );

		SharedNew<Compound> rc;
		SharedNew<Instance> ri;
		// ri->initialiser = SharedNew<Uninitialised>();
		SharedNew< SearchReplace::Star<Declaration> > rs;
		rc->members.insert( ri ); // Instance now in unordered decls part
		rc->members.insert( rs );
		rc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );
		rc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );

		SearchReplace::CouplingSet sms0((
			SearchReplace::Coupling((si, ri)),
			SearchReplace::Coupling((ss, rs)),
			SearchReplace::Coupling((sc->statements[0], rc->statements[0])),
			SearchReplace::Coupling((sc->statements[2], rc->statements[1])) ));

		sr0.Configure(sc, rc, sms0);
		sr0( context, proot );
	}
	{ // Do initialised ones by leaving an assign behind
		SearchReplace sr1;

		SharedNew<Compound> sc;
		SharedNew<Instance> si;
		si->identifier = SharedNew<InstanceIdentifier>();  // Only acting on initialised Instances
		si->initialiser = SharedNew<Expression>();  // Only acting on initialised Instances
		SharedNew< SearchReplace::Star<Declaration> > ss;
		sc->members.insert( ss );
		sc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );
		sc->statements.push_back( si ); // Instance is in the ordered statements part
		sc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );

		SharedNew<Compound> rc;
		SharedNew<Instance> ri;
		ri->initialiser = SharedNew<Uninitialised>();
		SharedNew< SearchReplace::Star<Declaration> > rs;
		rc->members.insert( ri ); // Instance now in unordered decls part
		rc->members.insert( rs );
		rc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );
		SharedNew<Assign> ra;
		ra->operands.push_back( SharedNew<InstanceIdentifier>() );
		ra->operands.push_back( SharedNew<Expression>() );
		rc->statements.push_back( ra );
		rc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );

		SearchReplace::CouplingSet sms1((
			SearchReplace::Coupling((si, ri)),
			SearchReplace::Coupling((ss, rs)),
			SearchReplace::Coupling((sc->statements[0], rc->statements[0])),
			SearchReplace::Coupling((sc->statements[2], rc->statements[2])),
			SearchReplace::Coupling((si->identifier, ra->operands[0])),
			SearchReplace::Coupling((si->initialiser, ra->operands[1])) ));

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
		rc->members.insert( ri ); // Instance in unordered decls part
		SharedNew< SearchReplace::Star<Declaration> > rs;
		rc->members.insert( rs );
		rc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );
		SharedNew<Assign> ra;
		ra->operands.push_back( SharedNew<InstanceIdentifier>() );
		ra->operands.push_back( SharedNew<Expression>() );
		rc->statements.push_back( ra );
		rc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );

		SharedNew<Compound> sc;
		SharedNew< SearchReplace::Star<Declaration> > ss;
		sc->members.insert( ss );
		sc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );
		SharedNew<Instance> si;
		si->identifier = SharedNew<InstanceIdentifier>();
		si->initialiser = SharedNew<Expression>();  // Only acting on initialised Instances
		sc->statements.push_back( si ); // Instance is in the ordered statements part
		sc->statements.push_back( SharedNew< SearchReplace::Star<Statement> >() );

		SearchReplace::CouplingSet sms1((
			SearchReplace::Coupling((si, ri)),
			SearchReplace::Coupling((ss, rs)),
			SearchReplace::Coupling((sc->statements[0], rc->statements[0])),
			SearchReplace::Coupling((sc->statements[2], rc->statements[2])),
			SearchReplace::Coupling((si->identifier, ri->identifier, ra->operands[0])),
			SearchReplace::Coupling((si->initialiser, ra->operands[1])) ));

		sr1.Configure(rc, sc, sms1);
	}
	sr1( context, proot );
}


void HackUpIfs::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace::CouplingSet sms1;
	SearchReplace sr1;
	{
		SharedNew<If> sif;
		SharedNew<Expression> stest;
		sif->condition = stest;
		SharedNew< SearchReplace::Stuff<Statement> > ssthen;
		sif->body = ssthen;
		ssthen->terminus = SharedNew< Expression >();
		ssthen->restrictor = SharedNew< Expression >();
		SharedNew<Compound> scelse;
		SharedNew< SearchReplace::Stuff<Statement> > sselse;
		sif->else_body = sselse;
		sselse->terminus = SharedNew< Statement >();
		sselse->restrictor = SharedNew< Statement >();

		SharedNew< SearchReplace::Stuff<Statement> > rs;
		SharedNew<PostIncrement> rpi;
		rs->terminus = rpi;
		rpi->operands.push_back( SharedNew< Expression >() );

		SearchReplace::CouplingSet sms1((
			SearchReplace::Coupling((ssthen, rs)),
			SearchReplace::Coupling((ssthen->terminus, sselse->terminus, rpi->operands[0])) ));

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
		SharedNew< SearchReplace::Stuff<Declaration> > s_stuff;
		s_record->members.insert( s_stuff );
		s_record->members.insert( SharedNew< SearchReplace::Star<Declaration> >() );
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

        SearchReplace::Coupling mar;
        mar.insert( sseot );
        mar.insert( rar );
        SearchReplace::Coupling me;
        me.insert( se );
        me.insert( re );
        set<SearchReplace::Coupling> sm;
        sm.insert( mar );
        sm.insert( me );

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
