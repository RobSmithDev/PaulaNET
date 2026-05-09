#include "l_option.h"
#include <stdarg.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#define NO_INLINE_STDARG
#include <proto/intuition.h>
#undef NO_INLINE_STDARG

extern struct ExecBase *SysBase;
extern struct IntuitionBase *IntuitionBase;

// Copies tags from stack to memory
struct TagItem *CopyTagsToMem(Tag first,va_list param)
{
	int i,cnt = 1; // for the final tag
	va_list param2;
	Tag tag = first;
	struct TagItem *tagItem;

	// Count number of tags
	va_copy(param2,param);

	while(tag != TAG_DONE)
	{
		va_arg(param2,ULONG); // ti_Data
		tag = (Tag)va_arg(param2,Tag); // ti_Tag
		cnt++;
	}

	va_end(param2);

	// Copy all tags to memory
	if ((tagItem = (struct TagItem *)AllocVec(cnt*sizeof(struct TagItem),MEMF_ANY)))
	{
		tag = first;

		for(i=0;i < cnt;i++)
		{
			tagItem[i].ti_Tag = tag;

			if(tag == TAG_DONE)
				break;

			tagItem[i].ti_Data = (ULONG)va_arg(param,ULONG); // ti_Data
			tag = (Tag)va_arg(param,Tag); // ti_Tag	
		}
	}

	return(tagItem);
}

// Frees the memory allocated to store tags
struct TagItem *FreeTagMem(struct TagItem *tagItem)
{
	FreeVec((APTR)tagItem);
}

// We need to redefine some Intuition functions because gcc does not accept parenthesis and accolades closing in reverse order (which may happen for ReAction GUI)
struct Window *OpenWindowTags(CONST struct NewWindow *newWindow, ULONG tag1Type, ...)
{
	struct Window * ret = NULL;
	va_list param;
	struct TagItem *tagItem;

	va_start(param,tag1Type);

	if((tagItem = CopyTagsToMem(tag1Type,param))) 
	{
		ret = OpenWindowTagList(newWindow,(struct TagItem *)tagItem);
		FreeTagMem(tagItem);
	}

	va_end(param);

	return(ret);
}

struct Screen *OpenScreenTags(CONST struct NewScreen *newScreen, ULONG tag1Type, ...)
{
	struct Screen * ret = NULL;
	va_list param;
	struct TagItem *tagItem;

	va_start(param,tag1Type);

	if((tagItem = CopyTagsToMem(tag1Type,param))) 
	{
		ret = OpenScreenTagList(newScreen,(struct TagItem *)tagItem);
		FreeTagMem(tagItem);
	}

	va_end(param);

	return(ret);
}

APTR NewObject(struct IClass *classPtr, CONST_STRPTR classID, ULONG tag1, ...)
{
	APTR ret = NULL;
	va_list param;
	struct TagItem *tagItem;

	va_start(param,tag1);

	if((tagItem = CopyTagsToMem(tag1,param))) 
	{
		ret = NewObjectA(classPtr,classID,(struct TagItem *)tagItem);
		FreeTagMem(tagItem);
	}

	va_end(param);

	return(ret);
}

ULONG SetAttrs(APTR object, ULONG tag1, ...)
{
	ULONG ret = NULL;
	va_list param;
	struct TagItem *tagItem;

	va_start(param,tag1);

	if((tagItem = CopyTagsToMem(tag1,param))) 
	{
		ret = SetAttrsA(object,(struct TagItem *)tagItem);
		FreeTagMem(tagItem);
	}

	va_end(param);

	return(ret);
}

ULONG SetGadgetAttrs(struct Gadget *gadget, struct Window *window, struct Requester *requester, ULONG tag1, ...)
{
	ULONG ret = NULL;
	va_list param;
	struct TagItem *tagItem;

	va_start(param,tag1);

	if((tagItem = CopyTagsToMem(tag1,param))) 
	{
		ret = SetGadgetAttrsA(gadget,window,requester,(struct TagItem *)tagItem);
		FreeTagMem(tagItem);
	}

	va_end(param);

	return(ret);
}

void SetWindowPointer(struct Window *win, ULONG tag1, ...)
{
	va_list param;
	struct TagItem *tagItem;

	va_start(param,tag1);

	if((tagItem = CopyTagsToMem(tag1,param))) 
	{
		SetWindowPointerA(win,(struct TagItem *)tagItem);
		FreeTagMem(tagItem);
	}

	va_end(param);
}

