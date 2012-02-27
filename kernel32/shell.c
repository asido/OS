/******************************************************************************
 *      Shell
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <error.h>
#include <callback.h>

#define PROMPT_SIZE 10

static char *prompt;

/*
 * Sets shell prompt.
 */
static int set_prompt(char *prmpt)
{
    if (strlen(prmpt) > PROMPT_SIZE)
    {
        error = ESIZE;
        return -error;
    }

    prompt = prmpt;

    return 0;
}

static void update_time_cb(void *data)
{
    cursor_save();
    goto_xy(63, 0);
    printf("day: %d | %d:%d:%d", hw_time.day, hw_time.hour, hw_time.min, hw_time.sec);
    cursor_load();
}

/*
 * Initializes the shell.
 */
int shell_init(char *prmpt)
{
    if (set_prompt(prmpt))
        return -1;

    struct time_t delay;
    delay.sec = 1;
    delay.day = delay.hour = delay.min = delay.mm = 0;
    if (register_callback(CALLBACK_REPEAT, &delay, update_time_cb))
        return -2;

    return 0;
}

void shell_loop()
{

}
