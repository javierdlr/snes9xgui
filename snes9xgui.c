#define CATCOMP_NUMBERS
#define CATCOMP_BLOCK
#define CATCOMP_CODE
struct LocaleInfo li;

#include "includes.h"
#include "debug.h"

int32 GetRoms(STRPTR);
void LaunchShowRom(struct Window **, BOOL launch); // TRUE:launchROM; FALSE:showPNG
BOOL OpenLibs(void);
void CloseLibs(void);
int32 beginCommand(char *);
//APTR SleepWindow(struct Window *);
//void WakeWindow(struct Window *, APTR);
void ObtainColors(void);
void ReleasePens(void);
void updateButtonImage(STRPTR fn, CONST_STRPTR fb_img, CONST_STRPTR fb_str, uint32 OID_btn, struct Window *);
int32 SaveToolType(STRPTR iconname, STRPTR ttpName, STRPTR ttpArg);


extern void CreateGUIwindow(void);
extern uint32 DoMessage(char *, char, STRPTR);
extern struct Screen *FrontMostScr(void);
extern void GetGUIGadgets(void);
extern void GetSavestates(struct Window *, STRPTR fn);


struct Library *AIN_Base = NULL;
struct Library *IconBase = NULL;
//struct Library *DOSBase;
struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;
struct Library *UtilityBase = NULL;
struct Library *LocaleBase = NULL;
struct AIN_IFace *IAIN = NULL;
struct IconIFace *IIcon = NULL;
extern struct DOSIFace *IDOS;
struct IntuitionIFace *IIntuition = NULL;
struct GraphicsIFace *IGraphics = NULL;
struct UtilityIFace *IUtility = NULL;
struct LocaleIFace *ILocale = NULL;
struct Library *ClickTabBase = NULL, *ListBrowserBase = NULL, *LayoutBase = NULL,
               *ChooserBase = NULL;
// the class library base
struct ClassLibrary *ButtonBase = NULL, *BitMapBase = NULL, *CheckBoxBase = NULL,
                    *LabelBase = NULL, *WindowBase = NULL, *StringBase = NULL,
                    *RequesterBase = NULL, *SpaceBase = NULL, *IntegerBase = NULL,
                    *SliderBase = NULL;
// the class pointer
Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *StringClass,
      *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
      *RequesterClass, *SpaceClass, *IntegerClass, *SliderClass;
// some interfaces needed
struct ListBrowserIFace *IListBrowser = NULL;
struct ClickTabIFace *IClickTab = NULL;
struct LayoutIFace *ILayout = NULL;
struct ChooserIFace *IChooser = NULL;

//struct RomUserData *rud = NULL;
struct WBStartup *WBenchMsg = NULL;
struct List listbrowser_list, *settings_list;
struct DiskObject *iconify = NULL;
struct Screen *screen = NULL;
LONG PenArray[LAST_PEN];
BOOL MaxWinSize = FALSE, GUIFadeFx = TRUE;
ULONG color0 = 0x000000, color1 = 0xCCCCCC, color2 = 0xDDDDDD;
int32 last_rom_run = 0;
//char Snes9xExe[32] = "snes9x-sdl";
char previews_drw[32] = "Covers3D";


const char *version = VERSTAG;


extern Object *Objects[LAST_NUM];
extern int32 cfg_value[_LAST_CFG], cfg_default_val[_LAST_CFG];


BOOL OpenLibs(void)
{
DBUG("OpenLibs() - START\n",NULL);
	DOSBase = IExec->OpenLibrary("dos.library", 52);
	IDOS = (struct DOSIFace *)IExec->GetInterface(DOSBase, "main", 1, NULL);

	UtilityBase = IExec->OpenLibrary("utility.library", 52);
	IUtility = (struct UtilityIFace *)IExec->GetInterface(UtilityBase, "main", 1, NULL);

	IntuitionBase = IExec->OpenLibrary("intuition.library", 52);
	IIntuition = (struct IntuitionIFace *)IExec->GetInterface(IntuitionBase, "main", 1, NULL);

	GfxBase = IExec->OpenLibrary("graphics.library", 52);
	IGraphics = (struct GraphicsIFace *)IExec->GetInterface(GfxBase, "main", 1, NULL);

	li.li_Catalog = NULL;
	if( (LocaleBase=IExec->OpenLibrary("locale.library", 52))
	   &&  (ILocale=(struct LocaleIFace *)IExec->GetInterface(LocaleBase, "main", 1, NULL)) )
	{
		li.li_ILocale = ILocale;
		li.li_Catalog = ILocale->OpenCatalog(NULL, "snes9xgui.catalog",
		                                     OC_BuiltInLanguage, "english",
		                                     OC_PreferExternal, TRUE,
		                                    TAG_END);
	}
	else { IDOS->PutErrStr("Failed to use catalog system. Using built-in strings.\n"); }

	IconBase = IExec->OpenLibrary("icon.library", 52);
	IIcon = (struct IconIFace *)IExec->GetInterface(IconBase, "main", 1, NULL);

	AIN_Base = IExec->OpenLibrary("AmigaInput.library", 52);
	IAIN = (struct AIN_IFace *)IExec->GetInterface(AIN_Base, "main", 1, NULL);

	if(DOSBase==NULL  ||  UtilityBase==NULL  ||  IntuitionBase==NULL  ||  GfxBase==NULL
	   ||  LocaleBase==NULL  ||  IconBase==NULL) { return FALSE; }

	RequesterBase = IIntuition->OpenClass("requester.class", 52, &RequesterClass);
	IntegerBase = IIntuition->OpenClass("gadgets/integer.gadget", 52, &IntegerClass);
	SpaceBase = IIntuition->OpenClass("gadgets/space.gadget", 52, &SpaceClass);
	StringBase = IIntuition->OpenClass("gadgets/string.gadget", 52, &StringClass);
	CheckBoxBase = IIntuition->OpenClass("gadgets/checkbox.gadget", 52, &CheckBoxClass);
	ButtonBase = IIntuition->OpenClass("gadgets/button.gadget", 52, &ButtonClass);
	SliderBase = IIntuition->OpenClass("gadgets/slider.gadget", 52, &SliderClass);
	BitMapBase = IIntuition->OpenClass("images/bitmap.image", 52, &BitMapClass);
	LabelBase = IIntuition->OpenClass("images/label.image", 52, &LabelClass);
	WindowBase = IIntuition->OpenClass("window.class", 52, &WindowClass);

	ListBrowserBase = (struct Library *)IIntuition->OpenClass("gadgets/listbrowser.gadget", 52, &ListBrowserClass);
	ClickTabBase = (struct Library *)IIntuition->OpenClass("gadgets/clicktab.gadget", 52, &ClickTabClass);
	LayoutBase = (struct Library *)IIntuition->OpenClass("gadgets/layout.gadget", 52, &LayoutClass);
	ChooserBase = (struct Library *)IIntuition->OpenClass("gadgets/chooser.gadget", 52, &ChooserClass);

	if(RequesterBase==NULL  ||  IntegerBase==NULL  ||  SpaceBase==NULL  ||  StringBase==NULL
	   ||  CheckBoxBase==NULL  ||  ButtonBase==NULL  ||  BitMapBase==NULL  ||  LabelBase==NULL
	   ||  SliderBase==NULL  ||  WindowBase==NULL  ||  ListBrowserBase==NULL  ||  ClickTabBase==NULL
	   ||  LayoutBase==NULL  ||  ChooserBase==NULL) { return FALSE; }

	IListBrowser = (struct ListBrowserIFace *)IExec->GetInterface( (struct Library *)ListBrowserBase, "main", 1, NULL );
	IClickTab = (struct ClickTabIFace *)IExec->GetInterface( (struct Library *)ClickTabBase, "main", 1, NULL );
	ILayout = (struct LayoutIFace *)IExec->GetInterface( (struct Library *)LayoutBase, "main", 1, NULL );
	IChooser = (struct ChooserIFace *)IExec->GetInterface( (struct Library *)ChooserBase, "main", 1, NULL );
DBUG("OpenLibs() - END\n",NULL);
	return TRUE;
}

