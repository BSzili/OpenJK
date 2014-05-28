/*
 * Copyright (C) 2012 Szilard Biro
 * Copyright (C) 2002 Jarmo Laakkonen and Hans-Joerg Frieden
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * =======================================================================
 *
 * AHI sound driver
 *
 * =======================================================================
 */

#include <string.h>
//#include <exec/exec.h>
#include <devices/ahi.h>
#include <proto/exec.h>
#ifdef __amigaos4__
// ugly hack to make the code compile with -fsigned-char
#include <exec/interfaces.h>
#undef __cplusplus
#endif
#include <proto/ahi.h>
#ifdef __MORPHOS__
#include <utility/hooks.h>
#else
#include <SDI/SDI_hook.h>
#endif

#include "qcommon/q_shared.h"
#include "client/client.h"
#include "client/snd_local.h"

struct AHIChannelInfo
{
	struct AHIEffChannelInfo aeci;
	ULONG offset;
};

struct Library *AHIBase = NULL;
#ifdef __amigaos4__
struct AHIIFace *IAHI = NULL;
#endif
static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIio = NULL;
static BYTE AHIDevice = -1;
static struct AHIAudioCtrl *actrl = NULL;
static ULONG rc = 1;
static struct AHIChannelInfo info;
static struct Hook EffectHook;
static byte *dmabuf = NULL;

#ifdef __MORPHOS__
static void EffectFunction()
{
	struct Hook *hook = (struct Hook *)REG_A0;
	struct AHIEffChannelInfo *aeci = (struct AHIEffChannelInfo *)REG_A1;
	hook->h_Data = (APTR)aeci->ahieci_Offset[0];
}
static struct EmulLibEntry EffectFunc =
{
	TRAP_LIB, 0, (void (*)(void))EffectFunction
};
#else
HOOKPROTO(EffectFunc, void, struct AHIAudioCtrl *actrl, struct AHIEffChannelInfo *aeci)
{
	hook->h_Data = (APTR)aeci->ahieci_Offset[0];
}
#endif

void SNDDMA_Shutdown(void)
{
	if (actrl)
	{
		info.aeci.ahie_Effect = AHIET_CHANNELINFO | AHIET_CANCEL;
		AHI_SetEffect(&info, actrl);
		AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_END);
	}

	if (rc == 0)
	{
		AHI_UnloadSound(0, actrl);
		rc = 1;
	}

	if (dmabuf)
	{
		free(dmabuf);
		dmabuf = NULL;
	}

	if (actrl)
	{
		AHI_FreeAudio(actrl);
		actrl = NULL;
	}

	if (AHIDevice == 0)
	{
		CloseDevice((struct IORequest *)AHIio);
		AHIDevice = -1;
	}

	if (AHIio)
	{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IAHI);
#endif
		DeleteIORequest((struct IORequest *)AHIio);
		AHIio = NULL;
	}

	if (AHImp)
	{
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}

	memset ((void *)&dma, 0, sizeof (dma));
}

qboolean SNDDMA_Init(void)
{
	struct AHISampleInfo sample;
	char name[256];
	int speed;
	int mixspeed;
	int buflen;

	info.aeci.ahieci_Channels = 1;
	info.aeci.ahieci_Func = &EffectHook;
	info.aeci.ahie_Effect = AHIET_CHANNELINFO;
	EffectHook.h_Data = 0;
	EffectHook.h_Entry = (HOOKFUNC)&EffectFunc;

	if (s_khz->integer == 44) 
		speed = 44100;
	else if (s_khz->integer == 22) 
		speed = 22050;
	else if (s_khz->integer == 11)
		speed = 11025;
	else
		speed = 22050;

	if ((AHImp = CreateMsgPort()) == NULL)
	{
		SNDDMA_Shutdown();
		Com_Printf("ERROR: Can't create AHI message port\n");
		return qfalse;
	}

	if ((AHIio = (struct AHIRequest *) CreateIORequest(AHImp, sizeof(struct AHIRequest))) == NULL)
	{
		SNDDMA_Shutdown();
		Com_Printf("ERROR: Can't create AHI io request\n");
		return qfalse;
	}

	AHIio->ahir_Version = 4;

	if ((AHIDevice = OpenDevice((STRPTR)"ahi.device", AHI_NO_UNIT, (struct IORequest *)AHIio, 0)) != 0)
	{
		SNDDMA_Shutdown();
		Com_Printf("Can't open ahi.device version 4\n");
		return qfalse;
	}

	AHIBase = (struct Library *) AHIio->ahir_Std.io_Device;
#ifdef __amigaos4__
	IAHI = (struct AHIIFace *)GetInterface(AHIBase, (STRPTR)"main", 1, NULL);
#endif

	if ((actrl = AHI_AllocAudio(AHIA_AudioID, AHI_DEFAULT_ID,
						AHIA_MixFreq, speed,
						AHIA_Channels, 1,
						AHIA_Sounds, 1,
						TAG_END)) == NULL)
	{
		SNDDMA_Shutdown();
		Com_Printf("Can't allocate audio\n");
		return qfalse;
	}

	AHI_GetAudioAttrs(AHI_INVALID_ID, actrl, 
				AHIDB_BufferLen, sizeof(name),
				AHIDB_Name, (IPTR)&name,
				TAG_END);

	AHI_ControlAudio(actrl, AHIC_MixFreq_Query, (IPTR)&mixspeed, TAG_END);
	//buflen = 16384 * (speed / 11025) * (ahibits / 8) * ahichannels;
	buflen = (65536/2) * (mixspeed / 11025);
#warning Is this ok?!
	//if (mixspeed == 11025 || mixspeed == 22050 || mixspeed == 44100)
	//	speed = mixspeed;

	if ((dmabuf = (byte *)calloc(1, buflen)) == NULL)
	{
		SNDDMA_Shutdown();
		Com_Printf("Can't allocate AHI dma buffer\n");
		return qfalse;
	}

	dma.buffer = dmabuf;
	dma.channels = 2; // ahichannels
	dma.speed = speed;
	dma.samplebits = 16; // ahibits
	dma.samples = buflen / (dma.samplebits / 8);
	dma.submission_chunk = 1;

	sample.ahisi_Type = AHIST_S16S;
	sample.ahisi_Address = (APTR)dmabuf;
	sample.ahisi_Length = buflen / AHI_SampleFrameSize(AHIST_S16S);

	if ((rc = AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample, actrl)) != 0)
	{
		SNDDMA_Shutdown();
		Com_Printf("Can't load sound\n");
		return qfalse;
	}
 
	if (AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_END) != 0)
	{
		SNDDMA_Shutdown();
		Com_Printf("Can't start playback\n");
		return qfalse;
	}

	Com_Printf("Using AHI mode \"%s\" for audio output\n", name);
	Com_Printf("Channels: %d bits: %d frequency: %d\n", dma.channels, dma.samplebits, /*dma.speed*/mixspeed);

	AHI_Play(actrl,
			AHIP_BeginChannel, 0,
			AHIP_Freq, speed,
			AHIP_Vol, 0x10000,
			AHIP_Pan, 0x8000,
			AHIP_Sound, 0,
			AHIP_EndChannel, NULL,
			TAG_END);

	AHI_SetEffect(&info, actrl);

	return qtrue;
}

int SNDDMA_GetDMAPos(void)
{
	return ((int)EffectHook.h_Data * dma.channels);
}

void SNDDMA_Submit(void)
{
	// unused
}

void SNDDMA_BeginPainting(void)
{
	// unused
}
