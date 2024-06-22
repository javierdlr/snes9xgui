#include "includes.h"
#include "debug.h"

extern struct DOSIFace *IDOS;
extern struct IntuitionIFace *IIntuition;
extern struct UtilityIFace *IUtility;

//STRPTR RomNumberUniversal;
char *ConfFile = "PROGDIR:snes9X.cfg";
/* use 'printCFG 1' tool to get "updated" keywords */
char *cfg_key[_LAST_CFG] = {"frameskip", "fullscreen", "returnromnumber", "joystick",
                            "effect", "nointerleave", "emulated", "filterhardware",
                            "shaders", "renderer", "software", "snapshot", "compositing",
                            "opengl", "egl_wrap", "skinfx", "skinfxchoice", "hirom",
                            "hackvideo", "height", "width", "superfxoverclock",
                            "overclockcpu", "disablespritelimit", "displaytime",
                            "displayrender", "ntsc", "region", "inputrate", "buffer",
                            "sync", "interpol", "soundthread", "joynostartbuttonP1",
                            "joynostartbuttonP2", "nosoundgui", "fps", "qwerty", "vsync",
                            "turboframeskip", "turboframeskipframes", "nooverscan",
                            "msu1", "rewinding", "rewindbuffer", "rewindgranularity",
                            "player1joy", "player2joy", "nostereo",
                            "j1buttonx", "j1buttona", "j1buttonb", "j1buttony",
                            "j1buttonl", "j1buttonr", "j1buttonselect", "j1buttonstart",
                            "j2buttonx", "j2buttona", "j2buttonb", "j2buttony",
                            "j2buttonl", "j2buttonr", "j2buttonselect", "j2buttonstart",
                            "k1left", "k1right", "k1up", "k1down", "k1buttona",
                            "k1buttonb", "k1buttonx", "k1buttony", "k1buttonl",
                            "k1buttonr", "k1buttonselect", "k1buttonstart",
                            "k2left", "k2right", "k2up", "k2down", "k2buttona",
                            "k2buttonb", "k2buttonx", "k2buttony", "k2buttonl",
                            "k2buttonr", "k2buttonselect", "k2buttonstart"};
int32 cfg_value[_LAST_CFG_INT],
/* use 'printCFG' tool to get "updated" keywords values */
      cfg_default_int[_LAST_CFG_INT] = {1,   0,   0, 0, 0, 1, 0, 0, 0,
                                        2,   0,   0, 1, 0, 0, 0, 0, 0,
                                        0, 640, 480, 0, 0, 0, 0, 0, 0,
                                        0,   4,   0, 0, 2, 1, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 1, 0, 2, 2, 1, 1, 0,
                                        3, 2, 0, 1, 4, 6, 8, 9,  /* j1button#? */
                                        3, 2, 0, 1, 4, 6, 8, 9}; /* j2button#? */
char cfg_value_str[24][32], /* P1/P2 defined keys (k1#? / k2#?) */
     *cfg_default_str[24] = {"CURSOR_LEFT", "CURSOR_RIGHT", "CURSOR_UP", "CURSOR_DOWN",
                             "d", "c", "s", "x",
                             "a", "z", "SPACE", "RETURN",
                             "h", "k", "u", "j",
                             "r", "t", "y", "i",
                             "o", "m", "PAGE_UP", "PAGE_DOWN"};


extern Object *Objects[LAST_NUM];

extern void updateSettings_rendertype(uint32 rend_type);


