isc - intiger spreadsheet calculator
====================================
spreadsheet calculator made to be ass small as possible


Requirements
------------
yacc + lex (bison + flex)


Insalation
----------
edit config.h to change color and number of columns (changes the way 
spreadsheets are allocated and read from files (note: number of rows
changes automaticaly on terminal resize).

to install run (as root if necessary):

	make instal


Running isc
-----------
isc takes just one command like argument, it is the file name
(if no filename is given isc writes into out.isc). example:

	isc mysc.isc


Using isc
----------
isc uses vim bindings (hjkl) to move the cursor.
`w`  to write to a file
`q`  to quit
`>`  isert text (text is not saved)
`=`  set number to an expression
`+`  add expression to the number
`Enter`  use text as an expression to set the number

commands:
most C like expressions are suported
`1 + 4,   a += 4,   f /= (c4 + 3) * 22`.
C++ style comments `//.*$` are suported.
there are 25 variables `a-z`.
cells can be addsseres as `[a-z][0-9]+`.
range operators are operators that work on a range of numbers in a single column.
they are:
	- `>` larges value
	- `<` smalles value
	- `*` moltiply all nonzero values
	- `+` add all values
	- `-` substract all values
	- `&` and
	- `|` or
	- `^` xor
range assignment operators are similar to assignmest oprerators, but return 0
`=, +=, *=, ...`, `~` assigns a random value to a range


File format
-----------
isc format is made to be human readable.
lines represent rows and columns are separated by `,`. each item can contain
a number or `,` (or newline) terminated string starting with `>`, optionaly preceedet
by a number. lines starting with `#` are ignored.
example:
```
# contains random numbers and hello world
1563,55,>hello
8645,3,0>world,>!,
```
