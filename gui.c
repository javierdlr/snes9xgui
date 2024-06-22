#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
extern struct LocaleInfo li;

#include "includes.h"
#include "debug.h"


/* Using parts of code:
 * E-UAE - The portable Amiga Emulator
 * AmigaInput joystick driver
 * Copyright 2005 Richard Drummond
 */
#define MAX_INPUT_DEVICES  6
#define MAX_JOYSTICKS  MAX_INPUT_DEVICES
#define MAX_AXES       2
#define MAX_BUTTONS    12

// A handy container to encapsulate the information we
// need when enumerating joysticks on the system.
struct enumPacket {
 APTR             context;
 uint32          *count;
 struct joystick *joyList;
};

// Per-joystick data private to driver
struct joystick {
 AIN_DeviceID     id;
 STRPTR           name;
 AIN_DeviceHandle *handle;
 APTR             context;
 uint32           axisCount;
 uint32           buttonCount;
// uint32           axisBufferOffset[MAX_AXES];
// int32            axisData[MAX_AXES];
 uint32           buttonBufferOffset[MAX_BUTTONS];
// int32            buttonData[MAX_BUTTONS];
};

static APTR joystickContext;
static uint32 joystickCount;
static struct joystick joystickList[MAX_JOYSTICKS];
static struct MsgPort *AI_Port = NULL;
static unsigned int joynum = 1;
static uint32 btn, btn_OID = OID_JPAD_NO_BTN, btn_pressed = 0;

BOOL enumerateJoysticks(AIN_Device *, void *UserData);
void close_joysticks(void);
unsigned int get_joystick_count(void);
STRPTR get_joystick_name(unsigned int);
//void read_joysticks(void);
int acquire_joy(unsigned int, int);
void unacquire_joy(unsigned int);
uint32 getGamepadButton(unsigned int);


void CreateGUIwindow(void);
BOOL ProcessGUI(struct Window **);
void free_chooser_list(struct List *);
BOOL make_chooser_list(BOOL, struct List *, char **);       // using (array) strings
BOOL make_chooser_list2(BOOL, struct List *, int32, int32); // using CATCOMP_NUMBERS and index
void append_tab(uint16, STRPTR);
uint32 DoMessage(char *, char, STRPTR);
struct Screen *FrontMostScr(void);
uint32 selectListEntry(struct Window *, uint32 res_val);
uint32 selectListEntryNode(struct Window *, struct Node *);
void GetSavestates(struct Window *, STRPTR fn);
void updateSettings_rendertype(uint32 rend_type);
void replace_SettingsLB_Object(Object *, struct Window *);


extern void LaunchShowRom(struct Window **, BOOL); // TRUE:launchROM; FALSE:showPNG
extern void ReadConfig(void);
extern void SetGUIGadgets(void);
extern void DefaultSettings(void);
extern BOOL SaveConfig(int); // ALL:all options; JOY_REMAP:only AI's B0..B9
extern void ObtainColors(void);
extern void ReleasePens(void);

extern struct AIN_IFace *IAIN;
extern struct IconIFace *IIcon;
extern struct DOSIFace *IDOS;
extern struct IntuitionIFace *IIntuition;
extern struct GraphicsIFace *IGraphics;
extern struct UtilityIFace *IUtility;

// the class pointer
extern Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *StringClass,
             *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
             *RequesterClass, *SpaceClass, *IntegerClass, *SliderClass;
// some interfaces needed
extern struct ListBrowserIFace *IListBrowser;
extern struct ClickTabIFace *IClickTab;
extern struct LayoutIFace *ILayout;
extern struct ChooserIFace *IChooser;

extern struct List listbrowser_list, *settings_list;
extern struct WBStartup *WBenchMsg;
extern struct DiskObject *iconify;
extern struct Screen *screen;
extern BOOL MaxWinSize, GUIFadeFx;
extern LONG PenArray[LAST_PEN];

extern int32 cfg_value[_LAST_CFG_INT];
extern char cfg_value_str[24][32]; /* P1/P2 defined keys */
extern int32 last_rom_run;

struct List labels;
Object *Objects[LAST_NUM];
//struct DrawInfo *drinfo = NULL;
uint32 res_prev; // avoid "reload" already selected ROM
struct List savestates_list;
struct Hook *ahibuf_lvlHook = NULL; // AHI buffer hook


void append_tab(uint16 num, STRPTR label)
{
	struct Node *node = IClickTab->AllocClickTabNode(TNA_Number,num, TNA_Text,label, TAG_DONE);
	if(node) {
		IExec->AddTail(&labels, node);
	}
}

BOOL make_chooser_list(BOOL mode, struct List *list, char **strings)
{
	struct Node *node;

	if(mode == NEW_LIST) { IExec->NewList(list); }

	while(*strings)
	{
		//if(**strings == '_')
		// node = IChooser->AllocChooserNode(CNA_Separator,TRUE, TAG_DONE);
		//else
			node = IChooser->AllocChooserNode(CNA_Text,*strings, TAG_DONE);

		if(node) { IExec->AddTail(list, node); }
		else { return FALSE; }

		strings++;
	}

	return TRUE;
}

BOOL make_chooser_list2(BOOL mode, struct List *list, int32 str_num, int32 index)
{
	struct Node *node;
	int32 j;

	if(mode == NEW_LIST) { IExec->NewList(list); }
	for(j=0; j<index; j++)
	{
		node = IChooser->AllocChooserNode(CNA_CopyText, TRUE,
		                                  CNA_Text, GetString(&li, str_num+j),
		                                 TAG_DONE);
		if(node) { IExec->AddTail(list, node); }
		else { return FALSE; }
	}

	return TRUE;
}

void free_chooser_list(struct List *list)
{
	struct Node *node;

	while( (node=IExec->RemTail(list)) ) {
		IChooser->FreeChooserNode(node);
	}
}