void SetGUIGadgets(void)
{
	BOOL value = FALSE, vsync_status = FALSE;
	int32 i;//, chooser_opt = 0;
	//char str[32] = "";
DBUG("SetGUIGadgets()\n",NULL);
	// Global Settings
	value = cfg_value[_EGLW_RENDER];
	IIntuition->SetAttrs(OBJ(OID_MODE_WF), CHOOSER_Selected,cfg_value[_FULLSCREEN], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_VIDMODE), GA_Disabled,value, CHOOSER_Selected,cfg_value[_EFFECT], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SKIP_F),  CHOOSER_Selected,cfg_value[_FRAMESKIP], TAG_DONE);

	// Settings:GRAPHICS gadgets
	IIntuition->SetAttrs(OBJ(OID_INTERLEAVE), GA_Selected,cfg_value[_NOINTERLEAVE], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_MEMMAP),     CHOOSER_Selected,cfg_value[_HIROM], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_PALNTSC),    CHOOSER_Selected,cfg_value[_NTSC], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_OC_SFX),     CHOOSER_Selected,cfg_value[_OCSFX], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_NOOVERSCAN), GA_Selected,cfg_value[_NOOVERSCAN], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_EMULATED),   GA_Selected,cfg_value[_EMULATED], TAG_DONE);
	vsync_status = cfg_value[_EMULATED];
	i = cfg_value[_SKIN] + 1;
	if(cfg_value[_USE_SKIN] == 0) { i = 0; } // 1st entry in chooser is 'NO'
