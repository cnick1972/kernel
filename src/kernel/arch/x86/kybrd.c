#include "kybrd.h"
#include "irq.h"
#include "io.h"
#include "../../stdio.h"
#include "../../util/binary.h"

// keyboard encoder ------------------------------------------

enum KYBRD_ENCODER_IO {

	KYBRD_ENC_INPUT_BUF	=	0x60,
	KYBRD_ENC_CMD_REG	=	0x60
};

enum KYBRD_ENC_CMDS {

	KYBRD_ENC_CMD_SET_LED				=	0xED,
	KYBRD_ENC_CMD_ECHO					=	0xEE,
	KYBRD_ENC_CMD_SCAN_CODE_SET			=	0xF0,
	KYBRD_ENC_CMD_ID					=	0xF2,
	KYBRD_ENC_CMD_AUTODELAY				=	0xF3,
	KYBRD_ENC_CMD_ENABLE				=	0xF4,
	KYBRD_ENC_CMD_RESETWAIT				=	0xF5,
	KYBRD_ENC_CMD_RESETSCAN				=	0xF6,
	KYBRD_ENC_CMD_ALL_AUTO				=	0xF7,
	KYBRD_ENC_CMD_ALL_MAKEBREAK			=	0xF8,
	KYBRD_ENC_CMD_ALL_MAKEONLY			=	0xF9,
	KYBRD_ENC_CMD_ALL_MAKEBREAK_AUTO	=	0xFA,
	KYBRD_ENC_CMD_SINGLE_AUTOREPEAT		=	0xFB,
	KYBRD_ENC_CMD_SINGLE_MAKEBREAK		=	0xFC,
	KYBRD_ENC_CMD_SINGLE_BREAKONLY		=	0xFD,
	KYBRD_ENC_CMD_RESEND				=	0xFE,
	KYBRD_ENC_CMD_RESET					=	0xFF
};


// keyboard controller ---------------------------------------

enum KYBRD_CTRL_IO {

	KYBRD_CTRL_STATS_REG=	0x64,
	KYBRD_CTRL_CMD_REG	=	0x64
};

enum KYBRD_CTRL_STATS_MASK {

	KYBRD_CTRL_STATS_MASK_OUT_BUF	=	1,		//00000001
	KYBRD_CTRL_STATS_MASK_IN_BUF	=	2,		//00000010
	KYBRD_CTRL_STATS_MASK_SYSTEM	=	4,		//00000100
	KYBRD_CTRL_STATS_MASK_CMD_DATA	=	8,		//00001000
	KYBRD_CTRL_STATS_MASK_LOCKED	=	0x10,	//00010000
	KYBRD_CTRL_STATS_MASK_AUX_BUF	=	0x20,	//00100000
	KYBRD_CTRL_STATS_MASK_TIMEOUT	=	0x40,	//01000000
	KYBRD_CTRL_STATS_MASK_PARITY	=	0x80	//10000000
};

// scan error codes ------------------------------------------

enum KYBRD_ERROR {

	KYBRD_ERR_BUF_OVERRUN			=	0,
	KYBRD_ERR_ID_RET				=	0x83AB,
	KYBRD_ERR_BAT					=	0xAA,	//note: can also be L. shift key make code
	KYBRD_ERR_ECHO_RET				=	0xEE,
	KYBRD_ERR_ACK					=	0xFA,
	KYBRD_ERR_BAT_FAILED			=	0xFC,
	KYBRD_ERR_DIAG_FAILED			=	0xFD,
	KYBRD_ERR_RESEND_CMD			=	0xFE,
	KYBRD_ERR_KEY					=	0xFF
};

//! current scan code
static char _scancode;

//! lock keys
static bool _numlock, _scrolllock, _capslock;

//! shift, alt, and ctrl keys current state
static bool _shift, _alt, _ctrl;

//! set if keyboard error
static int _kkybrd_error = 0;

//! set if the Basic Assurance Test (BAT) failed
static bool _kkybrd_bat_res = false;

//! set if diagnostics failed
static bool _kkybrd_diag_res = false;

//! set if system should resend last command
static bool _kkybrd_resend_res = false;

//! set if keyboard is disabled
static bool _kkybrd_disable = false;

