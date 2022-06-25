# Vida Nova Node Interface
        
This document explains how to define nodes for Vida Nova so that programs may be represented using trees (in the compiler sense ) made up of these nodes. It will be useful to all users of Vida Nova.

## 1 Introduction        
        
Vida Nova tree nodes implement a set of virtual functions defined by the base class `Node`, as well as observing certain stylistic conventions (termed topology-oriented). Both of these are documented here.

The reason for defining an interface at this level is to allow algorithms to process trees (for analysis and transformation) without needing knowledge of the particular language that is being represented. In effect, a tree for a particular language is the implementation behind the node interface, and a generic algorithm is a client of the node interface. We also aim to keep nodes easy to manipulate by algorithms that are aware of the language being represented.

Program trees are networks of nodes (actually acyclic directed graphs). We do not attempt to place a tree within a single container object which could marshal the incoming method calls, because this approach is too inflexible - algorithms need to get "close" to the nodes themselves. So instead we expose a tree as a multiplicity of nodes and ensure that client code can move between nodes as required.

## 2 Topologically-Oriented tree

Vida Nova nodes are typically inheritance hierarchies under `Node`, including intermediate nodes which represent categories. We use the term _category_ rather than _class_ to clarify that actual C++ classes needn't (in principle) be the underlying representation of a system of tree node definitions. It is useful to see such a hierarchy in a set-theoretical sense, where subcategory is equivalent to subset. Thus the base category called `Node` is equivalent to the set of all nodes, and an intermediate node is equivalent to the subset of nodes that inherit from it. 

Multiple inheritance is allowed, and by convention Vida Nova trees use virtual inheritance so that diamonds do not duplicate the base (our inheritance is true specialisation and so we want the set-union of members, not composition in disguise where we would want concatenation).

Only final (i.e. not intermediate) nodes may appear in the tree for a program. Intermediate nodes can, however, legally be constructed in other contexts such as search and replace patterns.

In general, we try to express as much information as possible through
 - the types of nodes and their super-categories and
 - the structure of the links between nodes.
 
For example if a node naturally seems to contain a boolean or enum, we will prefer to either
 - redefine the node as intermediate, and derive a final node for each option or
 - create a new intermediate and final nodes called eg `Properties`, and add a child pointer to the new intermediate to our node
 
Both of these approaches remove the need for a bool/enum _within_ a node. The information has been represented in the topological aspect of the tree. Sometimes it is inconvenient to represent data topologically, for example with int, float and string. So to be topologically-oriented it is sufficient to minimise the number of nodes defined with such data members.

A common practice in Vida Nova is the use of so-called archetypes. These are objects (in Vida Nova, they are nodes) whose purpose is only to indicate their type - i.e. the values of any members are irrelevent. An archetype is useful for storing and passing around type information at run-time without the need to resort to ugly and inflexible RTTI objects. By way of support for archetypes, we require that all node types should be constructible (including intermediates) and that they all contain a default (parameterless) constructor.

## 3 Node Virtual Functions

The `Node` class is actually built from a collection of sub-base classes, each of which defines one element of the interface and supplies a default implementation. The important sub-bases follow:

### 3.1 `Matcher`

Defines 2 virtual functions:
 - `virtual bool IsSubcategory( const Matcher *source_archetype ) const = 0;`
 - `virtual bool IsLocalMatch( const Matcher *candidate ) const;`

`IsSubcategory()` returns true if `source_archetype` is a non-strict subcategory of the category upon which the method was called. This method needs to be implemented separately in each type of node due to the limitations of C++. However, the required implementation is produced by including the macro `MATCHER_FUNCTION` (no semicolon required) in the node's class definition.

`IsLocalMatch()` returns true if the candidate is (a) the same as or (b) a member of the category described by the object upon which the method was called. It defaults to a call to `IsSubcategory()`.

Under our set-theoretical view of a tree, subcategory corresponds to subset. So `IsSubcategory()` would be the correct implementation for `IsLocalMatch()` on nodes that do not contain any data members and are therefore entirely defined by their type. If a node has members that point to other nodes, we ignore them in the matcher, which is the reason for the word `Local` in the function name. If there are data members of other type, it may be necessary to override `IsLocalMatch()` with a new, stricter check on the data members. 

Ultimately the choice of how to override `IsLocalMatch()` lies with the user, but the semantics must be consistent across the whole node interface the presented to the generic tools.

### 3.2 `Itemiser`

Defines the following virtual function:
 - `virtual vector< Itemiser::Element * > Itemise(const Itemiser *itemise_object) const = 0;`