//DBUG("cfg_value[_SKIN] = %ld [%ld]\n",cfg_value[_SKIN],cfg_value[_USE_SKIN]);
	IIntuition->SetAttrs(OBJ(OID_SKINS),       GA_Disabled,!cfg_value[_USE_SKIN], CHOOSER_Selected,i, TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_DISP_RENDER), GA_Selected,cfg_value[_DISP_RENDER], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_RENDERTYPE),  CHOOSER_Selected,cfg_value[_RENDERER], TAG_DONE); // "moved" to Global Settings
	IIntuition->SetAttrs(OBJ(OID_FILTER),      GA_Disabled,FALSE, CHOOSER_Selected,cfg_value[_FILTERHW], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SHADERS),     GA_Disabled,value, CHOOSER_Selected,cfg_value[_SHADERS], TAG_DONE);

	// Settings:AUDIO gadgets (and enable/disable)
	IIntuition->SetAttrs(OBJ(OID_AHI_PBR), CHOOSER_Selected,cfg_value[_INPUTRATE], TAG_DONE);
	if(cfg_value[_INPUTRATE] == AHI_PBR_MUTE) { value = TRUE; }
	//IIntuition->SetAttrs(OBJ(OID_AHI_BUF),     GA_Disabled,value, CHOOSER_Selected,cfg_value[_AHIBUFFER], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_AHI_BUF),     GA_Disabled,value, SLIDER_Level,cfg_value[_AHIBUFFER], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_AHI_SYNC),    GA_Disabled,value, GA_Selected,cfg_value[_SYNC], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_AHI_NOST),    GA_Disabled,value, GA_Selected,cfg_value[_NOSTEREO], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SND_IMETHOD), GA_Disabled,value, CHOOSER_Selected,cfg_value[_SND_IMETHOD], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SND_THREAD),  GA_Disabled,value, GA_Selected,cfg_value[_SND_THREAD], TAG_DONE);
	// Settings:ENGINE gadgets (and enable/disable)
	IIntuition->SetAttrs(OBJ(OID_TURBO_FSF),    CHOOSER_Selected,cfg_value[_TURBO_FSF], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_MSU1),         GA_Selected,cfg_value[_MSU1], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SPRITE_LIMIT), GA_Selected,cfg_value[_SPRITE_LIMIT], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_HACKVIDEO), GA_Selected,cfg_value[_HACKVIDEO], TAG_DONE);
	value = !cfg_value[_HACKVIDEO];
	IIntuition->SetAttrs(OBJ(OID_HV_WIDTH),  GA_Disabled,value, INTEGER_Number,cfg_value[_HV_WIDTH], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_HV_HEIGHT), GA_Disabled,value, INTEGER_Number,cfg_value[_HV_HEIGHT], TAG_DONE);

	updateSettings_rendertype(cfg_value[_RENDERER]); // depends on RENDERTYPE and VIDEOHACK

	// Settings:MISC gadgets
	IIntuition->SetAttrs(OBJ(OID_FPS),       GA_Selected,cfg_value[_FPS], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_DISP_TIME), GA_Selected,cfg_value[_DISP_TIME], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_QWERTY), GA_Selected,cfg_value[_QWERTY], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_OC_CPU),    CHOOSER_Selected,cfg_value[_OCCPU], TAG_DONE);
	// Settings:REWINDING gadgets
	IIntuition->SetAttrs(OBJ(OID_REWINDING),  GA_Selected,cfg_value[_REWINDING], TAG_DONE);
	value = !cfg_value[_REWINDING];
	vsync_status |= cfg_value[_REWINDING];
	IIntuition->SetAttrs(OBJ(OID_REW_BUF), GA_Disabled,value, INTEGER_Number,cfg_value[_REW_BUF], TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_REW_GRA), GA_Disabled,value, INTEGER_Number,cfg_value[_REW_GRA], TAG_DONE);
	// Settings:MISC gadgets (and enable/disable; depends on _EMULATED and _REWINDING)
	IIntuition->SetAttrs(OBJ(OID_VSYNC), GA_Disabled,vsync_status, GA_Selected,cfg_value[_VSYNC], TAG_DONE);

	// AmigaInput Settings
	IIntuition->SetAttrs(OBJ(OID_P1_KJ), CHOOSER_Selected,cfg_value[_PLAYER1JOY], TAG_DONE);
	for(i=0; i<8; i++) {
		//IIntuition->SetAttrs(OBJ(OID_P1_BTN_X+i), CHOOSER_Selected,cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
//		IIntuition->SetAttrs(OBJ(OID_P1_BTN_X+i), BUTTON_Integer,cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
IIntuition->SetAttrs(OBJ(OID_P1_BTN_X+i), BUTTON_VarArgs,&cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
	}
	IIntuition->SetAttrs(OBJ(OID_P2_KJ), CHOOSER_Selected,cfg_value[_PLAYER2JOY], TAG_DONE);
	for(i=0; i<8; i++) {
		//IIntuition->SetAttrs(OBJ(OID_P2_BTN_X+i), CHOOSER_Selected,cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
//		IIntuition->SetAttrs(OBJ(OID_P2_BTN_X+i), BUTTON_Integer,cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
IIntuition->SetAttrs(OBJ(OID_P2_BTN_X+i), BUTTON_VarArgs,&cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
	}
	value = cfg_value[_PLAYER2JOY] > 1? TRUE : FALSE;
//	IIntuition->SetAttrs(OBJ(OID_P2_K_COL0), GA_Disabled,value, TAG_DONE);
//	IIntuition->SetAttrs(OBJ(OID_P2_K_COL1), GA_Disabled,value, TAG_DONE);
//	IIntuition->SetAttrs(OBJ(OID_P2_K_COL2), GA_Disabled,value, TAG_DONE);
IIntuition->SetAttrs(OBJ(OID_P2_KJ_COLS), GA_Disabled,value, TAG_DONE);
	for(i=0; i<24; i++) {
DBUG("  %s '%s'\n",cfg_key[_P1_KEY_LF+i],cfg_value_str[i]);
		IIntuition->SetAttrs(OBJ(OID_P1_K_LF+i), STRINGA_TextVal,cfg_value_str[i], TAG_DONE);
	}

}

void GetGUIGadgets(void)
{
	int32 i;//, chooser_opt = 0;
	STRPTR res_str;
DBUG("GetGUIGadgets()\n",NULL);
	cfg_value[_TURBO_FS] = 0;

	// Global Settings
	IIntuition->GetAttrs(OBJ(OID_MODE_WF), CHOOSER_Selected,&cfg_value[_FULLSCREEN], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_VIDMODE), CHOOSER_Selected,&cfg_value[_EFFECT], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_SKIP_F),  CHOOSER_Selected,&cfg_value[_FRAMESKIP], TAG_DONE);

	// Settings:GRAPHICS gadgets
	IIntuition->GetAttrs(OBJ(OID_INTERLEAVE), GA_Selected,&cfg_value[_NOINTERLEAVE], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_MEMMAP),     CHOOSER_Selected,&cfg_value[_HIROM], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_PALNTSC),    CHOOSER_Selected,&cfg_value[_NTSC], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_OC_SFX),     CHOOSER_Selected,&cfg_value[_OCSFX], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_NOOVERSCAN), GA_Selected,&cfg_value[_NOOVERSCAN], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_EMULATED),   GA_Selected,&cfg_value[_EMULATED], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_SKINS),      CHOOSER_Selected,&cfg_value[_SKIN], TAG_DONE);
	cfg_value[_USE_SKIN] = 0;
	if(cfg_value[_SKIN] != 0) { cfg_value[_USE_SKIN] = 1; --cfg_value[_SKIN]; }
	IIntuition->GetAttrs(OBJ(OID_DISP_RENDER), GA_Selected,&cfg_value[_DISP_RENDER], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_RENDERTYPE),  CHOOSER_Selected,&cfg_value[_RENDERER], TAG_DONE);
