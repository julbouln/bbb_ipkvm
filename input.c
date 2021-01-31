#include "ipkvm.h"

// INPUT
typedef struct {
    uint8_t keycode;          /* HID keysym */
    uint32_t keysym; /* X windows keysym */
} keymap_t;

static keymap_t keymaps[] = {
        {KEY_A,          XK_A},
        {KEY_B,          XK_B},
        {KEY_C,          XK_C},
        {KEY_D,          XK_D},
        {KEY_E,          XK_E},
        {KEY_F,          XK_F},
        {KEY_G,          XK_G},
        {KEY_H,          XK_H},
        {KEY_I,          XK_I},
        {KEY_J,          XK_J},
        {KEY_K,          XK_K},
        {KEY_L,          XK_L},
        {KEY_M,          XK_M},
        {KEY_N,          XK_N},
        {KEY_O,          XK_O},
        {KEY_P,          XK_P},
        {KEY_Q,          XK_Q},
        {KEY_R,          XK_R},
        {KEY_S,          XK_S},
        {KEY_T,          XK_T},
        {KEY_U,          XK_U},
        {KEY_V,          XK_V},
        {KEY_W,          XK_W},
        {KEY_X,          XK_X},
        {KEY_Y,          XK_Y},
        {KEY_Z,          XK_Z},
        {KEY_1,          XK_exclam},
        {KEY_2,          XK_at},
        {KEY_3,          XK_numbersign},
        {KEY_4,          XK_dollar},
        {KEY_5,          XK_percent},
        {KEY_6,          XK_asciicircum},
        {KEY_7,          XK_ampersand},
        {KEY_8,          XK_asterisk},
        {KEY_9,          XK_parenleft},
        {KEY_0,          XK_parenright},
        {KEY_1,          XK_1},
        {KEY_2,          XK_2},
        {KEY_3,          XK_3},
        {KEY_4,          XK_4},
        {KEY_5,          XK_5},
        {KEY_6,          XK_6},
        {KEY_7,          XK_7},
        {KEY_8,          XK_8},
        {KEY_9,          XK_9},
        {KEY_0,          XK_0},
        {KEY_F1,         XK_F1},
        {KEY_F2,         XK_F2},
        {KEY_F3,         XK_F3},
        {KEY_F4,         XK_F4},
        {KEY_F5,         XK_F5},
        {KEY_F6,         XK_F6},
        {KEY_F7,         XK_F7},
        {KEY_F8,         XK_F8},
        {KEY_F9,         XK_F9},
        {KEY_F10,        XK_F10},
        {KEY_F11,        XK_F11},
        {KEY_F12,        XK_F12},
        {KEY_SEMICOLON,  XK_colon},
        {KEY_SEMICOLON,  XK_semicolon},
        {KEY_APOSTROPHE, XK_apostrophe},
        {KEY_APOSTROPHE, XK_quotedbl},
        {KEY_COMMA,      XK_comma},
        {KEY_COMMA,      XK_less},
        {KEY_SLASH,      XK_slash},
        {KEY_SLASH,      XK_question},
        {KEY_DOT,        XK_period},
        {KEY_DOT,        XK_greater},
        {KEY_BACKSLASH,  XK_backslash},
        {KEY_BACKSLASH,  XK_bar},
        {KEY_LEFTBRACE,  XK_bracketleft},
        {KEY_LEFTBRACE,  XK_braceleft},
        {KEY_RIGHTBRACE, XK_bracketright},
        {KEY_RIGHTBRACE, XK_braceright},
        {KEY_MINUS,      XK_minus},
        {KEY_MINUS,      XK_underscore},
        {KEY_EQUAL,      XK_equal},
        {KEY_EQUAL,      XK_plus},
        {KEY_SPACE,      XK_space},
        {KEY_GRAVE,      XK_grave},
        {KEY_GRAVE,      XK_asciitilde},
        {KEY_ENTER,      XK_Return},
        {KEY_BACKSPACE,  XK_BackSpace},
        {KEY_ESC,        XK_Escape},
        {KEY_HOME,       XK_Home},
        {KEY_END,        XK_End},
        {KEY_INSERT,     XK_Insert},
        {KEY_DELETE,     XK_Delete},
        {KEY_TAB,        XK_Tab},
        {KEY_CAPSLOCK,   XK_Caps_Lock},
        {KEY_LEFTSHIFT,  XK_Shift_L},
        {KEY_RIGHTSHIFT, XK_Shift_R},
        {KEY_LEFTALT,    XK_Alt_L},
        {KEY_RIGHTALT,   XK_Alt_R},
        {KEY_LEFTCTRL,   XK_Control_L},
        {KEY_RIGHTCTRL,  XK_Control_R},
        {KEY_SYSRQ,      XK_Print},
        {KEY_UP,         XK_Up},
        {KEY_LEFT,       XK_Left},
        {KEY_DOWN,       XK_Down},
        {KEY_RIGHT,      XK_Right},
        {KEY_PAGEUP,     XK_Prior},
        {KEY_PAGEDOWN,   XK_Next},
        {KEY_NUMLOCK,    XK_Num_Lock},
        {KEY_SCROLLLOCK, XK_Scroll_Lock},
        {KEY_PAUSE,      XK_Pause},
        {KEY_KP0,        XK_KP_0},
        {KEY_KP1,        XK_KP_1},
        {KEY_KP2,        XK_KP_2},
        {KEY_KP3,        XK_KP_3},
        {KEY_KP4,        XK_KP_4},
        {KEY_KP5,        XK_KP_5},
        {KEY_KP6,        XK_KP_6},
        {KEY_KP7,        XK_KP_7},
        {KEY_KP8,        XK_KP_8},
        {KEY_KP9,        XK_KP_9},
        {KEY_KP0,        XK_KP_Insert},
        {KEY_KP1,        XK_KP_End},
        {KEY_KP2,        XK_KP_Down},
        {KEY_KP3,        XK_KP_Page_Down},
        {KEY_KP4,        XK_KP_Left},
        {KEY_KP6,        XK_KP_Right},
        {KEY_KP7,        XK_KP_Home},
        {KEY_KP8,        XK_KP_Up},
        {KEY_KP9,        XK_KP_Page_Up},
        {KEY_KPMINUS,    XK_KP_Subtract},
        {KEY_KPPLUS,     XK_KP_Add},
        {KEY_KPENTER,    XK_KP_Enter},
        {KEY_KPDOT,      XK_KP_Delete},
        {KEY_KPDOT,      XK_KP_Decimal},
        {KEY_KPEQUAL,    XK_KP_Equal},
        {KEY_KPASTERISK, XK_KP_Multiply},
        {KEY_KPSLASH,    XK_KP_Divide},
        {KEY_COMPOSE,    XK_Menu},
        {0x08,           XK_Super_L},
        {0x80,           XK_Super_R},
        {0,              0}};

