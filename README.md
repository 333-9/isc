isc - intiger spreadsheet calculator
====================================
simple intiger based spreadsheet calculator


Insalation
----------
edit config.h to change color and number of columns (changes the way 
spreadsheets are allocated and read from files (note: number of rows
changes automatically on terminal resize).

to install run (as root if necessary):

	make instal


Running isc
-----------
isc takes just one command like argument, it is the file name
(if no filename is given isc writes into out.isc). example:

	isc mysc.csv


Using isc
---------
isc uses vim bindings to move the cursor.
`hjkl`  movement (20j moves 20 rows down)
`w`  write to file
`q`  quit
`>`  isert text
`=`  store result of expression in current cell
`+`  add the result of an expression to the current cell
`Enter`  parses the text in the current box as an expression, or
         sets curent box to the modifier number

Commands
--------
C-like syntax: `1 + 4,   (4 + 3) * 22`.
Cells can be accessed as row, column: `20f`.
Result of operations can be "piped" into another cell
with `>`: `4a + 20 > 2b`.
You can also specify execution of cells command:
``` c
a0 >! a0   pipe value to a2 and execute it's command
a1 >& a1   execute if is nonzero
a2 >| a2   execute if is zero

b0 ;  b0   don't pipe value, only execute command
b1 ;& b1   only execute if nonzero
b2 ;| b2   only execute if zero

a3 >? a3 : c3   pipe and execute command a3 if nonzero, otherwise execute c3
b3 ;? b3 : c3   only execute command b3 if nonzero, otherwise execute c3
```
NOTE: `>& ; >|` always pipe the value, only execution is conditional


File format
-----------
.csv format, where numbers and strings can occupy the same cell
```
1563,55,1"hello", # comment
8645,30,0"world","!",
```
