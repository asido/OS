/******************************************************************************
 *      Keyboard driver
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

/*
 * NOTE: To remain compatible with old software, the motherboard emulates USB
 * keyboards and mice as PS/2 devices.
 */

#include <libc.h>
#include <shell.h>
#include <x86/cpu.h>
#include <x86/i8259.h>

static int kbrd_enable();
static int kbrd_disable();

/* Table reference: http://www.brokenthorn.com/Resources/OSDevScanCodes.html */

/*
 * Alphanumeric Keys
 */

#define KEY_SPACE              ' '
#define KEY_0                  '0'
#define KEY_1                  '1'
#define KEY_2                  '2'
#define KEY_3                  '3'
#define KEY_4                  '4'
#define KEY_5                  '5'
#define KEY_6                  '6'
#define KEY_7                  '7'
#define KEY_8                  '8'
#define KEY_9                  '9'

#define KEY_A                  'a'
#define KEY_B                  'b'
#define KEY_C                  'c'
#define KEY_D                  'd'
#define KEY_E                  'e'
#define KEY_F                  'f'
#define KEY_G                  'g'
#define KEY_H                  'h'
#define KEY_I                  'i'
#define KEY_J                  'j'
#define KEY_K                  'k'
#define KEY_L                  'l'
#define KEY_M                  'm'
#define KEY_N                  'n'
#define KEY_O                  'o'
#define KEY_P                  'p'
#define KEY_Q                  'q'
#define KEY_R                  'r'
#define KEY_S                  's'
#define KEY_T                  't'
#define KEY_U                  'u'
#define KEY_V                  'v'
#define KEY_W                  'w'
#define KEY_X                  'x'
#define KEY_Y                  'y'
#define KEY_Z                  'z'

#define KEY_RETURN             '\r'
#define KEY_ESCAPE             0x1001
#define KEY_BACKSPACE          '\b'

/*
 * Arrow Keys
 */

#define KEY_UP                 0x1100
#define KEY_DOWN               0x1101
#define KEY_LEFT               0x1102
#define KEY_RIGHT              0x1103

/*
 * Function Keys
 */

#define KEY_F1                 0x1201
#define KEY_F2                 0x1202
#define KEY_F3                 0x1203
#define KEY_F4                 0x1204
#define KEY_F5                 0x1205
#define KEY_F6                 0x1206
#define KEY_F7                 0x1207
#define KEY_F8                 0x1208
#define KEY_F9                 0x1209
#define KEY_F10                0x120a
#define KEY_F11                0x120b
#define KEY_F12                0x120b
#define KEY_F13                0x120c
#define KEY_F14                0x120d
#define KEY_F15                0x120e

#define KEY_DOT                '.'
#define KEY_COMMA              ','
#define KEY_COLON              ':'
#define KEY_SEMICOLON          ';'
#define KEY_SLASH              '/'
#define KEY_BACKSLASH          '\\'
#define KEY_PLUS               '+'
#define KEY_MINUS              '-'
#define KEY_ASTERISK           '*'
#define KEY_EXCLAMATION        '!'
#define KEY_QUESTION           '?'
#define KEY_QUOTEDOUBLE        '\"'
#define KEY_QUOTE              '\''
#define KEY_EQUAL              '='
#define KEY_HASH               '#'
#define KEY_PERCENT            '%'
#define KEY_AMPERSAND          '&'
#define KEY_UNDERSCORE         '_'
#define KEY_LEFTPARENTHESIS    '('
#define KEY_RIGHTPARENTHESIS   ')'
#define KEY_LEFTBRACKET        '['
#define KEY_RIGHTBRACKET       ']'
#define KEY_LEFTCURL           '{'
#define KEY_RIGHTCURL          '}'
#define KEY_DOLLAR             '$'
#define KEY_POUND              '£'
#define KEY_EURO               '$'
#define KEY_LESS               '<'
#define KEY_GREATER            '>'
#define KEY_BAR                '|'
#define KEY_GRAVE              '`'
#define KEY_TILDE              '~'
#define KEY_AT                 '@'
#define KEY_CARRET             '^'

/*
 * Numeric Keypad
 */