void CloseLibs(void)
{
DBUG("CloseLibs()\n",NULL);
	if(IIntuition) {
		IIntuition->CloseClass(RequesterBase);
		IIntuition->CloseClass(IntegerBase);
		IIntuition->CloseClass(SpaceBase);
		IIntuition->CloseClass(StringBase);
		IIntuition->CloseClass(LabelBase);
		IIntuition->CloseClass(BitMapBase);
		IIntuition->CloseClass(ButtonBase);
		IIntuition->CloseClass(CheckBoxBase);
		IIntuition->CloseClass(SliderBase);
		IIntuition->CloseClass(WindowBase);

		IExec->DropInterface( (struct Interface *)IChooser );
		IIntuition->CloseClass( (struct ClassLibrary *)ChooserBase );
		IExec->DropInterface( (struct Interface *)ILayout );
		IIntuition->CloseClass( (struct ClassLibrary *)LayoutBase );
		IExec->DropInterface( (struct Interface *)IClickTab );
		IIntuition->CloseClass( (struct ClassLibrary *)ClickTabBase );
		IExec->DropInterface( (struct Interface *)IListBrowser );
		IIntuition->CloseClass( (struct ClassLibrary *)ListBrowserBase );
	}

	if(ILocale)
	{
		ILocale->CloseCatalog(li.li_Catalog);
		IExec->DropInterface( (struct Interface *)ILocale );
	}
	IExec->CloseLibrary( (struct Library *)LocaleBase );


	IExec->DropInterface( (struct Interface *)IAIN );
	IExec->CloseLibrary(AIN_Base);
	IExec->DropInterface( (struct Interface *)IIcon );
	IExec->CloseLibrary(IconBase);
	IExec->DropInterface( (struct Interface *)IGraphics );
	IExec->CloseLibrary(GfxBase);
	IExec->DropInterface( (struct Interface *)IIntuition );
	IExec->CloseLibrary(IntuitionBase);

	IExec->DropInterface( (struct Interface *)IUtility );
	IExec->CloseLibrary(UtilityBase);

	IExec->DropInterface( (struct Interface *)IDOS );
	IExec->CloseLibrary(DOSBase);
}


/********/
/* MAIN */
/********/
int main(int argc, char **argv)
{
	int32 error = -1;
	char text_buf[256];
	struct DiskObject *micon = NULL;
	STRPTR ttp;
DBUG("*** START "VERS" ***\n",NULL);
	if(argc == 0) { WBenchMsg = (struct WBStartup *)argv; }

	if(OpenLibs() == TRUE) {
		IExec->NewList(&listbrowser_list);
		settings_list = IExec->AllocSysObject(ASOT_LIST, NULL);

		if(WBenchMsg) // launched from WB/icon
		{
			IUtility->Strlcpy( text_buf, WBenchMsg->sm_ArgList->wa_Name, sizeof(text_buf) );

			// Reset icon X/Y positions so it iconifies properly on Workbench
			iconify = IIcon->GetIconTags(text_buf, ICONGETA_FailIfUnavailable,FALSE, TAG_END);
			iconify->do_CurrentX = NO_ICON_POSITION;
			iconify->do_CurrentY = NO_ICON_POSITION;

			micon = IIcon->GetDiskObjectNew(text_buf);
//DBUG("micon 0x%08lx\n",micon);
			if(micon)
			{
				ttp = IIcon->FindToolType(micon->do_ToolTypes, "PUBSCREEN");
//DBUG("[1]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
DBUG("SCREEN tooltype set to '%s'\n",ttp);
					screen = IIntuition->LockPubScreen(ttp);
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "MAX_WINSIZE");
//DBUG("[2]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					MaxWinSize = TRUE;
					//ShowCover = TRUE;
DBUG("MAX_WINSIZE tooltype enabled.\n");
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "TEXT");
//DBUG("[3]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					IDOS->HexToLong(ttp, &color0);
DBUG("TEXT tooltype set to 0x%06lx\n",color0);
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "COLOR1");
//DBUG("[4]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					IDOS->HexToLong(ttp, &color1);
DBUG("COLOR1 tooltype set to 0x%06lx\n",color1);
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "COLOR2");
//DBUG("[5]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					IDOS->HexToLong(ttp, &color2);
DBUG("COLOR2 tooltype set to 0x%06lx\n",color2);
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "NO_GUI_FADE");
//DBUG("[6]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					GUIFadeFx = FALSE;
DBUG("NO_GUI_FADE tooltype enabled.\n");
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "PREVIEWS_DRAWER");
//DBUG("[6]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					IUtility->Strlcpy( previews_drw, ttp, sizeof(previews_drw) );
				}
