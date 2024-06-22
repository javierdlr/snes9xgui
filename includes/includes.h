#ifndef INCLUDE_H
#define INCLUDE_H


// gui.c: 'make_chooser_list(<mode>,..)' & 'make_chooser_list2(<mode>,..)'
#define ADD_LIST 0
#define NEW_LIST 1
// snes9xgui.c, cfg.c & gui.c: AHI_PLAYBACK_RATE's MUTE option "index" (0..n)
#define AHI_PBR_MUTE 10
// cfg.c & gui.c: 'SaveConfig(<value>)'
#define ALL 1
#define JOY_REMAP 2


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/listbrowser.h>
#include <proto/clicktab.h>
#include <proto/chooser.h>
#include <proto/layout.h>
#include <proto/graphics.h>
#include <proto/icon.h>
//#include <proto/locale.h>
#include <proto/amigainput.h>

#include <workbench/icon.h>
#include <workbench/startup.h>
#include <libraries/keymap.h> // RAWKEY_#? codes
#include <classes/window.h>
#include <classes/requester.h>
#include <gadgets/clicktab.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/checkbox.h>
#include <gadgets/chooser.h>
#include <gadgets/string.h>
#include <gadgets/space.h>
#include <gadgets/integer.h>
#include <gadgets/slider.h>
#include <images/label.h>
#include <images/bitmap.h>

#include "snes9xgui_rev.h"
#include "snes9xgui_strings.h"


#define OBJ(x) Objects[x]
#define GAD(x) (struct Gadget *)Objects[x]

#define ROMS "PROGDIR:"
#define ROMSF "Roms-French"
#define ROMSU "Roms-Universal"
#define ROMSX "Roms-SuperFX"
#define ROMSM "Roms-MSU1"
#define ROMSO "Roms-Optical"
#define DATAS "PROGDIR:Datas"
#define SAVES "PROGDIR:savestate"


enum {
 COL_TYPE = 0,
 COL_ROM,
 COL_FMT, // ZIP SFC SMC ... (extensions)
 LAST_COL
};

enum {
 // Pages/tabs
 OID_ABOUT = 0,  // 0_About
 OID_GENERAL,    // 1_General
 OID_SETTINGS,   // 2_Settings
 OID_AMIGAINPUT, // 3_AmigaInput
 OID_ABOUT1_IMG,
 OID_ABOUT2_IMG,
 OID_DONATE_BTN,
 OID_DONATE_IMG,
 // Main objects
 OID_MAIN,
 OID_BANNER_IMG,
 // Global Settings
 OID_SETTINGS_GLOBAL,
 OID_VIDMODE,
 OID_MODE_WF,
 OID_SKIP_F,
 OID_PAGE,
 // General objects
 OID_LISTBROWSER,
 OID_PREVIEWS,
 OID_PREVIEW_BTN,
 OID_PREVIEW_IMG, // MUST BE after OID_PREVIEW_BTN [updateButtonImage()]
 OID_SAVESTATES,
 OID_TOTALROMS,
 // Settings objects
OID_SETTINGS_LB,
OID_SETTINGS_GROUP,
OID_GROUP_ACTIVE,
OID_GROUP_01,
OID_GROUP_02,
OID_GROUP_03,
OID_GROUP_04,
OID_GROUP_05,
 OID_GFX_OPTIONS,
 OID_INTERLEAVE,
 OID_MEMMAP,
 OID_PALNTSC,
 OID_OC_SFX,
 OID_NOOVERSCAN,
 OID_EMULATED,
OID_SKINS,
OID_DISP_RENDER,
OID_RENDERTYPE, // "moved" to Global Settings
OID_FILTER,
OID_SHADERS,
 OID_AHI_OPTIONS,
 OID_AHI_PBR,
 OID_AHI_BUF,
 OID_AHI_SYNC,
 OID_AHI_NOST,
 OID_SND_IMETHOD,
 OID_SND_THREAD,
 OID_ENGINE_OPTIONS,
 OID_HACKVIDEO_GROUP,
 OID_HACKVIDEO,
 OID_HV_WIDTH,
 OID_HV_HEIGHT,
 OID_TURBO_FSF,
 OID_SPRITE_LIMIT,
 OID_MSU1,
 OID_MISC_OPTIONS,
 OID_FPS,
 OID_DISP_TIME,
 OID_VSYNC,
OID_QWERTY,
 OID_OC_CPU,
 OID_REWINDING_OPTIONS,
 OID_REWINDING,
 OID_REW_BUF,
 OID_REW_GRA,
 // AmigaInput objects
 OID_P1_OPTIONS,
 OID_P1_KJ,
// OID_P1_STARTB,
 OID_P1_KJ_BTN0,
 OID_P1_KJ_BTN1,
OID_JPAD_NO_BTN = OID_P1_KJ_BTN1,
 OID_P1_BTN_X,
 OID_P1_BTN_A,
 OID_P1_BTN_B,
 OID_P1_BTN_Y,
 OID_P1_BTN_L,
 OID_P1_BTN_R,
 OID_P1_BTN_SEL,
 OID_P1_BTN_STA,
// OID_JOYPAD,
 OID_JOYPAD_IMG,
 OID_P2_OPTIONS,
 OID_P2_KJ,
 OID_P2_KJ_COLS, // used to enable/disable keys/joypad_buttons
// OID_P2_K_COL0, // used to enable/disable keys
// OID_P2_K_COL1, // used to enable/disable keys
// OID_P2_K_COL2, // used to enable/disable keys
// OID_P2_STARTB,
// OID_P2_KJ_BTN0,
// OID_P2_KJ_BTN1,
 OID_P2_BTN_X,
 OID_P2_BTN_A,
 OID_P2_BTN_B,
 OID_P2_BTN_Y,
 OID_P2_BTN_L,
 OID_P2_BTN_R,
 OID_P2_BTN_SEL,
 OID_P2_BTN_STA,
 // AmigaInput: user defined keys
 OID_P1_K_LF,
 OID_P1_K_RI,
 OID_P1_K_UP,
 OID_P1_K_DW,
 OID_P1_K_A,
 OID_P1_K_B,
 OID_P1_K_X,
 OID_P1_K_Y,
 OID_P1_K_L,
 OID_P1_K_R,
 OID_P1_K_SEL,
 OID_P1_K_STA,
 OID_P2_K_LF,
 OID_P2_K_RI,
 OID_P2_K_UP,
 OID_P2_K_DW,
 OID_P2_K_A,
 OID_P2_K_B,
 OID_P2_K_X,
 OID_P2_K_Y,
 OID_P2_K_L,
 OID_P2_K_R,
 OID_P2_K_SEL,
 OID_P2_K_STA,
 // Buttons
 OID_SAVE,
 OID_LOAD,
 OID_QUIT,
 LAST_NUM
};

