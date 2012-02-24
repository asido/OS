/******************************************************************************
 *      Shell
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>

#define PROMPT_SIZE 10

static char *prompt;

/*
 * Sets shell prompt.
 */
static int set_prompt(char *prmpt)
{
    if (strlen(prmpt) > PROMPT_SIZE)
        return -1;

    prompt = prmpt;

    return 0;
}

/*
 * Initializes the shell.
 */
int shell_init(char *prmpt)
{
    if (set_prompt(prmpt))
        return -1;

    return 0;
}

void shell_loop()
{

}