#define KEY_KP_0               '0'
#define KEY_KP_1               '1'
#define KEY_KP_2               '2'
#define KEY_KP_3               '3'
#define KEY_KP_4               '4'
#define KEY_KP_5               '5'
#define KEY_KP_6               '6'
#define KEY_KP_7               '7'
#define KEY_KP_8               '8'
#define KEY_KP_9               '9'
#define KEY_KP_PLUS            '+'
#define KEY_KP_MINUS           '-'
#define KEY_KP_DECIMAL         '.'
#define KEY_KP_DIVIDE          '/'
#define KEY_KP_ASTERISK        '*'
#define KEY_KP_NUMLOCK         0x300f
#define KEY_KP_ENTER           0x3010

#define KEY_TAB                0x4000
#define KEY_CAPSLOCK           0x4001

/*
 * Modify Keys
 */

#define KEY_LSHIFT             0x4002
#define KEY_LCTRL              0x4003
#define KEY_LALT               0x4004
#define KEY_LWIN               0x4005
#define KEY_RSHIFT             0x4006
#define KEY_RCTRL              0x4007
#define KEY_RALT               0x4008
#define KEY_RWIN               0x4009

#define KEY_INSERT             0x400a
#define KEY_DELETE             0x400b
#define KEY_HOME               0x400c
#define KEY_END                0x400d
#define KEY_PAGEUP             0x400e
#define KEY_PAGEDOWN           0x400f
#define KEY_SCROLLLOCK         0x4010
#define KEY_PAUSE              0x4011
#define KEY_PRINT              0x4012


/*
 * Scan codes of the keyboard.
 * In my tests all emulators returned me Set 1 codes, so let's use it here.
 * (Might be because my host is set like that)
 */
static const short SCAN_CODES_SINGLE[] = {
    [0x1]   = KEY_ESCAPE,
    [0x2]   = KEY_1,
    [0x3]   = KEY_2,
    [0x4]   = KEY_3,
    [0x5]   = KEY_4,
    [0x6]   = KEY_5,
    [0x7]   = KEY_6,
    [0x8]   = KEY_7,
    [0x9]   = KEY_8,
    [0xA]   = KEY_9,
    [0xB]   = KEY_0,
    [0xC]   = KEY_MINUS,
    [0xD]   = KEY_EQUAL,
    [0xE]   = KEY_BACKSPACE,
    [0xF]   = KEY_TAB,
    [0x10]  = KEY_Q,
    [0x11]  = KEY_W,
    [0x12]  = KEY_E,
    [0x13]  = KEY_R,
    [0x14]  = KEY_T,
    [0x15]  = KEY_Y,
    [0x16]  = KEY_U,
    [0x17]  = KEY_I,
    [0x18]  = KEY_O,
    [0x19]  = KEY_P,
    [0x1A]  = KEY_LEFTBRACKET,
    [0x1B]  = KEY_RIGHTBRACKET,
    [0x1C]  = KEY_RETURN,
    [0x1D]  = KEY_LCTRL,
    [0x1E]  = KEY_A,
    [0x1F]  = KEY_S,
    [0x20]  = KEY_D,
    [0x21]  = KEY_F,
    [0x22]  = KEY_G,
    [0x23]  = KEY_H,
    [0x24]  = KEY_J,
    [0x25]  = KEY_K,
    [0x26]  = KEY_L,
    [0x27]  = KEY_SEMICOLON,
    [0x28]  = KEY_QUOTE,
    [0x29]  = KEY_GRAVE,
    [0x2A]  = KEY_LSHIFT,
    [0x2B]  = KEY_BACKSLASH,
    [0x2C]  = KEY_Z,
    [0x2D]  = KEY_X,
    [0x2E]  = KEY_C,
    [0x2F]  = KEY_V,
    [0x30]  = KEY_B,
    [0x31]  = KEY_N,
    [0x32]  = KEY_M,
    [0x33]  = KEY_COMMA,
    [0x34]  = KEY_DOT,
    [0x35]  = KEY_SLASH,
    [0x36]  = KEY_RSHIFT,
    [0x37]  = KEY_KP_ASTERISK,
    [0x38]  = KEY_LALT,
    [0x39]  = KEY_SPACE,
    [0x3A]  = KEY_CAPSLOCK,
    [0x3B]  = KEY_F1,
    [0x3C]  = KEY_F2,
    [0x3D]  = KEY_F3,
    [0x3E]  = KEY_F4,
    [0x3F]  = KEY_F5,
    [0x40]  = KEY_F6,
    [0x41]  = KEY_F7,
    [0x42]  = KEY_F8,
    [0x43]  = KEY_F9,
    [0x44]  = KEY_F10,
    [0x45]  = KEY_KP_NUMLOCK,
    [0x46]  = KEY_SCROLLLOCK,
    [0x47]  = KEY_KP_7,
    [0x48]  = KEY_KP_8,
    [0x49]  = KEY_KP_9,
    [0x4A]  = KEY_KP_MINUS,
    [0x4B]  = KEY_KP_4,
    [0x4C]  = KEY_KP_5,
    [0x4D]  = KEY_KP_6,
    [0x4E]  = KEY_KP_PLUS,
    [0x4F]  = KEY_KP_1,
    [0x50]  = KEY_KP_2,
    [0x51]  = KEY_KP_3,
    [0x52]  = KEY_KP_0,
    [0x53]  = KEY_KP_DECIMAL,
    [0x56]  = KEY_LESS
};

