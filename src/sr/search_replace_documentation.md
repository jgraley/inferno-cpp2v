# Vida Nova Search and Replace User Guide

## 1 The `VNTransfomation` class

Vida Nova uses a search and replace engine for the majority of transformation steps. The generic search and replace implementation may be found in the `VNSequence` and `VNTransformation` classes and a number of supporting classes. A `VNTransformation` object is configured by supplying a search pattern and a replace pattern, both in the form of trees. One or more `VNTransformation` objects must be placed in a `VNSequence` object. This may be used to plan and then execute a sequence of transformations.

This documentation explains how to set up the correct patterns to the `VNTransformation` class so that it will create the transformation you require.

### 1.1 Configuring `VNTransformation`

The first two argument to `VNTransformation::Configure()` must be one of `SEARCH_REPLACE` or `COMPARE_REPLACE`. This is followed by the search pattern and the replace pattern. Each pattern is a pointer to the root of a subtree of nodes. As with Vida Nova program trees, branches are allowed to merge, making them strictly speaking acyclic directed graphs, but we call them trees in line with compiler parlance. 

If `SEARCH_REPLACE` is specified, `VNTransfomation` will search input trees for an occurrence of the search pattern and replace it with a duplicate of the replace pattern if found. If `COMPARE_REPLACE` is specified, `VNTransfomation` will compare input trees with the search pattern and replace it with a duplicate of the replace pattern if it matches.

### 1.2 Special nodes

S&R supports additional nodes called "special" nodes that do not appear in the program representations for any language. They invoke additional S&R functionality when used in search and/or replace patterns. Since Vida Nova tree pointers are strongly typed (links are pointers to an intermediate type corresponding to the subset of child node types allowed), special nodes are templated on a user-specified type called the base type. Normally, the base type should be the same type as the pointer that will point to the special node, but see pre-restriction under topological wildcarding.

### 1.3 Termination

The `VNTransfomation` algorithm repeatedly attempts to find a match to the search pattern, and substitutes the replace pattern, until it can no longer find a match to the search pattern. Therefore, it can run forever if for example the replace pattern re-creates a search pattern match. It is the user's responsibility to prevent this from occurring: this is discussed in more detail later.

## 2 Topological wildcarding

Vida Nova nodes are organised in an inheritance hierarchy beginning with `Node`, from which certain intermediates like `Statement` and `Type` are derived, and then the final nodes are derived from the intermediates. In many cases there are multiple levels of intermediates between `Node` and the final node type, and multiple inheritance is allowed (with virtual inheritance). This hierarchy is best understood as a Euler diagram, where subclass is synonymous with subset.

The trees for programs are only allowed to consist of final nodes. So `Node` and the intermediates are not allowed in program trees. They are allowed, however, in search and replace patterns as long as they would not end up in the output of the search and replace transformation. 

When an intermediate node is seen in a search pattern, it serves as a wildcard for any node derived from it. For example, `Return` and `Goto` are both derived from `Statement`, so a `Statement` node in a search pattern will match `Return`, `Goto` and any other node derived from `Statement`. In set theoretic terms, we have restricted the search to the set of statements.

As an example of multiple layers of intermediates, consider that `Expression` is derived from `Statement`, making expressions a subset of statements. This is consistent with the rule in C that an expression may be used in place of a statement, but not the other way around. Consequently, a node of type `Statement` in a search pattern will match any expression as well as other kinds of statements, but an `Expression` node in a search pattern will only match expressions.

Due to Vida Nova's type-safe tree, it is not possible to specify nonsensically broad intermediates in a search pattern. This is because the pointers used in nodes are typically pointers to intermediate node types, and only identical or more derived (subset) nodes may be pointed to. Therefore the broadest wildcard possible is one whose type is the same as that of the pointer that will point to it. Such a pointer is called maximally wild and will match all input program subtrees.

Maximally wild pointers may be considered similar to the use of `.` character in regular expressions. Other intermediate topological wildcards could be likened to a value range in certain textual schemes for example `[a-zA-Z]` though it should be noted that topological (set-theoretic) restriction is more concise than linear range restriction.

### 2.1 Can populate the wild card's members

In the Vida Nova tree, some intermediate nodes have members: C++ copies these members into derived node classes during inheritance. Members may be filled in when intermediates are used in a search pattern. These members will be matched as normal and will serve to restrict the search pattern.