DBUG("  RENDERER = %ld\n",cfg_value[_RENDERER]);
	i = cfg_value[_RENDERER];
	if(i != 0) {
		// "Workaround" #?_RENDER: chooser is 1:opengl 2:compo, but..
		if(cfg_value[_RENDERER] == 2) { i = 1; } //..in .cfg is before opengl
		if(cfg_value[_RENDERER] == 1) { i = 2; } //..in .cfg is after composite
		// And in .cfg _SNAPSHOT is "between" _SOFT_RENDER and _COMP_RENDER, so we skip it
		++i;
	}
	cfg_value[_SOFT_RENDER] = cfg_value[_COMP_RENDER] = cfg_value[_OPENGL_RENDER] = cfg_value[_EGLW_RENDER] = 0;
	cfg_value[_SOFT_RENDER + i]  = 1;
DBUG("  RENDER: _SOFT=%ld  _COMP=%ld  _OPENGL=%ld  _EGLW=%ld\n",cfg_value[_SOFT_RENDER],cfg_value[_COMP_RENDER],cfg_value[_OPENGL_RENDER],cfg_value[_EGLW_RENDER]);
	IIntuition->GetAttrs(OBJ(OID_FILTER),      CHOOSER_Selected,&cfg_value[_FILTERHW], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_SHADERS),     CHOOSER_Selected,&cfg_value[_SHADERS], TAG_DONE);
	// Settings:AUDIO gadgets
	IIntuition->GetAttrs(OBJ(OID_AHI_PBR),     CHOOSER_Selected,&cfg_value[_INPUTRATE], TAG_DONE);
	//IIntuition->GetAttrs(OBJ(OID_AHI_BUF),     CHOOSER_Selected,&cfg_value[_AHIBUFFER], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_AHI_BUF),     SLIDER_Level,&cfg_value[_AHIBUFFER], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_AHI_SYNC),    GA_Selected,&cfg_value[_SYNC], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_AHI_NOST),    GA_Selected,&cfg_value[_NOSTEREO], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_SND_IMETHOD), CHOOSER_Selected,&cfg_value[_SND_IMETHOD], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_SND_THREAD),  GA_Selected,&cfg_value[_SND_THREAD], TAG_DONE);
	// Settings:ENGINE gadgets
	IIntuition->GetAttrs(OBJ(OID_TURBO_FSF),  CHOOSER_Selected,&cfg_value[_TURBO_FSF], TAG_DONE);
//DBUG("OID_TURBO_FSF = %ld\n",chooser_opt);
	cfg_value[_TURBO_FS] = 0;
	if(cfg_value[_TURBO_FSF] != 0) { cfg_value[_TURBO_FS] = 1; }
	IIntuition->GetAttrs(OBJ(OID_MSU1),         GA_Selected,&cfg_value[_MSU1], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_SPRITE_LIMIT), GA_Selected,&cfg_value[_SPRITE_LIMIT], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_HACKVIDEO),    GA_Selected,&cfg_value[_HACKVIDEO], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_HV_WIDTH),     INTEGER_Number,&cfg_value[_HV_WIDTH], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_HV_HEIGHT),    INTEGER_Number,&cfg_value[_HV_HEIGHT], TAG_DONE);
	// Settings:MISC gadgets
	IIntuition->GetAttrs(OBJ(OID_FPS),       GA_Selected,&cfg_value[_FPS], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_DISP_TIME), GA_Selected,&cfg_value[_DISP_TIME], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_VSYNC),     GA_Selected,&cfg_value[_VSYNC], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_QWERTY),    GA_Selected,&cfg_value[_QWERTY], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_OC_CPU),    CHOOSER_Selected,&cfg_value[_OCCPU], TAG_DONE);
	// Settings:REWINDING gadgets
	IIntuition->GetAttrs(OBJ(OID_REWINDING),  GA_Selected,&cfg_value[_REWINDING], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_REW_BUF), INTEGER_Number,&cfg_value[_REW_BUF], TAG_DONE);
	IIntuition->GetAttrs(OBJ(OID_REW_GRA), INTEGER_Number,&cfg_value[_REW_GRA], TAG_DONE);

	// AmigaInput Settings
	IIntuition->GetAttrs(OBJ(OID_P1_KJ), CHOOSER_Selected,&cfg_value[_PLAYER1JOY], TAG_DONE);