DBUG("PREVIEWS_DRAWER set to '%s'\n",previews_drw);

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "LAST_ROM_LAUNCHED");
//DBUG("[5]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					IDOS->StrToLong(ttp, &last_rom_run);
DBUG("LAST_ROM_LAUNCHED tooltype set to %ld\n",last_rom_run);
				}

				/*ttp = IIcon->FindToolType(micon->do_ToolTypes, "SNES9X_EXEC");
DBUG("[7]ttp 0x%08lx\n",ttp);
				if(ttp)
				{
					IUtility->Strlcpy( Snes9xExe, ttp, sizeof(Snes9xExe) );
				}
DBUG("SNES9X_EXEC: '%s'\n",Snes9xExe);*/
			} // END if(micon)
		}

		if(screen == NULL) { screen = IIntuition->LockPubScreen(NULL); }
DBUG("pubscreen=0x%lx '%s'\n",screen,screen->Title);

		ObtainColors();

		//rud = IExec->AllocVecTags(sizeof(struct RomUserData), AVT_ClearWithValue,0, TAG_DONE);

		error += GetRoms(ROMSU); // Universal rompath
		error += GetRoms(ROMSF); // French rompath
		error += GetRoms(ROMSX); // SuperFX rompath
		error += GetRoms(ROMSM); // MSU1 rompath
		error += GetRoms(ROMSO); // Optical rompath

		if(error == -1) { DoMessage( (STRPTR)GetString(&li,MSG_ERROR_NOROMS), REQIMAGE_ERROR, NULL ); } //"No roms found." 
		else { CreateGUIwindow(); }

		//IExec->FreeVec(rud);

		ReleasePens();

		IIntuition->UnlockPubScreen(NULL, screen);
		IListBrowser->FreeListBrowserList(&listbrowser_list);
		IListBrowser->FreeListBrowserList(settings_list);
		IExec->FreeSysObject(ASOT_LIST, settings_list);
		//settings_list = NULL;
	}
	else { DoMessage( (STRPTR)GetString(&li,MSG_ERROR_REQUIRED), REQIMAGE_ERROR, NULL ); } //"Couldn't find required library/classes."

	CloseLibs();
DBUG("*** END "VERS" ***\n",NULL);

	return error;
}

#define MAX_FULLFILEPATH MAX_DOS_PATH+MAX_DOS_FILENAME
int32 GetRoms(STRPTR RomsDir)
{
	APTR context = NULL;
	int32 resul = 0, i = 0;
	char rom[MAX_FULLFILEPATH], ext[4] = ""; // extension + '\0'
	struct Node *n = NULL;
	STRPTR RomType[] = {"U", "F", "FX", "MS", "OP"}, // 'U'niversal, 'F'rench, Super'FX', 'MS'U1, 'OP'tical
	       pattern_ms = IExec->AllocVecTags(512, TAG_DONE),
	       romfullpath = IExec->AllocVecTags(MAX_FULLFILEPATH, TAG_DONE);
DBUG("GetRoms()\n",NULL);
	// Search roms
	IDOS->ParsePatternNoCase("#?.(zip|smc|sfc|fig|swc)", pattern_ms, 64);

	IUtility->Strlcpy(romfullpath, ROMS, MAX_FULLFILEPATH);
	IDOS->AddPart(romfullpath, RomsDir, MAX_FULLFILEPATH);
	context = IDOS->ObtainDirContextTags(EX_StringNameInput, romfullpath,
	                                     EX_DataFields, (EXF_NAME|EXF_TYPE),
	                                     EX_MatchString, pattern_ms,
	                                    TAG_END);

	if(IUtility->Stricmp(RomsDir, ROMSF) == 0) { i = 1; }
	else if(IUtility->Stricmp(RomsDir, ROMSX) == 0) { i = 2; }
	     else if(IUtility->Stricmp(RomsDir, ROMSM) == 0) { i = 3; }
	          else if(IUtility->Stricmp(RomsDir, ROMSO) == 0) { i = 4; }
	//else { i = 0; }

	if(context)
	{
		struct ExamineData *dat;
		int32 len, newpos;

		while( (dat=IDOS->ExamineDir(context)) )
		{
			if( EXD_IS_FILE(dat) )
			{
				len = IUtility->Strlen(dat->Name) - sizeof(ext);
				newpos = IDOS->SplitName(dat->Name, '.', rom, len, MAX_DOS_FILENAME);
//DBUG("rom = '%s'\n",dat->Name);
				// Get extension
				ext[0] = IUtility->ToUpper( *(dat->Name + newpos) );
				ext[1] = IUtility->ToUpper( *(dat->Name + newpos + 1) );
				ext[2] = IUtility->ToUpper( *(dat->Name + newpos + 2) );
//DBUG("  extension = '%s'\n",ext);
				IUtility->Strlcpy(rom, dat->Name, newpos);
//DBUG("  name = '%s'\n",rom);
				n = IListBrowser->AllocListBrowserNode(LAST_COL,
				                                       LBNA_Column,COL_TYPE,
				                                       LBNA_Flags,LBFLG_CUSTOMPENS, LBNCA_FGPen,PenArray[TXT_R],
				                                         LBNCA_CopyText,TRUE, LBNCA_Text,RomType[i],
				                                         LBNCA_HorizJustify, LCJ_CENTER,
				                                       LBNA_Column,COL_ROM,
				                                       LBNA_Flags,LBFLG_CUSTOMPENS, LBNCA_FGPen,PenArray[TXT_R],
				                                         LBNCA_CopyText,TRUE, LBNCA_Text,rom,
				                                       LBNA_Column,COL_FMT,
				                                       LBNA_Flags,LBFLG_CUSTOMPENS, LBNCA_FGPen,PenArray[TXT_R],
				                                         LBNCA_CopyText,TRUE, LBNCA_Text,ext,
				                                         LBNCA_HorizJustify, LCJ_CENTER,
				                                      TAG_DONE);
				IExec->AddTail(&listbrowser_list, n);
				resul++;
			}
		}
DBUG("  '%s' %ld\n",RomsDir,resul);
		if(IDOS->IoErr() == ERROR_NO_MORE_ENTRIES) { resul = 1; }
		else
		{
			IDOS->Fault(IDOS->IoErr(), NULL, rom, 1024);
			DoMessage(rom, REQIMAGE_ERROR, NULL);
		}
	} // END if(context)
	else
	{
		STRPTR romdrawer[] = {"Universal", "French", "SuperFX", "MSU1", "Optical"};
		IDOS->Fault(IDOS->IoErr(), NULL, pattern_ms, 1024); // 'pattern_ms' used as temp-buffer
		IUtility->SNPrintf(rom, 1024, GetString(&li,MSG_ERROR_ROMDRAWER),romdrawer[i],pattern_ms);
		DoMessage(rom, REQIMAGE_ERROR, NULL);
	}

	IDOS->ReleaseDirContext(context);
	IExec->FreeVec(romfullpath);
	IExec->FreeVec(pattern_ms);

	return resul;
}

