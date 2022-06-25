# Case Studies of a Transformations

This document contains examinations of the rationale behind a selection of transformations. It will be useful to all users of Vida Nova. A small example, two medium examples and one large one follow. 

## removing `Nop`

![Graph of CleanUpNop pattern](/test/reference_graphs/pattern/050-CleanupNop.svg)

`Nop` is a `Statement` node that represents "no operation". `CleanupNop` uses `VNTransfomation` to eliminate obviously redundant `Nop`s from code. The search pattern is a `Compound` statement with a `Star<Declaration>` node in the `Declaration`s collection. The sequence of `Statement`s in the `Compound` block (an ordered container) contain the following: `Star<Statement>`, `Nop`, `Star<Statement>`. All the `Star` nodes have no pre-restriction and are maximally wild, so they'll match anything. 

Our search can therefore match a `Compound` block with any decls, and any statements as long as there is at least one `Nop` (this transformation only deals with `Nop`s in `Compound` blocks). The `Nop` can be anywhere in the sequence of statements: the layout of `Star`, `X`, `Star` resembles `.*X.*` in regular expressions.

For replacing, we wish to preserve everything in the compound block except the `Nop`. And we wish to preserve the statements in the correct order. We supply a new `Compound` block in the replace pattern, and fill in its declarations and statements by pointing to the same `Star` nodes we created in the search pattern. Thus the `Star` nodes each have two parent nodes, and are coupled. The effect of coupling the star nodes is to reproduce whatever they matched (which could be 0 or mode nodes, complete with any subtrees) in the output program at that position. 

The pattern is strictly reductive: each hit will reduce the number of `Nop` statements by one, and the transformation will terminate when the program contains no `Nop` statements. The repeating nature of the Vida Nova S&R algorithm means we will keep trying until we don't get any hits. This means that a compound block containing many `Nop` statements will be correctly transformed after multiple hits. On each hit, the `Star<Statement>` nodes will match different subsequences of the statements in the compound block. For this step (and I believe in general) it does not matter whether the first `Nop` matched is the one at the top, or bottom or middle of the block.

## Generate implicit casts

`GenerateImplicitCasts` adds a C-style cast to every function call argument that is not of the same type as the parameter in the function declaration. 

To do this we search for a call to a function (the `Call` node). Because function calls can be expressions (involving function pointers) the `Call` node specifies the function using an expression. The signature of the function is the type of this expression, and it contains the types of the parameters. To get the type of the expression, we specify `TransformOf<Expression>(Typeof)` node. The type is then compared with the pattern given as the child of `TypeOf`. Here we specify `Procedure`, which is the type for a callable entity that has parameters (but not necessarily a return value). `Function` derives from `Procedure`.

Note: Vida Nova stores parameters in an unordered collection, which differs from C source language which relies on parameter ordering. Arguments to a call are then specified in the form of a map, with the key being the parameter's identifier and the value being the argument expression. Vida Nova does not support maps directly (only sequences and collections) so the tree specification for C builds a map from a collection of nodes called `MapOperand` which act as key-value pairs. A call is therefore conceptually more like a named parameter call e.g. `foo( param1:arg1, param2:arg2 ...)`. This type of structure is easy to manipulate in search and replace using couplings. Ordered parameters would be more difficult because it would require an ordinal correspondence (index coupling). However, the reason for doing it this way is not S&R simplicity, but rather a desire not to store redundant information in the tree.

Since we will act on one parameter at a time, we use `Star` to match the other parameters. The one we are interested in will be an `Instance` node, since a parameter is a declaration of a variable-like construct. The identifier and type are given maximal wildcards to allow any parameter to match. In the arguments collection of the search pattern's `Call` node, we again supply a `Star` for other arguments, and a `MapOperand`, as explained above, for the argument in question. To ensure the `MapOperand` corresponds to the same parameter that we picked out of the declaration, we couple the identifier (which is the key of the map) to that of the parameter. 

