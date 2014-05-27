/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <devices/input.h>
#ifdef __amigaos4__
#include <newmouse.h>
#else
#include <devices/rawkeycodes.h>
#endif
#include <intuition/intuition.h>
#ifdef __MORPHOS__
#include <intuition/intuitionbase.h>
#endif
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/keymap.h>
#include <SDI/SDI_interrupt.h>

#include "client/client.h"
#include "rd-vanilla/tr_local.h"
#include "sys/sys_local.h"

static struct Window *awindow = NULL;

static qboolean mouseinitialized = qfalse;
static qboolean	mouseactive = qfalse;

cvar_t	*in_mouse;
cvar_t	*in_joystick;

static struct Interrupt InputHandler;
static struct MsgPort *inputport = NULL;
static struct IOStdReq *inputreq = NULL;
static UWORD *pointermem = NULL;
static qboolean in_dograb = qtrue;

// ring buffer
#define MAXIMSGS 32
static struct InputEvent imsgs[MAXIMSGS];
static int imsglow;
static int imsghigh;
static int mx;
static int my;

static qboolean IN_AddEvent(struct InputEvent *coin)
{
	if ((imsghigh > imsglow && !(imsghigh == MAXIMSGS - 1 && imsglow == 0)) ||
		(imsghigh < imsglow && imsghigh != imsglow - 1) ||
		(imsglow == imsghigh))
	{
		CopyMem(coin, &imsgs[imsghigh], sizeof(imsgs[0]));
		imsghigh++;
		imsghigh %= MAXIMSGS;

		return qtrue;
	}

	return qfalse;
}

static struct InputEvent *IN_GetNextEvent(void)
{
	struct InputEvent *ie = NULL;

	if (imsglow != imsghigh)
	{
		ie = &imsgs[imsglow];
		imsglow++;
		imsglow %= MAXIMSGS;
	}

	return ie;
}

#ifdef __MORPHOS__
static struct InputEvent *IN_KeyboardHandlerFunc(void);
static struct EmulLibEntry IN_KeyboardHandler =
{
	TRAP_LIB, 0, (void (*)(void))IN_KeyboardHandlerFunc
};
static struct InputEvent *IN_KeyboardHandlerFunc()
{
	struct InputEvent *moo = (struct InputEvent *)REG_A0;
	//APTR id = (APTR)REG_A1;
#else
HANDLERPROTO(IN_KeyboardHandler, struct InputEvent *, struct InputEvent *moo, APTR id)
{
#endif
	struct InputEvent *coin;

	ULONG screeninfront;

	if (!awindow || !(awindow->Flags & WFLG_WINDOWACTIVE))
		return moo;

	coin = moo;

	if (awindow->WScreen)
	{
#ifdef __MORPHOS__
		if (IntuitionBase->LibNode.lib_Version > 50 || (IntuitionBase->LibNode.lib_Version == 50 && IntuitionBase->LibNode.lib_Revision >= 56))
			GetAttr(SA_Displayed, awindow->WScreen, &screeninfront);
		else
#endif
			screeninfront = (awindow->WScreen == IntuitionBase->FirstScreen);
	}
	else
		screeninfront = 1;

	do
	{
		// mouse buttons, mouse wheel and keyboard
		if (coin->ie_Class == IECLASS_RAWKEY ||
			((coin->ie_Class == IECLASS_RAWMOUSE || coin->ie_Class == IECLASS_NEWMOUSE) && coin->ie_Code != IECODE_NOBUTTON))
		{
			if (!IN_AddEvent(coin))
				Com_DPrintf(S_COLOR_YELLOW "WARNING: dropped input event\n");

			if ((coin->ie_Class == IECLASS_RAWMOUSE || coin->ie_Class == IECLASS_NEWMOUSE) && screeninfront && in_dograb)
				coin->ie_Code = IECODE_NOBUTTON;
		}

		// mouse movement
		if ((coin->ie_Class == IECLASS_RAWMOUSE) && screeninfront && in_dograb)
		{
			mx += coin->ie_position.ie_xy.ie_x;
			my += coin->ie_position.ie_xy.ie_y;
			coin->ie_position.ie_xy.ie_x = 0;
			coin->ie_position.ie_xy.ie_y = 0;
		}

		coin = coin->ie_NextEvent;
	} while (coin);

	return moo;
}

void IN_ActivateMouse (void)
{
	if (!mouseinitialized)
		return;

	if (mouseactive)
		return;

	mouseactive = qtrue;

	if (pointermem)
		SetPointer(awindow, pointermem, 16, 16, 0, 0);

	in_dograb = qtrue;
}

void IN_DeactivateMouse (void)
{
	if (!mouseinitialized)
		return;

	if (!mouseactive)
		return;

	ClearPointer(awindow);
	in_dograb = qfalse;

	mouseactive = qfalse;
}

void IN_StartupMouse (void)
{
	mouseinitialized = qtrue;
}

void IN_Init(void *windowData)
{
	awindow = (struct Window *)windowData;

	in_joystick = Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE);