static const short SCAN_CODES_MULTI[] = {
    [0x1C]  = KEY_KP_ENTER,
    [0x1D]  = KEY_RCTRL,
    [0x35]  = KEY_KP_DIVIDE,
    [0x37]  = KEY_PRINT,
    [0x38]  = KEY_RALT,
    [0x47]  = KEY_HOME,
    [0x48]  = KEY_UP,
    [0x49]  = KEY_PAGEUP,
    [0x4B]  = KEY_LEFT,
    [0x4D]  = KEY_RIGHT,
    [0x4F]  = KEY_END,
    [0x50]  = KEY_DOWN,
    [0x51]  = KEY_PAGEDOWN,
    [0x52]  = KEY_INSERT,
    [0x53]  = KEY_DELETE
};

static const short SCAN_CODES_BREAK[] = {
    [0xAA]  = KEY_LSHIFT,
    [0xB6]  = KEY_RSHIFT,
    [0xBA]  = KEY_CAPSLOCK,
    [0xD9]  = KEY_LCTRL
};

static const short SCAN_CODES_SYMBOLS[] = {
    [0x2]   = KEY_EXCLAMATION,
    [0x3]   = KEY_AT,
    [0x4]   = KEY_HASH,
    [0x5]   = KEY_DOLLAR,
    [0x6]   = KEY_PERCENT,
    [0x7]   = KEY_CARRET,
    [0x8]   = KEY_AMPERSAND,
    [0x9]   = KEY_ASTERISK,
    [0xA]   = KEY_LEFTPARENTHESIS,
    [0xB]   = KEY_RIGHTPARENTHESIS,
    [0xC]   = KEY_UNDERSCORE,
    [0xD]   = KEY_PLUS,
    [0x1A]  = KEY_LEFTCURL,
    [0x1B]  = KEY_RIGHTCURL,
    [0x27]  = KEY_COLON,
    [0x28]  = KEY_QUOTEDOUBLE,
    [0x29]  = KEY_TILDE,
    [0x2B]  = KEY_BAR,
    [0x33]  = KEY_LESS,
    [0x34]  = KEY_GREATER,
    [0x35]  = KEY_QUESTION,
    [0x56]  = KEY_GREATER
};


/* Ports */
#define KBRD_PORT_ENCODER 0x60 /* controller inside the keyboard itself */
#define KBRD_PORT_CTRL 0x64  /* i8042 controller on the moherboard */

