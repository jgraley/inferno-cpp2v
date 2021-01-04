//
// Test covering PointerIs
//
// CleanupStatementExpression uses PointerIs help find a statement context into 
// which it can expand the statements from a statement-expression.
//
// We provide a reasonable case, which should hit and transform correctly, 
// as well as an unreasonable case which the step should leave alone.
//

// If PointerIs is stuck on match, CleanupStatementExpression will try to covert
// this into a statement, and fail an internal type-consistency check. Such as
// OOStd inferred dynamic cast has failed: from CPPTree::Compound#30-339-0 to type CPPTree::Declaration
// (Note: gcc does not accept this usage of statement-expression)
int a = 99 * ({ do {} while(0); 233; }) + 44;

int main()
{
    // If PointerIs is stuck on misatch, this will be left as a statement-expression
	return 9 * ({ do {} while(0); 23; }) + 4 + a;
}