/*APTR SleepWindow(struct Window *win)
{
 struct Requester *lock = NULL;

 if(win)
  if( (lock=IExec->AllocVecTags(sizeof(struct Requester), AVT_ClearWithValue,0, TAG_DONE)) )
  {
   IIntuition->InitRequester(lock);
   IIntuition->Request(lock, win);
   if(win->FirstRequest == lock)
    IIntuition->SetWindowPointer(win, WA_BusyPointer,TRUE, TAG_END);
   else
   {
    IExec->FreeVec(lock);
    lock = NULL;
   }
  }

 return(lock);
}

void WakeWindow(struct Window *win, APTR lock)
{
 if(lock)
 {
  IIntuition->EndRequest(lock, win);
  IExec->FreeVec(lock);
  IIntuition->SetWindowPointer(win, TAG_END);
 }
}*/

#define CMDLINE_LENGTH 2048
int32 beginCommand(char *romfile)
{
	STRPTR cmdline = NULL;
	int32 res_value = 0, res_value2 = 0,
	      ahi_pbr[] = {48000, 44100, 35200, 32040, 31850, 30000, 22000, 16000, 11000, 8000};
	uint8 ahi_buf[] = {210, 194, 176, 160, 144, 128, 112, 96, 80, 64, 48, 32},
	      oc_sfx[7] = {0, 20, 40, 60, 80, 90, 100};
	char vm_char[] = {'0', '1','2','3','4','5','6','7','8', 'x','r','t','s','g'}; // first item in array "changes" in SWITCH() below
	STRPTR port2[]  = {NULL,NULL, "mouse1", "superscope", "justifier", "macsrifle"},
	       snes9x[] = {" -software", " -opengl", "COMP", "EGL"};

	// Check if romfile exists
	struct ExamineData *dat = IDOS->ExamineObjectTags(EX_StringNameInput,romfile, TAG_END);
	if(dat == NULL) { return -1; }
DBUG("ExamineObjectTags(): '%s' %s%lld bytes\n",dat->Name,"",dat->FileSize);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat);

	cmdline = IExec->AllocVecTags(CMDLINE_LENGTH, TAG_DONE);

	//IUtility->Strlcpy(cmdline, Snes9xExe, CMDLINE_LENGTH);

	// Snes9x main executable
	IIntuition->GetAttrs(OBJ(OID_RENDERTYPE), CHOOSER_Selected,&res_value2, TAG_DONE);
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "snes9x-sdl%s",snes9x[res_value2]);

	// Global Settings
	// window/fullscreen
	IIntuition->GetAttrs(OBJ(OID_MODE_WF), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value==1) {//  &&  res_value2!=13  &&  res_value2!=14) {
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -fullscreen",cmdline);
	}
	if(res_value2 != 3) { // not available on EGL_WRAP
		// video mode (vm_char[0] "changes")
		IIntuition->GetAttrs(OBJ(OID_VIDMODE), CHOOSER_Selected,&res_value2, TAG_DONE);
		switch(res_value2) {
			/*case 16: // sdlCOMP: Nearest
				res_value2 = 0;
				vm_char[0] = 'd';
			break;
			case 15: // sdlCOMP: Linear
				res_value2 = 0;
				vm_char[0] = 'c';
			break;*/
			case 14: // sdl: 320x240 blend
				res_value2 = 0;
				if(res_value == 1) { vm_char[0] = 'f'; }
				else { vm_char[0] = 'b'; }
			break;
			case 13: // sdl: 320x240 standard
				res_value2 = 0;
				// '0' is "default" in vm_char[0]
				if(res_value != 1) { vm_char[0] = 'p'; }
			break;
			default: ++res_value2;
		}
//DBUG("video mode (%ld) '%lc'\n",res_value2,vm_char[res_value2]);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -v%lc",cmdline,vm_char[res_value2]);
	}
	// frameskip
	IIntuition->GetAttrs(OBJ(OID_SKIP_F), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value == 0) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -frameskip auto",cmdline); }
	else { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -frameskip %ld",cmdline,--res_value); }

	// display remder (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_DISP_RENDER), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) {
DBUG("  OID_DISP_RENDER\n",NULL);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -displayrender",cmdline);
	}

	// skins (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_SKINS), GA_Disabled,&res_value2, CHOOSER_Selected,&res_value, TAG_DONE);