BOOL ProcessGUI(struct Window **pw)
{
	BOOL done = TRUE;
	uint16 code = 0;
	uint32 result = WMHI_LASTMSG, res_value = 0, res_totnode = 0, res_temp = 0,
	       siggot = 0, wsigmask = 0;
	STRPTR res_s = NULL;

	IIntuition->GetAttr(WINDOW_SigMask, OBJ(OID_MAIN), &wsigmask);

	siggot = IExec->Wait(wsigmask | (1L << AI_Port->mp_SigBit) | SIGBREAKF_CTRL_C);

	if(siggot & SIGBREAKF_CTRL_C) { return FALSE; }

	while( (result=IIntuition->IDoMethod(OBJ(OID_MAIN), WM_HANDLEINPUT, &code)) != WMHI_LASTMSG )
	{
//DBUG("result=0x%lx\n",result);
	switch(result & WMHI_CLASSMASK)
	{
		case WMHI_CLOSEWINDOW:
			done = FALSE;
		break;
		case WMHI_ICONIFY:
DBUG("WMHI_ICONIFY (win=0x%08lx)\n",*pw);
			if( IIntuition->IDoMethod(OBJ(OID_MAIN), WM_ICONIFY) ) {
				//IIntuition->UnlockPubScreen(NULL, screen);
				(*pw) = NULL;
			}
		break;
		case WMHI_UNICONIFY:
			if( ((*pw)=(struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL)) ) {
DBUG("WMHI_UNICONIFY (win=0x%08lx)\n",*pw);
				screen = (*pw)->WScreen;
				IIntuition->ScreenToFront(screen);
//IIntuition->GetAttrs(OBJ(OID_PAGE), PAGE_Current,&res_temp, TAG_DONE);
//DBUG("PAGE_Current = %ld\n",res_temp);
				//LaunchShowRom(pw, FALSE);
			}
			else { done = FALSE; }
		break;
		/*case WMHI_JUMPSCREEN:
			IIntuition->UnlockPubScreen(NULL, screen);
			IIntuition->IDoMethod(OBJ(OID_MAIN), WM_CLOSE);
			(*pw) = NULL;
ReleasePens();
			screen = FrontMostScr();
ObtainColors();
DBUG("[WMHI_JUMPSCREEN]pubscreen=0x%08lx '%s'\n",screen,screen?screen->Title:"Workbench");
			screen = IIntuition->LockPubScreen(screen->Title);
DBUG("[WMHI_JUMPSCREEN]pubscreen=0x%08lx '%s'\n",screen,screen?screen->Title:"Workbench");
			if( ((*pw)=(struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL)) )
			{
				//IIntuition->GetAttr(WA_PubScreen, OBJ(OID_MAIN), (uint32 *)&screen);
//				IIntuition->LockPubScreen((*pw)->WScreen->Title);
//				IIntuition->SetAttrs(OBJ(OID_ABOUT), LABEL_DrawInfo,drinfo, TAG_DONE);
				IIntuition->ScreenToFront((*pw)->WScreen);
			}
			else { done = FALSE; }
		break;*/
		case WMHI_VANILLAKEY:
//DBUG("[WMHI_VANILLAKEY] = 0x%lx (0x%lx)\n",code,result&WMHI_KEYMASK);
			if(code == 0x1b) // ESC
			{
				done = FALSE;
				break;
			}

			IIntuition->GetAttr(CLICKTAB_Current, OBJ(OID_PAGE), &res_value);
			if(res_value == 0) // GENERAL tab/page
			{
				struct Node *node1 = NULL, *next_node1 = NULL;
				STRPTR node_val, next_n_val;
				char char_node,
				     char_keyb = (char)(result & WMHI_KEYMASK & 0xDF); // uppercase'd (key pressed)
// NOTE: upercase using '& 0xDF' doesn't work for numbers
DBUG("[WMHI_VANILLAKEY] 0x%lx\n",char_keyb);
				if(char_keyb == 0x0d) // ENTER/RETURN
				{
					LaunchShowRom(pw, TRUE);
					break;
				}

				IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_SelectedNode,&node1, TAG_DONE);
				IListBrowser->GetListBrowserNodeAttrs(node1, LBNA_Column,COL_ROM, LBNCA_Text,&node_val, TAG_DONE);
				next_node1 = IExec->GetSucc(node1);
				IListBrowser->GetListBrowserNodeAttrs(next_node1, LBNA_Column,COL_ROM, LBNCA_Text,&next_n_val, TAG_DONE);
DBUG("  Actual Node -> Next Node\n",NULL);
DBUG("   0x%08lx -> 0x%08lx\n",node1, next_node1);
DBUG("          '%lc' -> '%lc'\n",(*(node_val)&0xDF), (*(next_n_val)&0xDF));
				// SELECT IT: NEXT node starts with KEY pressed and NEXT node = ACTUAL node
				if( char_keyb==(*(next_n_val)&0xDF)  &&  (*(next_n_val)&0xDF)==(*(node_val)&0xDF) )
				{
					res_prev = selectListEntryNode(*pw, next_node1);
					LaunchShowRom(pw, FALSE);
				}
				// GO TO KEY PRESSED FIRST NODE: 1)ACTUAL node starts with KEY pressed and NEXT node != ACTUAL node
				// OR 2)pressed another KEY OR 3)reached end of listbrowser (next_node1=NULL)
				if( (char_keyb==(*(node_val)&0xDF)  &&  (*(next_n_val)&0xDF)!=(*(node_val)&0xDF))
				   ||  char_keyb!=(*(node_val)&0xDF)  ||  next_node1==NULL )
				{
					next_node1 = node1; // avoid refreshing/reloading single ROM filename entries
					for(node1=IExec->GetHead(&listbrowser_list); node1!=NULL; node1=IExec->GetSucc(node1) ) {
						IListBrowser->GetListBrowserNodeAttrs(node1, LBNA_Column,COL_ROM, LBNCA_Text,&res_s, TAG_DONE);
						char_node = (*(res_s)&0xDF); // uppercase'd (romfile 1st letter)
						//char_node = IUtility->ToUpper(*res_s); // uppercased (romfile 1st letter)
						if(char_node==char_keyb  &&  node1!=next_node1) {
							res_prev = selectListEntryNode(*pw, node1);
							LaunchShowRom(pw, FALSE);
							node1 = NULL;
						}
					}
				}

			}
		break;
		case WMHI_RAWKEY:
		{
			int32 sel_entry = -1; // -1: key pressed not valid
DBUG("[WMHI_RAWKEY] 0x%lx (win=0x%08lx)\n",code,*pw);
			/*if(code == RAWKEY_ESC)
			{
				done = FALSE;
				break;
			}*/

			IIntuition->GetAttr(CLICKTAB_Current, OBJ(OID_PAGE), &res_value);

			if(res_value == 1) { // SETTINGS tab/page
DBUG("settings page\n");
				// Get "offset" (ID) of settings listbrowser
				IIntuition->GetAttrs(OBJ(OID_SETTINGS_LB), LISTBROWSER_Selected,&res_value,
				                     LISTBROWSER_TotalNodes,&res_totnode, TAG_DONE);

				// CURSOR UP key
				if(code==CURSORUP  &&  res_value!=0) { --res_value; }
				// CURSOR DOWN key
				if(code==CURSORDOWN  &&  res_value!=res_totnode-1) { ++res_value; }

				IIntuition->SetAttrs(OBJ(OID_SETTINGS_LB), LISTBROWSER_Selected,res_value, TAG_DONE);
				//ILayout->SetPageGadgetAttrs(GAD(OID_SETTINGS_LB), OBJ(OID_SETTINGS), *(pw), NULL,
				//                            LISTBROWSER_Selected,res_value, TAG_DONE);
				ILayout->RefreshPageGadget(GAD(OID_SETTINGS_LB), OBJ(OID_SETTINGS), *(pw), NULL);
				replace_SettingsLB_Object(OBJ(OID_GROUP_01+res_value), *pw);
				break;
			}

			if(res_value != 0) { break; } // NOT in GENERAL tab/page

			IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,&res_value,
			                     LISTBROWSER_TotalNodes,&res_totnode, TAG_DONE);
DBUG("  sel=%ld  nodes=%ld\n",res_value,res_totnode);
			// HOME key
			if(code==RAWKEY_HOME  &&  res_value!=0) {
				sel_entry = 0;
			}
			// END key
			if(code==RAWKEY_END  &&  res_value!=res_totnode-1) {
				sel_entry = res_totnode - 1;
			}
			// CURSOR UP key
			if(code==CURSORUP  &&  res_value!=0) {
						sel_entry = res_value - 1;
			}
			// PAGE UP key
			if(code==RAWKEY_PAGEUP  &&  res_value!=0) {
				IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Top,&res_value, LISTBROWSER_Bottom,&res_temp, TAG_DONE);
				sel_entry = res_temp - res_value;
				sel_entry = res_value - sel_entry;
//DBUG("  %ld\n",sel_entry);
				if(sel_entry < 0) { sel_entry = 0; }
			}
			// CURSOR DOWN key
			if(code==CURSORDOWN  &&  res_value!=res_totnode-1) {
				sel_entry = res_value + 1;
			}
			// PAGE DOWN key
			if(code==RAWKEY_PAGEDOWN  &&  res_value!=res_totnode-1) {
				IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Bottom,&res_value, TAG_DONE);
				sel_entry = res_value;
			}

			if(sel_entry != -1) {
				res_prev = selectListEntry(*pw, sel_entry);
				LaunchShowRom(pw, FALSE);
			}

/*
			// RETURN/ENTER key
			if(code==RAWKEY_RETURN  ||  code==RAWKEY_ENTER)
			{
				LaunchShowRom(pw, TRUE);
			}
*/
		}
		break;
		case WMHI_GADGETUP:
DBUG("[WMHI_GADGETUP] code = %ld (0x%08lx)\n",code,code);
			switch(result & WMHI_GADGETMASK)
			{
				case OID_LOAD:
					DefaultSettings(); // load default snes9x settings
					SetGUIGadgets();   // set in GUI snes9x settings
					// Refresh global settings gadgets
					IIntuition->RefreshGadgets(GAD(OID_SETTINGS_GLOBAL), (*pw), NULL);
					// Refresh general/settings/amigainput page gadgets
					ILayout->RefreshPageGadget(GAD(OID_SETTINGS_GROUP), OBJ(OID_SETTINGS), (*pw), NULL);
					ILayout->RefreshPageGadget(GAD(OID_P1_OPTIONS), OBJ(OID_AMIGAINPUT), (*pw), NULL);
					ILayout->RefreshPageGadget(GAD(OID_P2_OPTIONS), OBJ(OID_AMIGAINPUT), (*pw), NULL);
				break;
				case OID_SAVE:
					if(SaveConfig(ALL) == FALSE) {
						DoMessage((STRPTR)GetString(&li,MSG_ERROR_SAVING), REQIMAGE_ERROR, NULL); // "ERROR saving 'snes9x.cfg' file."
					}
				break;
				case OID_QUIT:
					done = FALSE;
				break;
				case OID_PREVIEW_BTN:
					IIntuition->GetAttrs( OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,&res_temp, TAG_DONE);
DBUG("OID_PREVIEW_BTN: %ld\n",res_temp);
					if(res_temp != -1) { LaunchShowRom(pw, TRUE); } // avoid launching if no ROMs in listbrowser
				break;
				case OID_LISTBROWSER:
					IIntuition->GetAttrs( OBJ(OID_LISTBROWSER), LISTBROWSER_RelEvent,&res_value, LISTBROWSER_Selected,&res_temp, TAG_DONE);
					if(res_value == LBRE_DOUBLECLICK) { LaunchShowRom(pw, TRUE); }
					else
					{// avoid "reload" already selected ROM (and launching if no ROMs in listbrowser)
DBUG("  Selected: [old]%ld == [new]%ld\n",res_prev,res_temp);
						if(res_temp!=-1  &&  res_prev!=res_temp) {
							res_prev = res_temp;
							LaunchShowRom(pw, FALSE);
						}
					}
				break;
				case OID_RENDERTYPE:
					updateSettings_rendertype(code);
					// Refresh Global Settings (video mode) gadget
					IIntuition->RefreshGadgets(GAD(OID_VIDMODE), (*pw), NULL);
					// Refresh settings page Graphics gadgets
					ILayout->RefreshPageGadget(GAD(OID_GFX_OPTIONS), OBJ(OID_SETTINGS), (*pw), NULL);
					// Refresh settings page Engine/VideoHack gadgets
					ILayout->RefreshPageGadget(GAD(OID_ENGINE_OPTIONS), OBJ(OID_SETTINGS), (*pw), NULL);
				break;
				case OID_SETTINGS_LB:
					replace_SettingsLB_Object(OBJ(OID_GROUP_01+code), *pw);
				break;
				case OID_AHI_PBR:
				{
					BOOL value = FALSE;

					//IIntuition->GetAttr(CHOOSER_Selected, OBJ(OID_AHI_PBR), &res_value);
					if(code == AHI_PBR_MUTE) { value = TRUE; } // mute selected
					// Refresh settings page Audio/AHI gadgets
					ILayout->SetPageGadgetAttrs(GAD(OID_AHI_BUF),     OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_AHI_SYNC),    OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_AHI_NOST),    OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_SND_IMETHOD), OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_SND_THREAD),  OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,value, TAG_DONE);
					ILayout->RefreshPageGadget(GAD(OID_AHI_OPTIONS), OBJ(OID_SETTINGS), (*pw), NULL);
				}
				break;
				case OID_HACKVIDEO:
					IIntuition->GetAttr(GA_Selected, OBJ(OID_HACKVIDEO), &res_value);
					// Refresh settings page Engine/VideoHack gadgets
					ILayout->SetPageGadgetAttrs(GAD(OID_HV_WIDTH),  OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,!res_value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_HV_HEIGHT), OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,!res_value, TAG_DONE);
					ILayout->RefreshPageGadget(GAD(OID_HACKVIDEO_GROUP), OBJ(OID_SETTINGS), (*pw), NULL);
				break;
				case OID_EMULATED:
					IIntuition->GetAttr(GA_Selected, OBJ(OID_EMULATED), &res_value);
					IIntuition->GetAttr(GA_Selected, OBJ(OID_REWINDING), &res_temp);
					if(res_temp) { break; } // OID_REWINDING still active(enable
					ILayout->SetPageGadgetAttrs(GAD(OID_VSYNC), OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
					ILayout->RefreshPageGadget(GAD(OID_MISC_OPTIONS), OBJ(OID_SETTINGS), (*pw), NULL);
				break;
				case OID_REWINDING:
					IIntuition->GetAttr(GA_Selected, OBJ(OID_REWINDING), &res_value);
					IIntuition->GetAttr(GA_Selected, OBJ(OID_EMULATED), &res_temp);
					// Refresh settings page Engine/VideoHack gadgets
					ILayout->SetPageGadgetAttrs(GAD(OID_REW_BUF), OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,!res_value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_REW_GRA), OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,!res_value, TAG_DONE);
					ILayout->RefreshPageGadget(GAD(OID_REWINDING_OPTIONS), OBJ(OID_SETTINGS), (*pw), NULL);
					if(res_temp) { break; } // OID_EMULATED still active(enable
					ILayout->SetPageGadgetAttrs(GAD(OID_VSYNC), OBJ(OID_SETTINGS), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
					ILayout->RefreshPageGadget(GAD(OID_MISC_OPTIONS), OBJ(OID_SETTINGS), (*pw), NULL);
				break;
				case OID_P2_KJ:
					res_value = code > 1? TRUE : FALSE;
//					ILayout->SetPageGadgetAttrs(GAD(OID_P2_K_COL0), OBJ(OID_AMIGAINPUT), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
//					ILayout->SetPageGadgetAttrs(GAD(OID_P2_K_COL1), OBJ(OID_AMIGAINPUT), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
//					ILayout->SetPageGadgetAttrs(GAD(OID_P2_K_COL2), OBJ(OID_AMIGAINPUT), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
//					ILayout->SetPageGadgetAttrs(GAD(OID_P2_KJ_BTN0), OBJ(OID_AMIGAINPUT), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
//					ILayout->SetPageGadgetAttrs(GAD(OID_P2_KJ_BTN1), OBJ(OID_AMIGAINPUT), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
					ILayout->SetPageGadgetAttrs(GAD(OID_P2_KJ_COLS), OBJ(OID_AMIGAINPUT), (*pw), NULL, GA_Disabled,res_value, TAG_DONE);
					ILayout->RefreshPageGadget(GAD(OID_P2_OPTIONS), OBJ(OID_AMIGAINPUT), (*pw), NULL);
				break;
				case OID_P1_BTN_A:
				case OID_P1_BTN_B:
				case OID_P1_BTN_X:
				case OID_P1_BTN_Y:
				case OID_P1_BTN_R:
				case OID_P1_BTN_L:
				case OID_P1_BTN_SEL:
				case OID_P1_BTN_STA:
				case OID_P2_BTN_A:
				case OID_P2_BTN_B:
				case OID_P2_BTN_X:
				case OID_P2_BTN_Y:
				case OID_P2_BTN_R:
				case OID_P2_BTN_L:
				case OID_P2_BTN_SEL:
				case OID_P2_BTN_STA:
					if(btn_OID!=(result & WMHI_GADGETMASK)  &&  btn_OID!=OID_JPAD_NO_BTN) {
						ILayout->SetPageGadgetAttrs(GAD(btn_OID), OBJ(OID_AMIGAINPUT), *(pw), NULL, GA_Selected,FALSE, TAG_END);
					}
					btn_OID = result & WMHI_GADGETMASK;
					IIntuition->GetAttr(GA_Selected, OBJ(btn_OID), &btn_pressed);
DBUG("  btn_OID = 0x%08lx (OID_P#_BTN_#?)\n",btn_OID);
					/*if(SaveConfig(JOY_REMAP) == FALSE) {
						DoMessage((STRPTR)GetString(&li,MSG_ERROR_SAVING), REQIMAGE_ERROR, NULL); // "ERROR saving 'snes9x.cfg' file."
					}*/
				break;
				case OID_DONATE_BTN:
				{
					BPTR handle = IDOS->Open("URL:http://hunoppc.amiga-projects.net/", MODE_OLDFILE);
//DBUG("OID_DONATE_BTN\n",NULL);
					if(handle) { IDOS->Close(handle); }
					else { DoMessage((STRPTR)GetString(&li,MSG_ERROR_URLOPEN), REQIMAGE_ERROR, NULL); }
				}
				break;
			}
		break;
		} // END switch
	} // END while( (result..

	//amigainput stuff
	btn = getGamepadButton(joynum);
	if(btn_pressed  &&  btn!=OID_JPAD_NO_BTN  &&  btn_OID!=OID_JPAD_NO_BTN) {
		uint32 idx;
		// "Obtain" index of button 'value' to change..
		if(btn_OID < OID_P2_BTN_X) { idx = (btn_OID - OID_P1_BTN_X) + _PLAYER1_BTN_X; }
		else { idx = (btn_OID - OID_P2_BTN_X) + _PLAYER2_BTN_X; }
DBUG("  P#_%ld/%ld = %ld",idx,res_temp,cfg_value[idx]);
		cfg_value[idx] = btn; //..and update it
DBUGN(" -> %ld\n",cfg_value[_PLAYER1_BTN_X+idx]);
//DBUG("  button remapped -> %ld\n",btn);
		ILayout->SetPageGadgetAttrs(GAD(btn_OID), OBJ(OID_AMIGAINPUT), *(pw), NULL,
		                            BUTTON_VarArgs,&cfg_value[idx], GA_Selected,FALSE, TAG_END);
//		                            BUTTON_Integer,btn, GA_Selected,FALSE, TAG_END);
		//ILayout->RefreshPageGadget(GAD(btn_OID), OBJ(OID_AMIGAINPUT), *(pw), NULL);
		/*if(SaveConfig(JOY_REMAP) == FALSE) {
			DoMessage((STRPTR)GetString(&li,MSG_ERROR_SAVING), REQIMAGE_ERROR, NULL); // "ERROR saving 'snes9x.cfg' file."
		}*/
		btn_OID = OID_JPAD_NO_BTN; // "reset" btn_OID
	}

	return done;
}

