#include "keyboard.h"
#include "interrupt.h"
#include "console.h"
#include "debug.h"
#include "io.h"
#include "io_queue.h"

#define KEYBOARD_BUF_PORT 0x60
#define KEYBOARD_OTHER_PORT 0x64

#define ESC         '\x1b'
#define BACKSPACE   '\b'
#define TAB         '\t'
#define ENTER       '\r'
#define DELETE      '\x7f'
#define SPACE       ' '

#define CONTROL_KEY 0
#define CAPSLOCK    CONTROL_KEY
#define LEFT_SHIFT  CONTROL_KEY
#define LEFT_CTRL   CONTROL_KEY
#define LEFT_ALT    CONTROL_KEY
#define RIGHT_SHIFT CONTROL_KEY
#define RIGHT_CTRL  CONTROL_KEY
#define RIGHT_ALT   CONTROL_KEY

#define LEFT_SHIFT_MAKECODE  0x2a
#define RIGHT_SHIFT_MAKECODE 0x36
#define LEFT_ALT_MAKECODE    0x38
#define RIGHT_ALT_MAKECODE   0xe038
#define LEFT_CTRL_MAKECODE   0x1d
#define RIGHT_CTRL_MAKECODE  0xe01d
#define RIGHT_ALT_BREAKCODE  0xe0b8
#define RIGHT_CTRL_BREAKCODE 0xe09d
#define CAPSLOCK_MAKECODE    0x3a

//键盘扫描码和ascii码的映射表
//索引为键盘扫描码，数组项为该key对应的ascii码和key+shift对应的ascii码
static char scancode_ascii_map[][2] = {
    {0, 0},                             //0x00
    {ESC, ESC},                         //0x01
    {'1', '!'},                         //0x02
    {'2', '@'},                         //0x03
    {'3', '#'},                         //0x04
    {'4', '$'},                         //0x05
    {'5', '%'},                         //0x06
    {'6', '^'},                         //0x07
    {'7', '&'},                         //0x08
    {'8', '*'},                         //0x09
    {'9', '('},                         //0x0a
    {'0', ')'},                         //0x0b
    {'-', '_'},                         //0x0c
    {'=', '+'},                         //0x0d
    {BACKSPACE, BACKSPACE},             //0x0e
    {TAB, TAB},                         //0x0f
    {'q', 'Q'},                         //0x10
    {'w', 'W'},                         //0x11
    {'e', 'E'},                         //0x12
    {'r', 'R'},                         //0x13
    {'t', 'T'},                         //0x14
    {'y', 'Y'},                         //0x15
    {'u', 'U'},                         //0x16
    {'i', 'I'},                         //0x17
    {'o', 'O'},                         //0x18
    {'p', 'P'},                         //0x19
    {'[', '{'},                         //0x1a
    {']', '}'},                         //0x1b
    {ENTER, ENTER},                     //0x1c
    {LEFT_CTRL, LEFT_CTRL},             //0x1d
    {'a', 'A'},                         //0x1e
    {'s', 'S'},                         //0x1f
    {'d', 'D'},                         //0x20
    {'f', 'F'},                         //0x21
    {'g', 'G'},                         //0x22
    {'h', 'H'},                         //0x23
    {'j', 'J'},                         //0x24
    {'k', 'K'},                         //0x25
    {'l', 'L'},                         //0x26
    {';', ':'},                         //0x27
    {'\'', '"'},                        //0x28
    {'`', '~'},                         //0x29
    {LEFT_SHIFT, LEFT_SHIFT},           //0x2a
    {'\\', '|'},                        //0x2b
    {'z', 'Z'},                         //0x2c
    {'x', 'X'},                         //0x2d
    {'c', 'C'},                         //0x2e
    {'v', 'V'},                         //0x2f
    {'b', 'B'},                         //0x30
    {'n', 'N'},                         //0x31
    {'m', 'M'},                         //0x32
    {',', '<'},                         //0x33
    {'.', '>'},                         //0x34
    {'/', '?'},                         //0x35
    {RIGHT_SHIFT, RIGHT_SHIFT},         //0x36
    {'*', '*'},                         //0x37
    {LEFT_ALT, LEFT_ALT},               //0x38
    {SPACE, SPACE},                     //0x39
    {CAPSLOCK, CAPSLOCK}                //0x3a
    
    //other keys, todo
};

static bool ctrl_down;
static bool shift_down;
static bool alt_down;
static bool capslock_down;
static bool ext_scancode;
io_queue_t keyboard_buffer;


static void keyboard_interrupt_hanlder() {
    uint16_t scancode = inb(KEYBOARD_BUF_PORT);

    //如果开头为0xe0，那么该键扫描码有多个字节
    if (scancode == 0xe0) {
        ext_scancode = true;
        return;
    }

    if (ext_scancode) {
        scancode = ((0xe000) | scancode);
        ext_scancode = false;
    }

    bool is_breakcode = !!(0x0080 & scancode);
    if (is_breakcode) {
        scancode &= 0xff7f;
        uint16_t makecode = scancode;
        if (makecode == LEFT_CTRL_MAKECODE || makecode == RIGHT_CTRL_MAKECODE) {
            ctrl_down = false;
        } else if (makecode == LEFT_SHIFT_MAKECODE || makecode == RIGHT_SHIFT_MAKECODE) {
            shift_down = false;
        } else if (makecode == LEFT_ALT_MAKECODE || makecode == RIGHT_ALT_MAKECODE) {
            alt_down = false;
        }
        return;
    } else if ((scancode > 0x00 && scancode < 0x3b) || 
               (scancode == RIGHT_CTRL_MAKECODE) ||
               (scancode == RIGHT_ALT_MAKECODE)) {
        bool has_shift = false;

        //为代表两个字符的键，不包括字母
        if ((scancode < 0x0e) || (scancode == 0x2a) ||
               (scancode >= 0x27 && scancode <= 0x29) || 
               (scancode >= 0x33 && scancode <= 0x35) ||
               (scancode == 0x1a) || (scancode == 0x1b)) {
            if (shift_down) {
                has_shift = true;
            }
        } else {
            if ((shift_down == true && capslock_down == false) ||
                (shift_down == false && capslock_down == true)) {
                has_shift = true;
            }
        }

        scancode &= 0x00ff;
        uint8_t index = scancode;
        char ascii_char = has_shift ? scancode_ascii_map[index][1] : scancode_ascii_map[index][0];
        if (ascii_char != 0) {
            io_queue_putchar(&keyboard_buffer, ascii_char);
            return;
        }

        //记录此次按键是否为控制键
        if (scancode == LEFT_CTRL_MAKECODE || scancode == RIGHT_CTRL_MAKECODE) {
            ctrl_down = true;
        } else if (scancode == LEFT_SHIFT_MAKECODE || scancode == RIGHT_SHIFT_MAKECODE) {
            shift_down = true;
        } else if (scancode == LEFT_ALT_MAKECODE || scancode == RIGHT_ALT_MAKECODE) {
            alt_down = true;
        } else if (scancode == CAPSLOCK_MAKECODE) {
            capslock_down = !capslock_down;
        }
    } else {
        console_put_int(scancode);
        ASSERT("error" == "unknow key!");
    }
}

void keyboard_init() {
    io_queue_init(&keyboard_buffer);
    intr_set_handler(0x21, keyboard_interrupt_hanlder);
}

