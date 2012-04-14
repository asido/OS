/******************************************************************************
 *      Shell
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <error.h>
#include <callback.h>
#include <mm.h>
#include "shell.h"

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

struct shell_t {
    char *prompt;
    char *cmd_buf;
    struct frame_t frame;
};

static struct shell_t shell;

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

    shell.prompt = prmpt;

    return 0;
}

/*
 * Draws the prompt.
 */
static void prompt_draw()
{
    color_save();
    set_color(SHELL_PROMPT_BG, SHELL_PROMPT_FG);
    printf("%s", shell.prompt);
    color_load();
}

static void clock_redraw()
{
    cursor_save();
    set_color(SHELL_HEAD_CLR_BG, SHELL_HEAD_CLR_FG);
    goto_xy(71, 0);
    if (hw_time.hour < 10)
        putchar('0');
    printf("%d:", hw_time.hour);
    if (hw_time.min < 10)
        putchar('0');
    printf("%d:", hw_time.min);
    if (hw_time.sec < 10)
        putchar('0');
    printf("%d", hw_time.sec);
    cursor_load();
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

    clock_redraw();

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
    printf("Developed for UCN 4th semester specialization project.");

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
	puts("\tls [folder] - print folder content");
}

static char *get_cmd_token(char *cmd, size_t offset)
{
	char *start;
	int in_bracket; // 1 = ' | 2 = "

	cmd += offset * sizeof(char);

	if (!cmd || *cmd == '\0')
		return NULL;

	while (*cmd == ' ')
		cmd++;

	if (*cmd == '\'')
	{
		in_bracket = 1;
		cmd++;
	}
	else if (*cmd == '"')
	{
		in_bracket = 2;
		cmd++;
	}
	else
		in_bracket = 0;

	start = cmd;
	
	while ((in_bracket || *cmd != ' ') && *cmd != '\0')
	{
		if ((*cmd == '\'' && in_bracket == 1) ||
			(*cmd == '"' && in_bracket == 2))
		{
			*cmd = '\0';
			break;
		}

		cmd++;
	}

	if (*cmd != '\0')
		*cmd = '\0';

	return start;
}

/*
 * Executes a command.
 */
static int exec_cmd(char *cmd)
{
	int argc = 0;
	int offset = 0;
	char **argv = (char **) kalloc(sizeof(char **) * SHELL_MAX_ARGC);

	while (argv[argc] = get_cmd_token(cmd, offset))
	{
		offset += strlen(argv[argc]) + 1;
		argc++;
		if (argc > SHELL_MAX_ARGC)
		{
			printf("Too many arguments in command: %s\n", cmd);
			return -1;
		}
	}

	if (argc == 0)
		return 0;

    if (strcmp(argv[0], "help") == 0)
        print_help();
    else if (strcmp(argv[0], "clear") == 0)
        content_redraw();
    else if (strcmp(argv[0], "info") == 0)
        info_main(argc, argv);
	else if (strcmp(argv[0], "ls") == 0)
		ls_main(argc, argv);
    else if (strcmp(argv[0], "") != 0)
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
        shell.cmd_buf[kbrd_idx] = '\0';
        exec_cmd(shell.cmd_buf);
        kbrd_idx = 0;
        prompt_draw();
    }
    else if (c == '\b')
    {
        if (kbrd_idx > 0)
        {
            putchar(c);
            kbrd_idx--;
            shell.cmd_buf[kbrd_idx] = '\0';
        }
    }
    else
    {
        putchar(c);
        shell.cmd_buf[kbrd_idx] = c;
        kbrd_idx++;
    }
}

/*
 * Callback function every second.
 */
static void update_time_cb(void *data)
{
    clock_redraw();
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
    shell.cmd_buf = (char *) kalloc(4000);
    shell.frame.top = 1;    /* leave space for header/footer */
    shell.frame.bottom = MAX_CRS_Y - 1;
    activate_frame(&shell.frame);

    /* simulate Enter press to show the prompt */
    shell_kbrd_cb('\r');

    return 0;
}