We must restrict the search to cases where the types differ, so we use the but-not pattern, by placing `TypeOf` on the argument expression, and coupling to the parameter type under a `Negation` node. Therefore we will only hit when the argument matches the parameter by identifier, but strictly differs in type.

The replace pattern for this transformation is relatively simple. In the replace pattern's `Call`, we couple the other args and the expression that chooses the function, so that they are the same (when you couple to a `TypeOf` node, you get the expression; to get the type, couple to its child). We replace the missing argument with a new `MapOperand`, coupling the identifier to that of the `MapOperand` in the search pattern. For the expression, we supply a `Cast` node, which corresponds to a C-style cast. Its type is the type of the parameter in the declaration. The expression is coupled to the the argument expression. This will generate a cast that casts the expression in the original call to the type of the parameter in the function's declaration, as required.

Note that we see some three-way couplings around the parameter type and identifier. This works just fine, Vida Nova is cool about that sort of thing.

## For to While

`ForToWhile` transforms For loops into semantically equivalent `While` loops. C makes this easy and hard. Easy because the three elements of a `For` loop are general C constructs that can simply be moved to the appropriate places around a `While` loop; hard because of `Break` and `Continue`. We do not have to worry about `Break` here because it has already been handled by another step, but `Continue` requires explicit treatment (`Continue` works in While loops, but we have to be careful about the semantics: the increment could be skipped).

Our master search pattern describes a general `For` loop. There is a `For` node, and its expression (the test) is filled in with an `Expression` node which is a maximal wildcard that will match any expression. Its init condition, increment and body are all maximally wild `Statement` nodes. Therefore, any `For` loop will match (Vida Nova fills absent statements with `Nop`, which will match).

The master replace pattern begins with a `Compound` node. This is there to make the pattern simpler, and there are clean-up steps that remove them where they are redundant. Generous use of `Compound` blocks is encouraged style. The compound has only 2 statements: the init statement from the `For` loop via a coupling, followed by a `While` node. The `While` loop's test expression is coupled in from the `For` loop's test expression. In the body of the `While` loop we place the `For` loop body (but see below) followed by the increment from the `For` loop. This completes the basic transformation - by inspection, you can determine that the constructs from the `For` loop are in the right places. 

In order to deal with `Continue`, we insert a `SlaveCompareReplace` into the master replace pattern, just above where we couple the body of the `For` loop. Recall that the through member is simply there to allow the master replace pattern to continue. During each master replace (in other words, as S&R is building the While loop) the slave will act on the replaced version of the subtree under the through link. This is the body of the original `For` loop (via coupling).

In this body, we wish to change the behaviour of `Continue`: we wish to insert a copy of the increment statement just before each continue, to compensate for continue skipping the increment we placed at the end of the body. The positioning of the slave ensures we do not change continues outside of `For` loops. Were it not for two awkward issues, we would simply use `SlaveSearchReplace`, search for continue and replace with a compound containing the increment condition (coupled) followed by continue. This is indeed approximately how the slave is laid out, but there are extra nodes to solve the problems:

1. The slave replace pattern contains continue, so the slave would not terminate. We want each continue in the input program (under a For) to be replaced exactly once by the compound of increment and continue. More or fewer replacements would increment too many or too few times. The `GreenGrass` node ensures we only replace continues that were in the input tree, and not ones that came from our replace pattern.

2. Not all continue statements under a `For` loop apply to that loop, because there could be nested loops (see the C spec). We cannot just use a `SlaveSearchReplace` because it would find continues that we don't want to change. The actual pattern used is a stuff node, which can recurse to find a continue (continue is the terminus, via the `Delta` node), and it has a recurse restriction filled in with `Negation` and a `Loop` wildcard. This part of the pattern means "not any kind of `Loop`" and the `Stuff` node will therefore not recurse through any loops as it looks for a terminus match. `Stuff` is followed by a `Delta` node, which allows the new `Compound` to be overlayed over the original `Continue`.

