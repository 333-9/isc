const unsigned short  columns  = 12;
const unsigned short  box_size = 20;


/* colors */
enum Color {
	Black   = 0,
	Red     = 1,
	Green   = 2,
	Yellow  = 3,
	Blue    = 4,
	Magenta = 5,
	Aqua    = 6,
	White   = 7,
	Grey    = 8,
};

const char C_cursor     = 46;//226;
const char C_command    = 46;//226;
const char C_ind        = 240;//240;
const char C_number     = 70;//245;
const char C_special    = 6;
const char C_text       = 250;//82;
const char C_error      = Red;//Red;
