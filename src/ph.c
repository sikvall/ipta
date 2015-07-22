#include <stdlib.h>
#include <stdio.h>

#define PH_JUST_L 0x01 // Adjust left field
#define PH_JUST_R 0x02 // Adjust right field
#define PH_JUST_C 0x03 // Adjust center field

struct ph_format {
	char *name;
	char *separator;
	int len;
	int justify;
};


struct ph_format ph_list[] = {
	{"Testfield one", " ", 5, PH_JUST_L},
	{NULL}
};


int ph_print(struct ph_format *ph_list) 
{
	int i;

	return 0;
}

