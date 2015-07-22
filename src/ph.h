#ifndef __PH_H
#define __PH_H 1

/***********************************
 ** Header file for print headers **
 ***********************************/

struct ph_header {
	char *name;
	char *sep;
	int align;
};

#define PH_ALIGN_L 0
#define PH_ALIGN_R 1
#define PH_ALIGN_C 2

#endif
