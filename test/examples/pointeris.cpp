//
// Test covering PointerIs
//
// CleanupCompoundExpression uses PointerIs help find a statement context into 
// which it can expand the statements from a statement-expression.
//
// We provide a reasonable case, which should hit and transform correctly, 
// as well as an unreasonable case which the step should leave alone.
//

// If PointerIs is stuck on match, CleanupCompoundExpression will try to covert
// this into a statement, and fail an internal type-consistency check.
// (Note: gcc does not accept this)
int a = 99 * ({ do {} while(0); 233; }) + 44;

int main()
{
    // If PointerIs is stuck on misatch, this will be left as a statement-expression
	return 9 * ({ do {} while(0); 23; }) + 4 + a;
}