static int _kkybrd_scancode_std [] = {

	//! key			scancode
	KEY_UNKNOWN,	//0
	KEY_ESCAPE,		//1
	KEY_1,			//2
	KEY_2,			//3
	KEY_3,			//4
	KEY_4,			//5
	KEY_5,			//6
	KEY_6,			//7
	KEY_7,			//8
	KEY_8,			//9
	KEY_9,			//0xa
	KEY_0,			//0xb
	KEY_MINUS,		//0xc
	KEY_EQUAL,		//0xd
	KEY_BACKSPACE,	//0xe
	KEY_TAB,		//0xf
	KEY_Q,			//0x10
	KEY_W,			//0x11
	KEY_E,			//0x12
	KEY_R,			//0x13
	KEY_T,			//0x14
	KEY_Y,			//0x15
	KEY_U,			//0x16
	KEY_I,			//0x17
	KEY_O,			//0x18
	KEY_P,			//0x19
	KEY_LEFTBRACKET,//0x1a
	KEY_RIGHTBRACKET,//0x1b
	KEY_RETURN,		//0x1c
	KEY_LCTRL,		//0x1d
	KEY_A,			//0x1e
	KEY_S,			//0x1f
	KEY_D,			//0x20
	KEY_F,			//0x21
	KEY_G,			//0x22
	KEY_H,			//0x23
	KEY_J,			//0x24
	KEY_K,			//0x25
	KEY_L,			//0x26
	KEY_SEMICOLON,	//0x27
	KEY_QUOTE,		//0x28
	KEY_GRAVE,		//0x29
	KEY_LSHIFT,		//0x2a
	KEY_BACKSLASH,	//0x2b
	KEY_Z,			//0x2c
	KEY_X,			//0x2d
	KEY_C,			//0x2e
	KEY_V,			//0x2f
	KEY_B,			//0x30
	KEY_N,			//0x31
	KEY_M,			//0x32
	KEY_COMMA,		//0x33
	KEY_DOT,		//0x34
	KEY_SLASH,		//0x35
	KEY_RSHIFT,		//0x36
	KEY_KP_ASTERISK,//0x37
	KEY_RALT,		//0x38
	KEY_SPACE,		//0x39
	KEY_CAPSLOCK,	//0x3a
	KEY_F1,			//0x3b
	KEY_F2,			//0x3c
	KEY_F3,			//0x3d
	KEY_F4,			//0x3e
	KEY_F5,			//0x3f
	KEY_F6,			//0x40
	KEY_F7,			//0x41
	KEY_F8,			//0x42
	KEY_F9,			//0x43
	KEY_F10,		//0x44
	KEY_KP_NUMLOCK,	//0x45
	KEY_SCROLLLOCK,	//0x46
	KEY_HOME,		//0x47
	KEY_KP_8,		//0x48	//keypad up arrow
	KEY_PAGEUP,		//0x49
	KEY_KP_2,		//0x50	//keypad down arrow
	KEY_KP_3,		//0x51	//keypad page down
	KEY_KP_0,		//0x52	//keypad insert key
	KEY_KP_DECIMAL,	//0x53	//keypad delete key
	KEY_UNKNOWN,	//0x54
	KEY_UNKNOWN,	//0x55
	KEY_UNKNOWN,	//0x56
	KEY_F11,		//0x57
	KEY_F12			//0x58
};

//! invalid scan code. Used to indicate the last scan code is not to be reused
const int INVALID_SCANCODE = 0;

uint8_t kybrd_ctrl_read_status()
{
    return x86_inb(KYBRD_CTRL_STATS_REG);
}

uint8_t kybrd_enc_read_buf() {
    return x86_inb(KYBRD_ENC_INPUT_BUF);
}