// AHI BUFFER slider level formatting
int32 ahibuf_lvlFunc(struct Hook *hook, APTR slider, struct TagItem *tags)
{
	int32 ahi_buf[] = {210, 194, 176, 160, 144, 128, 112, 96, 80, 64, 48, 32};
	//uint32 offset = 32, // AHI BUFFER starts from 32
	//       val = 11 - IUtility->GetTagData(SLIDER_Level, 0, tags);
DBUG("ahibuf_lvlFunc() 0x%08lx (0x%08lx)\n",slider,OBJ(OID_AHI_BUF));
	//if(val > 9) { offset = 32+2; } // last 2 values are (16 * val) + offset + 2

	//return(val * 16 + offset);
	return ahi_buf[IUtility->GetTagData(SLIDER_Level, 0, tags)];
}

void CreateGUIwindow(void)
{
	struct Node *node2 = NULL;
	struct MsgPort *gAppPort = NULL;
	struct ColumnInfo *columninfo;
	struct List memmap_list, snd_imet_list, skip_f_list, mode_wf_list, turbofs_list,
	            vid_mode_list, oc_cpu_list, render_list, filter_list, shader_list,
	            player1_kj_list, player2_kj_list;
	// Arrays strings on choosers, 'NULL' is replaced by translation (last NULL is like a "EOF")
	STRPTR oc_sfx[]  = {NULL, "+20%", "+40%", "+60%", "+80%", "+90%", "+100%", NULL},
	       palntsc[] = {NULL, "PAL", "NTSC (60Hz)", NULL},
	       skins[]   = {NULL, "0", "1", "2", "3", "4", "5", "6", "7", "8", NULL},
	       ahi_pbr[] = {"48 kHz", "44 kHz", "35 kHz", "32 kHz (SNES)", "31 kHz (AOS4)", "30 kHz", "22 kHz", "16 kHz", "11 kHz", "8 kHz", NULL, NULL},
	       //joy_btn[] = {"B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", NULL},
	       /* MSG_GUI_SG_VIDEOMODE + vid_mode to generate vid_mode_list */
	       vid_mode[] = {/*default*/ "TV", "Smooth", "SuperEagle", "2xSaI", "Super2xSaI", "EPX", "HQ2x", "2xBR", "2xBR-lv1", "DDT", "Scanlines 25%", "Dot Matrix", "320x240", "320x240Blend", NULL};
	char text_buf[256];
	int i;
	WORD max_w_type = IGraphics->TextLength(&screen->RastPort, "WWW", 3), // rom type pixel width
	     max_w_ext  = IGraphics->TextLength(&screen->RastPort, GetString(&li,MSG_GUI_TITLE_COL_FORM), IUtility->Strlen(GetString(&li,MSG_GUI_TITLE_COL_FORM))+1), // rom extension pixel width
	     max_w_lb   = IGraphics->TextLength(&screen->RastPort, GetString(&li,MSG_GUI_SETTINGS_GROUP_GFX), IUtility->Strlen(GetString(&li,MSG_GUI_SETTINGS_GROUP_GFX))+1); // listbrowser pixel width
	uint32 res_totnode;
	struct Window *pwindow = NULL;
DBUG("CreateGUIwindow()\n",NULL);

	gAppPort = IExec->AllocSysObjectTags(ASOT_PORT, TAG_END);
	AI_Port = IExec->AllocSysObjectTags(ASOT_PORT, TAG_END); // amigainput stuff

	// Get max listbrowser pixel width
//DBUG("  [0]max LB width: %ldpx\n",max_w_lb);
	for(i=1; i!=5; i++) {
		WORD tmp = IGraphics->TextLength(&screen->RastPort, GetString(&li,MSG_GUI_SETTINGS_GROUP_GFX+i), IUtility->Strlen(GetString(&li,MSG_GUI_SETTINGS_GROUP_GFX+i))+1);
		if(tmp > max_w_lb) { max_w_lb = tmp; }
//DBUG("  [%ld]max LB width: %ldpx\n",i,max_w_lb);
	}
	max_w_lb += IGraphics->TextLength(&screen->RastPort, "W", 1); // letter 'W' is the widest
//DBUG("  adding \"security width ('W')\" %ldpx\n",max_w_lb);

	IExec->NewList(&labels);
	append_tab( 0, (STRPTR)GetString(&li,MSG_GUI_PAGE_GENERAL)    ); // '_1 General'
	append_tab( 1, (STRPTR)GetString(&li,MSG_GUI_PAGE_SETTINGS)   ); // '_2 Settings'
	append_tab( 2, (STRPTR)GetString(&li,MSG_GUI_PAGE_AMIGAINPUT) ); // '_3 AmigaInput'
	append_tab( 3, (STRPTR)GetString(&li,MSG_GUI_PAGE_ABOUT)      ); // 'About'

	columninfo = IListBrowser->AllocLBColumnInfo(LAST_COL,
	                            LBCIA_Column,COL_TYPE, LBCIA_Title,GetString(&li, MSG_GUI_TITLE_COL_TYPE), //"ROM",
	                                                   LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                   LBCIA_Width,max_w_type,
	                            LBCIA_Column,COL_ROM, LBCIA_Title,GetString(&li, MSG_GUI_TITLE_COL_ROM), //" Game",
	                                                  LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                  LBCIA_Weight,100,
	                            LBCIA_Column,COL_FMT, LBCIA_Title,GetString(&li, MSG_GUI_TITLE_COL_FORM), //"Format",
	                                                  LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                  LBCIA_Width,max_w_ext,
	                           TAG_DONE);
	// chooserlist: SETTINGS page, GRAPHICS MEMORY_MAP
	make_chooser_list2(NEW_LIST, &memmap_list, MSG_GUI_SETTINGS_GFX_DEFAULT, 1);
	make_chooser_list2(ADD_LIST, &memmap_list, MSG_GUI_SETTINGS_GFX_MEMMAP_HIROM, 2);
	// chooser array: SETTINGS page, GRAPHICS OC_SFX
	oc_sfx[0] = (STRPTR)GetString(&li,MSG_GUI_SETTINGS_GFX_OVERSFX_NORMAL);
	// chooser array: SETTINGS page, GRAPHICS VIDEO
	palntsc[0] = (STRPTR)GetString(&li,MSG_GUI_SETTINGS_GFX_DEFAULT);
// chooser array: SETTINGS page, GRAPHICS SKINS
	skins[0] = (STRPTR)GetString(&li,MSG_GUI_SETTINGS_GFX_SKINS_NO);
// chooserlist: SETTINGS page, GRAPHICS RENDERTYPES
	make_chooser_list2(NEW_LIST, &render_list, MSG_GUI_SETTINGS_GFX_SOFTWARE, 4);
	//make_chooser_list2(NEW_LIST, &render_list, MSG_GUI_GENERAL_RENDER_SOFT, 4); // "moved" to Global Settings
// chooserlist: SETTINGS page, GRAPHICS FILTERS
	make_chooser_list2(NEW_LIST, &filter_list, MSG_GUI_SETTINGS_GFX_FILTERS_LINEAR, 2);
// chooserlist: SETTINGS page, GRAPHICS SHADERS
	make_chooser_list2(NEW_LIST, &shader_list, MSG_GUI_SETTINGS_GFX_SHADERS_FILTER1, 5);

	// chooser array: SETTINGS page, AUDIO AHI_PLAYBACK_RATE
//DBUG("ahi_pbr[%ld]\n",sizeof(ahi_pbr)/sizeof(STRPTR) - 2);
	ahi_pbr[sizeof(ahi_pbr)/sizeof(STRPTR) - 2] = (STRPTR)GetString(&li,MSG_GUI_SETTINGS_SND_MUTE);
	// chooserlist: SETTINGS page, AUDIO SND_IMETHOD
	make_chooser_list2(NEW_LIST, &snd_imet_list, MSG_GUI_SETTINGS_SND_INTERPOL_OFF, 5);

	// chooserlist: SETTINGS page, ENGINE TURBOFRAMESKIP
	make_chooser_list2(NEW_LIST, &turbofs_list, MSG_GUI_SETTINGS_ENG_TFRAMESKIP_OFF, 1);
	for(i=1; i<11; i++) // may change if you add/remove TURBOFRAMESKIPs
	{
		IUtility->SNPrintf(text_buf, sizeof(text_buf), (STRPTR)GetString(&li,MSG_GUI_SETTINGS_ENG_TFRAMESKIP_FRAMES),i);
		node2 = IChooser->AllocChooserNode(CNA_CopyText,TRUE, CNA_Text,text_buf, TAG_DONE);
		if(node2) { IExec->AddTail(&turbofs_list, node2); }
	}
	node2 = NULL;

	// chooserlist: SETTINGS page, MISC OVERCLOCK_CPU
	make_chooser_list2(NEW_LIST, &oc_cpu_list, MSG_GUI_SETTINGS_OVERCLOCK_OFF, 4);

	// chooserm array: GLOBAL, VIDEO_MODE
	make_chooser_list2(NEW_LIST, &vid_mode_list, MSG_GUI_SG_VIDEOMODE_DEFAULT, 1);
//DBUG("vid_mode[%ld]\n",sizeof(vid_mode)/sizeof(STRPTR) - 1);
	for(i=0; i<(sizeof(vid_mode)/sizeof(STRPTR) - 1); i++)
	{
		IUtility->SNPrintf(text_buf, sizeof(text_buf), (STRPTR)GetString(&li,MSG_GUI_SG_VIDEOMODE),vid_mode[i]);
		node2 = IChooser->AllocChooserNode(CNA_CopyText,TRUE, CNA_Text,text_buf, TAG_DONE);
		if(node2) { IExec->AddTail(&vid_mode_list, node2); }
	}
	node2 = NULL;
	// chooserlist: GLOBAL, FRAMESKIP
	make_chooser_list2(NEW_LIST, &skip_f_list, MSG_GUI_SG_FRAMESKIP_AUTO, 2);
	for(i=1; i<7; i++) // may change if you add/remove FRAMESKIPs
	{
		IUtility->SNPrintf(text_buf, sizeof(text_buf), (STRPTR)GetString(&li,MSG_GUI_SG_FRAMESKIP),i);
		node2 = IChooser->AllocChooserNode(CNA_CopyText,TRUE, CNA_Text,text_buf, TAG_DONE);
		if(node2) { IExec->AddTail(&skip_f_list, node2); }
	}
	node2 = NULL;
	// chooserlist: GLOBAL, WINDOW_FULLSCREEN
	make_chooser_list2(NEW_LIST, &mode_wf_list, MSG_GUI_SG_WINDOW, 2);
	// chooserlist: AMIGAINPUT page, KEYOARD_JOYPAD
	make_chooser_list2(NEW_LIST, &player1_kj_list, MSG_GUI_AI_KEYBOARD, 2);
	make_chooser_list2(NEW_LIST, &player2_kj_list, MSG_GUI_AI_KEYBOARD, 6);
	// chooserlist: GENERAL, LOAD_SAVESTATE
	make_chooser_list2(NEW_LIST, &savestates_list, MSG_GUI_GENERAL_SAVESTATES_NONE, 1);

	// Settings groups listbrowser
	for(i=0; i!=5; i++) {
		node2 = IListBrowser->AllocListBrowserNode(1,
		                                           LBNCA_CopyText, TRUE,
		                                           LBNCA_Text, GetString(&li,MSG_GUI_SETTINGS_GROUP_GFX+i),
		                                          TAG_DONE);
		if(node2) { IExec->AddTail(settings_list, node2); }
	}
	node2 = NULL;

	ahibuf_lvlHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry,ahibuf_lvlFunc, TAG_END);
