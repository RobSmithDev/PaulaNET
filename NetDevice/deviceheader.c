#include <exec/resident.h>
#include <exec/nodes.h>
#include <exec/initializers.h>
#include <exec/libraries.h>
#ifdef HAVE_VERSION_H
#include "version.h"
#endif
#include "compiler.h"
#include "device.h"

#if  1

ASM LONG LibNull( void )
{
	return 0;
}

extern const char DeviceName[];
extern const char DeviceVersionString[];
extern const APTR DeviceInitTab[];

static const struct Resident _00RomTag = {
	RTC_MATCHWORD,
	( struct Resident* ) &_00RomTag,
	( struct Resident* ) &_00RomTag + 1,
	RTF_AUTOINIT,
	DEVICEVERSION,
	NT_DEVICE,
	0,
	(char*)DeviceName,
	(char*)DeviceVersionString+6,
	(APTR)DeviceInitTab
};
#endif



