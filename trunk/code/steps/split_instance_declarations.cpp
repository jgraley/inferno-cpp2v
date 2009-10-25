#include "split_instance_declarations.hpp"

SplitInstanceDeclarations::SplitInstanceDeclarations()
{
	{ // Do uninitialised ones
		shared_ptr<Compound> sc( new Compound );
		 shared_ptr<Instance> si( new Instance );
		  si->initialiser = shared_new<Uninitialised>();  // Only acting on uninitialised Instances
		 shared_ptr< SearchReplace::Star<Declaration> > ss( new SearchReplace::Star<Declaration> );
		 sc->members.insert( ss );
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		 sc->statements.push_back( si ); // Instance is in the ordered statements part
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		shared_ptr<Compound> rc( new Compound );
		 shared_ptr<Instance> ri( new Instance );
		 // ri->initialiser = shared_new<Uninitialised>();
		 shared_ptr< SearchReplace::Star<Declaration> > rs( new SearchReplace::Star<Declaration> );
		 rc->members.insert( ri ); // Instance now in unordered decls part
		 rc->members.insert( rs );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SearchReplace::MatchSet ms0;
		ms0.insert( si ); ms0.insert( ri ); sms0.insert( ms0 );
		SearchReplace::MatchSet ms1;
		ms1.insert( ss ); ms1.insert( rs ); sms0.insert( ms1 );
		SearchReplace::MatchSet ms2;
		ms2.insert( sc->statements[0] ); ms2.insert( rc->statements[0] ); sms0.insert( ms2 );
		SearchReplace::MatchSet ms3;
		ms3.insert( sc->statements[2] ); ms3.insert( rc->statements[1] ); sms0.insert( ms3 );

		sr0.Configure(sc, rc, &sms0);
	}
	{ // Do initialised ones by leaving an assign behind
		shared_ptr<Compound> sc( new Compound );
		 shared_ptr<Instance> si( new Instance );
		  si->identifier = shared_new<InstanceIdentifier>();  // Only acting on initialised Instances
		  si->initialiser = shared_new<Expression>();  // Only acting on initialised Instances
		 shared_ptr< SearchReplace::Star<Declaration> > ss( new SearchReplace::Star<Declaration> );
		 sc->members.insert( ss );
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		 sc->statements.push_back( si ); // Instance is in the ordered statements part
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		shared_ptr<Compound> rc( new Compound );
		 shared_ptr<Instance> ri( new Instance );
		  ri->initialiser = shared_new<Uninitialised>();
		 shared_ptr< SearchReplace::Star<Declaration> > rs( new SearchReplace::Star<Declaration> );
		 rc->members.insert( ri ); // Instance now in unordered decls part
		 rc->members.insert( rs );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		  shared_ptr<Assign> ra( new Assign );
		   ra->operands.push_back( shared_new<InstanceIdentifier>() );
		   ra->operands.push_back( shared_new<Expression>() );
		 rc->statements.push_back( ra );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SearchReplace::MatchSet ms0;
		ms0.insert( si ); ms0.insert( ri ); sms1.insert( ms0 );
		SearchReplace::MatchSet ms1;
		ms1.insert( ss ); ms1.insert( rs ); sms1.insert( ms1 );
		SearchReplace::MatchSet ms2;
		ms2.insert( sc->statements[0] ); ms2.insert( rc->statements[0] ); sms1.insert( ms2 );
		SearchReplace::MatchSet ms3;
		ms3.insert( sc->statements[2] ); ms3.insert( rc->statements[2] ); sms1.insert( ms3 );
		SearchReplace::MatchSet ms4;
		ms4.insert( si->identifier ); ms4.insert( ra->operands[0] ); sms1.insert( ms4 );
		SearchReplace::MatchSet ms5;
		ms5.insert( si->initialiser ); ms5.insert( ra->operands[1] ); sms1.insert( ms5 );

		sr1.Configure(sc, rc, &sms1);
	}
}

void SplitInstanceDeclarations::operator()( shared_ptr<Program> program )
{
	sr0( program );
	//sr1( program );
}

