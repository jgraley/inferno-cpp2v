# Beyond Compare utilities

For users of Beyond Compare (assumes v4 pro)

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