`Itemise()` searches `itemise_object` for members derived from the class named `Itemiser::Element`, and returns them in a vector. The node on which `Itemise()` is called must be a non-strict supercategory of `itemise_object`. If the two types differ, `Itemise()` will only return members of `itemise_object` for which there exists a corresponding element in the object on which `Itemise()` was called.

To clarify, a full itemisation of all members of node `X` is achieved using `X.Itemise(X)` (a shorthand form, `X.Itemise()` is also allowed) because the called-on object is the same as the itemise_object and so has the same members. On the other hand, `X.Itemise(Y)` is only legal if `X.IsSubcategory(Y)==true`, meaning `X` is a super-category for `Y` and has some subset `{M(X)}` of the members of `Y`, `{M(Y)}`. Itemise will return the members of `Y` that correspond to members of `X` i.e. `{Y.M(X)}`. This behaviour is useful in generic algorithms because it allows (as far as possible) children of `Y` to be compared with or copied to/from the children of `X` without needing to know the internals of either `X` or `Y`.

`Itemise()` needs to be implemented separately in each type of node due to the limitations of C++. However, the required implementation is produced by including the macro `ITEMISE_FUNCTION` in the node's class definition.

The pre-existing `Itemiser::Elements` are: `TreePtr<>`, `Collection<>` and `Sequence<>` (see below). These are exactly the members that can point to other nodes, so `Itemise()` will supply all the members that relate to graph topology and none that relate to the values held within the node. 

The `Itemiser` class is obviously fairly flexible but to be compliant with the node interface defined here, you need to not override `Itemise()`, not add more `Itemiser::Element` types and not add new non-itemisable types that can point to nodes. You may add non-itemisable members that do not point to other nodes, i.e. ordinary data members, and the pre-existing itemisable types. This is so that generic algorithms like walk and search-replace can traverse trees.

### 3.3 `Cloner`

Defines 2 virtual functions:
 - `virtual shared_ptr<Cloner> Clone() const = 0;`
 - `virtual shared_ptr<Cloner> Duplicate( shared_ptr<Cloner> p ) {...}`

`Clone()` is the usual clone implementation: a new object is created, identical to the one on which `Clone()` was called. This method needs to be implemented separately in each type of node due to the limitations of C++. However, the required implementation is produced by including the macro `CLONE_FUNCTION` in the node's class definition. `Clone()` returns a `shared_ptr<>` because `shared_ptr<>` manages the lifecycle for all nodes. This `shared_ptr<Cloner>` is compatible with `TreePtr<Node>`.

`Duplicate()` has a default implementation to call `Clone()` in order to make a copy of a node. However, it is not mandatory to do this. A particular node implementation could, for example, do a shallow copy by simply returning the same node. This would tend to break if the returned node is then modified - an unexpected modification would occur to the original node (aliassing). Therefore there must not be any non-const members that would cause problems if modified unexpectedly. In particular, non-const data members should not be tested in `IsLocalMatch()` in order to maintain consistency.

The node interface requires that if we do `X = Y->Duplicate(Y)`, then `X->IsLocalMatch(Y)` must return true - i.e. duplicates should match.

An unfortunate but necessary (I believe) wrinkle in the `Duplicate()` interface is that, to enable the function to be virtual, we must make it a non-static member of the node we wish to duplicate, *but* we cannot implement a shallow-copy version without access to the `shared_ptr<>` that manages the storage. To just create a new `shared_ptr<X>` from a `X *` would register the object `X` with the `shared_ptr<>` implementation code a second time, possibly leading to double deletion. I suspect a parasitic smart pointer scheme might not have this restriction. At present, `Duplicate()` must always be called as `PX->Duplicate(PX)`. _But see #567_ 

### 3.4 Notes

Base classes `Magic` and `Traceable` are just for debugging - magic tests for "this" pointer corruption and/or memory corruption and Traceable provides a cast to std::string.

Two macros, `NODE_FUNCTIONS` and `NODE_FUNCTIONS_FINAL` are defined to use in nodes. `NODE_FUNCTIONS` should be used in intermediate nodes and `NODE_FUNCTIONS_FINAL` should be used in final nodes. The macros include all the `_FUNCTIONS` macros described in the above sections as well as a function 
 - `virtual bool IsFinal() {...}`
 
that reports whether the node is final.

## 4 Standard topological members

These are the objects that are used by nodes to point to child nodes. These objects are loosely based STL containers, and a wrapper named `OOStd` has been created to support Vida Nova's requirements while maintaining the original look and feel. 

Specifically:
 - sub-bass class insertion,
 - base classes for templates and
 - a common container interface 
are added in the `OOStd` layer, which we then specialise for the `Node` class. 