Note that if there was no need for a recurse restriction, we would use `SlaveSearchReplace` and get a more readable pattern.

## Generate stacks

`GenerateStacks` is one of the more complex steps, so I'll just describe the strategy, a few salient points and some future directions for this transformation.

We adopt the pessimistic approach that all automatic variables may be subject to recursion (we don't look for non-recursing functions or variables that are not live at recursion points). Each is given a stack in the form of an array and a stack index is provided, which is like a stack pointer but is in the form of an index into the aforementioned arrays. We arbitrarily make the stacks 10 elements deep.

The stacks and stack index are all local static variables. In future, these will be made members of some containing class, which will improve locality.

A single static stack pointer is declared for each function. It is incremented at the top and decremented at the bottom. The master `VNTransfomation` does this. It must also be decremented before return statements. A slave does this. Another slave detects all the declarations of automatic variables in the function and replaces them with static arrays (stacks). This slave has a sub slave which finds usages of the variable, and inserts an indexing operation into the array, using the stack index.

Salient points:

The `Subroutine` `Instance` at the root of the pattern is coupled, and the part we wish to change is separated out again using a `Delta` node. This is an equally effective alternative to having separate `Instance` nodes for search and replace and keying the members that do not change. It might be more tolerant of future changes in the definition of the `Instance` node.

The search pattern continues through the `Delta` nodes through pointer. We match a generic `Compound` so that we can couple into the replace pattern, preserving the function's existing body. We add a `Conjunction` branch which goes though a `Stuff` node to an `Automatic` node. This ensures the master does not spin, adding more and more stack indexes. It also means we do not modify functions that already do not have automatic variables.

The master replace is reached by passing through the through pointers of the two first-level slaves. It produces a new compound, inserting a new declaration for the stack index into the decls collection, and placing an increment and decrement of the same variable at the top and bottom of the statements sequence. The identifier of the variable is built from the function name and `_stack_index` to make the output readable.

The first slave to act is the most-nested first-level slave, which is a `SlaveCompareReplace`. It emulates `VNTransfomation` using `Star` and `Delta`, as seen before. This time, the reason is to allow a sub-slave to be tucked in at the root of the replace pattern, above the `Stuff`. The `Delta` splits out into a search pattern for an automatic, and a replace pattern that declares an array type. Again, we build the array's identifier from that of the original variable name with `_stack` appended.

This slave's sub-slave searches for the original variable, and replaces it with a subscript of the new array by the new stack index. This does not hit the declaration, because slaves operate after the pattern under through has been replaced, and the original variable has been replaced with a new one that has a different identifier (it isn't the name that makes identifiers different, just the fact that they are separate nodes).

Finally, the other first-level slave looks for `Return` and replaces with a `Compound` containing a decrement of the stack index followed by a return. We use `GreenGrass` to stop this transformation happening again to the the returns we generate (which would spin). This uses the `Temp` storage class, which is not fully defined and may not be a permanent feature of Vida Nova. At present it is a variable that makes only those guarantees made by all of `Automatic`,`Static` and `Member`.

Future directions:

The step could probably be cleaned up, for example by taking advantage of the fact that every usage of a local variable must be in or under the compound in which it was declared - this would let all the slaves be `SlaveSearchReplace` and remove the need for the `Stuff``/`Delta` combination. 

As for enhancement, detection of non-recursive functions is complicated because it is non-local. But we can easily detect leaf functions and do a trivial `Automatic` to `Static` (or `Member` or `Temp`) conversion (perhaps in a separate step). This could be extended to detect leaf-scoped variables (ones whose scope has no outgoing calls). Further, we could restrict checks for outgoing calls to areas where the variable is "live" - this would be harder in S&R but might be possible.

Stack overflow detection would simply require boilerplate code at the point of the increment. It would invoke some kind of intrinsic debug assert node in case of overflow.
