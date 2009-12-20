#include "split_instance_declarations.hpp"
#include "tree/tree.hpp"

void SplitInstanceDeclarations::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
    // TODO only split for Auto variables - for others, take the whole lot including init into the members colelction
	{ // Do uninitialised ones
		SearchReplace sr0;
		SearchReplace::CouplingSet sms0;

		SharedPtr<Compound> sc( new Compound );
		 SharedPtr<Instance> si( new Instance );
		  si->initialiser = shared_new<Uninitialised>();  // Only acting on uninitialised Instances
		 SharedPtr< SearchReplace::Star<Declaration> > ss( new SearchReplace::Star<Declaration> );
		 sc->members.insert( ss );
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		 sc->statements.push_back( si ); // Instance is in the ordered statements part
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SharedPtr<Compound> rc( new Compound );
		 SharedPtr<Instance> ri( new Instance );
		 // ri->initialiser = shared_new<Uninitialised>();
		 SharedPtr< SearchReplace::Star<Declaration> > rs( new SearchReplace::Star<Declaration> );
		 rc->members.insert( ri ); // Instance now in unordered decls part
		 rc->members.insert( rs );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SearchReplace::Coupling ms0((si, ri));
		sms0.insert( &ms0 );
		SearchReplace::Coupling ms1((ss, rs));
		sms0.insert( &ms1 );
		SearchReplace::Coupling ms2((sc->statements[0], rc->statements[0]));
		sms0.insert( &ms2 );
		SearchReplace::Coupling ms3((sc->statements[2], rc->statements[1]));
		sms0.insert( &ms3 );

		sr0.Configure(sc, rc, sms0);
		sr0( context, proot );

	}
	{ // Do initialised ones by leaving an assign behind
		SearchReplace sr1;
		SearchReplace::CouplingSet sms1;

		SharedPtr<Compound> sc( new Compound );
		 SharedPtr<Instance> si( new Instance );
		  si->identifier = shared_new<InstanceIdentifier>();  // Only acting on initialised Instances
		  si->initialiser = shared_new<Expression>();  // Only acting on initialised Instances
		 SharedPtr< SearchReplace::Star<Declaration> > ss( new SearchReplace::Star<Declaration> );
		 sc->members.insert( ss );
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		 sc->statements.push_back( si ); // Instance is in the ordered statements part
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SharedPtr<Compound> rc( new Compound );
		 SharedPtr<Instance> ri( new Instance );
		  ri->initialiser = shared_new<Uninitialised>();
		 SharedPtr< SearchReplace::Star<Declaration> > rs( new SearchReplace::Star<Declaration> );
		 rc->members.insert( ri ); // Instance now in unordered decls part
		 rc->members.insert( rs );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		  SharedPtr<Assign> ra( new Assign );
		   ra->operands.push_back( shared_new<InstanceIdentifier>() );
		   ra->operands.push_back( shared_new<Expression>() );
		 rc->statements.push_back( ra );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SearchReplace::Coupling ms0((si, ri));
		sms1.insert( &ms0 );
		SearchReplace::Coupling ms1((ss, rs));
		sms1.insert( &ms1 );
		SearchReplace::Coupling ms2((sc->statements[0], rc->statements[0]));
		sms1.insert( &ms2 );
		SearchReplace::Coupling ms3((sc->statements[2], rc->statements[2]));
		sms1.insert( &ms3 );
		SearchReplace::Coupling ms4((si->identifier, ra->operands[0]));
		sms1.insert( &ms4 );
		SearchReplace::Coupling ms5((si->initialiser, ra->operands[1]));
		sms1.insert( &ms5 );

		sr1.Configure(sc, rc, sms1);
		sr1( context, proot );
	}
}


