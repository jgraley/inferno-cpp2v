# Vida Nova Stages

Note: _wind-in_ and _unwind_ are recursion styles, i.e. before recursion
or after respectively._Steppy_ indicates that the action of the stage is 
performed for each transformation step, more or less independently. Otherwise, 
it's more like a "one hit" stage for the whole sequence of steps.

## Build or load patterns and sequence `B`

Ends up with a `VNSequence` of `VNStep`s which have 
`CompareReplace` or `SearchReplace` engines.

## Pattern Transformation `X` (steppy)

Transformations applied to the pattern before planning:
 - Implement `SearchReplace` in terms of `CompareReplace`.
 - Break up `Disjunction`s of more than 2 inputs.

### Planning Stage One `P` (steppy)

In stage one, `SCREngine`s are created . There is one for root and
one for every embedded SCR agent defined within, etc _(wind-in)_. The
`SCREngine`s scan their pattern tree and categorise the agents:
 - Which pattern nodes really belong to me and not enclosing engine?
 - Which are reached during search, replace or both (the _phases_)?
 - What order are they reached during replace?
 - Which embedded `SCREngine`s will be reached?
 
### Planning Stage Two `Q` (steppy)

`SCREngine` configuration of the `Agent`s _(unwind)_. Informs the agent:
 - which is its owning `SCREngine` and
 - its phase.
 
### Planning Stage Three `R` (steppy)

Interleaved compare and replace planning across all the `SCREngine`s _(wind-in)_. 

Compare planning creates `AndRuleEngine`s which analyse thir search 
pattern:
 - And-rule regions: child `AndRuleEngine`s are generated for detected
   sub-regions.
 - Determine keyers and residuals.
 - Determine my, surrounding and boundary `Agent`s.
 - `AndRuleEngine` configuration of the `Agent`s: informs the agent:
   - keyer and residual links for coupled agents.
 - Extracts symbolics (satisfiability expressions) from agents.
 - Engine-wide manipulation of sat expression clauses.
 
Replace planning:
 - Prepare agents for overlay.
 - Configure agents that are keyers during replace phase (eg builders).
 
### Planning Stage Four `S`

X tree database planning. Central to the `VNSequence` (i.e. run once 
for all steps):
 - Deduce lacing intervals based on analysis of all pre-restrictions/
   type-restrictions, for category ordering/index.
 
### Planning stage Five `U` (steppy)

`AndRuleEngine`s build CSP problem _(wind-in)_:
 - Create constraints based on symbolic sat expressions:
   - Categorise variables.
   - Try to solve sat expression in various scenarios.
 - Create constraint solver:
   - Categorise variables.

### Analysis stage `A` (input tree required here)

Initial X tree database is built based on an analysis of the input tree.

## Transformations stage `T` (steppy)

Off it goes!