//DBUG("ahibuf_lvlHook = 0x%08lx\n",ahibuf_lvlHook);

	//Settings: GRAPHICS
	OBJ(OID_GROUP_01) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
	               LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
	               LAYOUT_SpaceOuter,  TRUE,
	               //LAYOUT_SpaceInner,  FALSE,
	               #include "gui_settingsLB_graphics.h"
	TAG_DONE);
//DBUG("OID_GROUP_01 = 0x%08lx\n",OBJ(OID_GROUP_01));

	//Settings: AUDIO
	OBJ(OID_GROUP_02) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
	               LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
	               LAYOUT_SpaceOuter,  TRUE,
	               //LAYOUT_SpaceInner,  FALSE,
	               #include "gui_settingsLB_audio.h"
	TAG_DONE);
//DBUG("OID_GROUP_02 = 0x%08lx\n",OBJ(OID_GROUP_02));

	//Settings: ENGINE
	OBJ(OID_GROUP_03) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
	               LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
	               LAYOUT_SpaceOuter,  TRUE,
	               //LAYOUT_SpaceInner,  FALSE,
	               #include "gui_settingsLB_engine.h"
	TAG_DONE);
//DBUG("OID_GROUP_03 = 0x%08lx\n",OBJ(OID_GROUP_03));

	//Settings: MISC
	OBJ(OID_GROUP_04) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
	               LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
	               LAYOUT_SpaceOuter,  TRUE,
	               //LAYOUT_SpaceInner,  FALSE,
	               #include "gui_settingsLB_misc.h"
	TAG_DONE);
//DBUG("OID_GROUP_04 = 0x%08lx\n",OBJ(OID_GROUP_04));

	//Settings: REWINDING
	OBJ(OID_GROUP_05) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
	               LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
	               LAYOUT_SpaceOuter,  TRUE,
	               //LAYOUT_SpaceInner,  FALSE,
	               #include "gui_settingsLB_rewinding.h"
	TAG_DONE);
//DBUG("OID_GROUP_05 = 0x%08lx\n",OBJ(OID_GROUP_05));

	OBJ(OID_GROUP_ACTIVE) = OBJ(OID_GROUP_01); // default active settings group

	if(MaxWinSize) { // joypad image in AMIGAINPUT tab/page
		OBJ(OID_JOYPAD_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
		                       //BITMAP_SourceFile, MaxWinSize? DATAS"/joynintendo.png" : DATAS"/joynintendo_1.png",
		                       BITMAP_SourceFile, DATAS"/joynintendo.png",
		                       BITMAP_Screen,screen, BITMAP_Masking,TRUE,
		                      TAG_DONE);
	}
//DBUG("AI joypad image: 0x%08lx\n",OBJ(OID_JOYPAD_IMG));

//	ReadConfig(); // read 'snes9x.cfg' settings

