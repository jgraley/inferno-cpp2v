# Transformation Case Studies

This document contains examinations of the rationale behind a selection of transformations. It will be useful to all users of Vida Nova. A small example, a medium example and one large one follow. 

## removing `Nop`

`Nop` is a `Statement` node that represents "no operation". `CleanupNop` uses `VNTransfomation` to eliminate obviously redundant `Nop`s from code. 

![Graph of CleanUpNop pattern](/test/reference/graphs/pattern/050-CleanupNop.svg)

The search pattern is a `Compound` statement with a `Star<Declaration>` node in the `Declaration`s collection. The sequence of `Statement`s in the `Compound` block (an ordered container) contain the following: `Star<Statement>`, `Nop`, `Star<Statement>`. All the `Star` nodes have no pre-restriction and are maximally wild, so they'll match any subsequence of nodes. 

Our search can therefore match a `Compound` block with any decls, and any statements as long as there is at least one `Nop` (this transformation only deals with `Nop`s in `Compound` blocks). The `Nop` can be anywhere in the sequence of statements: the layout of `Star`, `X`, `Star` resembles `.*X.*` in regular expressions.

For replacing, we wish to preserve everything in the compound block except the `Nop`. And we wish to preserve the statements in the correct order. We supply a new `Compound` block in the replace pattern, and fill in its declarations and statements by pointing to the same `Star` nodes we created in the search pattern. Thus the `Star` nodes each have two parent nodes, and are coupled. The effect of coupling the star nodes is to reproduce whatever they matched (which could be 0 or mode nodes, complete with any subtrees) in the output program at that position. 

The pattern is strictly reductive: each hit will reduce the number of `Nop` statements by one, and the transformation will terminate when the program contains no `Nop` statements. The repeating nature of the Vida Nova S&R algorithm means we will keep trying until we don't get any hits. This means that a compound block containing many `Nop` statements will be correctly transformed after multiple hits. On each hit, the `Star<Statement>` nodes will match different subsequences of the statements in the compound block. For this step (and I believe in general) it does not matter whether the first `Nop` matched is the one at the top, or bottom or middle of the block.

## For to While

`ForToWhile` transforms For loops into semantically equivalent `While` loops. C makes this easy and hard. Easy because the three elements of a `For` loop are general C constructs that can simply be moved to the appropriate places around a `While` loop; hard because of `Break` and `Continue`. We do not have to worry about `Break` here because it has already been handled by another step, but `Continue` requires explicit treatment (`Continue` works in While loops, but we have to be careful about the semantics: the increment could be skipped). We also implement a domain-specific restriction on when the transformation will act.

![Graph of CleanUpNop pattern](/test/reference/graphs/pattern/035-ForToWhile.svg)

The search pattern begins with a `Conjunction` node, with a conjunct made up of `Stuff` and then `Uncombable`. This is to provide a way of constraining when the transformation acts: it will act only if there is _some_ node in the category called `Uncombable` within the subtree located at the `For` node. The motivation for this is specific to the Inferno domain and has to do with the combinationalisability of certain constructs.

On the other conjunct, the search pattern describes a general `For` loop. There is a `For` node, and its expression (the test) is filled in with an `Expression` node which is a maximal wildcard that will match any expression. Its body, init condition and increment are all maximally wild `Statement` nodes. Note that for loop nodes, the body always comes first, because loop nodes all inherit from the `Loop` intermediate, which introduces thr `body` child pointer. Therefore, any `For` loop will match.

The enclosing replace pattern begins with a `Compound` node. This is there to make the pattern simpler, and there are clean-up steps that remove them where they are redundant. Generous use of `Compound` blocks is encouraged style. The compound has only 2 statements: the init statement from the `For` loop via a coupling, followed by a `While` node. The `While` loop's test expression is coupled in from the `For` loop's test expression. In the body of the `While` loop we place the `For` loop body (but see below) followed by a label (see below) followed by the increment from the `For` loop. This completes the basic transformation - by inspection, you can determine that the constructs from the `For` loop are in the right places. 

In order to deal with `Continue`, we insert a `EmbeddedCompareReplace` into the enclosing replace pattern, just above where we couple the body of the `For` loop. Recall that the through member is simply there to allow the enclosing replace pattern to continue. During each enclosing replace (in other words, as S&R is building the While loop) the embedded pattern will act on the replaced version of the subtree under the through link. This is the body of the original `For` loop (via coupling).

In this body, we need to change the behaviour of `Continue`: we wish to jump to a label the corresponds to the correct behaviour for continue. The positioning of the embedded pattern ensures we do not change continues outside of `For` loops. We will use `EmbeddedSearchReplace`, search for continue and replace with a goto to our label (which is coupled in from the enclosing replace pattern). But we have a problem:

Not all continue statements under a `For` loop apply to that loop, because there could be nested loops (see the C spec). We cannot just use a `EmbeddedSearchReplace` because it would find continues that we don't want to change. The actual pattern used is a stuff node, which can recurse to find a continue (continue is the terminus, via the `Delta` node), and it has a recurse restriction filled in with `Negation` and a `Loop` wildcard. This part of the pattern means "not any kind of `Loop`" and the `Stuff` node will therefore not recurse through any loops as it looks for a terminus match. `Stuff` is followed by a `Delta` node, which allows the new `Goto` to be overlayed over the original `Continue`.

Note that if there was no need for a recurse restriction, we would use `EmbeddedSearchReplace` and get a more readable pattern.

## Generate stacks

`GenerateStacks` is one of the more complex steps, so I'll just describe the strategy, a few salient points and some future directions for this transformation.

We adopt the pessimistic approach that all automatic variables may be subject to recursion (we don't look for non-recursing functions or variables that are not live at recursion points). Each is given a stack in the form of an array and a stack index is provided, which is like a stack pointer but is in the form of an index into the aforementioned arrays. We arbitrarily make the stacks 10 elements deep.

The stacks and stack index are inserted into the scope that contains the function (so that methods will have per-instance stacks, making class-level concurrancy easier). 

![Graph of CleanUpNop pattern](/test/reference/graphs/pattern/032-GenerateStacks.svg)

We look a function inside a `Scope`, that contains at least one automatic variable (the `Instance` is forced to be a function because it has a `Compound` as its initialiser). A single static stack index is created in the `Scope`, named `<function name>_stack_index`. The enclosing pattern inserts an increment at the top of the function body and embedded pattern inserts a decrement prior to `Return` statements. 

Another embedded pattern detects all the declarations of automatic variables anywhere in the function and replaces them with static arrays in the `Scope` (these are the stacks). `_stack` is appended to their names. This embedded pattern has its own recursively embedded pattern which finds usages of the variable, and inserts an indexing operation into the array, using the stack index.

#### Salient points

The function's body is a `Compound` so that we can couple into the replace pattern, preserving the function's existing body. We add a `Conjunction` here, with a conjunct that goes though a `Stuff` node to an `Automatic` node. Because we will remove this `Automatic` suring replace, this ensures the enclosing pattern does not spin, adding more and more stack indexes forever. It also means we do not modify functions that already _do not_ have automatic variables.

The first embedded pattern to act is the most-nested first-level one, which is a `EmbeddedCompareReplace`. It emulates `SEARCH_REPLACE` behaviour using `Star` and `Delta`, as seen before. This time, the reason is to allow a sub-embedded pattern to be tucked in at the root of the replace pattern, above the `Stuff`. The `Delta` splits out into a search pattern for an automatic, and a replace pattern that declares an array type. Again, we build the array's identifier from that of the original variable name with `_stack` appended. 

It may seem strange that the stack's declaration can be created in the `Scope`, a location somewhat less nested, i.e. shallower, than the automatic variable declaration it replaces. However, there are no embedded patterns between the two, so they are in the same search/replace action. The embedded engine that controls this is "located" at program root, like the enclosing engine, and could in fact make changes anywhere in the `Scope`. The automatic is reached through: the `Scope`, the function, a Stuff node and a `Compound`. 

This recurively located embedded pattern searches for the original variable, and replaces it with a subscript of the new array by the new stack index. This does not hit the declaration, because embedded patterns operate after the pattern under through has been replaced (`LATER` model), and the original variable has been replaced with a new one that has a different identifier (it isn't the name that makes identifiers different, just the fact that they are separate nodes).

Finally, the other first-level embedded pattern looks for `Return` and replaces with a `Compound` containing a decrement of the stack index followed by a return. We use `GreenGrass` to stop this transformation happening again to the the returns we generate (which would spin). This uses the `Temp` storage class, which is not fully defined and may not be a permanent feature of Vida Nova. At present it is a variable that makes only those guarantees made by all of `Automatic`,`Static` and `Member`.

#### Future directions

As for enhancement, detection of non-recursive functions is complicated because it is non-local. But we can easily detect leaf functions and do a trivial `Automatic` to `Static` (or `Member` or `Temp`) conversion (perhaps in a separate step). This could be extended to detect leaf-scoped variables (ones whose scope has no outgoing calls). Further, we could restrict checks for outgoing calls to areas where the variable is "live" - though we'd want to have "broken down" constructs in order to permit dataflow analysis for this.

Stack overflow detection would simply require boilerplate code at the point of the increment. It would invoke some kind of intrinsic debug assert node in case of overflow.
