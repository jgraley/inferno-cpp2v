# Vida Nova Search and Replace User Guide

This document explains how to create search and replace patterns for Vida Nova so that it will perform the transformation you require. It will be useful to all users of Vida Nova.

These patterns are thmselves trees, similar to program trees, but with a number of extensions, both in terms of additional kinds of nodes that can be used, and in terms of relaxing the rules that apply to program trees.

## 1 `VNTransfomation` and `VNSequence`

The generic search and replace implementation may be found in the `VNSequence` and `VNStep` classes and a number of supporting classes. A `VNStep` object is configured by supplying a search pattern and a replace pattern, both in the form of trees. One or more `VNStep` objects must be placed in a `VNSequence` object. This may be used to plan and then execute a sequence of transformations.

### 1.1 Configuring `VNStep`

The first two argument to `VNStep::Configure()` must be one of `SEARCH_REPLACE` or `COMPARE_REPLACE`. This is followed by the search pattern and the replace pattern. Each pattern is a pointer to the root of a subtree of nodes. As with Vida Nova program trees, branches are allowed to merge, making them strictly speaking acyclic directed graphs, but we call them trees in line with compiler parlance. 

If `SEARCH_REPLACE` is specified, `VNTransfomation` will search input trees for an occurrence of the search pattern and replace it with a duplicate of the replace pattern if found. If `COMPARE_REPLACE` is specified, `VNTransfomation` will compare input trees with the search pattern and replace it with a duplicate of the replace pattern if it matches.

### 1.2 Special nodes

S&R supports additional nodes called "special" nodes that do not appear in the program representations for any language. They invoke additional S&R functionality when used in search and/or replace patterns. Since Vida Nova tree pointers are strongly typed (links are pointers to an intermediate type corresponding to the subset of child node types allowed), special nodes are templated on a user-specified type called the base type. Normally, the base type should be the same type as the pointer that will point to the special node, but see pre-restriction under topological wildcarding.

### 1.3 Termination

The `VNTransfomation` algorithm repeatedly attempts to find a match to the search pattern, and substitutes the replace pattern, until it can no longer find a match to the search pattern. Therefore, it can run forever if for example the replace pattern re-creates a search pattern match. It is the user's responsibility to prevent this from occurring: this is discussed in more detail later.

## 2 Topological wildcarding

Vida Nova nodes are organised in a hierarchy of _categories_ beginning with `Node`, from which certain intermediate categoriess like `Statement` and `Type` are derived, and then the final nodes are derived from the intermediates. In many cases there are multiple levels of intermediate categories between `Node` and the final node type, and multiple inheritance is allowed. This hierarchy is best understood as a Euler diagram, where subcategory is synonymous with subset.

The trees for programs are only allowed to consist of final nodes. So `Node` and the intermediates are not allowed in program trees. They are allowed, however, in search and replace patterns as long as they would not end up in the output of the search and replace transformation. 

When an intermediate node is seen in a search pattern, it serves as a wildcard for any node derived from it. For example, `Return` and `Goto` are both derived from `Statement`, so a `Statement` node in a search pattern will match `Return`, `Goto` and any other node derived from `Statement`. In set theoretic terms, we have restricted the search to the set of statements.

As an example of multiple layers of intermediates, consider that `Expression` is derived from `Statement`, making expressions a subset of statements. This is consistent with the rule in C that an expression may be used in place of a statement, but not the other way around. Consequently, a node of type `Statement` in a search pattern will match any expression as well as other kinds of statements, but an `Expression` node in a search pattern will only match expressions.

Due to Vida Nova's type-safe tree, it is not possible to specify nonsensically broad intermediates in a search pattern. This is because the pointers used in nodes are typically pointers to intermediate node types, and only identical or more derived (subset) nodes may be pointed to. Therefore the broadest wildcard possible is one whose type is the same as that of the pointer that will point to it. Such a pointer is called maximally wild and will match all input program subtrees.

Maximally wild pointers may be considered similar to the use of `.` character in regular expressions. Other intermediate topological wildcards could be likened to a value range in certain textual schemes for example `[a-zA-Z]` though it should be noted that topological (set-theoretic) restriction is more concise than linear range restriction.

### 2.1 Can populate the wild card's members

In the Vida Nova tree, some intermediate nodes have members: C++ copies these members into derived node categories during inheritance. Members may be filled in when intermediates are used in a search pattern. These members will be matched as normal and will serve to restrict the search pattern.

