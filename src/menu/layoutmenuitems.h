/*
** layoutmenuitems.h
** Control items for option menus
**
**---------------------------------------------------------------------------
** Copyright 2016 Derek Sanchez
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

//=============================================================================
//
// base class for menu items
//
//=============================================================================

#include "v_video.h"
#include "v_font.h"
#include "cmdlib.h"
#include "gstrings.h"
#include "g_level.h"
#include "gi.h"
#include "v_palette.h"
#include "d_gui.h"
#include "d_event.h"
#include "c_dispatch.h"
#include "c_console.h"
#include "c_cvars.h"
#include "c_bind.h"
#include "gameconfigfile.h"
#include "menu/menu.h"
#include "p_acs.h"



//=============================================================================
//
// base class for menu items
//
//=============================================================================

class FLayoutMenuItem : public FListMenuItem
{
protected:
	char *mLabel;
	bool mCentered;

public:

	FLayoutMenuItem(int xpos = 0, int ypos = 0, FName action = NAME_None)
		: FListMenuItem(xpos, ypos, action)
	{

		//mLabel = copystring(text);
		//mCentered = center;
	}

	//~FLayoutMenuItem();
	/*virtual int Draw(FLayoutMenuDescriptor *desc, int y, int indent, bool selected);
	virtual bool Selectable();
	virtual int GetIndent();
	*/
	//virtual bool MouseEvent(int type, int x, int y);
};

/*
FLayoutMenuItem::~FLayoutMenuItem()
{
}


int FLayoutMenuItem::Draw(FLayoutMenuDescriptor *desc, int y, int indent, bool selected)
{
	return indent;
}


// [DS] I Don't think this will be necessary.
int  FLayoutMenuItem::GetIndent()
{
	return mCentered ? 0 : SmallFont->StringWidth(mLabel);
}

bool FLayoutMenuItem::Selectable()
{
	return false;
}



bool FLayoutMenuItem::MouseEvent(int type, int x, int y)
{
	return false;
}
*/

class FLayoutMenuItemStaticPatch : public FLayoutMenuItem
{
protected:
	FTextureID mTexture;
	bool mCentered;

public:
	FLayoutMenuItemStaticPatch(int x, int y, FTextureID patch, bool centered);
	void Drawer(bool selected);
};

class FLayoutMenuItemStaticText : public FLayoutMenuItem
{
protected:
	const char *mText;
	FFont *mFont;
	EColorRange mColor;
	bool mCentered;

public:
	FLayoutMenuItemStaticText(int x, int y, const char *text, FFont *font, EColorRange color, bool centered);
	~FLayoutMenuItemStaticText();
	void Drawer(bool selected);
};

//=============================================================================
//
// selectable items
//
//=============================================================================

class FLayoutMenuItemSelectable : public FLayoutMenuItem
{
protected:
	int mHotkey;
	int mWidth;
	int mHeight;
	int mParam;

public:
	FLayoutMenuItemSelectable(int x, int y, FName childmenu, int mParam = -1);
	bool CheckCoordinate(int x, int y);
	bool Selectable();
	bool CheckHotkey(int c);
	bool Activate();
	bool MouseEvent(int type, int x, int y);
	FName GetAction(int *pparam);
};

class FLayoutMenuItemText : public FLayoutMenuItemSelectable
{
protected:
	const char *mText;
	FFont *mFont;
	EColorRange mColor;
	EColorRange mColorSelected;
public:
	FLayoutMenuItemText(int x, int y, int hotkey, const char *text, FFont *font, EColorRange color, EColorRange color2, FName child, int param = 0);
	~FLayoutMenuItemText();
	bool CheckCoordinate(int x, int y);
	void Drawer(bool selected);
	int GetWidth();
};

class FLayoutMenuItemPatch : public FLayoutMenuItemSelectable
{
protected:
	FTextureID mTexture;
public:
	FLayoutMenuItemPatch(int x, int y, int width, int height, int hotkey, FTextureID patch, FName child, int param = 0);
	bool CheckCoordinate(int x, int y);
	void Drawer(bool selected);
	int GetWidth();
};


class FLayoutMenuItemGlobalText : public FLayoutMenuItemText
{
protected:
	bool enabled;
	int global;		// The index of an ACS Global Variable.
	int idx;		// The index of the array within that global (optional).
	int gComparator; // The offset within the ACS global array that has to be > 0 for this item to be enabled. Calculated by multiplying by consoleplayer.
	const char *mDisabledText;
public:
	FLayoutMenuItemGlobalText(int x, int y, int hotkey, const char *text, const char *disabledText, FFont *font, EColorRange color, EColorRange color2, FName action, int globalVar, int indexOffset, int param = 0);
	//~FLayoutMenuItemGlobalText();
	bool CheckCoordinate(int x, int y);
	void Drawer(bool selected);
	void Ticker();
	bool Selectable();
	int GetWidth();

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		C_DoCommand(mAction);
		return true;
	}
};

class FLayoutMenuItemGlobalSwitchText : public FLayoutMenuItemGlobalText
{
	int mSwitchGlobal;
	int mSwitchComparator;
	bool locked;
	const char *mLockedText;
public:
	FLayoutMenuItemGlobalSwitchText(int x, int y, int hotkey, const char *text, const char *disabledText, const char *lockedText, FFont *font, EColorRange color, EColorRange color2, FName action, int globalVar, int indexOffset, int switchGlobal, int switchComparator, int param = 0);
	void Ticker();
	void Drawer(bool selected);
	//bool CheckCoordinate(int x, int y);
};


