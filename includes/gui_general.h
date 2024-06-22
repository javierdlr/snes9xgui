#ifndef GUI_GENERAL_H
#define GUI_GENERAL_H

            PAGE_Add, OBJ(OID_GENERAL) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
             //LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
             LAYOUT_SpaceOuter,     TRUE,
             //LAYOUT_SpaceInner,     FALSE,
             LAYOUT_HorizAlignment, LALIGN_CENTER,
//LAYOUT_VertAlignment, LALIGN_TOP,
             LAYOUT_BevelStyle,     BVS_SBAR_VERT,
             LAYOUT_Label,          GetString(&li, MSG_GUI_GENERAL_GROUP),//"Game / Preview",
// ROMLIST
             LAYOUT_AddChild, OBJ(OID_LISTBROWSER) = IIntuition->NewObject(ListBrowserClass, NULL, //"listbrowser.gadget",
              GA_ID,        OID_LISTBROWSER,
              GA_RelVerify, TRUE,
              LISTBROWSER_SortColumn,     COL_ROM,
              //LISTBROWSER_AutoFit,        TRUE,
              LISTBROWSER_Labels,         &listbrowser_list,
              LISTBROWSER_ColumnInfo,     columninfo,
              LISTBROWSER_ColumnTitles,   TRUE,
              LISTBROWSER_ShowSelected,   TRUE,
              LISTBROWSER_Selected,       -1,
              //LISTBROWSER_MinVisible,     10,
              //LISTBROWSER_Striping,       LBS_ROWS,
              LISTBROWSER_TitleClickable, TRUE,
              //LISTBROWSER_HorizontalProp, TRUE,
             TAG_DONE),
             //CHILD_MinWidth, 300, // pixels width of listbrowser
             CHILD_WeightedWidth, 60,
// BUTTON/IMAGE + TOTALROMS
             LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
               LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
               //LAYOUT_HorizAlignment, LALIGN_CENTER,
               LAYOUT_SpaceOuter,     FALSE,

// BUTTON/IMAGE
               LAYOUT_AddChild, OBJ(OID_PREVIEWS) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
                 //LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
                 LAYOUT_HorizAlignment, LALIGN_CENTER,
                 LAYOUT_SpaceOuter,  FALSE,
                 LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
                   SPACE_MinWidth, 10,
                 TAG_DONE),
                 //CHILD_WeightedWidth, 0,
                 LAYOUT_AddChild, OBJ(OID_PREVIEW_BTN) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
                   GA_ID,         OID_PREVIEW_BTN,
                   GA_RelVerify,  TRUE,
                   GA_Underscore, 0,
                   //GA_Text,       "RUN GAME",
                   BUTTON_BevelStyle,  BVS_THIN,
                   //BUTTON_Transparent, TRUE,
                   //BUTTON_BackgroundPen, BLOCKPEN,
                   //BUTTON_FillPen,       BLOCKPEN,
                   BUTTON_RenderImage, OBJ(OID_PREVIEW_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
                    //IA_Scalable, TRUE,
                    BITMAP_Screen, screen,
                    //BITMAP_Masking, TRUE,
                   TAG_DONE),
                 TAG_DONE),
                 CHILD_MaxWidth, 316+2, // pixels width of preview + button border
                 CHILD_MinWidth, 316+2, // pixels width of preview + button border
                 CHILD_MaxHeight, 224+2, // pixels height of preview + button border
                 CHILD_MinHeight, 224+2, // pixels height of preview + button border
                 //CHILD_NoDispose, TRUE,
                 LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
                   SPACE_MinWidth, 10,
                 TAG_DONE),
                 //CHILD_WeightedWidth, 0,
               TAG_DONE), // END of BUTTON/IMAGE
               CHILD_WeightedHeight, 0,
// SAVESTATES +TOTALROMS
               LAYOUT_AddChild, OBJ(OID_SAVESTATES) = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
                 //GA_ID,         OID_SAVESTATES,
                 //GA_RelVerify,  TRUE,
                 GA_Underscore, 0,
                 CHOOSER_Labels,   &savestates_list,
                 CHOOSER_Selected, 0,
               TAG_DONE),
               CHILD_Label, IIntuition->NewObject(LabelClass, NULL,// "label.image",
                LABEL_Text, GetString(&li, MSG_GUI_GENERAL_SAVESTATES),//"Load savestate",
               TAG_DONE),
               //CHILD_WeightedWidth, 0,

               LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
/*
               LAYOUT_AddChild, OBJ(OID_DONATE_BTN) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
                 GA_ID,        OID_DONATE_BTN,
                 GA_RelVerify, TRUE,
                 //GA_Text, "PayPal DONATION",
                 //BUTTON_Transparent, TRUE,
                 //BUTTON_BevelStyle,  BVS_NONE,
                 BUTTON_RenderImage, OBJ(OID_DONATE_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
                   BITMAP_SourceFile, DATAS"/paypal.png",
                   BITMAP_Screen,     screen,
                   BITMAP_Masking,    TRUE,
                 TAG_DONE),
               TAG_DONE),
               CHILD_WeightedWidth,  0,
               CHILD_WeightedHeight, 0,
*/
               LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),

               LAYOUT_AddChild, OBJ(OID_TOTALROMS) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
                 //GA_ID,         OID_TOTALROMS,
                 //GA_RelVerify,  TRUE,
                 GA_ReadOnly,   TRUE,
                 GA_Underscore, 0,
                 GA_Text,       GetString(&li, MSG_GUI_GENERAL_TOTALROMS),
                 BUTTON_Justification, BCJ_LEFT,
                 BUTTON_BevelStyle,    BVS_NONE,
                 BUTTON_Transparent,   TRUE,
               TAG_DONE),
               //CHILD_WeightedWidth,  0,
               CHILD_WeightedHeight, 0,

             TAG_DONE), // END of BUTTON/IMAGE + TOTALROMS
             CHILD_WeightedWidth, 40,

            TAG_DONE), // END of GENERAL page/tab
            //CHILD_WeightedHeight, 0,

#endif