### 4.1 `TreePtr<>` (_singular_)

`TreePtr<>` is a `Node` specialisation of `OOStd::SharedPtr<>` which is the `OOStd` wrapper for `std::shared_ptr<>`. We use it to point to a single child node, i.e. a 1:1 relationship. `TreePtr<>` is also used in general to handle `Node`s - we do not store nodes under any other kind of storage or management scheme. 

`TreePtr<>` has similar semantics to `std::shared_ptr<>`. The `OOStd` adds `TreePtrInterface` which is a base for all `TreePtr<X>` (if `Y` derives from `X` then sadly `TreePtr<Y>` does not derive from `TreePtr<X>`). The node interface adds `Itemiser::Element` as a sub base class.

In trees for programs, TreePtr<X> can never be NULL. This rule does not apply to other trees like search/replace patterns. We call this direct use of `TreePtr<>` a _singluar_ parent-child link.

### 4.2 `Sequence<>`

`Sequence<>` is a `TreePtr<>` specialisation of `OOStd::Sequence<>` and `std::list<>`. We use it to hold zero or more `TreePtr<>` objects which will point to other nodes and retain the order in which elements were first added, i.e. an ordered 1:n relationship.

`Sequence<>` has similar semantics to `list< TreePtr<> >`. `SequenceInterface` is a base class for all `Sequence<X>`. The node interface adds `Itemiser::Element` as a sub-base class. 

### 4.3 `Collection<>`

`Collection<>` is a `TreePtr<>` specialisation of `OOStd::SimpleAssociativeContainer<>` and `std::multiset<>`. We use it to hold zero or more `TreePtr<>` objects which will point to other nodes and discard the order in which elements were first added i.e. an unordered 1:n relationship. The use of `multiset` ensures that multiple references to the same node in a collection will not disappear. Both `Sequence` and `Collection` preserve the number of `TreePtr<>`s inserted.

`Collection<>` has similar semantics to `multiset< TreePtr<> >`. `CollectionInterface` is a base class for all `Collection<X>`. The node interface adds `Itemiser::Element` as a sub base class.

### 4.4 Notes

The `OOStd` provides `Container<>`, which is a common base interface for `Sequence<>` and `Collection<>` as well as `ContainerInterface<>` which is a base for all `Container<X>`. Containers support iteration and the other container operations that are found in both `std::list` and `std::multiset`.

We need a scheme for iterators that supports inheritance of their host containers. This is done using a proxy iterator for the interface that maintains a copy of the implementation's iterator. This introduces extra new and delete as well as an additional layer of indirection. However, it uses copy-on-write semantics to reduce the extra work, and for large/complex iterators, could be a performance improvement. 

`Sequence` and `Collection` both support the `Overwrite()` method. Given an iterator `i`, you can call `i.Overwrite( p )` where `p` is a compatible `TreePtr<>`. In `Sequence`s, this is equivalent to `*i = p`, but in `Collection`s you cannot do this because `*i` is const, so you need `Overwrite()` which does `erase()` and `insert()`. `Overwrite()` is available in the common `ContainerInterface<>` interface.

By enabling `USE_LIST_FOR_COLLECTION`, it is possible to use the `Sequence<>` implementation for `Collection<>` too, so that the pair become functionally identical. This is possible because the ordering for `std::multiset<>`, when used for `Collection<>` is based on the addresses of the contained nodes, and is somewhat arbitrary as well as unique. Thus we don't rely on the order and we don't use `find()`. The remaining operation we need (eg insert, erase) are already supported by `Sequence<>`. Choosing this option makes execution more repeatable because iterating through the container will then give an ordering that depends only on previous inserts and erases on the same `Collection<>` instance.

## 5 Examples

These examples are taken from the tree definitions for C/C++ at the time of writing.

### 5.1 An intermediate and a final node

```
struct Type : virtual Node { NODE_FUNCTIONS };
struct Boolean : Type { NODE_FUNCTIONS_FINAL };
```

Here, `Type` is an intermediate and so cannot appear in program trees (but can in patterns). It represents the set of types - a node is a type if it derives from `Type`. `Boolean` is a final node and can appear in program trees. It represents the C++ data type `bool`. 

With `TreePtr<Node> T` pointing to a `Type` node and `B` pointing to a Boolean node, we have `T.IsSubcategory(B)` is true and `B.IsSubcategory(T)` is false. `IsLocalMatch()` would give the same results.

### 5.2 Topologically-oriented style

Integer types may be signed or unsigned. We might expect to use a node like this

```
struct Integral : Numeric { NODE_FUNCTIONS_FINAL; bool unsigned; ...}
```
    