MergeInstanceDeclarations::MergeInstanceDeclarations()
{
	{
		// This is the hard kind of search pattern where Stars exist in two
		// separate containers and have a match set linking them together
		shared_ptr<Compound> rc( new Compound );
   	     shared_ptr<Instance> ri( new Instance );
		  ri->identifier = shared_new<InstanceIdentifier>();
          ri->initialiser = shared_new<Uninitialised>();
 		 rc->members.insert( ri ); // Instance in unordered decls part
		 shared_ptr< SearchReplace::Star<Declaration> > rs( new SearchReplace::Star<Declaration> );
		 rc->members.insert( rs );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		  shared_ptr<Assign> ra( new Assign );
		   ra->operands.push_back( shared_new<InstanceIdentifier>() );
		   ra->operands.push_back( shared_new<Expression>() );
		 rc->statements.push_back( ra );
		 rc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		shared_ptr<Compound> sc( new Compound );
		 shared_ptr< SearchReplace::Star<Declaration> > ss( new SearchReplace::Star<Declaration> );
		 sc->members.insert( ss );
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		  shared_ptr<Instance> si( new Instance );
		   si->identifier = shared_new<InstanceIdentifier>();
		   si->initialiser = shared_new<Expression>();  // Only acting on initialised Instances
		 sc->statements.push_back( si ); // Instance is in the ordered statements part
		 sc->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		SearchReplace::MatchSet ms0;
		ms0.insert( si ); ms0.insert( ri ); sms1.insert( ms0 ); // Instance
		SearchReplace::MatchSet ms1;
		ms1.insert( ss ); ms1.insert( rs ); sms1.insert( ms1 ); // * in members
		SearchReplace::MatchSet ms2;
		ms2.insert( sc->statements[0] ); ms2.insert( rc->statements[0] ); sms1.insert( ms2 ); // 1st * in statements
		SearchReplace::MatchSet ms3;
		ms3.insert( sc->statements[2] ); ms3.insert( rc->statements[2] ); sms1.insert( ms3 ); // last * in statements
		SearchReplace::MatchSet ms4;
		ms4.insert( si->identifier ); ms4.insert( ri->identifier );
		ms4.insert( ra->operands[0] ); sms1.insert( ms4 ); // id of instance
		SearchReplace::MatchSet ms5;
		ms5.insert( si->initialiser ); ms5.insert( ra->operands[1] ); sms1.insert( ms5 ); // init expression

		sr1.Configure(rc, sc, &sms1);
	}
}

void MergeInstanceDeclarations::operator()( shared_ptr<Program> program )
{
	sr1( program );
}

HackUpIfs::HackUpIfs()
{
	{
		shared_ptr<If> sif( new If );
		  shared_ptr<Expression> stest( new Expression );
		  sif->condition = stest;
	      shared_ptr< SearchReplace::Stuff<Statement> > ssthen( new SearchReplace::Stuff<Statement> );
		  sif->body = ssthen;
		    ssthen->terminus = shared_new< Expression >();
		    ssthen->restrictor = shared_new< Expression >();
		  shared_ptr<Compound> scelse( new Compound );
  	      shared_ptr< SearchReplace::Stuff<Statement> > sselse( new SearchReplace::Stuff<Statement> );
		  sif->else_body = sselse;
		    sselse->terminus = shared_new< Statement >();
		    sselse->restrictor = shared_new< Statement >();

        shared_ptr< SearchReplace::Stuff<Statement> > rs( new SearchReplace::Stuff<Statement> );
          shared_ptr<PostIncrement> rpi( new PostIncrement );
          rs->terminus = rpi;
            rpi->operands.push_back( shared_new< Expression >() );

  		SearchReplace::MatchSet ms0;
  		ms0.insert( ssthen ); ms0.insert( rs ); sms1.insert( ms0 ); // statement of interest
		SearchReplace::MatchSet ms1;
		ms1.insert( ssthen->terminus ); ms1.insert( sselse->terminus ); ms1.insert( rpi->operands[0] ); sms1.insert( ms1 ); // statement of interest

		sr1.Configure(sif, rs, &sms1);
	}
}

void HackUpIfs::operator()( shared_ptr<Program> program )
{
	sr1( program );
}


CrazyNine::CrazyNine()
{
	// Replaces entire records with 9 if it has a 9 in it
	{
		shared_ptr<Record> s_record( new Record );
		  shared_ptr< SearchReplace::Stuff<Declaration> > s_stuff( new SearchReplace::Stuff<Declaration> );
		  s_record->members.insert( s_stuff );
		  s_record->members.insert( shared_new< SearchReplace::Star<Declaration> >() );
		    shared_ptr<SpecificInteger> s_nine( new SpecificInteger(9) );
		    s_stuff->terminus = s_nine;

        shared_ptr<Union> r_union( new Union );
          shared_ptr<SpecificTypeIdentifier> r_union_name( new SpecificTypeIdentifier );
          r_union->identifier = r_union_name;
            r_union_name->name = string("nine"); // In the end, there can be only nine!!!1

		sr1.Configure(s_record, r_union, &sms1);
	}
}

void CrazyNine::operator()( shared_ptr<Program> program )
{
	sr1( program );
}







/*
 * A rather hacked up and bit-rotted searc/replace turning *(array+i) into array[i]
    // TODO move this out of main()
    {
        shared_ptr<Dereference> sd( new Dereference );
        shared_ptr<Subtract> sa( new Subtract );
        sd->operands.push_back( sa );
        shared_ptr<SoftExpressonOfType> sseot( new SoftExpressonOfType );
        sa->operands.push_back( sseot );
        shared_ptr<Array> sar( new Array );
//           sar->element = SharedPtr<Type>();
//           sar->size = SharedPtr<Initialiser>();
        sseot->type_pattern = sar;
        shared_ptr<Expression> se( new Expression );
        sa->operands.push_back( se );

        shared_ptr<Subscript> rs( new Subscript );
        shared_ptr<Expression> rar( new Expression );
        rs->base = rar;
        shared_ptr<Expression> re( new Expression );
        rs->index = re;

        SearchReplace::MatchSet mar;
        mar.insert( sseot );
        mar.insert( rar );
        SearchReplace::MatchSet me;
        me.insert( se );
        me.insert( re );
        set<SearchReplace::MatchSet> sm;
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
