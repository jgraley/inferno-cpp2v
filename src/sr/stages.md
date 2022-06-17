# Vida Nova Stages

## Build or load patterns and sequence

Ends up with a `VNSequence` of `VNTransformation`s which have 
`CompareReplace` or `SearchReplace` engines.

## Pattern Transformation

Transformations applied to the pattern before planning:
 - Implement `SearchReplace` in terms of `CompareReplace`.
 - Break up `Disjunction`s of more than 2 inputs.

### Planning Stage One

In stage one, `SCREngine`s are created. There is one for master/outer and
one for every slave/inner defined within the master/outer. The
`SCREngine`s scan their pattern tree and categorise the agents:
 - Which pattern nodes really belong to me and not some outer/master?
 - Which are reached during search, replace or both (the _phases_)?
 - What order are they reached during replace?
 - Which slave/inner `SCREngine`s will be reached?
 
### Planning Stage Two

`SCREngine` configuration of the `Agent`s. Informs the agent:
 - which is its owning `SCREngine` and
 - its phase.
 
### Planning Stage Three

Interleaved compare and replace planning across all the `SCREngine`s. 

Compare planning creates `AndRuleEngine`s which analyse thir search 
pattern:
 - And-rule regions: child `AndRuleEngine`s are generated for detected
   sub-regions.
 - Determine keyers and residuals.
 - Determine my, master/outer and master/outer boundary `Agent`s.
 - `AndRuleEngine` configuration of the `Agent`s: informs the agent:
   - keyer and residual links for coupled agents.
 - Extracts symbolics (satisfiability expressions) from agents.
 - Engine-wide manipulation of sat expression clauses.
 
Replace planning:
 - Prepare agents for overlay.
 - Configure agents that are keyers during replace phase (eg builders).
 
### Planning Stage Four

Knowledge/database planning. Central to the `VNSequence` (i.e. run once 
for all steps):
 - Deduce lacing intervals based on analysis of all pre-restrictions/
   type-restrictions, for category ordering/index.
 
### Planning stage Five

`AndRuleEngine`s build CSP problem:
 - Create constraints based on symbolic sat expressions:
   - Categorise variables.
   - Try to solve sat expression in various scenarios.
 - Create constraint solver:
   - Categorise variables.

## Run It

Off it goes!
