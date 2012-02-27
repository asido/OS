/******************************************************************************
 *      Shell
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <error.h>
#include <callback.h>
#include <mm.h>

#define PROMPT_SIZE 30

/* Default colors */
#define SHELL_CLR_BG VID_CLR_BLACK
#define SHELL_CLR_FG VID_CLR_LIGHT_GRAY
#define SHELL_HEAD_CLR_BG VID_CLR_DARK_GRAY
#define SHELL_HEAD_CLR_FG VID_CLR_LIGHT_RED
#define SHELL_FOOT_CLR_BG VID_CLR_DARK_GRAY
#define SHELL_FOOT_CLR_FG VID_CLR_LIGHT_RED

static char *prompt;
static char *cmd_buf;

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

static void prompt_draw()
{
    printf("%s", prompt);
}

static void prompt_init()
{
    goto_xy(0, 1);
    prompt_draw();
}

static void header_redraw()
{
    size_t i;

    cursor_save();
    set_color(SHELL_HEAD_CLR_BG, SHELL_HEAD_CLR_FG);

    goto_xy(0, 0);
    for (i = 0; i < MAX_CRS_X; i++)
        putchar(' ');

    goto_xy(1, 0);
    printf("Welcome to AxidOS");

    goto_xy(63, 0);
    printf("day: %d | %d:%d:%d",
            hw_time.day, hw_time.hour, hw_time.min, hw_time.sec);

    cursor_load();
}

static void footer_redraw()
{
    size_t i;

    cursor_save();
    set_color(SHELL_FOOT_CLR_BG, SHELL_FOOT_CLR_FG);

    goto_xy(0, MAX_CRS_Y - 1);
    for (i = 0; i < MAX_CRS_X; i++)
        putchar(' ');

    goto_xy(1, MAX_CRS_Y - 1);
    printf("Developed for UCN 3rd semester specialization project.");

    cursor_load();
}

static void content_redraw()
{
    size_t i, sz;

    cursor_save(); /* not going to load this ever */
    set_color(SHELL_CLR_BG, SHELL_CLR_FG);

    goto_xy(0, 1);
    sz = MAX_CRS_X * (MAX_CRS_Y - 2);
    for (i = 0; i < sz; i++)
        putchar(' ');

    prompt_init();

    /* 
     * Don't cursor_load(), because this is where all output
     * from now on will go.
     */
}

static int exec_cmd(char *cmd)
{
    printf("  No such command: %s", cmd);
    puts("");

    return 0;
}

void shell_kbrd_cb(char c)
{
    static int kbrd_idx = 0;

    putchar(c);

    if (c == '\r')
    {
        cmd_buf[kbrd_idx] = '\0';
        exec_cmd(cmd_buf);
        kbrd_idx = 0;
        prompt_draw();
    }
    else
    {
        cmd_buf[kbrd_idx] = c;
        kbrd_idx++;
    }
}

static void update_time_cb(void *data)
{
    header_redraw();
}

/*
 * Initializes the shell.
 */
int shell_init(char *prmpt)
{
    struct time_t delay;

    delay.sec = 1;
    delay.day = delay.hour = delay.min = delay.mm = 0;
    if (register_callback(CALLBACK_REPEAT, &delay, update_time_cb))
        return -1;

    if (set_prompt(prmpt))
        return -2;

    clear_screen();
    header_redraw();
    footer_redraw();
    content_redraw();

    /* 4KB hopefully is enough */
    cmd_buf = (char *) kalloc(4000);

    return 0;
}