void input_open(ipkvm_t *ipkvm, char *keyboard_path, char *pointer_path) {
    ipkvm->keyboard_fd = open(keyboard_path, O_RDWR | O_NONBLOCK);
    if (ipkvm->keyboard_fd < 0) {
        fprintf(stderr, "Cannot open keyboard %s\n", keyboard_path);
    }

    ipkvm->pointer_fd = open(pointer_path, O_RDWR | O_NONBLOCK);
    if (ipkvm->pointer_fd < 0) {
        fprintf(stderr, "Cannot open pointer %s\n", pointer_path);
    }
}

void input_keyboard_event(ipkvm_t *ipkvm, rfbBool down, rfbKeySym keysym) {
    int i = 0;
    uint8_t code = 0;
    keymap_t *k_ptr;

    if (ipkvm->keyboard_fd >= 0) {
        if (keysym >= 0x61 && keysym <= 0x7a)
            keysym -= 0x20;

        for (k_ptr = keymaps; k_ptr->keycode != 0; k_ptr++)
            if (k_ptr->keysym == keysym) {
                code = k_ptr->keycode;
                break;
            }

        if (k_ptr->keycode == 0)
            return;

        switch (keysym) {
            case XK_Control_L:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x01 : ipkvm->keyboard_mod & ~0x01;
                break;
            case XK_Shift_L:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x02 : ipkvm->keyboard_mod & ~0x02;
                break;
            case XK_Alt_L:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x04 : ipkvm->keyboard_mod & ~0x04;
                break;
            case XK_Control_R:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x10 : ipkvm->keyboard_mod & ~0x10;
                break;
            case XK_Shift_R:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x20 : ipkvm->keyboard_mod & ~0x20;
                break;
            case XK_Alt_R:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x40 : ipkvm->keyboard_mod & ~0x40;
                break;
            case XK_Super_L:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x08 : ipkvm->keyboard_mod & ~0x08;
                break;
            case XK_Super_R:
                ipkvm->keyboard_mod = down ? ipkvm->keyboard_mod | 0x80 : ipkvm->keyboard_mod & ~0x80;
                break;
            default:
                break;
        }

        ipkvm->keyboard_report[0] = ipkvm->keyboard_mod;

        if (keysym < XK_Shift_L ||
            keysym > XK_Hyper_R ||
            keysym == XK_Caps_Lock) 
        {
            if (down) {
                for (i = 2; i < KEY_REPORT_LENGTH; i++)
                    if (ipkvm->keyboard_report[i] == 0) {
                        ipkvm->keyboard_report[i] = code;
                        break;
                    }
            } else {
                for (i = 2; i < KEY_REPORT_LENGTH; i++)
                    if (ipkvm->keyboard_report[i] == code) {
                        ipkvm->keyboard_report[i] = 0;
                        break;
                    }
            }
        }

        if (write(ipkvm->keyboard_fd, &ipkvm->keyboard_report, KEY_REPORT_LENGTH) != KEY_REPORT_LENGTH) {
            // something wrong happen
        }

        // clear
        memset(ipkvm->keyboard_report, 0, KEY_REPORT_LENGTH);
        if (write(ipkvm->keyboard_fd, ipkvm->keyboard_report, KEY_REPORT_LENGTH) != KEY_REPORT_LENGTH) {
            // something wrong happen
        }
    }
}