	pointermem = (UWORD *)calloc(16, 16);
	/*if (pointermem)
		SetPointer(awindow, pointermem, 16, 16, 0, 0);*/

	mx = my = imsglow = imsghigh = 0;

	if ((inputport = CreateMsgPort()))
	{
		if ((inputreq = (IOStdReq *)CreateIORequest(inputport, sizeof(*inputreq))))
		{
			if (!OpenDevice((STRPTR)"input.device", 0, (struct IORequest *)inputreq, 0))
			{
				InputHandler.is_Node.ln_Type = NT_INTERRUPT;
				InputHandler.is_Node.ln_Pri = 100;
				InputHandler.is_Node.ln_Name = (STRPTR)"Quake3 input handler";
				InputHandler.is_Code = (void (*)())&IN_KeyboardHandler;
				inputreq->io_Data = (void *)&InputHandler;
				inputreq->io_Command = IND_ADDHANDLER;
				if (!DoIO((struct IORequest *)inputreq))
				{
					IN_StartupMouse();
					return;
				}
			}
		}
	}

	Sys_Error("Couldn't install input handler");
}

void IN_Shutdown(void)
{
	if (inputreq)
	{
		inputreq->io_Data = (void *)&InputHandler;
		inputreq->io_Command = IND_REMHANDLER;
		DoIO((struct IORequest *)inputreq);

		CloseDevice((struct IORequest *)inputreq);
		DeleteIORequest((struct IORequest *)inputreq);

		inputreq = NULL;
	}

	if (inputport)
	{
		DeleteMsgPort(inputport);
		inputport = NULL;
	}

	if (pointermem)
	{
		ClearPointer(awindow);
		free(pointermem);
		pointermem = NULL;
	}

	IN_DeactivateMouse();

	awindow = NULL;

	mouseinitialized = qfalse;
}

void IN_MouseMove(void)
{
	if (mx || my)
		Sys_QueEvent(0, SE_MOUSE, mx, my, 0, NULL);
	mx = my = 0;
}

byte keyconv[128] =
{
	'`', // 0
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0', // 10
	'-',
	'=',
	'\\',
	0,
	A_KP_0,
	'q',
	'w',
	'e',
	'r',
	't', // 20
	'y',
	'u',
	'i',
	'o',
	'p',
	'[',
	']',
	0,
	A_KP_1,
	A_KP_2, // 30
	A_KP_3,
	'a',
	's',
	'd',
	'f',
	'g',
	'h',
	'j',
	'k',
	'l', // 40
	';',
	'\'',
	'\\',
	0,
	A_KP_4,
	A_KP_5,
	A_KP_6,
	'<',
	'z',
	'x', // 50
	'c',
	'v',
	'b',
	'n',
	'm',
	',',
	'.',
	'\\',
	0,
	A_KP_PERIOD, // 60
	A_KP_7,
	A_KP_8,
	A_KP_9,
	A_SPACE,
	A_BACKSPACE,
	A_TAB,
	A_KP_ENTER,
	A_ENTER,
	A_ESCAPE,
	A_DELETE, // 70
	A_INSERT,
	A_PAGE_UP,
	A_PAGE_DOWN,
	A_KP_MINUS,
	A_F11,
	A_CURSOR_UP,
	A_CURSOR_DOWN,
	A_CURSOR_RIGHT,
	A_CURSOR_LEFT,
	A_F1, // 80
	A_F2,
	A_F3,
	A_F4,
	A_F5,
	A_F6,
	A_F7,
	A_F8,
	A_F9,
	A_F10,
	0, // 90
	0,
	/*A_KP_STAR*/A_DIVIDE,
	/*A_KP_MINUS*/A_MULTIPLY,
	A_KP_PLUS,
	/*A_F11*/0,
	A_SHIFT,
	A_SHIFT,
	A_CAPSLOCK,
	A_CTRL,
	A_ALT, // 100
	A_ALT,
	0,
	/*A_F12*/0,
	0,
	0,
	0,
	A_SCROLLLOCK,
	A_PRINTSCREEN,
	A_NUMLOCK,
	A_PAUSE, // 110
	A_F12,
	A_HOME,
	A_END,
	0,
	0,
	0,
	0,
	0,
	0,
	0, // 120
	0,
	0,
	0,
	0,
	0,
	0,
	0
};
#define MAX_KEYCONV (sizeof keyconv / sizeof keyconv[0])

