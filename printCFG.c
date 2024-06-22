;/*
gcc printCFG.c -Wall -lauto -o printCFG
quit

USAGE:
printCFG 1 -> prints keywords
printCFG   -> prints keywords values
*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>


int main(int argc, char *argv[])
{
	int32 i = 0, res_int;
	BPTR fhConfFile = IDOS->FOpen("snes9X.cfg", MODE_OLDFILE, 0);

	IDOS->PutStr("USAGE:\n\tprintCFG 1 -> prints keywords\n\tprintCFG   -> prints keywords values\n\n");

	if(fhConfFile != ZERO) {
		struct FReadLineData *frld = IDOS->AllocDosObjectTags(DOS_FREADLINEDATA, 0);

		while(IDOS->FReadLine(fhConfFile, frld) > 0) {
			if(frld->frld_LineLength > 1) {
//IDOS->Printf("Line is %ld bytes: %s", frld->frld_LineLength, frld->frld_Line);
				if(frld->frld_Line[0] != '#') { // bypass comment '#' lines
					char kword[32] = "";
					int32 pos = IDOS->SplitName( frld->frld_Line, ' ', kword, 0, sizeof(kword) ); // get KEYWORD without VALUE ("frameskip 1")
					if(argc == 2) { IDOS->Printf("\"%s\", ",kword); ++i; }
					else if(IDOS->StrToLong(frld->frld_Line+pos, &res_int) != -1) { IDOS->Printf("%ld, ",res_int); ++i; }
				} // END if(frld->frld_Line[0]..
			} // END if(frld->frld_LineLength..
		} // END while(IDOS->FReadLine(f..
IDOS->Printf("\ntotal items: %ld\n",i);

		IDOS->FreeDosObject(DOS_FREADLINEDATA, frld);
		IDOS->FClose(fhConfFile);
	} // END if(fhConfFile..

	return RETURN_OK;
}
