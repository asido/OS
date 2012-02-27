/******************************************************************************
 *      Shell
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <error.h>
#include <callback.h>
#include <mm.h>

extern int info_main(int argc, const char *argv[]);

#define PROMPT_SIZE 30

/* Default colors */
#define SHELL_CLR_BG VID_CLR_BLACK
#define SHELL_CLR_FG VID_CLR_LIGHT_GRAY
#define SHELL_HEAD_CLR_BG VID_CLR_DARK_GRAY
#define SHELL_HEAD_CLR_FG VID_CLR_LIGHT_RED
#define SHELL_FOOT_CLR_BG VID_CLR_DARK_GRAY
#define SHELL_FOOT_CLR_FG VID_CLR_LIGHT_RED
#define SHELL_PROMPT_BG SHELL_CLR_BG
#define SHELL_PROMPT_FG VID_CLR_WHITE

static char *prompt;
static char *cmd_buf;

/*
 * Sets shell prompt string.
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

/*
 * Draws the prompt.
 */
static void prompt_draw()
{
    /* color_save(); */
    /* cursor_save(); */
    set_color(SHELL_PROMPT_BG, SHELL_PROMPT_FG);
    printf("%s", prompt);
    set_color(SHELL_CLR_BG, SHELL_CLR_FG);
    /* color_load(); */
}

/*
 * Redraws the header.
 */
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

/*
 * Redraws the footer.
 */
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

/*
 * Redraws the whole screen leaving shell header and footer untouched.
 */
static void content_redraw()
{
    size_t i, sz;

    cursor_save(); /* not going to load this ever */
    set_color(SHELL_CLR_BG, SHELL_CLR_FG);

    goto_xy(0, 1);
    sz = MAX_CRS_X * (MAX_CRS_Y - 2);
    for (i = 0; i < sz; i++)
        putchar(' ');
    goto_xy(0, 1);

    /* 
     * Don't cursor_load(), because this is where all output
     * from now on will go.
     */
}

static void print_help()
{
    puts("AxidOS commands:");
    puts("\thelp - prints this help");
    puts("\tclear - clears the screen");
    puts("\tinfo - prints some info about the system");
}

/*
 * Executes a command.
 */
static int exec_cmd(char *cmd)
{
    if (strcmp(cmd, "help") == 0)
        print_help();
    else if (strcmp(cmd, "clear") == 0)
        content_redraw();
    else if (strcmp(cmd, "info") == 0)
        info_main(1, &cmd);
    else if (strcmp(cmd, "") != 0)
    {
        printf("  No such command: %s", cmd);
        puts("");
    }

    return 0;
}

/*
 * Callback function by keyboard driver whenever a new char gets available.
 */
void shell_kbrd_cb(char c)
{
    static unsigned int kbrd_idx = 0;

    if (c == '\r')
    {
        putchar(c);
        cmd_buf[kbrd_idx] = '\0';
        exec_cmd(cmd_buf);
        kbrd_idx = 0;
        prompt_draw();
    }
    else if (c == '\b')
    {
        if (kbrd_idx > 0)
        {
            putchar(c);
            kbrd_idx--;
            cmd_buf[kbrd_idx] = '\0';
        }
    }
    else
    {
        putchar(c);
        cmd_buf[kbrd_idx] = c;
        kbrd_idx++;
    }
}

/*
 * Callback function every second.
 */
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

    goto_xy(0, 1);
    puts("Welcome to AxidOS! Type 'help' for help.");

    /* 4KB hopefully is enough */
    cmd_buf = (char *) kalloc(4000);

    /* simulate Enter press to show the prompt */
    shell_kbrd_cb('\r');

    return 0;
}