void IN_ProcessEvents(void)
{
	struct IntuiMessage *intuimsg;
	struct InputEvent *inputev;
	int sym, state;
	unsigned u_sys_msecs;
	int mapped;

	if (!awindow)
		return;

    while ((intuimsg = (struct IntuiMessage *)GetMsg(awindow->UserPort)))
    {
		switch (intuimsg->Class)
		{
			case IDCMP_CHANGEWINDOW:
				//if (!r_fullscreen->integer)
				if (!Cvar_VariableIntegerValue("r_fullscreen"))
				{
					Cvar_SetValue("vid_xpos", awindow->LeftEdge);
					Cvar_SetValue("vid_ypos", awindow->TopEdge);
				}
				break;
			case IDCMP_CLOSEWINDOW:
				Cbuf_ExecuteText(EXEC_NOW, "quit Closed window\n");
				break;
#ifdef __amigaos4__
			// I can't get the wheel data with the input handler interrupt,
			// so I convert them into NewMouse events here
			case IDCMP_EXTENDEDMOUSE:
				if (intuimsg->Code == IMSGCODE_INTUIWHEELDATA)
				{
					struct IntuiWheelData *iwd = (struct IntuiWheelData *)intuimsg->IAddress;
					struct InputEvent nmwheel;

					if (!iwd->WheelY)
						continue;

					nmwheel.ie_Class = IECLASS_NEWMOUSE;

					if (iwd->WheelY < 0)
						nmwheel.ie_Code = NM_WHEEL_UP;
					else
						nmwheel.ie_Code = NM_WHEEL_DOWN;

					IN_AddEvent(&nmwheel);
				}
				break;
#endif
		}

		ReplyMsg((struct Message *)intuimsg);
	}

	u_sys_msecs = (unsigned)Sys_Milliseconds();

	while ((inputev = IN_GetNextEvent()))
	{
		sym = 0;
		state = -1;
		mapped = 0;

		switch (inputev->ie_Class)
		{
			case IECLASS_RAWKEY:
				switch (inputev->ie_Code & ~IECODE_UP_PREFIX)
				{
#ifdef __AROS__
					case RAWKEY_NM_WHEEL_UP:
						sym = A_MWHEELUP;
						break;
					case RAWKEY_NM_WHEEL_DOWN:
						sym = A_MWHEELDOWN;
						break;
#endif
					default:
						state = !(inputev->ie_Code & IECODE_UP_PREFIX);
						inputev->ie_Code &= ~IECODE_UP_PREFIX;

#ifdef _JK2EXE
						if (state && Key_GetCatcher() & KEYCATCH_CONSOLE)
#else
						if (state && Key_GetCatcher() & (KEYCATCH_CONSOLE | KEYCATCH_MESSAGE))
#endif
						{
							if (MapRawKey(inputev, (STRPTR)&mapped, sizeof(mapped), NULL) <= 0 || mapped > MAX_KEYS)
							{
								mapped = 0;
							}
							//printf("MAPPED: %x NAME '%s'\n", mapped, Key_KeynumToString(mapped));
						}

						if (inputev->ie_Code < MAX_KEYCONV)
						{
							sym = keyconv[inputev->ie_Code];
							//printf("SCAN: %d GAME %d NAME '%s'\n", inputev->ie_Code, sym, Key_KeynumToString(sym));
						}
						break;
				}
				break;
			case IECLASS_RAWMOUSE:
				state = !(inputev->ie_Code & IECODE_UP_PREFIX);
				switch (inputev->ie_Code & ~IECODE_UP_PREFIX)
				{
					case IECODE_LBUTTON:
						sym = A_MOUSE1;
						break;
					case IECODE_RBUTTON:
						sym = A_MOUSE2;
						break;
					case IECODE_MBUTTON:
						sym = A_MOUSE3;
						break;
				}
				break;
#if defined(__MORPHOS__) || defined(__amigaos4__)
			case IECLASS_NEWMOUSE:
				switch (inputev->ie_Code & ~IECODE_UP_PREFIX)
				{
					case NM_WHEEL_UP:
						sym = A_MWHEELUP;
						break;
					case NM_WHEEL_DOWN:
						sym = A_MWHEELDOWN;
						break;
					case NM_BUTTON_FOURTH:
						state = !(inputev->ie_Code & IECODE_UP_PREFIX);
						sym = A_MOUSE4;
						break;
				}
				break;
#endif
		}

		// the console and text input fields require SE_CHAR events
		if (mapped)
		{
			Sys_QueEvent(u_sys_msecs, SE_CHAR, mapped, qfalse, 0, NULL);
		}

		if (sym)
		{
			if (state != -1)
			{
				Sys_QueEvent(u_sys_msecs, SE_KEY, sym, state, 0, NULL);
			}
			else
			{
				Sys_QueEvent(u_sys_msecs, SE_KEY, sym, qtrue, 0, NULL);
				Sys_QueEvent(u_sys_msecs, SE_KEY, sym, qfalse, 0, NULL);
			}
		}
	}
}

void IN_Frame(void)
{
	qboolean loading;

	IN_ProcessEvents();

	if (!mouseinitialized)
		return;

	// If not DISCONNECTED (main menu) or ACTIVE (in game), we're loading
	loading = (qboolean)(cls.state != CA_DISCONNECTED && cls.state != CA_ACTIVE);

	if (!cls.glconfig.isFullscreen && (Key_GetCatcher() & KEYCATCH_CONSOLE))
	{
		// Console is down in windowed mode
		IN_DeactivateMouse();
	}
	else if (!cls.glconfig.isFullscreen && loading)
	{
		// Loading in windowed mode
		IN_DeactivateMouse();
	}
	else
		IN_ActivateMouse();

	IN_MouseMove();
}

void IN_Restart(void)
{
}
