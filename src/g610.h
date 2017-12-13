#pragma once

#ifndef G610_H
#define G610_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi/hidapi.h"
#include "stdint.h"
#include "stdbool.h"

#define G610_VENDOR_ID 0x046d
#define G610_PRODUCT_ID 0xc333
#define G610_READ_MAX_SIZE 6 + 20 + 8 // dev_0 + dev_1 + modifiers


typedef struct g610_device g610_device;


typedef enum g610_address_group {
	ADDRESS_GROUP_LOGO = 0x00,
	ADDRESS_GROUP_INDICATORS = 0x01,
	ADDRESS_GROUP_MULTIMEDIA = 0x02,
	ADDRESS_GROUP_GKEYS = 0x03,
	ADDRESS_GROUP_KEYS = 0x04
} g610_address_group;


typedef enum g610_key { // 127 items
	// GROUP LOGO
	KEY_LOGO = ADDRESS_GROUP_LOGO << 8 | 0x01,
	KEY_LOGO2,

	// GROUP INDICATORS
	KEY_BACKLIGHT = ADDRESS_GROUP_INDICATORS << 8 | 0x01,
	KEY_GAME, KEY_CAPS, KEY_SCROLL, KEY_NUM,

	// GROUP MULTIMEDIA
	KEY_NEXT = ADDRESS_GROUP_MULTIMEDIA << 8 | 0xb5,
	KEY_PREV, KEY_STOP,
	KEY_PLAY = ADDRESS_GROUP_MULTIMEDIA << 8 | 0xcd,
	KEY_MUTE = ADDRESS_GROUP_MULTIMEDIA << 8 | 0xe2,

	// GROUP GKEYS
	KEY_G1 = ADDRESS_GROUP_GKEYS << 8 | 0x01,
	KEY_G2, KEY_G3, KEY_G4, KEY_G5, KEY_G6, KEY_G7, KEY_G8, KEY_G9,

	// GROUP KEYS
	KEY_A = ADDRESS_GROUP_KEYS << 8 | 0x04,
	KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N,
	KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

	// GROUP DIGITS
	KEY_N1, KEY_N2, KEY_N3, KEY_N4, KEY_N5, KEY_N6, KEY_N7, KEY_N8, KEY_N9, KEY_N0,

	// GROUP TYPING
	KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_OPEN_BRACKET, KEY_CLOSE_BRACKET,
	KEY_BACKSLASH, KEY_DOLLAR, KEY_SEMICOLON, KEY_QUOTE, KEY_TILDE, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_CAPS_LOCK,

	// GROUP FUNCTION
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

	// GROUP PAGE
	KEY_PRINT_SCREEN, KEY_SCROLL_LOCK, KEY_PAUSE_BREAK, KEY_INSERT,
	KEY_HOME, KEY_PAGE_UP, KEY_DEL, KEY_END, KEY_PAGE_DOWN,

	// GROUP ARROWS
	KEY_ARROW_RIGHT, KEY_ARROW_LEFT, KEY_ARROW_BOTTOM, KEY_ARROW_TOP,
	
	// GROUP NUMPAD
	KEY_NUM_LOCK, KEY_NUM_SLASH, KEY_NUM_ASTERISK,
	KEY_NUM_MINUS, KEY_NUM_PLUS, KEY_NUM_ENTER,
	KEY_NUM_1, KEY_NUM_2, KEY_NUM_3, KEY_NUM_4, KEY_NUM_5,
	KEY_NUM_6, KEY_NUM_7, KEY_NUM_8, KEY_NUM_9, KEY_NUM_0,
	KEY_NUM_DOT,
	
	// OUTSIDERS
	KEY_INTL_BACKSLASH, KEY_MENU,

	// GROUP MODIFIERS
	KEY_CTRL_LEFT = ADDRESS_GROUP_KEYS << 8 | 0xe0,
	KEY_SHIFT_LEFT, KEY_ALT_LEFT, KEY_WIN_LEFT,
	KEY_CTRL_RIGHT, KEY_SHIFT_RIGHT, KEY_ALT_RIGHT, KEY_WIN_RIGHT
} g610_key;