For example, giving the node `Integral` and pointing the width member to a `SpecificInteger` of value 32 (which we could write `Integral(width:SpecificInteger(32)))` will match 32-bit signed and unsigned data types since `Signed` and `Unsigned` both derive from `Integral`.

Note: to wildcard the width and fix the signedness we would use e.g. `Signed(width:Integer)`, in other words we use the final node for the type `Signed` and fill in the width with the maximal wildcard, which is `Integer` in this case. See tree.hpp for the definitions of these nodes.

### 2.2 NULL in search patterns

In trees for programs, we do not allow NULL (cases of disabled functionality must be dealt with explicitly, for example with nodes such as `Nop` and `Void`).

We do allow NULL for singular links in in search patterns, where it serves as shorthand for the maximal wild card. Therefore, `Signed(width:Integer)` is the same as `Signed(width:NULL)` as a search pattern.

### 2.4 Pre-restriction on special nodes

When searching, all special nodes support search restriction via the base type supplied as the first template parameter (recall that special nodes derive from the supplied base type). This base type is called the pre-restriction type. Regardless of what the special node is defined to do, all searches will restrict to nodes that are of non-strict subclasses of the pre-restriction type. 

Type-safety means that a tree pointer can only point to non-strict subclasses of the pointer type. Therefore if the pre-restriction type is the same type, the pre-restriction will be maximally wild and effectively disabled. Only if a strict subclass is given will a restriction be seen to take effect. The term _pre-restriction_ reminds us that it seems to apply before the special node's special algorithm runs (which will typically apply some further restriction independently of the pre-restriction under an "and" rule).

### 2.5 Notes on topology-oriented tree

In order to maximise the utility of topological wildcarding (and its coupling support) the Vida Nova tree has been contrived to express as much information as possible in the node hierarchy and through pointers to child nodes. So for example where a node might naturally contain a boolean (such as the signedness of an integral type) or an enum (such as an access specifier which can be public, private or protected) one of two techniques is used:

- the options for node `X` are expressed in a set of final nodes derived from `X`, so that `X` becomes an intermediate node. This can only be done for one field of node `X`. For example, consider nodes `Signed` and `Unsigned`, which both derive from the original node `Integral`, and indicate signedness. For a pattern in which an integral type can be signed or unsigned, `Integral` is the appropriate wildcard. `Signed` or `Unsigned` are used when the signedness matters.

- a new hierarchy of nodes under a new intermediate `Y` is created, and the original node `X` contains a pointer to `Y` in place of the original field. This may be applied to any field of any given `X`. For example `Public`, `Private` and `Protected` are all subclasses of `AccessSpec`, and the `Definition` node points to `AccessSpec`.

In fact, this policy leads most nodes to contain only singulars, collections and sequences. Only a small number of nodes contain other data types such as int, string etc that cannot be accommodated topologically. This is termed a topologically-oriented tree and is a reasonably consistent canonical form for program elements.

## 3 Joining and splitting the patterns

### 3.1 Couplings

When a wildcard has been specified in a search pattern, it is often useful to be able to make use of the actual subtree in the input program that the wildcard matched. We may wish to:

- restrict the search so that two wildcards in the search pattern must match the same subtree in the input program and/or

- introduce the input program subtree that matched a search pattern wildcard into the output program, as a substitute for part of the replace pattern.

### 3.2 Coupling inside search pattern

Suppose we use the wildcard `Integral` in two places in a search pattern. Each `Integral` node will individually match `Signed` or `Unsigned`, giving a total of four matching combinations: SS, SU, US and UU. But suppose we only want to match on SS or UU (perhaps because the two types must be the same for an optimisation to be correct). 

We can achieve this by placing only a single copy of the node (in our example, `Integral`) in the search pattern. Both of the nodes that would point to the `Integral` nodes now point to this single node, so that it has two parent nodes. This node is said to be coupled, and Vida Nova will only find a match when the two types in the input program are the same as each other. 

In fact, couplings restrict the entire subtree under the coupled nodes to be identical - so the two types in the above example would be restricted to have the same width regardless of what is supplied for the width in the search pattern. 

### 3.3 Coupling into the replace pattern

We can allow the replace pattern to point to a node that is already part of the search pattern. Now, whenever we find a match, the coupled node will be associated with the subtree of the input program that matched the coupled node. During the replace operation, `VNTransfomation` will substitute this subtree into the output program tree. 

### 3.4 Overlaying

Where a pattern node couples into both the search and the replace patterns, its children do not normally appear in the output program tree. These children only serve to restrict the matches found during search, and during replacement, the matched input program subtree is substituted for the output tree (instead of the coupled node or its children).

However, it is possible to arrange for some new subtree to appear under such a coupling, by using the `Delta` node. `Delta` has two child pointers: `through` and `overlay`. During search, S&R will try to match the subtree pointed to by `through`, so this subtree has the role of search restriction. During replace, S&R will substitute the input program subtree that was matched at `through`, and then it will "overlay" the subtree pointed to by `overlay` (after substituting if there are couplings).

The overlaying process is recursive, that is it can overlay a child of the node pointed to by overlay over the equivalent child of the matched input program node. Starting with pointers `PT` = `through` and `PO` = `overlay`, the rule is:

- If `PO` is non-NULL (`PT` is always non-NULL) and the node at `PT` is a non-strict subclass of the node at `PO`, then we overlay, by recursing for `PO'` = each pointer member of the node at `PO`, and `PT'` = the corresponding member of the node at `PT`. This correspondence is the reason for the subclass requirement. 

- If `PO` is NULL or the node at `PT` is not a non-strict subclass of the node at `PO`, then we overwrite the subtree at `PT` with a copy of the subtree at `PO`. We have now finished this branch and do not need to repeat.

- Containers (sequences and collections) always overwrite when encountered in the node at `PO`.

Note that the condition for overlaying versus overwriting depend only on the patterns provided for `overlay` and `though` in the `Delta` node, not on the contents of the input program tree.

### 3.5 Notes on keyability of couplings

Vida Nova search and replace has an implementation concept called keying, which is the process of choosing a particular input program subtree that will correspond to a particular coupling. Each coupling only needs to be keyed once for each search hit, and once it is keyed, the key-to-coupling relationship remains unchanged until the replace has completed.

Normally, Vida Nova search patterns exhibit an implicit "and" rule, in the sense that every node in the search pattern must match a node in the input program tree before a match is considered successful. This would make every node in the search pattern a candidate for keying a coupling. We say such nodes are in a "normal context". But some special nodes permit a match even when some subtree under that node does not match. These subtrees are said to be "abnormal contexts". 

An abnormal context extends throughout the entire subtree under the pattern node that caused it. So a normal context can contain an abnormal context as a subtree, but an abnormal context remains abnormal all the way down to the leaf nodes. The `Negation` node matches when its supplied subtree does not match, making its subtree pattern an abnormal context. See the documentation for `Negation` for more explanation. There are other sources of abnormal contexts, indicated below.

The rule is that you cannot couple nodes in separate abnormal contexts. A coupling must be reached through a normal context, or reached only from within one abnormal context.

## 4 `Star` search pattern

The Vida Nova tree supports one-to-many relationships using containers, of which two types are available: sequence, which preserves ordering and collection, which does not. When creating a search pattern it can be useful to be able to match zero or more elements of a container. We can do this using a special node called `Star<>`, which is templated on the collection's element type. So a container of pointers to `Statement`s can be wildcarded using `Star<Statement>`. 

If a pre-restriction is given, every container element matched by the `Star` must satisfy the pre-restriction (i.e. be a non-strict subclass of the supplied base class). Additionally, the `pattern` member may point to a subtree, which must be matched by every element that the star node matches. This pattern is an abnormal context.

`Star` nodes may be coupled into collections in the replace pattern, in order to reproduce all the nodes that were matched by the `Star` in the search pattern. 

A `Star` may only be placed in a container, not linked singularly. 

### 4.1 In Sequence

A `Star` node in a sequence in a search pattern acts like a `.*` character in a regular expression, in other words it can match zero or more elements at the position of the star. Star may be used multiple times and at any position in the pattern sequence. 

For example the sequence `(Star<Statement>, If, Star<Statement>, For, Star<Statement>)` is a search pattern for any sequence of `Statement`s that contains an `If` and a `For`, and which places the `If` before the `For`.

It is possible to place two star nodes consecutively in the sequence. In this case, the behaviour of the Vida Nova search engine is only well-defined when the restrictions of the two `Star` nodes (pre-restrictions and/or pattern restriction) are mutually exclusive. This is easiest to guarantee using pattern restrictions, where the patterns of the two `Star` nodes are identical except that one of the patterns has `Negation` inserted at the top. Since `Star` restriction patterns are abnormal contexts, the restriction subtree would need to be duplicated in this case, not coupled.

### 4.2 In Collection

A `Star` in a collection acts similarly as when in a sequence. However, since collections are unordered, the semantics differ slightly. A collection in a search pattern may only contain zero or one `Star` node. If a star node is present, it will try to match all nodes not matched by other (non-`Star`) elements of the collection. It is not possible to insert multiple `Star` nodes with different restrictions, though this feature may be added later.

For example, the collection `(Star<Declaration>, Typedef, Static)` will match any collection of `Declaration`s that includes a `Typedef` and a `Static`, and these may appear in any order. 

### 4.3 Notes on collection matching

To clarify, Vida Nova may use a non-order-preserving implementation to store collections, including ones in search and replace patterns. This means that the order in which elements are specified when constructing a Collection will not affect behaviour. So for example `(Static, Typedef, Star<Declaration>)` will be equivalent to the above example.

Only one non-star pattern nodes in a collection can match a given input tree node, so the above example can only match collections in the input program that contain at least two elements.

## 5 `Stuff` search pattern

Since Vida Nova search and replace operates on trees rather than just linear sequences of tokens, it is often useful to be able to wildcard a subtree. In fact, the maximally wild intermediate node will accomplish this through topological wildcarding, as discussed above. 

But it is even more useful to be able to explicitly match some part of the subtree, named the _terminus_, regardless of the path from the root of the subtree to the terminus. This is accomplished using the `Stuff<>` node. At the base of the subtree we place a `Stuff` node, templated on the pointed-to type. It has a member named `terminus`, which is a pointer to `Node`, and any node or subtree of nodes can be referenced from here.

Vida Nova will search the subtree under the `Stuff` node, including at the position of the `Stuff` node itself, for a match to the terminus. If a terminus match is found, the stuff node is considered a match. For example `Stuff<Compound>` with its terminus pointing to `If` may be given as the body of a function. It will match the terminus to any `If` node within the body as well as within nested `Compound` blocks, `For` or `While` loop bodies etc.

Stuff nodes may be coupled into the replace pattern in order to reproduce the subtree that `Stuff` matched to in the output program. When coupling the `Stuff` nodes, it is possible to overlay the terminus with an alternative subtree in order to modify only what is under the terminus. The terminus node itself and any node in the subtree under it may also be coupled into the replace pattern. 

### 5.1 Recurse restriction

Optionally, another member of the `Stuff` node called `recurse_restriction` may be filled in (otherwise it defaults to NULL). Whenever the `Stuff` node is about to recurse through a pointer in its search for a terminus match, it will skip any node that is not a match for the recurse restriction.

Therefore, `Stuff` will only find a terminus that can be reached via a path that satisfies the recurse restriction. This may be useful when, extending the above example, we wish to avoid matching an `If` that might be in a function that is declared inline in a class that is declared inside the function of interest. If we wish to exclude such cases, we can set `recurse_restriction` to a `Statement` node. Now an `If` node (our terminus) can only be found if there is a direct path of nested `Statements`, which includes `Compound`, `For`, `While` etc but does not include `Type` nodes. An unwanted `If` in this case would be within a `Type` subtree and so not found as the terminus.

Note that the `Stuff` node recurse restriction only applies when the stuff node actually recurses. It does not restrict a match at the base of the `Stuff` node and it does not restrict the terminus. So a `Stuff` node whose terminus matches the input program at the location of the Stuff node will match regardless of `recurse_restriction` (though the base is subject to restriction when the terminus is beneath the base). Also, if the `Stuff` node has recursed into the input program subtree, the terminus can still match nodes that would not match the recurse restriction.

Compare with the pre-restriction documented above, which applies to all special nodes. Pre-restriction only applies to the root of the `Stuff` node's subtree. In the general case, pre-restriction, recurse_restriction and terminus are all acting to restrict the Stuff node's subtree at different places in the input program subtree: pre-restriction at root, terminus at leaf and recurse_restriction at intervening nodes. 

Note that a recurse restriction pattern is an abnormal context.

### 5.2 `AnyNode` pattern

As mentioned above, the `Stuff` node can match at the root of the input program subtree, i.e. zero levels of recursion. Therefore a pattern of `If( Stuff( For(...) ) )` would match a program like `If( For(...) )`. `AnyNode` is an alternative to `Stuff` that matches only on exactly one level of indirection. A pattern like `If( AnyNode( For(...) )` will match for example `If( While( For(...) ) )` but not `If( For(...) )` or `If( While( Switch( For(...) ) ) )`.

`AnyNode` supports pre-restriction and has a `terminus` just like Stuff, but there is no recurse restriction. AnyNode is useful for anti-parenting, see below.

## 6 Boolean search patterns

Search patterns may be restricted according to boolean rules, as explained below. 

### 6.1 `Negation`

`Negation` implements the "not" function. The `Negation` node should be placed at the root of the subtree of interest, and the member named `negand` should be set to point to the subtree that should not match. The pattern can itself include wildcards, so a `Negation(Integral)` would match any data type other than signed or unsigned integer types.

The subtree under a `Negation` node in a search pattern is an abnormal context, in other words it cannot key a coupling. Consider a `Negation` node in a search pattern that is coupled to a node in a replace pattern. This will work as expected - the subtree that did not match the negand is substituted successfully. 

But consider coupling a node within the `Negation`'s subtree into the replace pattern. If there was an overall match of the search pattern, that must mean that some node in the negand did not match. It is possible that the `Negation` pattern is not even of the same layout (topology) as the corresponding subtree in the input program. If this is the case, it is not well defined which node or subtree should be substituted in the replace pattern.

### 6.2 `Conjunction`

`Conjunction` implements the "and" function. The `Conjunction` node should be placed at the root of the subtrees of interest, and the member named `conjuncts` (a collection) should be populated with all the subtrees that should match. `Conjunction` does creates a normal context for its pattern subtrees, so they can key couplings. Conjuncts are not invoked during replace (since it would be unclear which one to use) so a `Delta` node must be placed around the `Conjunction` node, coupling its overlay pattern to the desired element.

When one of the conjuncts is a `Negation` node, it may be easier to think of the `Conjunction` as meaning "but" rather than "and".

### 6.3 `Disjunction`

`Disjunction` implements the "or" function. The `Disjunction` node should be placed at the root of the subtree of interest, and the member named `disjuncts` (a collection) should be populated with all the subtrees that may match. The subtrees under a `Disjunction` node in a search pattern abnormal contexts because a given pattern element may not be a match even when the `Disjunction` itself does match.

## 7 `TransformOf` search patterns

Sometimes, a transformation will appear to be mostly suitable for the search and replace approach, but there may be aspects of the required matching criteria which cannot be described using the above wildcarding techniques. In such cases, Vida Nova supports the ability to run some user supplied algorithm during the search. The user supplies a pointer to an `OutOfPlaceTransformation` object to the constructor of `TransformOf<X>`. Objects of the resulting type derive from `X`, which is a pre-restriction. `TransformOf<>` has a member named `pattern` which is a pointer to any `Node`.

When the search engine encounters a `TransformOf<>` node, it applies the transformation to the current input program subtree (without modifying the original copy) and then proceeds to compare the result of the transformation with the pattern pointed to by the `pattern` member.

Transformer nodes allow you to couple both the transformer node itself (keying as the untransformed input program subtree) and the node pointed to by `pattern` (keying as the input program subtree after transformation).

### 7.1 `TypeOf`

`TypeOf` is a utility transformation in the form of a singleton class, which we can embed by constructing `TransformOf<Expression> mynode( &TypeOf::instance )`. It is used in the context of an `Expression`. During a search, the `TypeOf` algorithm will determine the type of an expression in the input program subtree, and then compare that with the type pointed to by the `pattern` member (note that types and expressions are represented as subtrees under `Type` and `Expression`, respectively). 

Note that if the pattern is maximally wild, then `TypeOf` will match any valid expression. However, it can be convenient to couple the `Type` wildcard with another `Type` in the search pattern, to restrict for type equivalence, or with a node in the replace pattern, to reproduce the type in the output program.

### 7.2 `GetDeclaration`

GetDeclaration is a utility transformation in the form of a singleton class, which we can embed by constructing `TransformOf<InstanceIdentifier> mynode( &GetDeclaration::instance )`. It is used in the context of an `InstanceIdentifier`. During a search, the `GetDeclaration` algorithm will search for the declaration of an identifier in the input program, and then compare that with the declaration pointed to by the `pattern` member. 

### 7.3 Notes on transformation interface

The `OutOfPlaceTransformation` interface guarantees not to modify the supplied tree and to return a newly created one; this is essential for search and replace usage. `OutOfPlaceTransformation` can indicate failure by returning a NULL pointer. Failure results in a non-match.

## 8 Other special search patterns

### 8.1 `IdentifierByName`

`IdentifierByName` matches only a `SpecificIdentifier` whose name matches the name created by the builder. This is the only way to search for a particular identifier name, since SpecificIdentifier hides the name in the general Vida Nova interface.

There are three variants:
 - `InstanceIdentifierByName`, which matches any `SpecificInstanceIdentifier` with the same name
 - `TypeIdentifierByName`, which matches any `SpecificTypeIdentifier` with the same name
 - `LabelIdentifierByName`, which matches any `SpecificLabelIdentifier` with the same name
  
Warning: names are an imprecise way to specify identifiers. Names are arbitrary in the input program, and may be generated during transformations by `BuildIdentifier` (below). Names are not guaranteed to be unique, and the code renderer has to uniquify in order to generate correct C output. `IdentifierByName` is expected to be used in early steps that detect language extensions embedded in identifiers with specific names.
  
### 8.2 `PointerIs`

`PointerIs` discovers the type of the `TreePtr<>` in the input program node that points to the current node. This is then matched to the subtree pointed to by member `pointer`. 

So, for example, one may detect an expression by simply using the wildcard `Expression`. However, to detect an expression only when pointed to by a `TreePtr<Statement>` but not `TreePtr<Expression>`, one would use `PointerIs( Negation( Expression ) )`. This will match `Expression` in eg `Compound( Expression )` because `Compound`'s statements collection uses `TreePtr<Statement>` but not eg `Negation( Expression )` since `Negation` uses a `TreePtr<Expression>`.

## 9 Builder replace patterns

Similar to `TransformOf` patterns, described above, it is sometimes desirable to generate subtrees using some algorithm apart from the search and replace, and embed these in the output program tree. This is done using builder nodes. At present, the identifier builder is only kind of builder node. 

In general, builder nodes are able to key a coupling. Therefore, it is possible to create a coupling entirely in a replace pattern, as long as it contains exactly one builder node.

### 9.1 `BuildIdentifier`

`BuildIdentifier` is a builder node that constructs identifiers with new names using a `printf()` format string. At present, `InstanceIdentifier` can be built using `BuildInstanceIdentifier`, and `LabelIdentifier` can be built using `BuildLabelIdentifier`. 

These special nodes have to be initialised with a format string and an optional flags string. Additionally, they contain a member called `sources`, which is a sequence of `TreePtr<Identifier>`. The format is a `printf()` format string for the name of the newly built identifier. Each `%s` format specifier will be replaced by the name of the node pointed to by the corresponding element of `sources` (can be any kind of identifier). The sources will normally need to be coupled into the search pattern in order to obtain a name from the input program tree. The builder node itself may be coupled in the replace pattern to create multiple references.

For example, if a replace pattern is to contain a new variable, and that variable is the count of times a label was jumped to, we might create the variable instance in the replace pattern, and for its identifier we point to a `BuildInstanceIdentifier` node. We might set the format to "%s_count" and place a pointer to the `LabelIdentifier` of the label whose jumps we are counting in the sources sequence. If a label is seen called `"EXIT"`, we will get a variable called `"EXIT_count"`. 

The optional flags field permits the following changes to behaviour:

 - `BYPASS_WHEN_IDENTICAL`: For use when there are 2 or more sources. If the names of the sources are all identical to each other, then this common name will be used for the generated identififer, bypassing the `printf()`-like operation. This can help avoid duplication in merging scenarios.

Usages of the new variable elsewhere in the replace pattern can just couple directly to the `BuildInstanceIdentifier` node.

## 10 Slave search and replace

It can be useful to build a 1:n relationship between two search and replace patterns. For example, we may wish to perform a transformation on a variable that modifies the declaration and all the usages of the variable. In this example, we would want each match to a variable's declaration (the master) to correspond with n matches to usages (the slaves), where n can be zero or more.

This is achieved by constructing the search and replace pattens for the master (the variable declaration in the example) and then placing a special node called `SlaveSearchReplace` into the master's replace pattern. The `SlaveSearchReplace` node is filled in with search and replace patterns for the slave (a usage of the variable in the example). For each match that master finds, the slave will operate repeatedly and will perform zero or more replace operations depending on how many matches it finds.

`SlaveSearchReplace` has three members:
 - `through`, which simply points to the remainder of the master's replace pattern;
 - `search` and
 - `replace` which specify the slave's behaviour as documented for `VNTransfomation`.

 You can also use `SlaveCompareReplace`, which has `through`, `compare` and `replace` members and behaves as per `CompareReplace` described above.

What makes slaves useful (as opposed to just running the program through the master and then the slave as two separate transformations) is the ability to restrict what the slave can match based on the context of a particular match in the master. This is achieved in two ways:

1. Nodes in the slave search pattern may be coupled into nodes in the master pattern. Each such coupling must be keyed in the master (that is, it must include a node in a normal context in the master search pattern or a builder node in the master replace pattern). In the example, the variable modified by the master would be coupled into the slave to ensure it only finds usages of the same variable.

2. The slave node is placed within the master replace pattern. Now the slave will only match at (`SlaveCompareReplace`) or under (`SlaveSearchReplace`) the position where it is found in the master replace pattern. In the example, the slave pattern could be placed inside the scope containing the variable's declaration, to restrict matches to that scope.

### 10.1 Nested

It is possible to use more than one slave in a pattern, they appear nested (that is as direct or indirect children of each other) and can nest in two distinct ways:

1. A second slave may be pointed to by the `through` member of the first. These slaves are conceptually at the same level, and the nesting just defines the order in which they operate: child first. Both slave patterns operate for each match of the master in the prescribed order. The relationship here is 1:n+m.

2. A second slave may be pointed to by the `replace` member of the first. This creates a sub-slave, which has the relationship described above to the first slave (so the first slave now acts as master to the second). The second slave acts for each match of the first. Overall, we get a 1:n:nm type of relationship.

Note: having these two nesting options can be confusing: the first style looks too much like the second and can be easily confused when studying graphs, so it's important to look out for this.

### 10.2 Important note

The patterns under slaves must be fully assembled before the slave is constructed or configured. This is because configuration (which is triggered by construction with pattern pointers) examines the patterns and may cache information about them or set hidden properties on them. 

## 11 Spinning prevention

As described above, `VNTransfomation` acts repeatedly until no further matches are found. This means the output program tree of a search and replace operation (that terminates) does not match the search pattern. This is a reliable post-invariant and serves the concept of Vida Nova as a principally _reductive_ or _goal-seeking_ algorithm. However, it is possible to create patterns that would repeat forever if every replace action creates a tree that will match. For this reason, any algorithm that re-tries a subtree that has been modified by a replace could spin forever.

Some program transformation approaches guarantee termination by for example traversing the input program exactly once while looking for matches on the way. This approach can suffer from missed matches when the replace action creates a match that was not present during the initial traversal (if the desired invariant is that there should not be a match, we have not obtained it). 

As a practical matter, a (buggy) search and replace pattern that spins forever may be spotted easily. Debugging this scenario is actually more straightforward than the alternative scenario of debugging a pattern that does not match due to traversal rules and thereby breaks the post-invariant condition. 

Typically, one uses the `-rn<x>` command-line option to permit repeating some finite number of times before terminating without error. In the resulting output program, the results of many (say 10 to 100) repetitions of the pattern should be obvious due to the repeted structures and possibly extreme nesting depth. 

There follow some methods for preventing spinning - these must be applied separately to the master and to any slaves. This is unlikely to be a complete list.

### 11.1 Reductive patterns

Any replace pattern that strictly reduces the number of nodes in the tree for each hit, must eventually terminate. We can estimate whether a pattern has this property by counting the number of non-special nodes in the search and replace pattern and noting that the replace pattern is indeed smaller. Care must be taken around `Delta` nodes, to determine the size change that results. 

Further, it suffices to reduce the number of occurrences of any single node type, as long as it is explicitly matched by the search pattern. For example, the `ForToWhile` transformation acts only when a `For` node is seen, and produces an output tree not including the For node that was matched. Even though the output trees are typically bigger, the number of `For` nodes strictly reduces and the transformation will terminate when there are no more `For` nodes in the program. 

These patterns are termed _strictly reductive_.

### 11.2 _But-not_ pattern

Where a pattern converts a general form of a construct to a specific form the reduction is less readily apparent. What is being removed is any construct of the general kind but not the specific kind. For example, converting general usage of`If` into a restricted form like `If(value:Expression, then:Goto, else:Nop)` is actually trying to reduce away the if statements that are not in the form shown.

A common solution is to insert `Conjunction` in the search pattern, with a `Negation` node on the other leg. So when reducing `X` to `Y`, the search pattern becomes `Conjunction( X, Negation(Y) )`. This provides an and-not or but-not type of restriction. The search pattern subtree `Y` will resemble the replace pattern, but must be supplied separately because a coupling would not make sense here (or be allowed because the subtree under `Negation` is an abnormal context).

#### 11.2.1 Anti-parenting

As an aside, the but-not pattern is useful for anti-parenting. This is where you want to find occurrences of a node whose parentage is _not_ some specific pattern. For example, if you want to differentiate between declarations and usages of a variable, the declarations may be matched easily as `Declaration( identifier:my_variable )`. But to find usages, we have to allow any other node that might point to an `InstanceIdentifier`, like `Operator`, `If` etc _including_ `Declaration` where the variable is used as the initialiser - obviously a usage.

There are a few ways to do this in Vida Nova, but the preferred one uses a combination of `Conjunction` and `Negation` to create an and-not pattern; the undesired parentage is expressed in the `Negation` branch, and the other branch points to the target node via `AnyNode` in order to match any other parentage. Written down, it looks like 

`Conjunction( Negation( bad_parent( my_node ) ), AnyNode( my_node ) )`

To modify just `my_node`, insert a `Delta` node before it in the `AnyNode` branch.

### 11.3 `GreenGrass` node

Vida Nova S&R supports a special node called `GreenGrass` to help in tricky termination cases. It is inserted into a search pattern via its through member. The immediate child node will be matched as normal, but only if it is a node from the input program tree. `GreenGrass` restricts the search to not match the node if it was produced by an earlier iteration of the replace algorithm. The specific rules for `GreenGrass` are as follows:
- During the first search in any `VNTransfomation` or slave, the `GreenGrass` node does not restrict at all
- During successive passes, `GreenGrass` restricts to nodes that reached the input program tree via `Stuff` node substitution in the previous pass. Note this includes the portion outside of the match when `VNTransfomation` is used.
- In cases where one pass creates a new node (that `GreenGrass` would reject) and then a later pass preserves it via `Stuff` node substitution, `GreenGrass` will still reject it.
- `GreenGrass` will restrict nodes created by the parent, even when located in a slave search pattern.

Broadly speaking, `GreenGrass` assumes that `Stuff` substitution is not really changing the nodes, just moving them around. It restricts matches to nodes from the original input tree that may be been moved around. 

## 12 Worked examples

A small example, two medium examples and one large one follow. 

### 12.1 removing `Nop`

`Nop` is a `Statement` node that represents "no operation". `CleanupNop` uses `VNTransfomation` to eliminate obviously redundant `Nop`s from code. The search pattern is a `Compound` statement with a `Star<Declaration>` node in the `Declaration`s collection. The sequence of `Statement`s in the `Compound` block (an ordered container) contain the following: `Star<Statement>`, `Nop`, `Star<Statement>`. All the `Star` nodes have no pre-restriction and are maximally wild, so they'll match anything. 

Our search can therefore match a `Compound` block with any decls, and any statements as long as there is at least one `Nop` (this transformation only deals with `Nop`s in `Compound` blocks). The `Nop` can be anywhere in the sequence of statements: the layout of `Star`, `X`, `Star` resembles `.*X.*` in regular expressions.

For replacing, we wish to preserve everything in the compound block except the `Nop`. And we wish to preserve the statements in the correct order. We supply a new `Compound` block in the replace pattern, and fill in its declarations and statements by pointing to the same `Star` nodes we created in the search pattern. Thus the `Star` nodes each have two parent nodes, and are coupled. The effect of coupling the star nodes is to reproduce whatever they matched (which could be 0 or mode nodes, complete with any subtrees) in the output program at that position. 

The pattern is strictly reductive: each hit will reduce the number of `Nop` statements by one, and the transformation will terminate when the program contains no `Nop` statements. The repeating nature of the Vida Nova S&R algorithm means we will keep trying until we don't get any hits. This means that a compound block containing many `Nop` statements will be correctly transformed after multiple hits. On each hit, the `Star<Statement>` nodes will match different subsequences of the statements in the compound block. For this step (and I believe in general) it does not matter whether the first `Nop` matched is the one at the top, or bottom or middle of the block.

### 12.2 Generate implicit casts

`GenerateImplicitCasts` adds a C-style cast to every function call argument that is not of the same type as the parameter in the function declaration. 

To do this we search for a call to a function (the `Call` node). Because function calls can be expressions (involving function pointers) the `Call` node specifies the function using an expression. The signature of the function is the type of this expression, and it contains the types of the parameters. To get the type of the expression, we specify `TransformOf<Expression>(Typeof)` node. The type is then compared with the pattern given as the child of `TypeOf`. Here we specify `Procedure`, which is the type for a callable entity that has parameters (but not necessarily a return value). `Function` derives from `Procedure`.

Note: Vida Nova stores parameters in an unordered collection, which differs from C source language which relies on parameter ordering. Arguments to a call are then specified in the form of a map, with the key being the parameter's identifier and the value being the argument expression. Vida Nova does not support maps directly (only sequences and collections) so the tree specification for C builds a map from a collection of nodes called `MapOperand` which act as key-value pairs. A call is therefore conceptually more like a named parameter call e.g. `foo( param1:arg1, param2:arg2 ...)`. This type of structure is easy to manipulate in search and replace using couplings. Ordered parameters would be more difficult because it would require an ordinal correspondence (index coupling). However, the reason for doing it this way is not S&R simplicity, but rather a desire not to store redundant information in the tree.

Since we will act on one parameter at a time, we use `Star` to match the other parameters. The one we are interested in will be an `Instance` node, since a parameter is a declaration of a variable-like construct. The identifier and type are given maximal wildcards to allow any parameter to match. In the arguments collection of the search pattern's `Call` node, we again supply a `Star` for other arguments, and a `MapOperand`, as explained above, for the argument in question. To ensure the `MapOperand` corresponds to the same parameter that we picked out of the declaration, we couple the identifier (which is the key of the map) to that of the parameter. 

We must restrict the search to cases where the types differ, so we use the but-not pattern, by placing `TypeOf` on the argument expression, and coupling to the parameter type under a `Negation` node. Therefore we will only hit when the argument matches the parameter by identifier, but strictly differs in type.

The replace pattern for this transformation is relatively simple. In the replace pattern's `Call`, we couple the other args and the expression that chooses the function, so that they are the same (when you couple to a `TypeOf` node, you get the expression; to get the type, couple to its child). We replace the missing argument with a new `MapOperand`, coupling the identifier to that of the `MapOperand` in the search pattern. For the expression, we supply a `Cast` node, which corresponds to a C-style cast. Its type is the type of the parameter in the declaration. The expression is coupled to the the argument expression. This will generate a cast that casts the expression in the original call to the type of the parameter in the function's declaration, as required.

Note that we see some three-way couplings around the parameter type and identifier. This works just fine, Vida Nova is cool about that sort of thing.

### 12.3 For to While

`ForToWhile` transforms For loops into semantically equivalent `While` loops. C makes this easy and hard. Easy because the three elements of a `For` loop are general C constructs that can simply be moved to the appropriate places around a `While` loop; hard because of `Break` and `Continue`. We do not have to worry about `Break` here because it has already been handled by another step, but `Continue` requires explicit treatment (`Continue` works in While loops, but we have to be careful about the semantics: the increment could be skipped).

Our master search pattern describes a general `For` loop. There is a `For` node, and its expression (the test) is filled in with an `Expression` node which is a maximal wildcard that will match any expression. Its init condition, increment and body are all maximally wild `Statement` nodes. Therefore, any `For` loop will match (Vida Nova fills absent statements with `Nop`, which will match).

The master replace pattern begins with a `Compound` node. This is there to make the pattern simpler, and there are clean-up steps that remove them where they are redundant. Generous use of `Compound` blocks is encouraged style. The compound has only 2 statements: the init statement from the `For` loop via a coupling, followed by a `While` node. The `While` loop's test expression is coupled in from the `For` loop's test expression. In the body of the `While` loop we place the `For` loop body (but see below) followed by the increment from the `For` loop. This completes the basic transformation - by inspection, you can determine that the constructs from the `For` loop are in the right places. 

In order to deal with `Continue`, we insert a `SlaveCompareReplace` into the master replace pattern, just above where we couple the body of the `For` loop. Recall that the through member is simply there to allow the master replace pattern to continue. During each master replace (in other words, as S&R is building the While loop) the slave will act on the replaced version of the subtree under the through link. This is the body of the original `For` loop (via coupling).

In this body, we wish to change the behaviour of `Continue`: we wish to insert a copy of the increment statement just before each continue, to compensate for continue skipping the increment we placed at the end of the body. The positioning of the slave ensures we do not change continues outside of `For` loops. Were it not for two awkward issues, we would simply use `SlaveSearchReplace`, search for continue and replace with a compound containing the increment condition (coupled) followed by continue. This is indeed approximately how the slave is laid out, but there are extra nodes to solve the problems:

1. The slave replace pattern contains continue, so the slave would not terminate. We want each continue in the input program (under a For) to be replaced exactly once by the compound of increment and continue. More or fewer replacements would increment too many or too few times. The `GreenGrass` node ensures we only replace continues that were in the input tree, and not ones that came from our replace pattern.

2. Not all continue statements under a `For` loop apply to that loop, because there could be nested loops (see the C spec). We cannot just use a `SlaveSearchReplace` because it would find continues that we don't want to change. The actual pattern used is a stuff node, which can recurse to find a continue (continue is the terminus, via the `Delta` node), and it has a recurse restriction filled in with `Negation` and a `Loop` wildcard. This part of the pattern means "not any kind of `Loop`" and the `Stuff` node will therefore not recurse through any loops as it looks for a terminus match. `Stuff` is followed by a `Delta` node, which allows the new `Compound` to be overlayed over the original `Continue`.

Note that if there was no need for a recurse restriction, we would use `SlaveSearchReplace` and get a more readable pattern.

### 12.4 Generate stacks

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

## 13 Notes on pattern development in C++

At present, S&R patterns are built by short C++ routines in the constructors for step classes. We derive step classes from S&R so that the S&R interfaces are inherited, and pass the patterns in via `Configure()`.

Vida Nova uses a template type called `TreePtr<>` to point to child nodes. This is derived from `std::shared_ptr`, but supports some extra functionality needed by Vida Nova.

### 13.1 Helpful syntactic sugar

`MakeTreePtr<>` can simplify the construction of `TreePtr<>` members. `MakeTreePtr<X>` may be constructed in the same way as `X`, but will then masquerade as a `TreePtr<X>` where the pointed-to `X` has been allocated using new. It is similar to `std::make_shared<>()` except that being a class with a constructor, rather than a free function, it may be used as a declaration as well as in a function-like way. One drawback is that if the constructor of `X` has a large number of parameters, the implementation of `MakeTreePtr<>` may need to be extended.

Vida Nova containers (sequence and collection) support initialisation directly from `TreePtr`s of the right type, and from comma-separated lists of `TreePtr`s (via `operator,` overloading). This can avoid the need for repeated calls to `insert()` or `push_back()`.

13.2 Style tips

When building a `Container` from `TreePtr`s using the comma operator, it is good style to enclose the entire list in parentheses. This looks better, and is necessary when the container is being passed to a function, so that the compiler does not assume the commas are there to separate function arguments.

Prefixes may be used with variable names to help clarify at the discretion of the programmer. I have used `s_` for search pattern nodes and `r_` for replace pattern nodes, with no prefix for nodes coupled into both (or `c_` could be used here). I prepend `l_` for slaves and sometimes insert `x` into nodes under a `Negation` node, such as with the but-not pattern. Further slaves at the same level (reached via `through`) are named `m_`, `n_`, `o_` etc. Sub-slaves recieve two letters, eg `ll_` or `nm_`. In this case `n` is master and `m` is second level slave.

I usually leave numbers and strings that are passed into `SpecificX` or `Builder` nodes as magic, possibly with a comment. Any macros, enums etc will be reduced to the raw number in a graph plot anyway.

The order in which pattern nodes are created and connected together is mostly flexible, since members are mostly assigned via direct member assignment. It is mostly possible to declare all the nodes and then connect them all together. One exception is the slave nodes, which are actually instances of the `VNTransfomation` engine, and require either to be constructed using a constructor call or via `Configure()`.

### 13.3 Warning about empty versus maximally wild

Remember that a singular `TreePtr<>` in a search pattern that is set to NULL is a maximal wildcard, i.e. will match any node that is type-compatible. On the other hand, a `Collection<>` or `Sequence<>` that is left empty only matches an empty `Collection<>` or `Sequence<>` in the input program. If, when creating steps, these members are left uninitialised, the `TreePtr<>` will be NULL and the `Collection<>` or `Sequence<>` will be left empty. These behaviours differ, and this can be a gotcha when writing steps.

It does not make sense for a singular `TreePtr<>` to be "empty" - this is not supported in Vida Nova, and singular relationships are always 1:1 in program trees. It does make sense to wildcard a `Collection<>` or `Sequence<>` - the way to do this is to insert a single `Star<>` of the contained type. It is unfortunately impossible to write

```
MakeTreePtr< Collection<X> > my_collection;
my_collection = ();
```

but it might be a good style to write

```
//my_collection = ();
```

so that the reader can see that omission means "be an empty container" rather than "be a wildcard".

### 13.4 Most important tip

It is a very good idea to use graph plots right from the start when developing steps. Look at the graphs for the kinds of input test programs you are interested in. Code up a program resembling the expected output program, and look at the graph for that too, side by side. These will give a starting point for what the search and replace patterns will need to look like. As soon as you have a transformation that compiles, even if incomplete, take a look at the pattern graph and see if it looks right. The graphs are not just for fun, they really help!

Good luck!