DBUG("screentitle height=%ld\n",screen->BarHeight + 1);
DBUG("screenfont='%s'/%ld\n",screen->Font->ta_Name,screen->Font->ta_YSize);

	OBJ(OID_MAIN) = IIntuition->NewObject(WindowClass, NULL, //"window.class",
        WA_ScreenTitle, VERS" "DATE,
        WA_Title,       "Snes9xGUI",
        WA_PubScreen,         screen,
        WA_PubScreenFallBack, TRUE,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        WA_SizeGadget,  MaxWinSize? FALSE : TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate,    TRUE,
        WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_RAWKEY,
        MaxWinSize? WA_Width : TAG_IGNORE, screen->Width,
        MaxWinSize? WA_Height : TAG_IGNORE, (screen->Height - screen->BarHeight - 1),
        GUIFadeFx? WA_FadeTime : TAG_IGNORE, 500000, // duration of transition in microseconds
        WINDOW_IconifyGadget, TRUE,
        WINDOW_AppPort,       gAppPort,
        WINDOW_Icon,          iconify,
        WINDOW_Position, MaxWinSize? WPOS_TOPLEFT : WPOS_CENTERSCREEN,
        WINDOW_PopupGadget,    TRUE,
        //WINDOW_JumpScreensMenu, TRUE,
        WINDOW_UniqueID,       "snes9xGUI_contrib",
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,  TRUE,

// BANNER + GLOBAL OPTIONS
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           LAYOUT_HorizAlignment, LALIGN_CENTER,

           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             BUTTON_BevelStyle,  BVS_NONE,
             BUTTON_Transparent, TRUE,
             BUTTON_RenderImage, OBJ(OID_BANNER_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
              //IA_Scalable, TRUE,
              BITMAP_Screen,     screen,
              //BITMAP_Transparent, TRUE,
              BITMAP_Masking,    TRUE,
              BITMAP_SourceFile, DATAS"/snes9x-logo.png",
             TAG_DONE),
           TAG_DONE),
           CHILD_WeightedWidth,  0,

         TAG_DONE),
         CHILD_WeightedHeight, 0,

         LAYOUT_AddChild, OBJ(OID_SETTINGS_GLOBAL) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
           //LAYOUT_SpaceOuter,  TRUE,
           //LAYOUT_SpaceInner,  FALSE,
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
            GA_Text,     GetString(&li, MSG_GUI_SETTINGS_GLOBAL),//"Global Settings",
            GA_ReadOnly, TRUE,
            BUTTON_BevelStyle,  BVS_NONE,
            BUTTON_Transparent, TRUE,
            BUTTON_SoftStyle,   FSF_BOLD,
           TAG_DONE),
           LAYOUT_AddChild, OBJ(OID_MODE_WF) = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
            //GA_ID,         OID_MODE_WF,
            //GA_RelVerify,  TRUE,
            GA_Underscore, 0,
            CHOOSER_Labels,   &mode_wf_list,
            CHOOSER_Selected, 0,
           TAG_DONE),
           LAYOUT_AddChild, OBJ(OID_VIDMODE) = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
            //GA_ID,         OID_VIDMODE,
            //GA_RelVerify,  TRUE,
            GA_Underscore, 0,
            CHOOSER_Labels,    &vid_mode_list,
            CHOOSER_Selected,   0,
            CHOOSER_MaxLabels, 24,
           TAG_DONE),
           LAYOUT_AddChild, OBJ(OID_SKIP_F) = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
            //GA_ID,         OID_SKIP_F,
            //GA_RelVerify,  TRUE,
            GA_Underscore, 0,
            CHOOSER_Labels,   &skip_f_list,
            CHOOSER_Selected, 0,
           TAG_DONE),
           /*LAYOUT_AddChild, OBJ(OID_RENDERTYPE) = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
            GA_ID,         OID_RENDERTYPE,
            GA_RelVerify,  TRUE,
            GA_Underscore, 0,
            CHOOSER_Labels,   &render_list,
            CHOOSER_Selected, 0,
           TAG_DONE),*/
         TAG_DONE),
         CHILD_WeightedWidth,  0,
         CHILD_WeightedHeight, 0, // align banner/image at top

         LAYOUT_AddChild, OBJ(OID_PAGE) = IIntuition->NewObject(ClickTabClass, NULL, //"clicktab.gadget",
           GA_RelVerify, TRUE,
           CLICKTAB_Labels,    &labels,
           CLICKTAB_PageGroup, IIntuition->NewObject(NULL, "page.gadget",

// GENERAL page
#include "gui_general.h"

// SETTINGS page
            PAGE_Add, OBJ(OID_SETTINGS) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
             //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
             LAYOUT_SpaceOuter, TRUE,
             //LAYOUT_SpaceInner,  FALSE,
             LAYOUT_AddChild, OBJ(OID_SETTINGS_LB) = IIntuition->NewObject(ListBrowserClass, NULL, //"listbrowser.gadget",
              GA_ID,        OID_SETTINGS_LB,
              GA_RelVerify, TRUE,
              LISTBROWSER_Labels,       settings_list,
              LISTBROWSER_ShowSelected, TRUE,
              LISTBROWSER_Selected,     0,
              LISTBROWSER_VerticalProp, FALSE,
             TAG_DONE),
             //CHILD_WeightedWidth, 20,
             CHILD_MinWidth, max_w_lb,
             CHILD_MaxWidth, max_w_lb,
             LAYOUT_AddChild, OBJ(OID_SETTINGS_GROUP) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
               LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
               LAYOUT_SpaceOuter,  TRUE,
               //LAYOUT_SpaceInner,  FALSE,
               LAYOUT_AddChild, OBJ(OID_GROUP_ACTIVE),
               CHILD_NoDispose,      TRUE, // or CHILD_ReplaceObject will dispose while navigating through options
               CHILD_WeightedHeight, 0,
             TAG_DONE),
             //CHILD_WeightedWidth, 80,
           TAG_DONE), // END of SETTINGS page/tab

// AMIGAINPUT page
#include "gui_amigainput.h"

// ABOUT page
#include "gui_about.h"

           TAG_DONE), // END of page.gadget
         TAG_DONE), // END of clicktab.gadget group

// BUTTONS
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
           LAYOUT_SpaceOuter,  TRUE,
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
            GA_ID,        OID_SAVE,
            GA_RelVerify, TRUE,
            GA_Text,      GetString(&li, MSG_GUI_SAVE_BTN),//"Save settings",
           TAG_DONE),
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
            GA_ID,        OID_LOAD,
            GA_RelVerify, TRUE,
            GA_Text,      GetString(&li, MSG_GUI_DEFAULT_BTN),//"Default settings",
           TAG_DONE),
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
            GA_ID,        OID_QUIT,
            GA_RelVerify, TRUE,
            GA_Text,      GetString(&li, MSG_GUI_QUIT_BTN),//"Quit",
           TAG_DONE),
         TAG_DONE), // END of BUTTONS
         CHILD_WeightedHeight, 0,

        TAG_DONE), // END of window layout group
	TAG_END);

	ReadConfig(); // read 'snes9x.cfg' settings
	SetGUIGadgets(); // set/refresh in GUI 'snes9x.cfg' settings

	// Select last launched ROM
	if(last_rom_run < 0) { last_rom_run = 0; }
	IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_TotalNodes,&res_totnode, TAG_DONE);
	if(last_rom_run >= res_totnode) { last_rom_run = res_totnode - 1; }
DBUG("last_rom_run=%ld (%ld)\n",last_rom_run,res_totnode);
	IIntuition->SetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,last_rom_run,
	                     LISTBROWSER_MakeVisible,last_rom_run, TAG_DONE);
	res_prev = last_rom_run;

	IIntuition->SetAttrs(OBJ(OID_TOTALROMS), BUTTON_VarArgs,&res_totnode, TAG_DONE);

	// "Paint" nodes using 'PenArray[0]'/'PenArray[1]' alternatively
	BOOL penrow = 0;
	for( node2=IExec->GetHead(&listbrowser_list); node2!=NULL; node2=IExec->GetSucc(node2) ) {
		IListBrowser->SetListBrowserNodeAttrs(node2,
		                                      LBNA_Column,COL_TYPE, LBNCA_BGPen,PenArray[penrow],
		                                      LBNA_Column,COL_ROM, LBNCA_BGPen,PenArray[penrow],
		                                      LBNA_Column,COL_FMT, LBNCA_BGPen,PenArray[penrow], TAG_DONE);
		penrow = !penrow;
	}

	// PayPal donation button image is missing
	if(OBJ(OID_DONATE_IMG) == NULL) { IIntuition->SetAttrs(OBJ(OID_DONATE_BTN), GA_Text,"DONATE PayPal 'HunoPPC'", TAG_DONE); }

	// amigainput stuff
	AIN_InputEvent *event = NULL;
	struct joystick *joy = &joystickList[joynum-1];
	struct TagItem tags[] = { {AINCC_Port,(ULONG)AI_Port}, {TAG_DONE,TAG_DONE} };
	joystickContext = IAIN->AIN_CreateContext(1, tags);
	if(joystickContext) {
		struct enumPacket packet = {joystickContext, &joystickCount, &joystickList[0]};

		IAIN->AIN_EnumDevices(joystickContext, enumerateJoysticks, &packet);
		if( get_joystick_count() ) { acquire_joy(joystickCount, 0); }
	}