int prev_x = -1;
int prev_y = -1;

void input_pointer_event(ipkvm_t *ipkvm, int mask, int x, int y) {
    if (ipkvm->pointer_fd >= 0) {
        uint16_t m_x = (uint16_t)(x * (SHRT_MAX + 1) / ipkvm->width);
        uint16_t m_y = (uint16_t)(y * (SHRT_MAX + 1) / ipkvm->height);
        unsigned char button = 0;
        unsigned char wheel = 0;

        memset(ipkvm->pointer_report, 0, PTR_REPORT_LENGTH);

        if (mask <= 4) {
            if (mask == 2)
                button = 4;
            else if (mask == 4)
                button = 2;
            else
                button = mask;
        } else {
            button = 0;
            if (mask == 8)
                wheel = 1;
            else if (mask == 16)
                wheel = 0xff;
        }

        ipkvm->pointer_report[0] = button;
        ipkvm->pointer_report[1] = m_x & 0xff;
        ipkvm->pointer_report[2] = (m_x >> 8) & 0xff;
        ipkvm->pointer_report[3] = m_y & 0xff;
        ipkvm->pointer_report[4] = (m_y >> 8) & 0xff;
        ipkvm->pointer_report[5] = wheel;

        if (write(ipkvm->pointer_fd, ipkvm->pointer_report, PTR_REPORT_LENGTH) != PTR_REPORT_LENGTH) {
            // something wrong happen
        }

#if 0
        memset(ipkvm->pointer_report, 0, PTR_REPORT_LENGTH);
        if (write(ipkvm->pointer_fd, ipkvm->pointer_report, PTR_REPORT_LENGTH) != PTR_REPORT_LENGTH) {
            // something wrong happen
        }
#endif

        prev_x = x;
        prev_y = y;
    }
}

void input_close(ipkvm_t *ipkvm) {
    if (ipkvm->keyboard_fd >= 0) {
        close(ipkvm->keyboard_fd);
        ipkvm->keyboard_fd = -1;
    }
    if (ipkvm->pointer_fd >= 0) {
        close(ipkvm->pointer_fd);
        ipkvm->pointer_fd = -1;
    }
}