void MergeInstanceDeclarations::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace::CouplingSet sms1;
	SearchReplace sr1;
	{
		// This is the hard kind of search pattern where Stars exist in two
		// separate containers and have a match set linking them together
		SharedPtr<Compound> rc( new Compound );
   	     SharedPtr<Instance> ri( new Instance );
		  ri->identifier = shared_new<InstanceIdentifier>();
          ri->initialiser = shared_new<Uninitialised>();
 		 rc->members.insert( ri ); // Instance in unordered decls part
		 SharedPtr< SearchReplace::Star<Declaration> > rs( new SearchReplace::Star<Declaration> );
		 rc->members.insert( rs );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		  SharedPtr<Assign> ra( new Assign );
		   ra->operands.push_back( shared_new<InstanceIdentifier>() );
		   ra->operands.push_back( shared_new<Expression>() );
		 rc->statements.push_back( ra );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SharedPtr<Compound> sc( new Compound );
		 SharedPtr< SearchReplace::Star<Declaration> > ss( new SearchReplace::Star<Declaration> );
		 sc->members.insert( ss );
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		  SharedPtr<Instance> si( new Instance );
		   si->identifier = shared_new<InstanceIdentifier>();
		   si->initialiser = shared_new<Expression>();  // Only acting on initialised Instances
		 sc->statements.push_back( si ); // Instance is in the ordered statements part
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SearchReplace::Coupling ms0((si, ri));
		sms1.insert( &ms0 );
		SearchReplace::Coupling ms1((ss, rs));
		sms1.insert( &ms1 );
		SearchReplace::Coupling ms2((sc->statements[0], rc->statements[0]));
		sms1.insert( &ms2 );
		SearchReplace::Coupling ms3((sc->statements[2], rc->statements[2]));
		sms1.insert( &ms3 );
		SearchReplace::Coupling ms4((si->identifier, ri->identifier, ra->operands[0]));
		sms1.insert( &ms4 ); // id of instance
		SearchReplace::Coupling ms5((si->initialiser, ra->operands[1]));
	    sms1.insert( &ms5 ); // init expression

		sr1.Configure(rc, sc, sms1);
	}
	sr1( context, proot );
}


void HackUpIfs::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace::CouplingSet sms1;
	SearchReplace sr1;
	{
		SharedPtr<If> sif( new If );
		  SharedPtr<Expression> stest( new Expression );
		  sif->condition = stest;
	      SharedPtr< SearchReplace::Stuff<Statement> > ssthen( new SearchReplace::Stuff<Statement> );
		  sif->body = ssthen;
		    ssthen->terminus = shared_new< Expression >();
		    ssthen->restrictor = shared_new< Expression >();
		  SharedPtr<Compound> scelse( new Compound );
  	      SharedPtr< SearchReplace::Stuff<Statement> > sselse( new SearchReplace::Stuff<Statement> );
		  sif->else_body = sselse;
		    sselse->terminus = shared_new< Statement >();
		    sselse->restrictor = shared_new< Statement >();

        SharedPtr< SearchReplace::Stuff<Statement> > rs( new SearchReplace::Stuff<Statement> );
          SharedPtr<PostIncrement> rpi( new PostIncrement );
          rs->terminus = rpi;
            rpi->operands.push_back( shared_new< Expression >() );

  		SearchReplace::Coupling ms0((ssthen, rs));
  		sms1.insert( &ms0 ); // statement of interest
		SearchReplace::Coupling ms1((ssthen->terminus, sselse->terminus));
		ms1.insert( rpi->operands[0] ); sms1.insert( &ms1 ); // statement of interest

		sr1.Configure(sif, rs, sms1);
	}
	sr1( context, proot );
}


void CrazyNine::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace sr1;
	// Replaces entire records with 9 if it has a 9 in it
	{
		SharedPtr<Record> s_record( new Record );
		  shared_ptr< SearchReplace::Stuff<Declaration> > s_stuff( new SearchReplace::Stuff<Declaration> );
		  s_record->members.insert( s_stuff );
		  s_record->members.insert( shared_new< SearchReplace::Star<Declaration> >() );
		    shared_ptr<SpecificInteger> s_nine( new SpecificInteger(9) );
		    s_stuff->terminus = s_nine;

        shared_ptr<Union> r_union( new Union );
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
