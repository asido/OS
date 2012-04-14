/******************************************************************************
 *      Shell
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef SHELL_IA938IEU
#define SHELL_IA938IEU

/* shell attributes */
#define SHELL_HEADER 1 /* 0001 */
#define SHELL_FOOTER 2 /* 0010 */

/* Limits */
#define SHELL_MAX_ARGC		10

int shell_init(char *prmpt);
void shell_kbrd_cb(char c);

#endif /* end of include guard: SHELL_IA938IEU */