#if defined(DEBUG)
	for(i=0; i<8; i++) {
		//IIntuition->GetAttrs(OBJ(OID_P1_BTN_X+i), CHOOSER_Selected,&cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
//		IIntuition->GetAttrs(OBJ(OID_P1_BTN_X+i), BUTTON_Integer,&cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
DBUG("  '%s' -> B%ld\n",cfg_key[_PLAYER1_BTN_X+i],cfg_value[_PLAYER1_BTN_X+i]);
	}
#endif
	IIntuition->GetAttrs(OBJ(OID_P2_KJ), CHOOSER_Selected,&cfg_value[_PLAYER2JOY], TAG_DONE);
#if defined(DEBUG)
	for(i=0; i<8; i++) {
		//IIntuition->GetAttrs(OBJ(OID_P2_BTN_X+i), CHOOSER_Selected,&cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
//		IIntuition->GetAttrs(OBJ(OID_P2_BTN_X+i), BUTTON_Integer,&cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
DBUG("  '%s' -> B%ld\n",cfg_key[_PLAYER2_BTN_X+i],cfg_value[_PLAYER2_BTN_X+i]);
	}
#endif
	for(i=0; i<24; i++) {
		IIntuition->GetAttrs(OBJ(OID_P1_K_LF+i), STRINGA_TextVal,&res_str, TAG_DONE);
		IUtility->Strlcpy( cfg_value_str[i], res_str, IUtility->Strlen(res_str)+1 );
DBUG("  %s '%s'\n",cfg_key[_P1_KEY_LF+i],cfg_value_str[i]);
	}

}

void DefaultSettings(void)
{
	int i, j;
	for(i=0; i!=_LAST_CFG_INT; i++) {
		cfg_value[i] = cfg_default_int[i];
//DBUG("%s=%ld\n",cfg_key[i],cfg_value[i]);
	}
	for(j=0; i<_LAST_CFG; i++,j++) { // P1/P2 defined keys
		IUtility->Strlcpy(cfg_value_str[j], cfg_default_str[j], 32);
//DBUG("%s=%s\n",cfg_key[i],cfg_value_str[j]);
	}
#if defined(DEBUG)
IExec->DebugPrintF("DEFAULT\n",NULL);
for(i=0; i<_LAST_CFG_INT; i++) { IExec->DebugPrintF("%s=%ld; ",cfg_key[i],cfg_value[i]); }
for(j=0; i<_LAST_CFG; i++,j++) { IExec->DebugPrintF("%s=%s; ",cfg_key[i],cfg_value_str[j]); }
IExec->DebugPrintF("\n");
#endif
}

