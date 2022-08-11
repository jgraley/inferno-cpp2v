# Beyond Compare utilities

For users of Beyond Compare (assumes v4 pro)

## Regexes

You can put these in the "Unimportant text" box to tell Beyond Compare 
that some parts of the logs are unimportant.

### A full serial number

Eg `#T12-561`

`#[A-Z]?\d*-\d+`

### A satellite serial number

Eg `#3->`

`#\d+->`