DBUG("JOYPAD: context=0x%08lx  handle=0x%08lx\n",joystickContext,joy->handle);

	if( (pwindow=(struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL)) )
	{
		IIntuition->ScreenToFront(pwindow->WScreen);

		// Show preview image
		LaunchShowRom(&pwindow, FALSE);

		// amigainput stuff
		if(joy->handle != NULL) {
			struct TagItem tags[] = { {AINCC_Window,(ULONG)pwindow}, {TAG_DONE,TAG_DONE} };
			IAIN->AIN_SetDeviceParameter(joy->context, joy->handle, AINDP_EVENT, TRUE);
			IAIN->AIN_Set(joy->context, tags);
		}

		while(ProcessGUI(&pwindow) == TRUE);

		// amigainput stuff
		if(joy->handle != NULL) { 
			IAIN->AIN_SetDeviceParameter(joy->context, joy->handle, AINDP_EVENT, FALSE);
			// Remove pending AI messages
			while( (event=IAIN->AIN_GetEvent(joy->context)) ) { IAIN->AIN_FreeEvent(joy->context, event); }
		}
	} // END if( ((*pw)dow..

	// amigainput stuff
	if(joystickContext) {
		unacquire_joy(joystickCount);
		close_joysticks();
	}

	IExec->FreeSysObject(ASOT_PORT, AI_Port); // amigainput stuff
	IExec->FreeSysObject(ASOT_PORT, gAppPort);

	IIntuition->DisposeObject( OBJ(OID_MAIN) );
	OBJ(OID_MAIN) = NULL;
	IIntuition->DisposeObject( OBJ(OID_BANNER_IMG) );
	IIntuition->DisposeObject( OBJ(OID_ABOUT1_IMG) );
	IIntuition->DisposeObject( OBJ(OID_ABOUT2_IMG) );
	IIntuition->DisposeObject( OBJ(OID_DONATE_IMG) );
	IIntuition->DisposeObject( OBJ(OID_PREVIEW_IMG) );
	//IIntuition->DisposeObject( OBJ(OID_BOXCOVER_IMG) );
	for(i=0; i!=4; i++) {
		IIntuition->DisposeObject( OBJ(OID_GROUP_01+i) );
		OBJ(OID_GROUP_01+i) = NULL;
	}

//ReleasePens();

	IExec->FreeSysObject(ASOT_HOOK, ahibuf_lvlHook);

	//IIntuition->FreeScreenDrawInfo(screen, drinfo);
	IListBrowser->FreeLBColumnInfo(columninfo);
	IClickTab->FreeClickTabList(&labels);

	free_chooser_list(&memmap_list);
	free_chooser_list(&render_list);
	free_chooser_list(&filter_list);
	free_chooser_list(&shader_list);
	free_chooser_list(&snd_imet_list);
	free_chooser_list(&turbofs_list);
	free_chooser_list(&oc_cpu_list);
	free_chooser_list(&vid_mode_list);
	free_chooser_list(&skip_f_list);
	free_chooser_list(&mode_wf_list);
	free_chooser_list(&player1_kj_list);
	free_chooser_list(&player2_kj_list);

	free_chooser_list(&savestates_list);
}

/* Show Errors/Warnings/... */
uint32 DoMessage(char *message, char reqtype, STRPTR buttons)
{
	uint32 button;
	Object *requester;

	if(IIntuition == NULL) { IDOS->PutErrStr(message); return 0; }

	requester = IIntuition->NewObject(RequesterClass, NULL, //"requester.class",
	                                 REQ_Image,      reqtype,
	                                 REQ_TitleText,  "snes9xGUI",
	                                 REQ_BodyText,   message,
	                                 REQ_GadgetText, buttons? buttons : GetString(&li,MSG_OK_GAD),
	                                 //REQ_StayOnTop, TRUE,
	                                TAG_DONE);

	button = IIntuition->IDoMethod( requester, RM_OPENREQ, NULL, NULL, FrontMostScr() );
	IIntuition->DisposeObject(requester);

	return button;
}

/* Get screen at front, used by DoMessage()/ObtainColors()/ReleasePens() */
struct Screen *FrontMostScr(void)
{
	ULONG intuition_lock;
	struct Screen *front_screen_address, *public_screen_address = NULL;
	struct List *public_screen_list;
	struct PubScreenNode *public_screen_node;

	intuition_lock = IIntuition->LockIBase(0L);

	front_screen_address = ((struct IntuitionBase *)IntuitionBase)->FirstScreen;
	if( (front_screen_address->Flags & PUBLICSCREEN) || (front_screen_address->Flags & WBENCHSCREEN) ) {
		IIntuition->UnlockIBase(intuition_lock);

		public_screen_list = IIntuition->LockPubScreenList();
		public_screen_node = (struct PubScreenNode *)public_screen_list->lh_Head;
		while(public_screen_node) {
			if(public_screen_node->psn_Screen == front_screen_address) {
				public_screen_address = public_screen_node->psn_Screen;
				break;
			}

			public_screen_node = (struct PubScreenNode *)public_screen_node->psn_Node.ln_Succ;
		}

		IIntuition->UnlockPubScreenList();
	}
	else {
		IIntuition->UnlockIBase(intuition_lock);
	}

	if(!public_screen_address) {
		public_screen_address = IIntuition->LockPubScreen(NULL);
		IIntuition->UnlockPubScreen(NULL, public_screen_address);
	}

//DBUG("%lx\n", (int)public_screen_address);
	return public_screen_address;
}

uint32 selectListEntry(struct Window *pw, uint32 res_val)
{
	ILayout->SetPageGadgetAttrs(GAD(OID_LISTBROWSER), OBJ(OID_GENERAL), pw, NULL,
	                            LISTBROWSER_Selected,res_val,
	                            LISTBROWSER_MakeVisible,res_val, TAG_DONE);
	ILayout->RefreshPageGadget(GAD(OID_LISTBROWSER), OBJ(OID_GENERAL), pw, NULL);

	return res_val;
}

uint32 selectListEntryNode(struct Window *pw, struct Node *n)
{
	uint32 res_val;
	ILayout->SetPageGadgetAttrs(GAD(OID_LISTBROWSER), OBJ(OID_GENERAL), pw, NULL,
	                           LISTBROWSER_SelectedNode,n,
	                           LISTBROWSER_MakeNodeVisible,n, TAG_DONE);
	ILayout->RefreshPageGadget(GAD(OID_LISTBROWSER), OBJ(OID_GENERAL), pw, NULL);
	IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,&res_val, TAG_DONE);

	return res_val;
}

static int32 compare_name_hookfunc(struct Hook *hk, struct Node *n1, struct Node *n2)
{
	//struct UtilityIFace *iutil = hk->h_Data;
	STRPTR n1_str, n2_str;

	IChooser->GetChooserNodeAttrs(n1, CNA_Text,&n1_str, TAG_DONE);
	IChooser->GetChooserNodeAttrs(n2, CNA_Text,&n2_str, TAG_DONE);
//DBUG("n1[0x%08lx]: '%s'\n",n1,n1_str);
//DBUG("n2[0x%08lx]: '%s'\n",n2,n2_str);
	return IUtility->Stricmp(n1_str, n2_str);
}

void GetSavestates(struct Window *pw, STRPTR fn)
{
	APTR context = NULL;
	struct Node *node;
	STRPTR filestate = IExec->AllocVecTags(MAX_DOS_FILENAME, TAG_DONE),
	       pattern_ms = IExec->AllocVecTags(2+MAX_DOS_FILENAME*2, TAG_DONE);
	int32 i = 0, j = 0;
DBUG("GetSavestates()\n",NULL);
	// "Neutralize" '( ) [ ]' for ParsePatternNoCase()
	for(; i!=IUtility->Strlen(fn); ++i,++j) {
		if(fn[i]=='('  ||  fn[i]==')' ||
		   fn[i]=='['  ||  fn[i]==']') { filestate[j++] = '\''; }
		filestate[j] = fn[i];
	}
	filestate[j] = '\0';
	//IUtility->Strlcpy(filestate, fn, MAX_DOS_FILENAME);
	IUtility->Strlcat(filestate, "00[0-9].frz", MAX_DOS_FILENAME);
DBUG("  searching '%s':\n",filestate);

	// Detach chooser list
	IIntuition->SetAttrs(OBJ(OID_SAVESTATES), CHOOSER_Labels,0, TAG_DONE);
	// Remove previous rom savestates list
	free_chooser_list(&savestates_list);

	// Search selected/active rom savestates
	IDOS->ParsePatternNoCase(filestate, pattern_ms, 2+MAX_DOS_FILENAME*2);
	context = IDOS->ObtainDirContextTags(EX_StringNameInput, SAVES,
	                                     EX_DataFields, (EXF_NAME|EXF_TYPE),
	                                     EX_MatchString, pattern_ms,
	                                    TAG_END);
	if(context) {
		struct ExamineData *dat;
		struct Hook HK = { .h_Entry = (HOOKFUNC)compare_name_hookfunc/*,
		                   .h_Data  = (APTR)IUtility*/
		                 };

		while( (dat=IDOS->ExamineDir(context)) )
		{
			if( EXD_IS_FILE(dat) )
			{
DBUG("    '%s'\n",dat->Name);
				node = IChooser->AllocChooserNode(CNA_CopyText,TRUE, CNA_Text,dat->Name, TAG_DONE);
				if(node) { IExec->AddTail(&savestates_list, node); }
			}
		}
		//if(IDOS->IoErr() == ERROR_NO_MORE_ENTRIES) {}
DBUG("  SortList() savestates\n",NULL);
		IUtility->SortList(&savestates_list, &HK);
	}

	// Add 'NO' string (MSG_GUI_GENERAL_SAVESTATES_NONE) at top/head of the list..
	node = IChooser->AllocChooserNode(CNA_Text,GetString(&li,MSG_GUI_GENERAL_SAVESTATES_NONE), TAG_DONE);
	IExec->AddHead(&savestates_list, node);
	//..and re-attach chooser list
	IIntuition->SetAttrs(OBJ(OID_SAVESTATES), CHOOSER_Labels,&savestates_list, TAG_DONE);
	ILayout->RefreshPageGadget(GAD(OID_SAVESTATES), OBJ(OID_GENERAL), pw, NULL);

	IDOS->ReleaseDirContext(context);
	IExec->FreeVec(pattern_ms);
	IExec->FreeVec(filestate);
}

