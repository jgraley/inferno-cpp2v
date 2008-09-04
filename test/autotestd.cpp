
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


int do_failable_test()
{
    int rc = system("make");
    if( rc != 0 )
    {
        printf("Build failed\n");
        return 1;
    }
    
    printf("Build succeeded\n");
    
    rc=chdir("test");
    assert( rc==0 );
    
    rc = system("make test");     
    if( rc != 0 )
    {
        printf("Tests failed\n");
        return 2;
    }
    
    printf("Tests passed\n");
    
    return 0;
}

void do_test( int revision )
{
    printf("BEGIN TEST REVISION %d\n", revision );
    
    // Should be run from trunk/test. Traverse a bit to confirm.
    // Only test the trunk (branches are for "blue sky" stuff,
    // wheras the trunk must work.
    int rc=chdir("../../../inferno-synth/trunk");
    assert( rc==0 );
    
    // Check out the requested revision (head might have moved on in the 
    // meantime)
    char s[100];
    sprintf( s, "svn update --revision %d\n", revision );
    rc = system(s);
    assert( rc==0 );
    
    rc = do_failable_test();    
    
    printf("END TEST REVISION %d\n", revision );
}

// Must be invoked in reverse order ie last lable first because
// we insert null terminators
char *locate( char *buf, const char *label )
{
    char *p = strstr( buf, label );
    if( !p )
        return NULL;
    *p = '\0'; // terminate the previous field
    return p+strlen(label);   
}


void handle_message( char *buf )
{   
    // Detect and null-terminate the fields corresponding to the specifed labels
    char *mp = locate( buf, "Modified Paths:\n--------------\n" );
    char *lm = locate( buf, "Log Message:\n-----------\n" );
    char *date = locate( buf, "Date:     " );
    char *auth = locate( buf, "Author:   " );
    char *rev = locate( buf, "Revision: " );
    if( !mp || !lm || !date || !auth || !rev )
        return;
    
    // Only proceed if a test was asked for in the log message. This is important
    // to prevent loops where test results trigger more tests.
    if( !strstr( lm, "test please" ) )
        return;
        
    do_test( atoi( rev ) );    
}


#define BUFLEN 10000
void handle_char( char c )
{
    static char buf[BUFLEN];
    static int i=0;
    assert( i+1 < BUFLEN );
    buf[i++]=c;
    buf[i] = '\0'; // Null-terminate what we have so far
    
    // This is at the end of a sourceforce svn commit email; we use it as a terminator.
    // Other emails should not reach us if the email client filters are set right. 
    if( strstr( buf, "This was sent by the SourceForge.net collaborative development platform, the world's largest Open Source development site.\n" ) != NULL )
    {
        handle_message( buf );
        i=0; 
    }        
}


int main( int argc, char **argv )
{
    // We must correctly handle >1 message in a row
    // as well as waiting when there are no meessags.
    // read() seems to return with 0 after the first 
    // message even though STDIN is blocking.
    while(1)
    {
        char b;
        int n = read( 0, &b, 1 );
        if( n==1 )
            handle_char(b);
        else
            sleep(1);
   }
}