For example, giving the node `Integral` and pointing the width member to a `SpecificInteger` of value 32 (which we could write `Integral(width:SpecificInteger(32)))` will match 32-bit signed and unsigned data types since `Signed` and `Unsigned` both derive from `Integral`.

Note: to wildcard the width and fix the signedness we would use e.g. `Signed(width:Integer)`, in other words we use the final node for the type `Signed` and fill in the width with the maximal wildcard, which is `Integer` in this case. See tree.hpp for the definitions of these nodes.

### 2.2 NULL in search patterns

In trees for programs, we do not allow NULL (cases of disabled functionality must be dealt with explicitly, for example with nodes such as `Nop` and `Void`).

We do allow NULL for singular links in in search patterns, where it serves as shorthand for the maximal wild card. Therefore, `Signed(width:Integer)` is the same as `Signed(width:NULL)` as a search pattern.

### 2.4 Pre-restriction on special nodes

When searching, all special nodes support search restriction via the base type supplied as the first template parameter (recall that special nodes derive from the supplied base type). This base type is called the pre-restriction type. Regardless of what the special node is defined to do, all searches will restrict to nodes that are of non-strict subcategories of the pre-restriction category. 

Type-safety means that a tree pointer can only point to non-strict subclasses of the pointer type. Therefore if the pre-restriction type is the same type, the pre-restriction will be maximally wild and effectively disabled. Only if a strict subcategory is given will a restriction be seen to take effect. The term _pre-restriction_ reminds us that it seems to apply before the special node's special algorithm runs (which will typically apply some further restriction independently of the pre-restriction under an "and" rule).

### 2.5 Notes on topology-oriented tree

In order to maximise the utility of topological wildcarding (and its coupling support) the Vida Nova tree has been contrived to express as much information as possible in the node hierarchy and through pointers to child nodes. So for example where a node might naturally contain a boolean (such as the signedness of an integral type) or an enum (such as an access specifier which can be public, private or protected) one of two techniques is used:

- the options for node `X` are expressed in a set of final nodes derived from `X`, so that `X` becomes an intermediate node. This can only be done for one field of node `X`. For example, consider nodes `Signed` and `Unsigned`, which both derive from the original node `Integral`, and indicate signedness. For a pattern in which an integral type can be signed or unsigned, `Integral` is the appropriate wildcard. `Signed` or `Unsigned` are used when the signedness matters.

- a new hierarchy of nodes under a new intermediate `Y` is created, and the original node `X` contains a pointer to `Y` in place of the original field. This may be applied to any field of any given `X`. For example `Public`, `Private` and `Protected` are all subcategories of `AccessSpec`, and the `Definition` node points to `AccessSpec`.

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

 - If `PO` is non-NULL (`PT` is always non-NULL) and the node at `PT` is a non-strict subcategory of the node at `PO`, then we overlay, by recursing for `PO'` = each pointer member of the node at `PO`, and `PT'` = the corresponding member of the node at `PT`. This correspondence is the reason for the subcategory requirement. 
 - If `PO` is NULL or the node at `PT` is not a non-strict subcategory of the node at `PO`, then we overwrite the subtree at `PT` with a copy of the subtree at `PO`. We have now finished this branch and do not need to repeat.
 - Containers (sequences and collections) always overwrite when encountered in the node at `PO`.
 - Local data members are taken from `PO` if it is of the same type as the input tree node being overlaid, otherwise they are left alone. This is only OK, and #593 aims to improve.

Note that the condition for overlaying versus overwriting depend only on the patterns provided for `overlay` and `though` in the `Delta` node, not on the contents of the input program tree.

### 3.5 Notes on keyability of couplings

Vida Nova search and replace has an implementation concept called keying, which is the process of choosing a particular input program subtree that will correspond to a particular coupling. Each coupling only needs to be keyed once for each search hit, and once it is keyed, the key-to-coupling relationship remains unchanged until the replace has completed.

Normally, Vida Nova search patterns exhibit an implicit "and" rule, in the sense that every node in the search pattern must match a node in the input program tree before a match is considered successful. This would make every node in the search pattern a candidate for keying a coupling. We say such nodes are in a "normal context". But some special nodes permit a match even when some subtree under that node does not match. These subtrees are said to be "abnormal contexts". 

An abnormal context extends throughout the entire subtree under the pattern node that caused it. So a normal context can contain an abnormal context as a subtree, but an abnormal context remains abnormal all the way down to the leaf nodes. The `Negation` node matches when its supplied subtree does not match, making its subtree pattern an abnormal context. See the documentation for `Negation` for more explanation. There are other sources of abnormal contexts, indicated below.

The rule is that you cannot couple nodes in separate abnormal contexts. A coupling must be reached through a normal context, or reached only from within one abnormal context.

## 4 `Star` search pattern

The Vida Nova tree supports one-to-many relationships using containers, of which two types are available: sequence, which preserves ordering and collection, which does not. When creating a search pattern it can be useful to be able to match zero or more elements of a container. We can do this using a special node called `Star<>`, which is templated on the collection's element type. So a container of pointers to `Statement`s can be wildcarded using `Star<Statement>`. 

If a pre-restriction is given, every container element matched by the `Star` must satisfy the pre-restriction (i.e. be a non-strict subcategory of the supplied base class). Additionally, the `pattern` member may point to a subtree, which must be matched by every element that the star node matches. This pattern is an abnormal context.

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

### 5.2 `Child` pattern

As mentioned above, the `Stuff` node can match at the root of the input program subtree, i.e. zero levels of recursion. Therefore a pattern of `If( Stuff( For(...) ) )` would match a program like `If( For(...) )`. `Child` is an alternative to `Stuff` that matches only on exactly one level of indirection. A pattern like `If( Child( For(...) )` will match for example `If( While( For(...) ) )` but not `If( For(...) )` or `If( While( Switch( For(...) ) ) )`.

`Child` supports pre-restriction and has a `terminus` just like Stuff, but there is no recurse restriction. Child is useful for anti-parenting, see below.

## 6 Boolean search patterns

Search patterns may be restricted according to boolean rules, as explained below. 

### 6.1 `Negation`

`Negation` implements the "not" function. The `Negation` node should be placed at the root of the subtree of interest, and the member named `negand` should be set to point to the subtree that should not match. The pattern can itself include wildcards, so a `Negation(Integral)` would match any data type other than signed or unsigned integer types.

The subtree under a `Negation` node in a search pattern is an abnormal context, in other words it cannot key a coupling. Consider a `Negation` node in a search pattern that is coupled to a node in a replace pattern. This will work as expected - the subtree that did not match the negand is substituted successfully. 

But consider coupling a node within the `Negation`'s subtree into the replace pattern. If there was an overall match of the search pattern, that must mean that some node in the negand did not match. It is possible that the `Negation` pattern is not even of the same layout (topology) as the corresponding subtree in the input program. If this is the case, it is not well defined which node or subtree should be substituted in the replace pattern.

### 6.2 `Conjunction`

`Conjunction` implements the "and" function. The `Conjunction` node should be placed at the root of the subtrees of interest, and the member named `conjuncts` (a collection) should be populated with all the subtrees that should match. `Conjunction` does creates a normal context for its pattern subtrees, so they can key couplings. Conjuncts are not invoked during replace (since it would be unclear which one to use) so a `Delta` node must be placed around the `Conjunction` node, coupling its overlay pattern to the desired element.

When one of the conjuncts is a `Negation` node, it may be helpful to think of the `Conjunction` as meaning "but" rather than "and".

### 6.3 `Disjunction`

`Disjunction` implements the "or" function. The `Disjunction` node should be placed at the root of the subtree of interest, and the member named `disjuncts` (a collection) should be populated with all the subtrees that may match. The subtrees under a `Disjunction` node in a search pattern abnormal contexts because a given pattern element may not be a match even when the `Disjunction` itself does match.

## 7 `TransformOf` search patterns

Sometimes, a transformation will appear to be mostly suitable for the search and replace approach, but there may be aspects of the required matching criteria which cannot be described using the above wildcarding techniques. In such cases, Vida Nova supports the ability to run some user supplied algorithm during the search. The user supplies a pointer to an `OutOfPlaceTransformation` object to the constructor of `TransformOf<X>`. Objects of the resulting type derive from `X`, which is a pre-restriction. `TransformOf<>` has a member named `pattern` which is a pointer to any `Node`.

When the search engine encounters a `TransformOf<>` node, it applies the transformation to the current input program subtree (without modifying the original copy) and then proceeds to compare the result of the transformation with the pattern pointed to by the `pattern` member.

Transformer nodes allow you to couple both the transformer node itself (keying as the untransformed input program subtree) and the node pointed to by `pattern` (keying as the input program subtree after transformation).

### 7.1 `TypeOf`

`TypeOf` is a utility transformation in the form of a singleton class, which we can embed by constructing `TransformOf<Expression> mynode( &TypeOf::instance )`. It is used in the context of an `Expression`. During a search, the `TypeOf` algorithm will determine the type of an expression in the input program subtree, and then compare that with the type pointed to by the `pattern` member (note that types and expressions are represented as subtrees under `Type` and `Expression`, respectively). 

Note that if the pattern is maximally wild, then `TypeOf` will match any valid expression. However, it can be convenient to couple the `Type` wildcard with another `Type` in the search pattern, to restrict for type equivalence, or with a node in the replace pattern, to reproduce the type in the output program.

### 7.2 `DeclarationOf`

DeclarationOf is a utility transformation in the form of a singleton class, which we can embed by constructing `TransformOf<InstanceIdentifier> mynode( &DeclarationOf::instance )`. It is used in the context of an `InstanceIdentifier`. During a search, the `DeclarationOf` algorithm will search for the declaration of an identifier in the input program, and then compare that with the declaration pointed to by the `pattern` member. 

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

## 10 Embedded Search/Compare and Replace

It can be useful to build a 1:n relationship between two search and replace patterns. For example, we may wish to perform a transformation on a variable that modifies the declaration and all the usages of the variable. In this example, we would want each match to a variable's declaration to correspond with n matches to usages, where n can be zero or more.

This is achieved by constructing the search and replace pattens for the root (the variable declaration in the example) and then placing a special node called `EmbeddedSearchReplace` into the root replace pattern. The `EmbeddedSearchReplace` node is filled in with search and replace patterns (a usage of the variable in the example). For each match of the root search/compare pattern, the replace pattern will be applied, _and then_ the embedded search/compare patterns will be applied to the result repeatedly until no match, for all embedded patterns in the root pattern. This works recursively: we will say that an embedded pattern node is embedded within its _enclosing_ pattern. 

`EmbeddedSearchReplace` has three members:
 - `through`, which simply points to the remainder of the enclosing replace pattern;
 - `search` and
 - `replace` which specify the embedded behaviour as documented for `VNTransfomation`.

You can also use `EmbeddedCompareReplace`, which has `through`, `compare` and `replace` members and behaves as per `CompareReplace` described above.

What makes embedded pattern useful (as opposed to eg sequencing multiple root transformations) is the ability to restrict what the embedded pattern can match based on the context of a particular match in the enclosing pattern. This is achieved in two ways:

1. Nodes in the embedded search pattern may be coupled into nodes in the enclosing pattern. Each such coupling must be keyed in the enclosing pattern. In the example, the variable modified in the enclosing pattern would be coupled into the embedded pattern to ensure it only finds usages of the same variable.

2. The embdeded node is placed within the enclosing replace pattern. Now the embedded node will only match at (`EmbeddedCompareReplace`) or under (`EmbeddedSearchReplace`) the position where it is found in the enclosing replace pattern. In the example, the embedded pattern could be placed inside the scope containing the variable's declaration, to restrict matches to that scope.

### 10.1 Nested embedded pattern

It is possible to use more than one embedded pattern. They always appear nested in a graph of the pattern (that is as direct or indirect children of each other) and can nest in two distinct ways:

1. A second embedded pattern may be pointed to by the `through` member of the first. These embedded pattern are conceptually at the same level, and the nesting just defines the order in which they operate: child first. Both embedded patterns operate for each match of the enclosing pattern in the prescribed order. The relationship here is 1:n+m.

2. A second embedded pattern may be pointed to by the `replace` member of the first. This is truly recursive, and embeds the second pattern into the first embedded pattern. The second embedded pattern acts for each match of the first. Overall, we get a 1:n:nm type of relationship.

Note: having these two nesting options can be confusing: the first style looks too much like the second and can be easily confused when studying graphs, so it's important to look out for this.

### 10.2 Important note

Conceptually there can be at least two policies for when embedded agents act. Defined for Vida Nova, we have:
 - `SOONER` - embedded search/compare and replace acts and runs to completion during enclosing replace, at the time replace reaches that node
 - `LATER` - embedded search/compare and replace acts and runs to completion only after enclosing replace has completed
 
We use the `LATER` policy, since `SOONER` can leave the run-time sequence of events ambiguous, and may not even be meaningful if the idea of a "replace sequence of events" stops being meaningful.

## 11 Spinning prevention

As described above, `VNTransfomation` acts repeatedly until no further matches are found. This means the output program tree of a search and replace operation (that terminates) does not match the search pattern. This is a reliable post-invariant and serves the concept of Vida Nova as a principally _reductive_ or _goal-seeking_ algorithm. However, it is possible to create patterns that would repeat forever if every replace action creates a tree that will match. For this reason, any algorithm that re-tries a subtree that has been modified by a replace could spin forever.

Some program transformation approaches guarantee termination by for example traversing the input program exactly once while looking for matches on the way. This approach can suffer from missed matches when the replace action creates a match that was not present during the initial traversal (if the desired invariant is that there should not be a match, we have not obtained it). 

As a practical matter, a (buggy) search and replace pattern that spins forever may be spotted easily. Debugging this scenario is actually more straightforward than the alternative scenario of debugging a pattern that does not match due to traversal rules and thereby breaks the post-invariant condition. 

Typically, one uses the `-rn<x>` command-line option to permit repeating some finite number of times before terminating without error. In the resulting output program, the results of many (say 10 to 100) repetitions of the pattern should be obvious due to the repeted structures and possibly extreme nesting depth. 

There follow some methods for preventing spinning - these must be applied separately to the root and any embedded patterns. This is unlikely to be a complete list.

### 11.1 Reductive patterns

Any replace pattern that strictly reduces the number of nodes in the tree for each hit, must eventually terminate. We can estimate whether a pattern has this property by counting the number of non-special nodes in the search and replace pattern and noting that the replace pattern is indeed smaller. Care must be taken around `Delta` nodes, to determine the size change that results. 

Further, it suffices to reduce the number of occurrences of any single node type, as long as it is explicitly matched by the search pattern. For example, the `ForToWhile` transformation acts only when a `For` node is seen, and produces an output tree not including the For node that was matched. Even though the output trees are typically bigger, the number of `For` nodes strictly reduces and the transformation will terminate when there are no more `For` nodes in the program. 

These patterns are termed _strictly reductive_.

### 11.2 _But-not_ pattern

Where a pattern converts a general form of a construct to a specific form the reduction is less readily apparent. What is being removed is any construct of the general kind but not the specific kind. For example, converting general usage of`If` into a restricted form like `If(value:Expression, then:Goto, else:Nop)` is actually trying to reduce away the if statements that are not in the form shown.

A common solution is to insert `Conjunction` in the search pattern, with a `Negation` node on the other leg. So when reducing `X` to `Y`, the search pattern becomes `Conjunction( X, Negation(Y) )`. This provides an and-not or but-not type of restriction. The search pattern subtree `Y` will resemble the replace pattern, but must be supplied separately because a coupling would not make sense here (or be allowed because the subtree under `Negation` is an abnormal context).

#### 11.2.1 Anti-parenting

As an aside, the but-not pattern is useful for anti-parenting. This is where you want to find occurrences of a node whose parentage is _not_ some specific pattern. For example, if you want to differentiate between declarations and usages of a variable, the declarations may be matched easily as `Declaration( identifier:my_variable )`. But to find usages, we have to allow any other node that might point to an `InstanceIdentifier`, like `Operator`, `If` etc _including_ `Declaration` where the variable is used as the initialiser - obviously a usage.

There are a few ways to do this in Vida Nova, but the preferred one uses a combination of `Conjunction` and `Negation` to create an and-not pattern; the undesired parentage is expressed in the `Negation` branch, and the other branch points to the target node via `Child` in order to match any other parentage. Written down, it looks like 

`Conjunction( Negation( bad_parent( my_node ) ), Child( my_node ) )`

To modify just `my_node`, insert a `Delta` node before it in the `Child` branch.

### 11.3 `GreenGrass` node

Vida Nova S&R supports a special node called `GreenGrass` to help in tricky termination cases. It is inserted into a search pattern via its through member. The immediate child node will be matched as normal, but only if it is a node from the input program tree. `GreenGrass` restricts the search to not match the node if it was produced by an earlier iteration of the replace algorithm. The specific rules for `GreenGrass` are as follows:
- During the first search in any `VNTransfomation` or embedded pattern, the `GreenGrass` node does not restrict at all
- During successive passes, `GreenGrass` restricts to nodes that reached the input program tree via `Stuff` node substitution in the previous pass. Note this includes the portion outside of the match when `VNTransfomation` is used.
- In cases where one pass creates a new node (that `GreenGrass` would reject) and then a later pass preserves it via `Stuff` node substitution, `GreenGrass` will still reject it.
- `GreenGrass` will restrict nodes created by the enclosing pattern, even when located in an embedded search pattern.

Broadly speaking, `GreenGrass` assumes that `Stuff` substitution is not really changing the nodes, just moving them around. It restricts matches to nodes from the original input tree that may be been moved around. 

## 12 Notes on pattern development in C++

At present, S&R patterns are built by short C++ routines in the constructors for step classes. We derive step classes from S&R so that the S&R interfaces are inherited, and pass the patterns in via `Configure()`.

Vida Nova uses a template type called `TreePtr<>` to point to child nodes. This is derived from `std::shared_ptr`, but supports some extra functionality needed by Vida Nova.

### 12.1 `MakePatternNode<>()`

`MakeTreeNode<>()` is similar to `std::make_shared<>()` but creates `TreePtr<>` instances. However, when building patterns you will normally use `MakePatternNode<>()` which:
 - Invokes `MakeTreeNode<>()` directly for special nodes.
 - Wraps ordinary tree nodes in a wrapper that allows them to be used in patterns.

### 12.2 Container initialisation

Vida Nova containers (sequence and collection) support initialisation directly from `TreePtr`s of the right type, and from comma-separated lists of `TreePtr`s (via `operator,` overloading). This can avoid the need for repeated calls to `insert()` or `push_back()`.

### 12.3 Style tips

When building a `Container` from `TreePtr`s using the comma operator, it is good style to enclose the entire list in parentheses. This looks better, and is necessary when the container is being passed to a function, so that the compiler does not assume the commas are there to separate function arguments.

Prefixes may be used with variable names to help clarify at the discretion of the programmer. I have used `s_` for search pattern nodes and `r_` for replace pattern nodes, with no prefix for nodes coupled into both (or `c_` could be used here). I prepend `l_` for embedded and sometimes insert `x` into nodes under a `Negation` node, such as with the but-not pattern. Further embedded at the same level (reached via `through`) are named `m_`, `n_`, `o_` etc. Recusing embedded recieve two letters, eg `ll_` or `nm_`. In this case `n` is root and `m` is second level embedded pattern.

I usually leave numbers and strings that are passed into `SpecificX` or `Builder` nodes as magic, possibly with a comment. Any macros, enums etc will be reduced to the raw number in a graph plot anyway.

### 12.4 Warning about empty versus maximally wild

Remember that a singular `TreePtr<>` in a search pattern that is set to NULL is a maximal wildcard, i.e. will match any node that is type-compatible. On the other hand, a `Collection<>` or `Sequence<>` that is left empty only matches an empty `Collection<>` or `Sequence<>` in the input program. If, when creating steps, these members are left uninitialised, the `TreePtr<>` will be NULL and the `Collection<>` or `Sequence<>` will be left empty. These behaviours differ, and this can be a gotcha when writing steps.

It does not make sense for a singular `TreePtr<>` to be "empty" - this is not supported in Vida Nova, and singular relationships are always 1:1 in program trees. It does make sense to wildcard a `Collection<>` or `Sequence<>` - the way to do this is to insert a single `Star<>` of the contained type. It is unfortunately impossible to write

```
MakePatternNode< Collection<X> > my_collection;
my_collection = ();
```

but it might be a good style to write

```
//my_collection = ();
```

so that the reader can see that omission means "be an empty container" rather than "be a wildcard".

### 12.5 Most important tip

It is a very good idea to use graph plots right from the start when developing steps. Look at the graphs for the kinds of input test programs you are interested in. Code up a program resembling the expected output program, and look at the graph for that too, side by side. These will give a starting point for what the search and replace patterns will need to look like. As soon as you have a transformation that compiles, even if incomplete, take a look at the pattern graph and see if it looks right. The graphs are not just for fun, they really help!

Good luck!