void updateSettings_rendertype(uint32 rend_type)
{
	uint32 res_value = 0;
	BOOL val_gfx = FALSE, val_gfxSKN = TRUE, val_gfxFLT = FALSE, val_gfxSHD = TRUE,
	     val_eng = FALSE, val_vmode = FALSE;
DBUG("updateSettings_rendertype() %ld\n",rend_type);
	switch(rend_type) {
		case 0: // software
			val_gfxSKN = FALSE;
			val_gfxFLT = TRUE;
			val_gfx = TRUE;
			val_eng = TRUE;
		break;
		case 1: // minigl/opengl
			// uses "default" val_#? values
		break;
		case 2: // compositing
			val_gfx = TRUE;
		break;
		case 3: // egl_wrap
			val_vmode = TRUE;
			val_gfxSHD = FALSE;
			val_eng = TRUE;
		break;
	}
	// Set Global Settings (video mode) gadget
	IIntuition->SetAttrs(OBJ(OID_VIDMODE), GA_Disabled,val_vmode, TAG_DONE);
	// Set settings page Graphics gadgets
	IIntuition->SetAttrs(OBJ(OID_NOOVERSCAN), GA_Disabled,val_gfx, TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_EMULATED),   GA_Disabled,!val_gfx, TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SKINS),      GA_Disabled,val_gfxSKN, TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_FILTER),     GA_Disabled,val_gfxFLT, TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_SHADERS),    GA_Disabled,val_gfxSHD, TAG_DONE);
	// Set settings page Engine/VideoHack gadgets
	IIntuition->SetAttrs(OBJ(OID_HACKVIDEO_GROUP), GA_Disabled,val_eng, TAG_DONE);
	if(!val_eng) { IIntuition->GetAttr(GA_Selected, OBJ(OID_HACKVIDEO), &res_value); }
//DBUG("  OID_HACKVIDEO 0x%08lx\n",res_value);
	IIntuition->SetAttrs(OBJ(OID_HV_WIDTH),  GA_Disabled,!res_value, TAG_DONE);
	IIntuition->SetAttrs(OBJ(OID_HV_HEIGHT), GA_Disabled,!res_value, TAG_DONE);
}

void replace_SettingsLB_Object(Object *obj, struct Window *pw)
{
	struct TagItem attrs[2];
DBUG("replace_SettingsLB_Object()\n",NULL);
	if(OBJ(OID_GROUP_ACTIVE) == obj) { return; }

	attrs[0].ti_Tag  = CHILD_ReplaceObject;
	attrs[0].ti_Data = (uint32)obj;
	attrs[1].ti_Tag  = TAG_DONE;
	attrs[1].ti_Data = 0;

	if( IIntuition->IDoMethod(OBJ(OID_SETTINGS_GROUP), LM_MODIFYCHILD, pw,
	                          OBJ(OID_GROUP_ACTIVE), &attrs) ) {
DBUG("  LM_MODIFYCHILD done (0x%08lx -> 0x%08lx)\n",OBJ(OID_GROUP_ACTIVE),obj);
		OBJ(OID_GROUP_ACTIVE) = obj;
		IIntuition->IDoMethod(OBJ(OID_MAIN), WM_RETHINK);
	}
}


uint32 getGamepadButton(unsigned int joynum)
{
	uint32 ret = OID_JPAD_NO_BTN;
	struct joystick *joy = &joystickList[joynum-1];

	if(joy->handle != NULL) {
		AIN_InputEvent *event = NULL;

		while( (event=IAIN->AIN_GetEvent(joy->context)) ) {
//DBUG("AIN_GetEvent() 0x%08lx\n",event->Type);
			/*switch(event->Type) {
				case AINET_BUTTON:
					ret = event->Index - joy->buttonBufferOffset[0];
DBUG("Button #%2ld [%ld]\n",ret, event->Value);
				break;
				default: break;
			}*/
			if(event->Type == AINET_BUTTON) {
				ret = event->Index - joy->buttonBufferOffset[0];
DBUG("Button #%2ld [%ld]\n",ret, event->Value);
			}
			IAIN->AIN_FreeEvent(joy->context, event);
		}
	}

	return ret;
}

/* Using parts of code:
 * E-UAE - The portable Amiga Emulator
 * AmigaInput joystick driver
 * Copyright 2005 Richard Drummond
 */
// Callback to enumerate joysticks
BOOL enumerateJoysticks(AIN_Device *device, void *UserData)
{
	APTR context = ((struct enumPacket *)UserData)->context;
	uint32 *count = ((struct enumPacket *)UserData)->count;
	struct joystick *joy = &((struct enumPacket *)UserData)->joyList[*count];

	BOOL result = FALSE;

	if(*count < MAX_JOYSTICKS) {
		if(device->Type == AINDT_JOYSTICK) {
			unsigned int i;

			joy->context     = context;
			joy->id          = device->DeviceID;
			joy->name        = (STRPTR)device->DeviceName;
			joy->axisCount   = device->NumAxes;
			joy->buttonCount = device->NumButtons;

			if(joy->axisCount > MAX_AXES) joy->axisCount = MAX_AXES;

			if(joy->buttonCount > MAX_BUTTONS) joy->buttonCount = MAX_BUTTONS;

			// Query offsets in ReadDevice buffer for axes' data
//			for(i=0; i<joy->axisCount; i++)
//				result = IAIN->AIN_Query(joy->context, joy->id, AINQ_AXIS_OFFSET, i, &(joy->axisBufferOffset[i]), 4);

			// Query offsets in ReadDevice buffer for buttons' data
			for(i=0; i<joy->buttonCount; i++) {
				result = /*result &&*/ IAIN->AIN_Query(joy->context, joy->id, AINQ_BUTTON_OFFSET, i, &(joy->buttonBufferOffset[i]), 4);
DBUG("AINQ_BUTTON_OFFSET #%ld = %ld\n",i,joy->buttonBufferOffset[i]);
			}

			if(result  &&  joy->id==4096) {
DBUG("Joystick #%ld (AI ID=%ld) '%s' with %ld axes, %ld buttons\n",*count, joy->id, joy->name, joy->axisCount, joy->buttonCount);
				(*count)++;
			}

		}
	}

	return result;
}

void close_joysticks(void)
{
	unsigned int i = joystickCount;

	while(i-- > 0) {
		struct joystick *joy = &joystickList[i];

		if(joy->handle) {
			IAIN->AIN_ReleaseDevice(joy->context, joy->handle);
			joy->handle = 0;
		}
	}
	joystickCount = 0;

	if(joystickContext) {
		IAIN->AIN_DeleteContext(joystickContext);
		joystickContext = NULL;
	}
}

// Query number of joysticks attached to system
unsigned int get_joystick_count(void)
{
	return joystickCount;
}

STRPTR get_joystick_name(unsigned int joynum)
{
	return (STRPTR)joystickList[joynum-1].name;
}

int acquire_joy(unsigned int joynum, int flags)
{
	struct joystick *joy = &joystickList[joynum-1];
	int result = 0;

	joy->handle = IAIN->AIN_ObtainDevice(joy->context, joy->id);
	if(joy->handle) result = 1;
	else IDOS->Printf("Failed to acquire joy\n");

	return result;
}

void unacquire_joy(unsigned int joynum)
{
	struct joystick *joy = &joystickList[joynum-1];

	if(joy->handle) {
		IAIN->AIN_ReleaseDevice(joy->context, joy->handle);
		joy->handle = 0;
	}
}