// Configuration/settings keywords
enum {
 _FRAMESKIP = 0,
 _FULLSCREEN,
 _RETURNROMNUMBER,
 _JOYSTICK,
 _EFFECT,
 _NOINTERLEAVE,
 _EMULATED,
_FILTERHW,
_SHADERS,
_RENDERER,
_SOFT_RENDER,
_SNAPSHOT,
_COMP_RENDER,
_OPENGL_RENDER,
_EGLW_RENDER,
_USE_SKIN,
_SKIN,
 _HIROM,
 _HACKVIDEO,
 _HV_HEIGHT,
 _HV_WIDTH,
 _OCSFX,
 _OCCPU,
 _SPRITE_LIMIT,
 _DISP_TIME,
_DISP_RENDER,
 _NTSC,
 _REGION,
 _INPUTRATE,
 _AHIBUFFER,
 _SYNC,
 _SND_IMETHOD,
 _SND_THREAD,
 _PLAYER1START,
 _PLAYER2START,
 _GUI_SOUND,
 _FPS,
_QWERTY,
 _VSYNC,
 _TURBO_FS,
 _TURBO_FSF,
 _NOOVERSCAN,
 _MSU1,
 _REWINDING,
 _REW_BUF,
 _REW_GRA,
 _PLAYER1JOY,
 _PLAYER2JOY,
 _NOSTEREO,
 _PLAYER1_BTN_X,
 _PLAYER1_BTN_A,
 _PLAYER1_BTN_B,
 _PLAYER1_BTN_Y,
 _PLAYER1_BTN_L,
 _PLAYER1_BTN_R,
 _PLAYER1_BTN_SEL,
 _PLAYER1_BTN_STA,
 _PLAYER2_BTN_X,
 _PLAYER2_BTN_A,
 _PLAYER2_BTN_B,
 _PLAYER2_BTN_Y,
 _PLAYER2_BTN_L,
 _PLAYER2_BTN_R,
 _PLAYER2_BTN_SEL,
 _PLAYER2_BTN_STA,
_LAST_CFG_INT
};
enum { /* user defined keys */
 _P1_KEY_LF = _LAST_CFG_INT,
 _P1_KEY_RI,
 _P1_KEY_UP,
 _P1_KEY_DW,
 _P1_KEY_A,
 _P1_KEY_B,
 _P1_KEY_X,
 _P1_KEY_Y,
 _P1_KEY_L,
 _P1_KEY_R,
 _P1_KEY_SEL,
 _P1_KEY_STA,
 _P2_KEY_LF,
 _P2_KEY_RI,
 _P2_KEY_UP,
 _P2_KEY_DW,
 _P2_KEY_A,
 _P2_KEY_B,
 _P2_KEY_X,
 _P2_KEY_Y,
 _P2_KEY_L,
 _P2_KEY_R,
 _P2_KEY_SEL,
 _P2_KEY_STA,
_LAST_CFG
};

// Listbrowser's pens
enum {
 ROW_O = 0,
 ROW_E,
 TXT_R,
 LAST_PEN
};
#define RGB8to32(RGB) ( (uint32)(RGB) * 0x01010101UL )

/*struct myToolTypes {
	//STRPTR romsdrawer;  // ROMS_DRAWER=<path>
	int32 last_rom_run; // LAST_ROM_LAUNCHED=<value>

	//STRPTR newttp, ttpBuf1, ttpBuf2; // only needed if using SaveToolType()
};*/

/*struct Snes9xGUI {
	struct Screen *screen; // PUSBCREEN=<screen_name>
	struct DiskObject *iconify;
	struct List *romlist;
	struct Window *win;
	struct myToolTypes myTT;
	//STRPTR snes9x_path;
	struct WBStartup *wbs;
	//struct List *savestates_list;
};*/


#endif