/*
 * KBRD_PORT_ENCODER `in` cmd list
 * ----------------------------------------------------------------------------
 * 0xED Set LEDs
 * 0xEE Echo command. Returns 0xEE to port 0x60 as a diagnostic test
 * 0xF0 Set alternate scan code set
 * 0xF2 Send 2 byte keyboard ID code as the next two bytes to be read from port 0x60
 * 0xF3 Set autrepeat delay and repeat rate
 * 0xF4 Enable keyboard
 * 0xF5 Reset to power on condition and wait for enable command
 * 0xF6 Reset to power on condition and begin scanning keyboard
 * 0xF7 Set all keys to autorepeat (PS/2 only)
 * 0xF8 Set all keys to send make code and break code (PS/2 only)
 * 0xF9 Set all keys to generate only make codes
 * 0xFA Set all keys to autorepeat and generate make/break codes
 * 0xFB Set a single key to autorepeat
 * 0xFC Set a single key to generate make and break codes
 * 0xFD Set a single key to generate only break codes
 * 0xFE Resend last result
 * 0xFF Reset keyboard to power on state and start self test*
 *
 * KBRD_PORT_ENCODER `out` error return list
 * ----------------------------------------------------------------------------
 * 0x0  Internal buffer overrun
 * 0x1-0x58, 0x81-0xD8  Keypress scan code
 * 0x83AB   Keyboard ID code returned from F2 command
 * 0xAA Returned during Basic Assurance Test (BAT) after reset. Also L. shift key make code
 * 0xEE Returned from the ECHO command
 * 0xF0 Prefix of certain make codes (Does not apply to PS/2)
 * 0xFA Keyboard acknowledge to keyboard command
 * 0xFC Basic Assurance Test (BAT) failed (PS/2 only)
 * 0xFD Diagonstic failure (Except PS/2)
 * 0xFE Keyboard requests for system to resend last command
 * 0xFF Key error (PS/2 only) 
 *
 * ============================================================================
 *
 * KBRD_PORT_CTRL cmd
 * ----------------------------------------------------------------------------
 * Common Commands
 * ~~~~~~~~~~~~~~~
 * 0x20 Read command byte
 * 0x60 Write command byte
 * 0xAA Self Test
 * 0xAB Interface Test
 * 0xAD Disable Keyboard
 * 0xAE Enable Keyboard
 * 0xC0 Read Input Port
 * 0xD0 Read Output Port
 * 0xD1 Write Output Port
 * 0xE0 Read Test Inputs
 * 0xFE System Reset
 * 0xA7 Disable Mouse Port
 * 0xA8 Enable Mouse Port
 * 0xA9 Test Mouse Port
 * 0xD4 Write To Mouse
 * _____________________
 * Non Standard Commands
 * ~~~~~~~~~~~~~~~~~~~~~
 * 0x00-0x1F    Read Controller RAM
 * 0x20-0x3F    Read Controller RAM
 * 0x40-0x5F    Write Controller RAM
 * 0x60-0x7F    Write Controller RAM
 * 0x90-0x93    Synaptics Multiplexer Prefix
 * 0x90-0x9F    Write port 13-Port 10
 * 0xA0 Read Copyright
 * 0xA1 Read Firmware Version
 * 0xA2 Change Speed
 * 0xA3 Change Speed
 * 0xA4 Check if password is installed
 * 0xA5 Load Password
 * 0xA6 Check Password
 * 0xAC Disagnostic Dump
 * 0xAF Read Keyboard Version
 * 0xB0-0xB5    Reset Controller Line
 * 0xB8-0xBD    Set Controller Line
 * 0xC1 Continuous input port poll, low
 * 0xC2 Continuous input port poll, high
 * 0xC8 Unblock Controller lines P22 and P23
 * 0xC9 Block Controller lines P22 and P23
 * 0xCA Read Controller Mode
 * 0xCB Write Controller Mode
 * 0xD2 Write Output Buffer
 * 0xD3 Write Mouse Output Buffer
 * 0xDD Disable A20 address line
 * 0xDF Enable A20 address line
 * 0xF0-0xFF    Pulse output bit*
 */