void kybrd_enc_send_cmd(uint8_t cmd) {
    while(1)
        if((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
            break;

    x86_outb(KYBRD_ENC_CMD_REG, cmd);
}

void x86_kybrd_irq(Registers* regs)
{
    static bool _extended =  false;

    int code = 0;

    if(kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_OUT_BUF)
    {
        code = kybrd_enc_read_buf();

        if(code == 0xE0 || code == 0xE1)
            _extended = true;
        else {
            _extended = false;

            if(code & 0x80) {
                code -= 0x80;

                int key = _kkybrd_scancode_std[code];

                switch (key) {
                case KEY_LCTRL:
                case KEY_RCTRL:
                    _ctrl = false;
                    break;

                case KEY_LSHIFT:
                case KEY_RSHIFT:
                    _shift = false;
                    break;

                case KEY_LALT:
                case KEY_RALT:
                    _alt = false;
                    break;
                }
            }
			else {

				//! this is a make code - set the scan code
				_scancode = code;

				//! grab the key
				int key = _kkybrd_scancode_std [code];

				//! test if user is holding down any special keys & set it
				switch (key) {

				case KEY_LCTRL:
				case KEY_RCTRL:
					_ctrl = true;
					break;

				case KEY_LSHIFT:
				case KEY_RSHIFT:
					_shift = true;
					break;

				case KEY_LALT:
				case KEY_RALT:
					_alt = true;
					break;

				case KEY_CAPSLOCK:
					_capslock = (_capslock) ? false : true;
					kkybrd_set_leds (_numlock, _capslock, _scrolllock);
					break;

				case KEY_KP_NUMLOCK:
					_numlock = (_numlock) ? false : true;
					kkybrd_set_leds (_numlock, _capslock, _scrolllock);
					break;

				case KEY_SCROLLLOCK:
					_scrolllock = (_scrolllock) ? false : true;
					kkybrd_set_leds (_numlock, _capslock, _scrolllock);
					break;
				}
			}
        }
		switch(code) {
		case KYBRD_ERR_BAT_FAILED:
			_kkybrd_bat_res = false;
			break;

		case KYBRD_ERR_DIAG_FAILED:
			_kkybrd_diag_res = false;
			break;

		case KYBRD_ERR_RESEND_CMD:
			_kkybrd_resend_res = true;
			break;
		}
    }
}

void kkybrd_set_leds(bool num, bool caps, bool scroll)
{
    uint8_t data = 0;

	if(scroll)
		FLAG_SET(data, 1);
	else
		FLAG_UNSET(data, 1);

	if(num)
		FLAG_SET(data, 2);
	else
		FLAG_UNSET(data,2);

	if(caps)
		FLAG_SET(data, 4);
	else
		FLAG_UNSET(data, 4);


    kybrd_enc_send_cmd(KYBRD_ENC_CMD_SET_LED);
    kybrd_enc_send_cmd(data);
}

uint8_t	kkybrd_get_last_scan ()
{
	return 0;
}

enum KEYCODE kkybrd_get_last_key ()
{
	return (_scancode!=INVALID_SCANCODE) ? ((enum KEYCODE)_kkybrd_scancode_std [_scancode]) : (KEY_UNKNOWN);
}

void kkybrd_discard_last_key ()
{
	_scancode = INVALID_SCANCODE;
}

void kkbrd_install(int i)
{
    x86_IRQ_RegisterHandler(1, x86_kybrd_irq);

	_kkybrd_bat_res = true;
	_scancode = 0;

	_numlock = _scrolllock = _capslock = false;

	kkybrd_set_leds(_numlock, _capslock, _scrolllock);

	_shift = _alt = _ctrl = false;
}

char kkybrd_key_to_ascii(enum KEYCODE code)
{
	uint8_t key = code;

	//! insure key is an ascii character
	if (isascii (key)) {

		//! if shift key is down or caps lock is on, make the key uppercase
		if (_shift || _capslock)
			if (key >= 'a' && key <= 'z')
				key -= 32;

		if (_shift && !_capslock) {
			if (key >= '0' && key <= '9') {
				switch (key) {

					case '0':
						key = KEY_RIGHTPARENTHESIS;
						break;
					case '1':
						key = KEY_EXCLAMATION;
						break;
					case '2':
						key = KEY_AT;
						break;
					case '3':
						key = KEY_EXCLAMATION;
						break;
					case '4':
						key = KEY_HASH;
						break;
					case '5':
						key = KEY_PERCENT;
						break;
					case '6':
						key = KEY_CARRET;
						break;
					case '7':
						key = KEY_AMPERSAND;
						break;
					case '8':
						key = KEY_ASTERISK;
						break;
					case '9':
						key = KEY_LEFTPARENTHESIS;
						break;
				}
			}
			else {

				switch (key) {
					case KEY_COMMA:
						key = KEY_LESS;
						break;

					case KEY_DOT:
						key = KEY_GREATER;
						break;

					case KEY_SLASH:
						key = KEY_QUESTION;
						break;

					case KEY_SEMICOLON:
						key = KEY_COLON;
						break;

					case KEY_QUOTE:
						key = KEY_QUOTEDOUBLE;
						break;

					case KEY_LEFTBRACKET :
						key = KEY_LEFTCURL;
						break;

					case KEY_RIGHTBRACKET :
						key = KEY_RIGHTCURL;
						break;

					case KEY_GRAVE:
						key = KEY_TILDE;
						break;

					case KEY_MINUS:
						key = KEY_UNDERSCORE;
						break;

					case KEY_PLUS:
						key = KEY_EQUAL;
						break;

					case KEY_BACKSLASH:
						key = KEY_BAR;
						break;
				}
			}
		}
		//! return the key
		return key;
	}
	//! scan code != a valid ascii char so no convertion is possible
	return 0;
}