DBUG("  OID_SKINS = %ld [@|  %ld  ]\n",res_value2,res_value);
	if(res_value2==FALSE  &&  res_value!=0) { // IS enabled and NOT 'No'
DBUG("  -skinfx -skinnumber %ld\n",res_value-1);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -skinfx -skinnumber %ld",cmdline,res_value-1);
	}

	// filters (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_FILTER), GA_Disabled,&res_value2, CHOOSER_Selected,&res_value, TAG_DONE);
DBUG("  OID_FILTER = %ld\n",res_value2);
	if(res_value2==FALSE  &&  res_value==0) { // is enabled and LINEAR
//DBUG("  -filterhardware\n",NULL);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -filterhardware",cmdline);
	}

	// shaders (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_SHADERS), GA_Disabled,&res_value2, CHOOSER_Selected,&res_value, TAG_DONE);
DBUG("  OID_SHADERS = %ld\n",res_value2);
	if(res_value2 == FALSE) { // is enabled
DBUG("  -filtershader %ld\n",res_value);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -filtershader %ld",cmdline,res_value);
	}

	// overclock superfx (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_OC_SFX), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value != 0) {
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -overclocksuperfx %ld",cmdline,oc_sfx[res_value]);
	}

	// rewinding (REWINDING)
	IIntuition->GetAttrs(OBJ(OID_REWINDING), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) {
		int32 rew_buf, rew_gra;
		IIntuition->GetAttrs(OBJ(OID_REW_BUF), INTEGER_Number,&rew_buf, TAG_DONE);
		IIntuition->GetAttrs(OBJ(OID_REW_GRA), INTEGER_Number,&rew_gra, TAG_DONE);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -rewinding -rewindbuffersize %ld -rewindgranularity %ld",cmdline,rew_buf,rew_gra);
	}
	res_value2 = res_value; // for VSYNC

	// video hack (ENGINE)
	IIntuition->GetAttrs(OBJ(OID_HACKVIDEO), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) {
		int32 hv_width, hv_height;
		IIntuition->GetAttrs(OBJ(OID_HV_WIDTH), INTEGER_Number,&hv_width, TAG_DONE);
		IIntuition->GetAttrs(OBJ(OID_HV_HEIGHT), INTEGER_Number,&hv_height, TAG_DONE);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -hackvideo -height %ld -width %ld",cmdline,hv_width,hv_height);
	}

	// turbosframeskip (ENGINE)
	IIntuition->GetAttrs(OBJ(OID_TURBO_FSF), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value != 0) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -turbomode -turboframeskipframe %ld",cmdline,res_value); }

	// disable interleave (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_INTERLEAVE), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -nointerleave",cmdline); }

	// memory map (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_MEMMAP), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value == 1) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -lorom",cmdline); }
	if(res_value == 2) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -hirom",cmdline); }

	// video (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_PALNTSC), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value == 1) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -pal",cmdline); }
	if(res_value == 2) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -ntsc",cmdline); }

	// overclock cpu (MISC)
	IIntuition->GetAttrs(OBJ(OID_OC_CPU), CHOOSER_Selected,&res_value, TAG_DONE);
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -overclockcpu %ld",cmdline,res_value);

	// disable sprite limit (ENGINE)
	IIntuition->GetAttrs(OBJ(OID_SPRITE_LIMIT), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -disablespritelimit 128",cmdline); }

	// emulated machine (GRAPHICS)
	IIntuition->GetAttrs(OBJ(OID_EMULATED), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) {
		res_value2 = res_value;
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -emulatedmachine",cmdline);
	}

	// vsync (MISC), takes into accout REWINDING (res_value2)
	IIntuition->GetAttrs(OBJ(OID_VSYNC), GA_Selected,&res_value, TAG_DONE);
	if(res_value2 == TRUE) { res_value = FALSE; }
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s %svsync",cmdline, res_value? "-":"-no");

	// activate msu1 (ENGINE)
	IIntuition->GetAttrs(OBJ(OID_MSU1), GA_Selected,&res_value, TAG_DONE);
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -%s",cmdline,res_value? "msu1":"nomsu1");

	// ahi playback (AUDIO)
	IIntuition->GetAttrs(OBJ(OID_AHI_PBR), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value == AHI_PBR_MUTE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -mute",cmdline); }
	else
	{
		// buffer length
		IIntuition->GetAttrs(OBJ(OID_AHI_BUF), CHOOSER_Selected,&res_value2, TAG_DONE);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -inputrate %ld -buffersize %ld",cmdline,ahi_pbr[res_value],ahi_buf[res_value2]);
		// interpolated method
		IIntuition->GetAttrs(OBJ(OID_SND_IMETHOD), CHOOSER_Selected,&res_value2, TAG_DONE);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -interpolationmethod %ld",cmdline,res_value2);
		// syncronized
		IIntuition->GetAttrs(OBJ(OID_AHI_SYNC), GA_Selected,&res_value2, TAG_DONE);
		if(res_value2==TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -soundsync",cmdline); }
		// no stereo
		IIntuition->GetAttrs(OBJ(OID_AHI_NOST), GA_Selected,&res_value2, TAG_DONE);
		if(res_value2==TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -nostereo",cmdline); }
		// sound thread
		IIntuition->GetAttrs(OBJ(OID_SND_THREAD), GA_Selected,&res_value2, TAG_DONE);
		if(res_value2==TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -soundthread",cmdline); }
	}

	// display time (MISC)
	IIntuition->GetAttrs(OBJ(OID_DISP_TIME), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -displaytime",cmdline); }

	// show fps (MISC)
	IIntuition->GetAttrs(OBJ(OID_FPS), GA_Selected,&res_value, TAG_DONE);
	if(res_value == TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -displayframerate",cmdline); }

	// AmigaInput Settings
//	IIntuition->GetAttrs(OBJ(OID_P1_KJ),     CHOOSER_Selected,&res_value, TAG_DONE);
//	IIntuition->GetAttrs(OBJ(OID_P1_STARTB), GA_Selected,&res_value2, TAG_DONE);
//	if(res_value==1 && res_value2==FALSE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -joynostartbuttonp1",cmdline); }
	IIntuition->GetAttrs(OBJ(OID_P2_KJ), CHOOSER_Selected,&res_value, TAG_DONE);
	if(res_value > 1) {
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -port2 %s",cmdline,port2[res_value]);
	}
//	IIntuition->GetAttrs(OBJ(OID_P2_STARTB), GA_Selected,&res_value2, TAG_DONE);
//	if(res_value==1 && res_value2==FALSE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -joynostartbuttonp2",cmdline); }

// qwerty (MISC)
IIntuition->GetAttrs(OBJ(OID_QWERTY), GA_Selected,&res_value, TAG_DONE);
if(res_value == TRUE) { IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -qwerty",cmdline); }

// savestate/snapshot (GENERAL)
struct Node *n;
IIntuition->GetAttrs(OBJ(OID_SAVESTATES), CHOOSER_Selected,&res_value, CHOOSER_SelectedNode,(uint32*)&n, TAG_DONE);
if(res_value != 0) {
	STRPTR str;
	char snapshot[MAX_DOS_FILENAME] = "";
	IChooser->GetChooserNodeAttrs(n, CNA_Text,&str, TAG_DONE);
	IUtility->Strlcpy(snapshot, str, IUtility->Strlen(str)-3); // removing extension
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -loadsnapshot \""SAVES"/%s\"",cmdline,snapshot);
//DBUG("%s -loadsnapshot \""SAVES"/%s\"\n",cmdline,snapshot);
}

	// Add romfile to commandline string
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s \"%s\"",cmdline,romfile);
DBUG("%s\n",cmdline);

	// Launch 'snes9x-sdl/COMP'
	res_value = IDOS->SystemTags(cmdline, SYS_Input,NULL, SYS_Output,NULL, //SYS_Error,NULL,
	                             NP_Priority,0, SYS_Asynch,FALSE, TAG_END);

	IExec->FreeVec(cmdline);

	return res_value;
}

#define FILENAME_LENGTH 1024
void LaunchShowRom(struct Window **pw, BOOL launch) // TRUE:launchROM; FALSE:showPNG
{
	uint32 res_n;
	STRPTR res_t = NULL, res_s = NULL, res_e = NULL, filename;
	char rom_ext[5] = ".";
	//APTR winlock;

	IIntuition->GetAttr(LISTBROWSER_SelectedNode, OBJ(OID_LISTBROWSER), (uint32 *)&res_n);
	if(res_n == 0) { return; }

	filename = IExec->AllocVecTags(FILENAME_LENGTH, TAG_DONE),

	IListBrowser->GetListBrowserNodeAttrs( (struct Node *)res_n, LBNA_Column,COL_ROM, LBNCA_Text,&res_s, TAG_DONE );
//	IListBrowser->GetListBrowserNodeAttrs( (struct Node *)res_n, LBNA_UserData,&rud, TAG_DONE );
	IListBrowser->GetListBrowserNodeAttrs( (struct Node *)res_n, LBNA_Column,COL_TYPE, LBNCA_Text,&res_t, TAG_DONE );
	IListBrowser->GetListBrowserNodeAttrs( (struct Node *)res_n, LBNA_Column,COL_FMT, LBNCA_Text,&res_e, TAG_DONE );
DBUG("res_n=0x%08lx -> res_s='%s' (res_e='%s')\n",res_n,res_s,res_e);
	IUtility->Strlcpy(filename, ROMS, FILENAME_LENGTH);
	IUtility->Strlcat( rom_ext, res_e, sizeof(rom_ext) );

	if(launch) // LAUNCH ROM
	{// Initial ROM path: "Roms-Universal", "Roms-Optical", "Roms-MSU1", Roms-SuperFX" or "Roms-French"
		switch( *(res_t) )
		{
			case 'U': IDOS->AddPart(filename, ROMSU, FILENAME_LENGTH); break;
			case 'O': IDOS->AddPart(filename, ROMSO, FILENAME_LENGTH); break;
			case 'M':
				IDOS->AddPart(filename, ROMSM, FILENAME_LENGTH);
				IDOS->AddPart(filename, res_s, FILENAME_LENGTH); // add filename as (sub)drawer
			break;
			case 'F':
				if(IUtility->Strlen(res_t) == 2) { IDOS->AddPart(filename, ROMSX, FILENAME_LENGTH); } // Super'FX'
				else { IDOS->AddPart(filename, ROMSF, FILENAME_LENGTH); }
			break;
			default: break;
		}
		IDOS->AddPart(filename, res_s, FILENAME_LENGTH);       // add ROM filename
		IUtility->Strlcat(filename, rom_ext, FILENAME_LENGTH); // add extension
DBUG("Starting ROM '%s'...\n",filename);

		//winlock = SleepWindow(pw);
//GetGUIGadgets();

DBUG("WM_CLOSE (win=0x%08lx)\n",(*pw));
		IIntuition->IDoMethod(OBJ(OID_MAIN), WM_CLOSE);
		(*pw) = NULL;
		if(beginCommand(filename) != 0) {
			DoMessage( (STRPTR)GetString(&li,MSG_ERROR_LAUNCHING), REQIMAGE_ERROR, NULL ); //"Error launching 'snes9x-sdl'!"
		}
		if( ((*pw)=(struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL)) ) {
			IIntuition->ScreenToFront( (*pw)->WScreen );
DBUG("WM_OPEN (win=0x%08lx))\n",(*pw));
			if(WBenchMsg) {
				IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,&last_rom_run, TAG_DONE);
				IUtility->SNPrintf(rom_ext, sizeof(rom_ext), "%ld",last_rom_run); // using 'rom_ext' as temp buffer
				SaveToolType(WBenchMsg->sm_ArgList->wa_Name, "LAST_ROM_LAUNCHED", rom_ext);
			}
		}
		//WakeWindow(pw, winlock);
	}
	else // SHOW COVER3D
	{
		GetSavestates( *(pw), res_s );

		IDOS->AddPart(filename, previews_drw, FILENAME_LENGTH);
		IDOS->AddPart(filename, res_s, FILENAME_LENGTH);      // add ROM filename
		IUtility->Strlcat(filename, ".png", FILENAME_LENGTH); // add extension
DBUG("COVER3D: '%s'\n",filename);
		updateButtonImage(filename, ROMS"Covers3D/availablecover3d.png", GetString(&li,MSG_GUI_GENERAL_NOPREVIEW), OID_PREVIEW_BTN, (*pw));

		/*if(ShowCover)
		{// The same for PREVIEW
			IUtility->Strlcpy(filename, ROMS, FILENAME_LENGTH);
			IDOS->AddPart(filename, "Previews", FILENAME_LENGTH);
			IDOS->AddPart(filename, res_s, FILENAME_LENGTH);      // add ROM filename
			IUtility->Strlcat(filename, ".png", FILENAME_LENGTH); // add extension
DBUG("PREVIEW: '%s'\n",filename);
			updateButtonImage(filename, ROMS"Previews/availablepreview.png", "", OID_BOXCOVER_BTN, (*pw));
		}*/ // END 'if(ShowCover)'
	}

	IExec->FreeVec(filename);
}


void ObtainColors(void)
{
	char txt_buf[256];
DBUG("ObtainColors() - START\n");
	struct Screen *MainScreen = FrontMostScr();

	PenArray[TXT_R] = IGraphics->ObtainBestPen(MainScreen->ViewPort.ColorMap, 
	                              RGB8to32( (color0 >> 16) & 255 ), // Red
	                              RGB8to32( (color0 >>  8) & 255 ), // Green
	                              RGB8to32(color0 & 255),           // Blue
	                             TAG_DONE);
	if(PenArray[TXT_R] == -1) {
		IUtility->SNPrintf(txt_buf, sizeof(txt_buf), (STRPTR)GetString(&li,MSG_FAIL_OBTAIN_PEN),"TEXT");
		DoMessage(txt_buf, REQIMAGE_WARNING, NULL );
	}

	PenArray[ROW_E] = IGraphics->ObtainBestPen(MainScreen->ViewPort.ColorMap, 
	                              RGB8to32( (color2 >> 16) & 255 ), // Red
	                              RGB8to32( (color2 >>  8) & 255 ), // Green
	                              RGB8to32(color2 & 255),           // Blue
	                             TAG_DONE);
	if(PenArray[ROW_E] == -1) {
		IUtility->SNPrintf(txt_buf, sizeof(txt_buf), (STRPTR)GetString(&li,MSG_FAIL_OBTAIN_PEN),"COLOR2");
		DoMessage(txt_buf, REQIMAGE_WARNING, NULL);
	}

	PenArray[ROW_O] = IGraphics->ObtainBestPen(MainScreen->ViewPort.ColorMap, 
	                              RGB8to32( (color1 >> 16) & 255 ), // Red
	                              RGB8to32( (color1 >>  8) & 255 ), // Green
	                              RGB8to32(color1 & 255),           // Blue
	                             TAG_DONE);
	if(PenArray[ROW_O] == -1) {
		IUtility->SNPrintf(txt_buf, sizeof(txt_buf), (STRPTR)GetString(&li,MSG_FAIL_OBTAIN_PEN),"COLOR1");
		DoMessage(txt_buf, REQIMAGE_WARNING, NULL);
	}
DBUG("ObtainColors() %ld - END\n",LAST_PEN);
}

void ReleasePens(void)
{
	int i;
DBUG("ReleasePens() - START\n");
	struct Screen *MainScreen = FrontMostScr();
	if(!MainScreen) return;

	for(i=0; i<LAST_PEN; i++)
	{
		IGraphics->ReleasePen(MainScreen->ViewPort.ColorMap, PenArray[i]);
		PenArray[i] = 0;
	}
DBUG("ReleasePens() %ld - END\n",i);
}

void updateButtonImage(STRPTR fn, CONST_STRPTR fb_img, CONST_STRPTR fb_str, uint32 OID_btn, struct Window *pw)
{
	Object *newobj = NULL;
	struct ExamineData *datF = NULL;
	uint32 OID_img = OID_btn + 1;
DBUG("updateButtonImage: '%s'\n",fn);
	// Check if filename exists
	datF = IDOS->ExamineObjectTags(EX_StringNameInput,fn, TAG_END);
	if(datF == NULL)
	{// Fallback image (fb_img) "available#?.png"
		IUtility->Strlcpy(fn, fb_img, 1024);
		datF = IDOS->ExamineObjectTags(EX_StringNameInput,fn, TAG_END);
	}

	if(datF)
	{
DBUG("  ExamineObjectTags(): '%s' %s%lld bytes\n",datF->Name,"",datF->FileSize); // FileSize is int64
		IDOS->FreeDosObject(DOS_EXAMINEDATA, datF);
		// Create and set button/image
		newobj = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
		                               //IA_Scalable, TRUE,
		                               //BITMAP_Masking, TRUE,
		                               BITMAP_Screen,  pw->WScreen,
		                               BITMAP_SourceFile, fn,
		                               BITMAP_SelectSourceFile, DATAS"/_launchROM.png",
		                              TAG_DONE);
DBUG("  new image: 0x%08lx\n",newobj);
		if(newobj) {
//			uint32 w;
//			IIntuition->GetAttrs(newobj, BITMAP_Width,&w, TAG_DONE);
//DBUG("            width = %ld px\n",w);
			ILayout->SetPageGadgetAttrs(GAD(OID_btn), OBJ(OID_GENERAL), pw, NULL, BUTTON_RenderImage,newobj, TAG_DONE); // pagetab
		}
	}
	else
	{// No image found -> show fallback string (fb_str)
		ILayout->SetPageGadgetAttrs(GAD(OID_btn), OBJ(OID_GENERAL), pw, NULL, GA_Text,fb_str, TAG_DONE); // pagetab
	}
DBUG("  old image: 0x%08lx (disposing)\n",OBJ(OID_img));
	IIntuition->DisposeObject( OBJ(OID_img) );
	OBJ(OID_img) = newobj;
	ILayout->RefreshPageGadget(GAD(OID_btn), OBJ(OID_GENERAL), pw, NULL);
}

// RETURNS: -1:created;  0:nothing done;  <value>:tooltype saved
int32 SaveToolType(STRPTR iconname, STRPTR ttpName, STRPTR ttpArg)
{
	int32 i = 0;
	char newttp[1024], ttpBuf1[64], ttpBuf2[64];
	struct DiskObject *micon = NULL;
DBUG("SaveToolType() - START\n",NULL);
	if(ttpArg)
	{
		IUtility->SNPrintf(newttp, sizeof(newttp), "%s=",ttpName); // "<tooltype>="
		IUtility->SNPrintf(ttpBuf2, sizeof(ttpBuf2), "(%s=",ttpName); // BUF2 = "(<tooltype>="
	}
	else
	{
		IUtility->Strlcpy( newttp, ttpName, sizeof(newttp) ); // "<tooltype>"
		IUtility->SNPrintf(ttpBuf2, sizeof(ttpBuf2), "(%s)",ttpName); // BUF2 = "(<tooltype>)"
	}

	IUtility->Strlcpy( ttpBuf1, newttp, sizeof(ttpBuf1) ); // BUF1 = NEWTTP

	micon = IIcon->GetDiskObject(iconname);
	if(micon)
	{
		STRPTR *oldttp = NULL;

		if(micon->do_ToolTypes) // icon with tooltypes..
		{
			for(; micon->do_ToolTypes[i]!=NULL; i++) //..parse tooltypes
			{
DBUG("  %2ld'%s'\n",i,micon->do_ToolTypes[i]);
				if( !IUtility->Strnicmp(micon->do_ToolTypes[i],ttpBuf1,IUtility->Strlen(ttpBuf1))
				   || !IUtility->Strnicmp(micon->do_ToolTypes[i],ttpBuf2,IUtility->Strlen(ttpBuf2)) )
				{// Found tooltype
					IUtility->Strlcat( newttp, ttpArg, sizeof(newttp) );
//DBUG("  '%s' == '%s'?\n",micon->do_ToolTypes[i],newttp);
					// Normal case tooltype -> tooltype=value OR (tooltype=value)
					if( ttpArg  &&  IUtility->Stricmp(micon->do_ToolTypes[i],newttp) )
					{// Tooltype loaded diffs. from icon's tooltype -> modify tooltype
						oldttp = micon->do_ToolTypes;
						micon->do_ToolTypes[i] = newttp;
						IIcon->PutDiskObject(iconname, micon);
						micon->do_ToolTypes = oldttp;
DBUG("  Tooltype modified: '%s'\n",newttp);
					}
/*
					// Special case tooltype (toggle/switch) -> 'tooltype' OR '(tooltype)'
					// NOTE: if it doesn't exists it will be created as "enabled" -> 'tooltype'
					if(!ttpArg)
					{
						oldttp = micon->do_ToolTypes;
						if(IUtility->Stricmp(micon->do_ToolTypes[i],ttpBuf1) == 0) {
							IUtility->SNPrintf(ttpBuf1, sizeof(ttpBuf1), "(%s)",ttpName); // BUF1 = "(<tooltype>)"
						}
						else {
							IUtility->Strlcpy( ttpBuf1, newttp, sizeof(ttpBuf1) ); // BUF1 = "<tooltype>"
						}

						micon->do_ToolTypes[i] = ttpBuf1;
						IIcon->PutDiskObject(iconname, micon);
						micon->do_ToolTypes = oldttp;
DBUG("  Tooltype toggled: '%s'\n",ttpBuf1);
					}
*/
					break;
				} // END if( !Strnicmp(micon->..
			} // END for
		} // END if(micon->do_ToolTypes)

		if(!micon->do_ToolTypes  ||  !micon->do_ToolTypes[i]) // No tooltypes or doesn't exists..
		{//..create tooltype entry and END
			STRPTR *newtts = IExec->AllocVecTags( (i+2)*sizeof(STRPTR), TAG_DONE );
//DBUG("  Tooltype '%s' NOT found (i=%ld)\n",newttp,i);
			IUtility->Strlcat( newttp, ttpArg, sizeof(newttp) );
			oldttp = micon->do_ToolTypes;
			IExec->CopyMem( oldttp, newtts, i*sizeof(STRPTR) ); // clone tooltypes
			newtts[i] = newttp;
			newtts[i+1] = NULL;
			micon->do_ToolTypes = newtts;
			IIcon->PutDiskObject(iconname, micon);
			micon->do_ToolTypes = oldttp;
			IExec->FreeVec(newtts);
DBUG("  Tooltype created: '%s'\n",newttp);
			i = -1;
		}

		IIcon->FreeDiskObject(micon);
	}
DBUG("SaveToolType() - END\n",NULL);
	return i;
}