/* Standard */
#define CTRL_CMD_READB 0x20
#define CTRL_CMD_WRITEB 0x60
#define CTRL_CMD_SELF_TEST 0xAA
#define CTRL_CMD_INTERF_TEST 0xAB
#define CTRL_CMD_KBRD_DISABLE 0xAD
#define CTRL_CMD_KBRD_ENABLE 0xAE
#define CTRL_CMD_READ_IN 0xC0
#define CTRL_CMD_READ_OUT 0xD0
#define CTRL_CMD_WRITE_OUT 0xD1
#define CTRL_CMD_READ_TEST_IN 0xE0
#define CTRL_CMD_SYS_RESET 0xFE
#define CTRL_CMD_MOUSE_DISABLE_PORT 0xA7
#define CTRL_CMD_MOUSE_ENABLE_PORT 0xA8
#define CTRL_CMD_MOUSE_TEST_PORT 0xA9
#define CTRL_CMD_MOUSE_WRITE 0xD4
/* TODO: there are some neat non-standard cmds */

/* Status register is read from KBRD_PORT_CTRL */
#define STATUS_READ_BUF_FULL(reg)   \
        ((char)(reg) & 0x1)
#define STATUS_WRITE_BUF_FULL(reg)  \
        ((char)(reg) & 0x2)
#define STATUS_SELF_TEST_DONE(reg)  \
        ((char)(reg) & 0x4)
#define STATUS_LAST_WRITE_DATA(reg) \
        ((char)(~(reg)) & 0x8) /* from KBRD_PORT_ENCODER */
#define STATUS_LAST_WRITE_CMD(reg)  \
        ((char)(reg) & 0x8) /* from KBRD_PORT_CTRL */
#define STATUS_LOCKED(reg)  \
        (((char)(~(reg))) & 0x10)
#define STATUS_AUXILIARY_READ_BUF_FULL  \
        ((char)(reg) & 0x20)
#define STATUS_TIMEOUT(reg) \
        ((char)(reg) & 0x40)
#define STATUS_PARITY_ERROR(reg)    \
        ((char)(reg) & 0x80)

/* error checks */
#define ENCOD_IS_MAKE_CODE(code)    \
     (((code) >= 0x1) && ((code) <= 0x58))
#define ENCOD_IS_BREAK_CODE(code)   \
     (((code) >= 0x81) && ((code) <= 0xDB))
#define ENCOD_IS_SCAN_CODE(rslt)    \
        (ENCOD_IS_MAKE_CODE(rslt) ||    \
         ENCOD_IS_BREAK_CODE(rslt))
#define ENCOD_BUF_OVERRUN(rslt) ((rslt) == 0)
#define ENCOD_DIAGNOSTIC_FAIL(rslt) ((rslt) == 0xFD)
#define ENCOD_BAT_FAIL(rslt) ((rslt) == 0xFC) /* BAT - Basic Assurance Test */
#define ENCOD_RESEND_REQ(rslt) ((rslt) == 0xFE)
#define ENCOD_KEY_ERROR(rslt) ((rslt) == 0xFF)
#define ENCOD_IS_ERROR(rslt)    \
    ((!ENCOD_IS_SCAN_CODE(rslt)) && \
         (ENCOD_BUF_OVERRUN(rslt) || ENCOD_DIAGNOSTIC_FAIL(rslt) || \
          ENCOD_BAT_FAIL(rslt) || ENCOD_RESEND_REQ(rslt) || \
          ENCOD_KEY_ERROR(rslt)))

#define IS_SHIFT_MAKE(code) \
     ((SCAN_CODES_SINGLE[(code)] == (KEY_LSHIFT)) ||    \
      (SCAN_CODES_SINGLE[(code)] == (KEY_RSHIFT)))
#define IS_SHIFT_BREAK(code)    \
     ((SCAN_CODES_BREAK[(code)] == (KEY_LSHIFT)) || \
      (SCAN_CODES_BREAK[(code)] == (KEY_RSHIFT)))
#define IS_CTRL_MAKE(code)  \
     ((SCAN_CODES_SINGLE[(code)] == (KEY_LCTRL)) || \
      (SCAN_CODES_SINGLE[(code)] == (KEY_RCTRL)))
#define IS_CTRL_BREAK(code) \
     ((SCAN_CODES_BREAK[(code)] == KEY_LCTRL) ||    \
      (SCAN_CODES_BREAK[(code)] == KEY_RCTRL))
#define IS_CAPS_MAKE(code)  \
     (SCAN_CODES_SINGLE[(code)] == (KEY_CAPSLOCK))

