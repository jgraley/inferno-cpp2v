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
   *
 operands
  operand
   ident
   expr
  *







                 */
