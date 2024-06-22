#ifndef GUI_SETTINGS_MISC_H
#define GUI_SETTINGS_MISC_H

LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
  LAYOUT_BevelStyle, BVS_SBAR_VERT,//BVS_GROUP,
  LAYOUT_Label,      GetString(&li, MSG_GUI_SETTINGS_MISC_GROUP),//"Miscellaneous Options",

  LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),

             LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
              LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
              LAYOUT_HorizAlignment, LALIGN_RIGHT,
              LAYOUT_SpaceOuter,     TRUE,
//LAYOUT_BevelStyle, BVS_GROUP,
               LAYOUT_AddChild, OBJ(OID_FPS) = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
                //GA_ID,        OID_FPS,
                //GA_RelVerify, TRUE,
                GA_Text,      GetString(&li, MSG_GUI_SETTINGS_MISC_FPS),//"Show _FPS",
                CHECKBOX_TextPlace, PLACETEXT_LEFT,
               TAG_DONE),
               CHILD_WeightedWidth, 0,
               LAYOUT_AddChild, OBJ(OID_DISP_TIME) = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
                //GA_ID,   OID_DISP_TIME,
                //GA_RelVerify, TRUE,
                GA_Text, GetString(&li, MSG_GUI_SETTINGS_TIME),//"Display Time",
                CHECKBOX_TextPlace, PLACETEXT_LEFT,
               TAG_DONE),
               CHILD_WeightedWidth, 0,
               LAYOUT_AddChild, OBJ(OID_VSYNC) = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
                //GA_ID,        OID_VSYNC,
                //GA_RelVerify, TRUE,
                GA_Text,      GetString(&li, MSG_GUI_SETTINGS_VSYNC),//"VSync on Snes9x",
                CHECKBOX_TextPlace, PLACETEXT_LEFT,
               TAG_DONE),
               CHILD_WeightedWidth, 0,
               LAYOUT_AddChild, OBJ(OID_QWERTY) = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
                //GA_ID,        OID_QWERTY,
                //GA_RelVerify, TRUE,
                GA_Text, GetString(&li, MSG_GUI_SETTINGS_QWERTY),//"Qwerty",
                CHECKBOX_TextPlace, PLACETEXT_LEFT,
               TAG_DONE),
               CHILD_WeightedWidth, 0,
               LAYOUT_AddChild, OBJ(OID_OC_CPU) = IIntuition->NewObject(ChooserClass, NULL, //"checkbox.gadget",
                //GA_ID,         OID_OC_CPU,
                GA_Underscore, 0,
                CHOOSER_Labels,   &oc_cpu_list,
                CHOOSER_Selected, 0,
               TAG_DONE),
               CHILD_WeightedWidth, 0,
               CHILD_Label, IIntuition->NewObject(LabelClass, NULL,// "label.image",
                LABEL_Text, GetString(&li, MSG_GUI_SETTINGS_OVERCLOCK),//"Overclock _CPU Snes9x",
               TAG_DONE),
             TAG_DONE),
             CHILD_WeightedWidth, 0,

  LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),

TAG_DONE), // END of 'Miscellaneous Options'
             //CHILD_WeightedHeight, 0,

#endif