//=============================================================================
//
// [DS] patch that is enabled depending on the value of an ACS global variable.
//
//=============================================================================
class FLayoutMenuItemGlobalPatch : public FLayoutMenuItemPatch
{
	int global;		// The index of an ACS Global Variable.
	int idx;		// The index of the array within that global (optional).
	int gComparator; // The value of the ACS Global has to be equal to or greater than this value for the item to be enabled.
	//int idx; // Which player-offset index is used for the comparison. If none is provided, it will be zero.
	int gLockGlobal;
	int lowerIdx; // The value of the ACS Global has to be greater that this value for the item to be unlocked, or shown at all.
	FTextureID mTextureDisabled;
protected:
	bool enabled;
	bool locked;
public:
	FLayoutMenuItemGlobalPatch(int x, int y, int width, int height, int hotkey, FTextureID enabledPatch, FTextureID disabledPatch, FName child, int globalVar, int comparator = 1, int param = 0, int lockGlobal = 0, int lowerIndex = 0);
	void Drawer(bool selected);
	void Ticker();
	bool Selectable();
	int GetWidth();
	bool CheckCoordinate(int x, int y);
};

class FLayoutMenuItemGlobalSubmenuPatch : public FLayoutMenuItemGlobalPatch
{
public:
	int global;
	int gComparator;
	void Ticker();
	FLayoutMenuItemGlobalSubmenuPatch(const char *menu, int x, int y, int width, int height, int hotkey, FTextureID enabledPatch, FTextureID disabledPatch, FName child, int globalvar, int comparator = 1, int param = 0)
		: FLayoutMenuItemGlobalPatch(x, y, width, height, hotkey, enabledPatch, disabledPatch, child, globalvar, comparator, param)
	{
		global = globalvar;
		gComparator = comparator;
		mAction = menu;
	}

	bool CheckCoordinate(int x, int y);


	bool Selectable()
	{
		return enabled;
	}

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		M_SetMenu(mAction, mParam);
		return true;
	}
};

class FLayoutMenuItemGlobalCommandPatch : public FLayoutMenuItemGlobalPatch
{
public:
	FLayoutMenuItemGlobalCommandPatch(const char *command, int x, int y, int width, int height, int hotkey, FTextureID enabledPatch, FTextureID disabledPatch, FName child, int globalvar, int comparator = 1, int param = 0)
		: FLayoutMenuItemGlobalPatch(x, y, width, height, hotkey, enabledPatch, disabledPatch, child, globalvar, comparator, param)
	{
		mAction = command;
	}
	bool CheckCoordinate(int x, int y);

	bool Selectable()
	{
		return mEnabled;
	}

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		C_DoCommand(mAction);
		return true;
	}
};

//=============================================================================
//
// opens a submenu, action is a submenu name
//
//=============================================================================

class FLayoutMenuItemSubmenu : public FLayoutMenuItemText
{
	int mParam;
public:
	FLayoutMenuItemSubmenu(const char *menu, int x, int y, int hotkey, const char *text, FFont *font, EColorRange color, EColorRange color2)
		: FLayoutMenuItemText(x, y, hotkey, text, font, color, color2, NAME_None, 0)
	{
		mParam = 0;
		mAction = menu;
	}

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		M_SetMenu(mAction, mParam);
		return true;
	}
};

//=============================================================================
//
// opens a submenu and displays a patch, action is a submenu name
//
//=============================================================================

class FLayoutMenuItemPatchSubmenu : public FLayoutMenuItemPatch
{
	int mParam;
public:
	FLayoutMenuItemPatchSubmenu(const char *menu, int x, int y, int width, int height, int hotkey, FTextureID patch, FName child, int param)
		: FLayoutMenuItemPatch(x, y, width, height, hotkey, patch, child, param)
	{
		mParam = 0;
		mAction = menu;
	}

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		M_SetMenu(mAction, mParam);
		return true;
	}
};

//=============================================================================
//
// Executes a CCMD, action is a CCMD name
//
//=============================================================================

class FLayoutMenuItemCommand : public FLayoutMenuItemSubmenu
{
public:
	FLayoutMenuItemCommand(const char *action, int x, int y, int hotkey, const char *text, FFont *font, EColorRange color, EColorRange color2)
		: FLayoutMenuItemSubmenu(action, x, y, hotkey, text, font, color, color2)
	{
		mAction = action;
	}

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		C_DoCommand(mAction);
		return true;
	}

};


//=============================================================================
//
// Executes a named ACS script, action is a script name
//
//=============================================================================

class FLayoutMenuItemACSCommand : public FLayoutMenuItemSubmenu
{
public:
	FLayoutMenuItemACSCommand(const char *action, int x, int y, int hotkey, const char *text, FFont *font, EColorRange color, EColorRange color2)
		: FLayoutMenuItemSubmenu(action, x, y, hotkey, text, font, color, color2)
	{
		mAction = action;
	}

	bool Activate()
	{
		S_Sound(CHAN_VOICE | CHAN_UI, "menu/choose", snd_menuvolume, ATTN_NONE);
		const char *cmd = "pukename ";
		char* result; result = (char*)calloc(strlen(cmd) + strlen(mAction) + 1, sizeof(char));
		strcpy(result, cmd);
		strcat(result, mAction);

		C_DoCommand(result);
		return true;
	}

};