Where `Numeric` derives from `Type` and `.unsigned` says whether the type is `int` or `unsigned int`. However in Vida Nova, we prefer to use types and interconnections to express information such as this. So we use:

```
struct Integral : Numeric { NODE_FUNCTIONS; ...}
struct Signed : Integral { NODE_FUNCTIONS_FINAL };
struct Unsigned : Integral { NODE_FUNCTIONS_FINAL };
```

There are now separate final nodes for signed and unsigned cases. However, algorithmic code that wishes to manipulate these nodes can still refer to the intermediate `Integral`. Through `Integral`, it can call `IsLocalMatch()`, `Itemise()` and `Duplicate()` as well as directly accessing any members of `Integral`. Therefore, algorithms that do not care about signedness can avoid ever mentioning the `Signed` or `Unsigned` final node classes.

### 5.3 A topological property

```
struct Constancy : Property { NODE_FUNCTIONS };
struct Const : Constancy { NODE_FUNCTIONS_FINAL };
struct NonConst : Constancy { NODE_FUNCTIONS_FINAL };
```

`Property` derives from `Node`. If we need to indicate whether or not a variable is `const`, instead of adding a `bool` member to the node, we will add a `TreePtr<Constancy>`. In program trees, `Constancy` is not allowed because it is intermediate. So we must have one of `Const` or `NonConst`.

### 5.4 A non-topological property (simplified for the example)

```
struct Integer : Number { NODE_FUNCTIONS };
struct SpecificInteger : Integer
{
    NODE_FUNCTIONS_FINAL
    int value;
    bool IsLocalMatch( const Matcher *candidate ) const override
    {
        const SpecificInteger *c = dynamic_cast<const SpecificInteger *>(candidate);
        return c && c->value == value;
    }
};
```

Here, `Integer` is the intermediate node for the property like `Constancy` in the above example. However, we prefer not to define 2^32 final classes for the values of an integer. We want to use a simple `int` variable, so we add it as a member. We consider different values of `value` to be different properties that should compare unequal. So we overload `IsLocalMatch()` to achieve this behaviour. First we must dynamically upcast to `SpecificInteger` (if this fails, return false since the node is of a different type and doesn't match). Second, we compare the values.

By overloading `IsLocalMatch()`, we have restricted the set of nodes that will match `SpecificInteger` from all other `SpecificInteger`s to just `SpecificInteger`s with the same value, which is a subset of the set of all `SpecificIdentifier`s. Hence the word "Specific" in the node name. 
 
As an aside, `Integer` *could* be built topologically as a sequence of `Bool` literals, to be interpreted in binary. Then e.g. constant expression optimisations would need to implement their own binary arithmetic - doing this in S&R would be an interesting exercise.

### 5.5 Representing identifiers

The final example deals with the need to represent identifiers. In Vida Nova we prefer not to do this using matching tags as seen in C source code (identifier tags are strings and must match, very clunky). We use an approach more like GCC, where an identifier node may have multiple parent nodes, and the identity of the node chooses the variable - two pointers to the same identifier node mean the same underlying entity; two pointers to two different identifier nodes mean two separate instances of the entity, even if the identifier nodes are the same as each other. Again simplifying, we have:

```
struct Identifier : virtual Property { NODE_FUNCTIONS };
struct SpecificIdentifier : virtual Property
{ 
    NODE_FUNCTIONS
    bool IsLocalMatch( const Matcher *candidate ) const override
    {
        return candidate == this;
    }
    shared_ptr<Cloner> Duplicate( shared_ptr<Cloner> p ) override
    {
        return p;
    }
};
```

To enforce the identity rule, we now overload `IsLocalMatch()` to test the address of the object, so that only `P->IsLocalMatch(P)` will ever return true. This is the strictest possible restriction we can implement for `IsLocalMatch()`. Under the set-theoretic view, the actual objects could be considered as the points of which the sets are made up - i.e. the universal set is the set of all objects of all classes derived from `Node`.

To meet the node interface rule that duplicates must match, we have to overload `Duplicate()`. The only thing `Duplicate()` can return in order to get a match is the same object. Therefore `Duplicate()` only returns the object passed to it: a shallow copy. As discussed earlier, a shallow copy is actually OK as long as no non-const data members affect the result of `IsLocalMatch()`. As shown above there are no data members, so the requirement is satisfied. In reality, `SpecificIdentifier` carries a string member, which is the nominal name of the identifier. This name is ignored, however, by `IsLocalMatch()`. It is only there to be helpful to the user when the identifier appears in output programs, where it is de-duplicated. So the string's contents do not matter to Vida Nova.

