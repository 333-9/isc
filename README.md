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
range operators are operators that work on a range of numbers
in a single column `[> c0:10]  // finds largest value in column c`,
they are:
	- `[> cr:r]` larges value
	- `[< cr:r]` smalles value
	- `[* cr:r]` moltiply all nonzero values
	- `[+ cr:r]` add all values
	- `[- cr:r]` substract all values
	- `[& cr:r]` and
	- `[| cr:r]` or
	- `[^ cr:r]` xor
range assignment operators are similar to assignmest oprerators, but return 0:
	- `[cr:r = a]`
	- `[cr:r += a]`
	- `[cr:r *= a]`
	...
	- `[cr:r ~]` assigns a random value