void ReadConfig(void)
{
	int i, j = 0;
	BPTR fhConfFile = IDOS->FOpen(ConfFile, MODE_OLDFILE, 0);

	if(fhConfFile != ZERO) {
		struct FReadLineData *frld = IDOS->AllocDosObjectTags(DOS_FREADLINEDATA, 0);

		while(IDOS->FReadLine(fhConfFile, frld) > 0) {
			if(frld->frld_LineLength > 1) {
//DBUG("Line is %ld bytes: %s", frld->frld_LineLength, frld->frld_Line);
				if(frld->frld_Line[0] != '#') { // bypass comment '#' lines
					char kword[32] = "";
					int32 pos = IDOS->SplitName( frld->frld_Line, ' ', kword, 0, sizeof(kword) ); // get KEYWORD without VALUE ("frameskip 1")
//DBUG("'%s' [%ld]\n",kword,pos);
					for(i=0; i<_LAST_CFG; i++)
					{
						if(IUtility->Stricmp(kword, cfg_key[i]) == 0)
						{
							if(i < _LAST_CFG_INT) { IDOS->StrToLong(frld->frld_Line+pos, &cfg_value[i]); } // VALUE to integer
							else { // VALUE to char/string (P1/P2 keys)
								IUtility->Strlcpy( cfg_value_str[j], (frld->frld_Line+pos), IUtility->Strlen(frld->frld_Line+pos) );
								++j;
							}
							break;
						}
					}
				} // END if(frld->frld_Line[0]..

			} // END if(frld->frld_LineLength..
		} // END while(IDOS->FReadLine(f..

		IDOS->FreeDosObject(DOS_FREADLINEDATA, frld);
		IDOS->FClose(fhConfFile);
#if defined(DEBUG)
IExec->DebugPrintF("READ\n");
for(i=0; i<_LAST_CFG_INT; i++) { IExec->DebugPrintF("%s=%ld; ",cfg_key[i],cfg_value[i]); }
for(j=0; i<_LAST_CFG; i++,j++) { IExec->DebugPrintF("%s=%s; ",cfg_key[i],cfg_value_str[j]); }
IExec->DebugPrintF("\n");
#endif
	} // END if(fhConfFile..
	else { DefaultSettings(); }
}