#define IS_MULTICODE(code)  \
     ((code) == 0xE0 || (code) == 0xE1)

static bool _shift_on;
static bool _caps_on;
static bool _ctrl_on;
static bool _multicode;

static char get_kbrd_buffer()
{
    return inportb(KBRD_PORT_ENCODER);
}

static char get_kbrd_status()
{
    return inportb(KBRD_PORT_CTRL);
}

static char shift_effect(char c, short code)
{
    if (ISDIGIT(c) || ISSYMBOL(c))
        return SCAN_CODES_SYMBOLS[code];
    else if (ISALPHA(c) && ISLOWER(c))
        return TOUPPER(c);

    return c;
}

static char caps_effect(char c)
{
    if (ISALPHA(c) && ISLOWER(c))
        return TOUPPER(c);

    return c;
}

static void handle_make_code(short code)
{
    char c;

    if (IS_SHIFT_MAKE(code))
    {
        _shift_on = true;
        return;
    }
    else if (IS_CTRL_MAKE(code))
    {
        _ctrl_on = true;
        return;
    }
    else if (IS_CAPS_MAKE(code))
    {
        _caps_on = !_caps_on;
        return;
    }

    if (_multicode)
    {
        c = SCAN_CODES_MULTI[code];
        _multicode = false;
    }
    else
        c = SCAN_CODES_SINGLE[code];

    if (!ISPRINTABLE(c))
        return;

    if (_shift_on)
        c = shift_effect(c, code);
    if (_caps_on)
        c = caps_effect(c);

    shell_kbrd_cb(c);
}

static void handle_break_code(short code)
{
    if (_multicode)
        _multicode = false;

    if (IS_SHIFT_BREAK(code))
    {
        _shift_on = false;
        return;
    }
    else if (IS_CTRL_BREAK(code))
    {
        _ctrl_on = false;
        return;
    }
}

/*
 * Keyboard interrupt handler
 */
void x86_kbr_irq_do_handle()
{
    unsigned char status, buf;

repeat:
    /* First check what state kbrd is in and if ready to give us something */
    status = get_kbrd_status();
    /* another choice might be to busy-loop while not ready. TODO: investigate */
    if (!STATUS_READ_BUF_FULL(status))
        goto exit;

    /* Try to get the scan code */
    buf = get_kbrd_buffer();

    if (!ENCOD_IS_SCAN_CODE(buf))
    {
        if (ENCOD_RESEND_REQ(buf))
            goto repeat;
        else if (IS_MULTICODE(buf))
        {
            _multicode = true;
            goto exit;
        }
        else
            goto exit;
    }

    if (ENCOD_IS_MAKE_CODE(buf))
        handle_make_code(buf);
    else if (ENCOD_IS_BREAK_CODE(buf))
        handle_break_code(buf);

    /* printf("%x", buf); */

exit:
    irq_done(IRQ1_VECTOR);
}

/*
 * Performs keyboard self-test and
 * returns 0 on success
 */
static int do_self_test()
{
    /* request self test */
    outportb(KBRD_PORT_CTRL, CTRL_CMD_SELF_TEST);

    while (!STATUS_SELF_TEST_DONE(get_kbrd_status()))
        ;

    /* return status; */
    if (get_kbrd_buffer() == 0x55)
        return 0;
    return -1;
}

/*
 * Disables keyboard.
 * No IRQs are going to be executed.
 */
static int kbrd_disable()
{
    outportb(KBRD_PORT_CTRL, CTRL_CMD_KBRD_DISABLE);
    return 0;
}

static int kbrd_enable()
{
    outportb(KBRD_PORT_CTRL, CTRL_CMD_KBRD_ENABLE);
    return 0;
}

/*
 * Performs a total system reset.
 */
void kbrd_sys_reset()
{
    outportb(KBRD_PORT_CTRL, CTRL_CMD_SYS_RESET);
}

/*
 * Keyboard initialization
 */
int kbrd_init()
{
    int status;

    _shift_on = false;
    _caps_on = false;
    _ctrl_on = false;
    _multicode = false;

    status = do_self_test();
    kbrd_disable();
    kbrd_enable();
    return status;
}
