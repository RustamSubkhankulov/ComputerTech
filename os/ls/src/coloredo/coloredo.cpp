#include <stdio.h>

//---------------------------------------------------------

#include "../../inc/coloredo/coloredo.h"

//=========================================================

void print_color(const char* color, const char* string, const char* style) 
{
	printf("%s", color);
	
	if (style) 
		printf("%s", style);

	printf("%s", string);

	printf(STYLE_RESET);
	printf(COLOR_RESET); 
}