BOOL SaveConfig(int val)
{
	BPTR fh = ZERO;
	BOOL bSaved = FALSE;
	int i;

	if(val == ALL) { GetGUIGadgets(); }
	else {
		for(i=0; i<8; i++) {
			//IIntuition->GetAttrs(OBJ(OID_P1_BTN_X+i), CHOOSER_Selected,&cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
			IIntuition->GetAttrs(OBJ(OID_P1_BTN_X+i), BUTTON_Integer,&cfg_value[_PLAYER1_BTN_X+i], TAG_DONE);
		}

		for(i=0; i<8; i++) {
			//IIntuition->GetAttrs(OBJ(OID_P2_BTN_X+i), CHOOSER_Selected,&cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
			IIntuition->GetAttrs(OBJ(OID_P2_BTN_X+i), BUTTON_Integer,&cfg_value[_PLAYER2_BTN_X+i], TAG_DONE);
		}
	}
#if defined(DEBUG)
{
int j;
IExec->DebugPrintF("SAVING...\n");
for(i=0; i<_LAST_CFG_INT; i++) { IExec->DebugPrintF("%s=%ld; ",cfg_key[i],cfg_value[i]); }
for(j=0; i<_LAST_CFG; i++,j++) { IExec->DebugPrintF("%s=%s; ",cfg_key[i],cfg_value_str[j]); }
IExec->DebugPrintF("\n");
}
#endif
	if( (fh=IDOS->FOpen(ConfFile,MODE_NEWFILE,0)) )
	{
		IDOS->FPrintf(fh, "# GUI SNES9X OPTIONS\n\n");

		IDOS->FPrintf(fh, "# enable auto frameskip\n%s %ld\n", cfg_key[_FRAMESKIP], cfg_value[_FRAMESKIP]);
		IDOS->FPrintf(fh, "# enable fullscreen\n%s %ld\n", cfg_key[_FULLSCREEN], cfg_value[_FULLSCREEN]);
		IDOS->FPrintf(fh, "# enable return to the last rom selected\n%s %ld\n", cfg_key[_RETURNROMNUMBER], cfg_value[_RETURNROMNUMBER]);
		IDOS->FPrintf(fh, "# enable joystick support\n%s %ld\n", cfg_key[_JOYSTICK], cfg_value[_JOYSTICK]);
		IDOS->FPrintf(fh, "# use the specified effect\n%s %ld\n", cfg_key[_EFFECT], cfg_value[_EFFECT]);
		IDOS->FPrintf(fh, "# disable interleave\n%s %ld\n", cfg_key[_NOINTERLEAVE], cfg_value[_NOINTERLEAVE]);
		IDOS->FPrintf(fh, "# emulated machine\n%s %ld\n", cfg_key[_EMULATED], cfg_value[_EMULATED]);
IDOS->FPrintf(fh, "# filterhardware\n%s %ld\n", cfg_key[_FILTERHW], cfg_value[_FILTERHW]);
IDOS->FPrintf(fh, "# shaders\n%s %ld\n", cfg_key[_SHADERS], cfg_value[_SHADERS]);
IDOS->FPrintf(fh, "# renderer\n%s %ld\n", cfg_key[_RENDERER], cfg_value[_RENDERER]);
IDOS->FPrintf(fh, "# software renderer\n%s %ld\n", cfg_key[_SOFT_RENDER], cfg_value[_SOFT_RENDER]);
IDOS->FPrintf(fh, "# snapshot\n%s %ld\n", cfg_key[_SNAPSHOT], cfg_value[_SNAPSHOT]);
IDOS->FPrintf(fh, "# compositing renderer\n%s %ld\n", cfg_key[_COMP_RENDER], cfg_value[_COMP_RENDER]);
IDOS->FPrintf(fh, "# opengl renderer\n%s %ld\n", cfg_key[_OPENGL_RENDER], cfg_value[_OPENGL_RENDER]);
IDOS->FPrintf(fh, "# egl_wrap renderer\n%s %ld\n", cfg_key[_EGLW_RENDER], cfg_value[_EGLW_RENDER]);
IDOS->FPrintf(fh, "# skinFX\n%s %ld\n", cfg_key[_USE_SKIN], cfg_value[_USE_SKIN]);
IDOS->FPrintf(fh, "# skinFX choice\n%s %ld\n", cfg_key[_SKIN], cfg_value[_SKIN]);
		IDOS->FPrintf(fh, "# force HiROM\n%s %ld\n", cfg_key[_HIROM], cfg_value[_HIROM]);
		IDOS->FPrintf(fh, "# enable hackvideo\n%s %ld\n", cfg_key[_HACKVIDEO], cfg_value[_HACKVIDEO]);
		IDOS->FPrintf(fh, "# height for hackvideo\n%s %ld\n", cfg_key[_HV_HEIGHT], cfg_value[_HV_HEIGHT]);
		IDOS->FPrintf(fh, "# width for hackvideo\n%s %ld\n", cfg_key[_HV_WIDTH], cfg_value[_HV_WIDTH]);
		IDOS->FPrintf(fh, "# overclocking SuperFX\n%s %ld\n", cfg_key[_OCSFX], cfg_value[_OCSFX]);
		IDOS->FPrintf(fh, "# overclock CPU\n%s %ld\n", cfg_key[_OCCPU], cfg_value[_OCCPU]);
		IDOS->FPrintf(fh, "# disable sprite limit\n%s %ld\n", cfg_key[_SPRITE_LIMIT], cfg_value[_SPRITE_LIMIT]);
		IDOS->FPrintf(fh, "# display time\n%s %ld\n", cfg_key[_DISP_TIME], cfg_value[_DISP_TIME]);
IDOS->FPrintf(fh, "# display render\n%s %ld\n", cfg_key[_DISP_RENDER], cfg_value[_DISP_RENDER]);
		IDOS->FPrintf(fh, "# force NTSC\n%s %ld\n", cfg_key[_NTSC], cfg_value[_NTSC]);
		IDOS->FPrintf(fh, "# region roms, universel=0, french=1, SuperFX=2 etc..\n%s %ld\n", cfg_key[_REGION], cfg_value[_REGION]);
		IDOS->FPrintf(fh, "# sound inputrate\n%s %ld\n", cfg_key[_INPUTRATE], cfg_value[_INPUTRATE]);
		IDOS->FPrintf(fh, "# sound buffer\n%s %ld\n", cfg_key[_AHIBUFFER], cfg_value[_AHIBUFFER]);
		IDOS->FPrintf(fh, "# sound sync\n%s %ld\n", cfg_key[_SYNC], cfg_value[_SYNC]);
		IDOS->FPrintf(fh, "# sound interpolation\n%s %ld\n", cfg_key[_SND_IMETHOD], cfg_value[_SND_IMETHOD]);
		IDOS->FPrintf(fh, "# sound thread\n%s %ld\n", cfg_key[_SND_THREAD], cfg_value[_SND_THREAD]);
		IDOS->FPrintf(fh, "# joy P1 no have start button\n%s %ld\n", cfg_key[_PLAYER1START], cfg_value[_PLAYER1START]);
		IDOS->FPrintf(fh, "# joy P2 no have start button\n%s %ld\n", cfg_key[_PLAYER2START], cfg_value[_PLAYER2START]);
		IDOS->FPrintf(fh, "# no sound on GUI\n%s %ld\n", cfg_key[_GUI_SOUND], cfg_value[_GUI_SOUND]);
		IDOS->FPrintf(fh, "# show fps\n%s %ld\n", cfg_key[_FPS], cfg_value[_FPS]);
IDOS->FPrintf(fh, "# activate qwerty\n%s %ld\n", cfg_key[_QWERTY], cfg_value[_QWERTY]);
		IDOS->FPrintf(fh, "# active vsync\n%s %ld\n", cfg_key[_VSYNC], cfg_value[_VSYNC]);
		IDOS->FPrintf(fh, "# activate turboframeskip\n%s %ld\n", cfg_key[_TURBO_FS], cfg_value[_TURBO_FS]);
		IDOS->FPrintf(fh, "# frames for turboframeskip\n%s %ld\n", cfg_key[_TURBO_FSF], cfg_value[_TURBO_FSF]);
		IDOS->FPrintf(fh, "# no overscan\n%s %ld\n", cfg_key[_NOOVERSCAN], cfg_value[_NOOVERSCAN]);
		IDOS->FPrintf(fh, "# enable msu1\n%s %ld\n", cfg_key[_MSU1], cfg_value[_MSU1]);
		IDOS->FPrintf(fh, "# enable rewinding\n%s %ld\n", cfg_key[_REWINDING], cfg_value[_REWINDING]);
		IDOS->FPrintf(fh, "# rewindbuffer\n%s %ld\n", cfg_key[_REW_BUF], cfg_value[_REW_BUF]);
		IDOS->FPrintf(fh, "# rewindgranularity\n%s %ld\n", cfg_key[_REW_GRA], cfg_value[_REW_GRA]);
		IDOS->FPrintf(fh, "# player1 joypad\n%s %ld\n", cfg_key[_PLAYER1JOY], cfg_value[_PLAYER1JOY]);
		IDOS->FPrintf(fh, "# player2 joypad\n%s %ld\n", cfg_key[_PLAYER2JOY], cfg_value[_PLAYER2JOY]);
		IDOS->FPrintf(fh, "# no stereo\n%s %ld\n", cfg_key[_NOSTEREO], cfg_value[_NOSTEREO]);
		for(i=0; i<8; i++) {
			IDOS->FPrintf(fh, "# %s\n%s %ld\n", cfg_key[_PLAYER1_BTN_X+i], cfg_key[_PLAYER1_BTN_X+i], cfg_value[_PLAYER1_BTN_X+i]);
		}
		for(i=0; i<8; i++) {
			IDOS->FPrintf(fh, "# %s\n%s %ld\n", cfg_key[_PLAYER2_BTN_X+i], cfg_key[_PLAYER2_BTN_X+i], cfg_value[_PLAYER2_BTN_X+i]);
		}
		for(i=0; i<24; i++) {
			IDOS->FPrintf(fh, "# %s\n%s %s\n", cfg_key[_P1_KEY_LF+i], cfg_key[_P1_KEY_LF+i], cfg_value_str[i]);
		}

		IDOS->FPrintf(fh, "\n");
		bSaved = (BOOL)IDOS->FClose(fh);
	}

	return bSaved;
}
