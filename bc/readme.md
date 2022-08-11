# Beyond Compare stuff

For users of Beyond Compare (assumes v4 pro)

## Scripts

### `show_divergent_sr_step.sh`

For when an `srtest` fails after a certain step. Shows a 3-way diff of the cpp outputs in BC. See script for details.

### `show_feature_logs.sh`

To see the effect of a feature flag on the contents of the logs. See script for details.

## Regexes

You can put these in the "Unimportant Text" box to tell Beyond Compare 
that some parts of the logs are unimportant.

### How to set

 - Menu: Session
 - Session Settings...
 - Importance tab
 - "+" button (not double click)
 - Check Regular Expression
 - and add a regex

### A full serial number

Eg `#T12-561`

Regex: `#[A-Z]?\d*-\d+`

### A satellite serial number

Eg `#3->`

Regex: `#\d+->`
