#include "split_instance_declarations.hpp"

SplitInstanceDeclarations::SplitInstanceDeclarations()
{
	shared_ptr<Compound> sc( new Compound );
	 shared_ptr<Instance> si( new Instance );
	  si->initialiser = shared_new<Uninitialised>();  // Only acting on uninitialised Instances
	 shared_ptr< Star<Declaration> > ss( new Star<Declaration> );
	 sc->members.insert( ss );
	 sc->statements.push_back( shared_new< Star<Statement> >() );
	 sc->statements.push_back( si ); // Instance is in the ordered statements part
	 sc->statements.push_back( shared_new< Star<Statement> >() );

	shared_ptr<Compound> rc( new Compound );
	 shared_ptr<Instance> ri( new Instance );
	 // ri->initialiser = shared_new<Uninitialised>();
	 shared_ptr< Star<Declaration> > rs( new Star<Declaration> );
	 rc->members.insert( ri ); // Instance now in unordered decls part
	 rc->members.insert( rs );
	 rc->statements.push_back( shared_new< Star<Statement> >() );
	 rc->statements.push_back( shared_new< Star<Statement> >() );

	SearchReplace::MatchSet ms0;
	ms0.insert( si ); ms0.insert( ri ); sms.insert( ms0 );
	SearchReplace::MatchSet ms1;
	ms1.insert( ss ); ms1.insert( rs ); sms.insert( ms1 );
	SearchReplace::MatchSet ms2;
	ms2.insert( sc->statements[0] ); ms2.insert( rc->statements[0] ); sms.insert( ms2 );
	SearchReplace::MatchSet ms3;
	ms3.insert( sc->statements[2] ); ms3.insert( rc->statements[1] ); sms.insert( ms3 );

    Configure(sc, rc, &sms);
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
                 */
