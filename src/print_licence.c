/**********************************************************************
 * Author: Anders "Ichimusai" Sikvall
 * anders@sikvall.se
 *
 * This header file is part of the ipta package and is maintained by the
 * package owner, see http://ichimusai.org/ipta/ for more info about this.
 * Any changes, patches, diffs etc that you would like to offer should be
 * sent by email to ichi@ichimusai.org for review before they will be applied
 * to the main code base.
 *
 * As usual this software is offered "as is" and placed in the public domain.
 * You are free to copy, modify, spread and make use of this software. For
 * the terms and conditions for this software you should refer to the
 * "LICENCE" file in the source directory or run a compiled binary with the
 * "--licence" option which will display the licence.
 *
 * Any modifications to this source MUST retain this header. You are however
 * allowed to add below your own changes and redistribute, as long as you do
 * not violate any terms and condition in the LICENCE.
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>

void print_license(void) 
{
	printf("LICENCE OF THE SOFTWARE\n\n"				\
	       "Copyright (c) 2014, Anders 'Ichimusai' Sikvall\n"	\
	       "Latest revision 2014-10-05\n\n"				\
	       "Permission is hereby granted, free of charge, to any person obtaining\n" \
	       "a copy of this software and associated documentation files (the\n" \
	       "'Software'), to deal in the Software without restriction, including\n" \
	       "without limitation the rights to use, copy, modify, merge, publish,\n" \
	       "distribute, sublicense, and/or sell copies of the Software, and to\n" \
	       "permit persons to whom the Software is furnished to do so, subject to\n" \
	       "the following conditions:\n\n"				\
	       "1. The above copyright notice and this permission notice shall be\n" \
	       "   included in all copies or substantial portions of the Software. It\n" \
	       "   is not allowed to modify this licence in any way. \n\n" \
	       "2a The original header must accompany all software files and are not\n" \
	       "   removed in redistribution. It is allowed to add to the headers to\n" \
	       "   describe modifications of the software and who has made these\n" \
	       "   modifications. I claim no right to your modifications, they stand\n" \
	       "   on their own merits.\n\n"
	       "2b The license shown when the software is invoked with the '--licence'\n" \
	       "   option is not removed or modified but must remain the same. You may\n" \
	       "   add your name to the bottom of the credit part if you have\n" \
	       "   contributed or modified the software.\n\n"		\
	       "3. You are allowed to add to the header files and the licence\n" \
	       "   information described in (2b) with changes and your own name, but\n" \
	       "   only as an addition at the bottom of the file.\n\n"	\
	       "4. Distributing the software must be done in a way so that the\n" \
	       "   software archive is intact and no necessary files (except external\n" \
	       "   libraries such as MySQL) is always included. The software should be\n" \
	       "   compilable after a reasonable set-up of the tool chain and\n" \
	       "   compiler.\n\n"					\
	       "5. If you are making modifications to this software i humbly suggest\n" \
	       "   you send a copy of your modifications to me on ichi@ichimusai.org\n" \
	       "   and I may include your modifications (if you allow it) as well as\n" \
	       "   give credit to you (if you want) in the next release of the\n" \
	       "   software. This is only a suggestion to keep the code base in a\n" \
	       "   single location but it is in no way a restriction to your right to\n" \
	       "   modify and redistribute this software.\n\n"		\
	       "THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,\n" \
	       "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n" \
	       "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n" \
	       "IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n" \
	       "CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n" \
	       "TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n" \
	       "SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n" \
	       "Credits for this software\n\n"				\
	       " All coding, documentation, testing of the original package and\n" \
	       " inventing the software in the first place: Anders 'Ichimusai' Sikvall\n" \
	       " <ichi@ichimusai.org> http://ichimusai.org/\n");
}

void print_usage(void) {
	printf("ipta - iptables traffic analyzer\n\n"		\
	       "USAGE ipta [OPTIONS] [ACTIONS] [ARGS]\n\n"	\
	       "OPTIONS\n"					\
	       "--version, -v\n"					\
	       "\t Show version information and build info.\n\n"	\
	       "--rdns, -r\n"						\
	       "\t Reverse DNS lookup. Tries to find the name behind every IP number\n"
	       "\t that you encounter.");
}