static const g610_key G610_KEY_GROUP_LOGO[] = {
	KEY_LOGO, KEY_LOGO2
};

static const g610_key G610_KEY_GROUP_INDICATORS[] = {
	KEY_BACKLIGHT, KEY_GAME, KEY_CAPS, KEY_SCROLL, KEY_NUM
};

static const g610_key G610_KEY_GROUP_MULTIMEDIA[] = {
	KEY_NEXT, KEY_PREV, KEY_PLAY, KEY_MUTE
};

static const g610_key G610_KEY_GROUP_GKEYS[] = {
	KEY_G1, KEY_G2, KEY_G3, KEY_G4, KEY_G5, KEY_G6, KEY_G7, KEY_G8, KEY_G9
};

static const g610_key G610_KEY_GROUP_KEYS[] = {
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
	KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
	KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
};

static const g610_key G610_KEY_GROUP_DIGITS[] = {
	KEY_N1, KEY_N2, KEY_N3, KEY_N4, KEY_N5,
	KEY_N6, KEY_N7, KEY_N8, KEY_N9, KEY_N0
};

static const g610_key G610_KEY_GROUP_TYPING[] = {
	KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE, KEY_MINUS,
	KEY_EQUAL, KEY_OPEN_BRACKET, KEY_CLOSE_BRACKET, KEY_BACKSLASH, KEY_DOLLAR,
	KEY_SEMICOLON, KEY_QUOTE, KEY_TILDE, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
	KEY_CAPS_LOCK
};

static const g610_key G610_KEY_GROUP_FUNCTION[] = {
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
	KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12
};

static const g610_key G610_KEY_GROUP_PAGE[] = {
	KEY_PRINT_SCREEN, KEY_SCROLL_LOCK, KEY_PAUSE_BREAK, KEY_INSERT,
	KEY_HOME, KEY_PAGE_UP, KEY_DEL, KEY_END, KEY_PAGE_DOWN
};

static const g610_key G610_KEY_GROUP_ARROWS[] = {
	KEY_ARROW_RIGHT, KEY_ARROW_LEFT, KEY_ARROW_BOTTOM, KEY_ARROW_TOP
};
	
static const g610_key G610_KEY_GROUP_NUMPAD[] = {
	KEY_NUM_LOCK, KEY_NUM_SLASH, KEY_NUM_ASTERISK,
	KEY_NUM_MINUS, KEY_NUM_PLUS, KEY_NUM_ENTER,
	KEY_NUM_1, KEY_NUM_2, KEY_NUM_3, KEY_NUM_4, KEY_NUM_5,
	KEY_NUM_6, KEY_NUM_7, KEY_NUM_8, KEY_NUM_9, KEY_NUM_0,
	KEY_NUM_DOT
};
	
static const g610_key G610_KEY_GROUP_MODIFIERS[] = {
	KEY_CTRL_LEFT, KEY_SHIFT_LEFT, KEY_ALT_LEFT, KEY_WIN_LEFT,
	KEY_CTRL_RIGHT, KEY_SHIFT_RIGHT, KEY_ALT_RIGHT, KEY_WIN_RIGHT
};


typedef enum g610_address_group_start {
	ADDRESS_GROUP_LOGO_START = KEY_LOGO,
	ADDRESS_GROUP_INDICATORS_START = KEY_BACKLIGHT,
	ADDRESS_GROUP_GKEYS_START = KEY_G1,
	ADDRESS_GROUP_KEYS_START = KEY_A,
	ADDRESS_GROUP_MODIFIERS_START = KEY_CTRL_LEFT
} g610_address_group_start;


typedef struct g610_led {
	g610_key key;
	uint8_t intensity;
} g610_led;



g610_device * g610_open();
void g610_close(g610_device * device);
g610_address_group g610_get_key_group(g610_key key);
bool g610_set_led(g610_device * device, g610_led led);
bool g610_set_leds(g610_device * device, g610_led * leds, size_t length);
size_t g610_get_keys(g610_device * device, g610_key * keys, size_t length);

#endif // G610_H
