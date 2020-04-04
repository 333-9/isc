# drawf
drawf prints a formated string to the terminal at apwcified coordinates.
the format syntax is similar to the one of printf, but simpler
and presents less options.


## field width:
an optional field width can be specified as a decimal number
denoting the maximum number of characters the field will span.
width can also be `*` (star), then the field width will be taken from
the next argument. ("%5s" may be ("%*s", 5, s)).
e,c ignore field width; d,x,o,i fill the field with `*` (star)
when there is not enough space for the intiger, p,s sprint
field width amount of characters -1 and append a single star (*).

## conversion specifier:
	- d,x,o  decimal, hexadecimal, or octal number respectively.
	- s  string of characters
	- c  a single character
	- e  a color escape of the form: struct {char type, color;}


### example:
``` c
// isc style cursor
drawf(20, 4, "%e[%e%i%e]", C_cursor, C_number, arr[i], C_cursor);

```


# drawf2
format suports loops on pointers
example: ```  "`4[` %d `]`\n"  ```
will draw numbers 1 to 4 seperated by a space
```  "{`*[` %d,`]` }"  ```
