/*
 * Copyright (C) 2005 Mark Olsen
 * Copyright (C) 2010 Krzysztof Smiechowicz
 * Copyright (C) 2013 Szilárd Biró
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#ifdef __amigaos4__
#include <proto/minigl.h>
#elif defined(__MORPHOS__)
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#include <tgl/gl.h>
#include <tgl/gla.h>
#else
#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#include <proto/mesa.h>
#endif

#include "rd-vanilla/tr_local.h"
#include "client/client.h"
#include "qcommon/q_shared.h"

// this little hack allows me to use this code in both in the MP and SP engine
#ifdef _JK2EXE
static refimport_t *riptr = &ri;
#else
#define riptr ri
#endif

cvar_t *vid_xpos;
cvar_t *vid_ypos;
bool g_bTextureRectangleHack = false;

static struct Window *awindow = NULL;
static struct Screen *screen = NULL;
#ifdef __AROS__
static AROSMesaContext context = NULL;
#define qwglGetProcAddress(x) AROSMesaGetProcAddress((GLubyte*)x)
#elif defined(__amigaos4__)
struct GLContextIFace *context = NULL;
#define qwglGetProcAddress(x) mglGetProcAddress(x)
#else // __MORPHOS__
GLContext *__tglContext = NULL;
static int context = 0;

static void stub_glMultiTexCoord2fARB(GLenum unit, GLfloat s, GLfloat t)
{
	glMultiTexCoord2fARB(unit, s, t);
}

static void stub_glActiveTextureARB(GLenum unit)
{
	glActiveTextureARB(unit);
}

static void stub_glClientActiveTextureARB(GLenum unit)
{
	glClientActiveTextureARB(unit);
}

static void stub_glLockArraysEXT(int a, int b)
{
	glLockArraysEXT(a, b);
}

static void stub_glUnlockArraysEXT()
{
	glUnlockArraysEXT();
}

void *qwglGetProcAddress(char *symbol)
{
	/* compiled vertex arrays */
	if (!strcmp(symbol, "glLockArraysEXT"))
		return (void *)stub_glLockArraysEXT;
	else if (!strcmp(symbol, "glUnlockArraysEXT"))
		return (void *)stub_glUnlockArraysEXT;

	/* ARB multitexture */
	else if (!strcmp(symbol, "glMultiTexCoord2fARB"))
		return (void *)stub_glMultiTexCoord2fARB;
	else if (!strcmp(symbol, "glActiveTextureARB"))
		return (void *)stub_glActiveTextureARB;
	else if (!strcmp(symbol, "glClientActiveTextureARB"))
		return (void *)stub_glClientActiveTextureARB;

	return NULL;
}
#endif

/* ARB multitexture */
void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum texture, GLfloat s, GLfloat t );
void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );

/* compiled vertex arrays */
void ( APIENTRY * qglLockArraysEXT)( int, int);
void ( APIENTRY * qglUnlockArraysEXT) ( void );

/* NV combiners */
void (APIENTRYP qglCombinerParameterfvNV) (GLenum pname,const GLfloat *params) = NULL;
void (APIENTRYP qglCombinerParameteriNV) (GLenum pname,GLint param) = NULL;
void (APIENTRYP qglCombinerInputNV) (GLenum stage,GLenum portion,GLenum variable,GLenum input,GLenum mapping,
		GLenum componentUsage) = NULL;
void (APIENTRYP qglCombinerOutputNV) (GLenum stage,GLenum portion,GLenum abOutput,GLenum cdOutput,GLenum sumOutput,
		GLenum scale, GLenum bias,GLboolean abDotProduct,GLboolean cdDotProduct,
		GLboolean muxSum) = NULL;
void (APIENTRYP qglFinalCombinerInputNV) (GLenum variable,GLenum input,GLenum mapping,GLenum componentUsage) = NULL;

/* ARB shaders */
PFNGLPROGRAMSTRINGARBPROC qglProgramStringARB = NULL;
PFNGLBINDPROGRAMARBPROC qglBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC qglDeleteProgramsARB = NULL;
PFNGLGENPROGRAMSARBPROC qglGenProgramsARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC qglProgramEnvParameter4fARB = NULL;

bool GL_CheckForExtension(const char *ext)
{
	const char *ptr = Q_stristr( glConfig.extensions_string, ext );
	if (ptr == NULL)
		return false;
	//ptr += strlen(ext);
	//return ((*ptr == ' ') || (*ptr == '\0'));  // verify it's complete string.
	return true;
}

extern bool g_bDynamicGlowSupported;
static void GLimp_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		Com_Printf ("*** IGNORING OPENGL EXTENSIONS ***\n" );
		g_bDynamicGlowSupported = false;
		riptr->Cvar_Set( "r_DynamicGlow","0" );
		return;
	}

	Com_Printf ("Initializing OpenGL extensions\n" );

	// Use modern texture compression extensions
	if ( GL_CheckForExtension( "ARB_texture_compression" ) && GL_CheckForExtension( "EXT_texture_compression_s3tc" ) )
	{
		if ( r_ext_compressed_textures->value )
		{
			glConfig.textureCompression = TC_S3TC_DXT;
			Com_Printf ( "...using GL_EXT_texture_compression_s3tc\n" );
		}
		else
		{
			glConfig.textureCompression = TC_NONE;
			Com_Printf ( "...ignoring GL_EXT_texture_compression_s3tc\n" );
		}
	}
	// Or check for old ones
	else if ( GL_CheckForExtension( "GL_S3_s3tc" ) )
	{
		if ( r_ext_compressed_textures->value )
		{
			glConfig.textureCompression = TC_S3TC;
			Com_Printf ( "...using GL_S3_s3tc\n" );
		}
		else
		{
			glConfig.textureCompression = TC_NONE;
			Com_Printf ( "...ignoring GL_S3_s3tc\n" );
		}
	}
	else
	{
		glConfig.textureCompression = TC_NONE;
		Com_Printf ( "...no texture compression found\n" );
	}

	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qfalse;
#ifndef __MORPHOS__
	if ( GL_CheckForExtension( "EXT_texture_env_add" ) )
	{
		if ( r_ext_texture_env_add->integer )
		{
			glConfig.textureEnvAddAvailable = qtrue;
			Com_Printf ("...using GL_EXT_texture_env_add\n" );
		}
		else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			Com_Printf ("...ignoring GL_EXT_texture_env_add\n" );
		}
	}
	else
#endif
	{
		Com_Printf ("...GL_EXT_texture_env_add not found\n" );
	}

	// GL_EXT_texture_filter_anisotropic
	glConfig.maxTextureFilterAnisotropy = 0;
	if ( GL_CheckForExtension( "EXT_texture_filter_anisotropic" ) )
	{
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF	//can't include glext.h here ... sigh
		qglGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glConfig.maxTextureFilterAnisotropy );
		Com_Printf ("...GL_EXT_texture_filter_anisotropic available\n" );

		if ( r_ext_texture_filter_anisotropic->integer > 1 )
		{
			Com_Printf ("...using GL_EXT_texture_filter_anisotropic\n" );
		}
		else
		{
			Com_Printf ("...ignoring GL_EXT_texture_filter_anisotropic\n" );
		}
		riptr->Cvar_SetValue( "r_ext_texture_filter_anisotropic_avail", glConfig.maxTextureFilterAnisotropy );
		if ( r_ext_texture_filter_anisotropic->value > glConfig.maxTextureFilterAnisotropy )
		{
			riptr->Cvar_SetValue( "r_ext_texture_filter_anisotropic_avail", glConfig.maxTextureFilterAnisotropy );
		}
	}
	else
	{
		Com_Printf ("...GL_EXT_texture_filter_anisotropic not found\n" );
		riptr->Cvar_Set( "r_ext_texture_filter_anisotropic_avail", "0" );
	}

	// GL_EXT_clamp_to_edge
#ifndef __MORPHOS__
	glConfig.clampToEdgeAvailable = qfalse;
	if ( GL_CheckForExtension( "GL_EXT_texture_edge_clamp" ) )
#endif
	{
		glConfig.clampToEdgeAvailable = qtrue;
		Com_Printf ( "...Using GL_EXT_texture_edge_clamp\n" );
	}

	// GL_ARB_multitexture
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	if ( GL_CheckForExtension( "GL_ARB_multitexture" ) )
	{
		if ( r_ext_multitexture->value )
		{
			qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) qwglGetProcAddress( "glMultiTexCoord2fARB" );
			qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) qwglGetProcAddress( "glActiveTextureARB" );
			qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) qwglGetProcAddress( "glClientActiveTextureARB" );

			if ( qglActiveTextureARB )
			{
				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &glConfig.maxActiveTextures );
				Com_Printf ( "...using GL_ARB_multitexture\n" );
			}
			else
			{
				Com_Printf ( "...blind search for ARB_multitexture failed\n" );
			}
		}
		else
		{
			Com_Printf ( "...ignoring GL_ARB_multitexture\n" );
		}
	}
	else
	{
		Com_Printf ( "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	if ( GL_CheckForExtension( "GL_EXT_compiled_vertex_array" ) )
	{
		if ( r_ext_compiled_vertex_array->value )
		{
			Com_Printf ( "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( int, int ) ) qwglGetProcAddress( "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) qwglGetProcAddress( "glUnlockArraysEXT" );
			if (!qglLockArraysEXT || !qglUnlockArraysEXT) {
				Com_Error (ERR_FATAL, "bad getprocaddress");
			}
		}
		else
		{
			Com_Printf ( "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}
	else
	{
		Com_Printf ( "...GL_EXT_compiled_vertex_array not found\n" );
	}

	// NOTE: Vertex and Fragment Programs are very dependant on each other - this is actually a
	// good thing! So, just check to see which we support (one or the other) and load the shared
	// function pointers. ARB rocks!

	// Vertex Programs.
	bool bARBVertexProgram = false;
	if ( GL_CheckForExtension( "GL_ARB_vertex_program" ) )
	{
		bARBVertexProgram = true;
	}
	else
	{
		bARBVertexProgram = false;
		Com_Printf ("...GL_ARB_vertex_program not found\n" );
	}

	// Fragment Programs.
	bool bARBFragmentProgram = false;
	if ( GL_CheckForExtension( "GL_ARB_fragment_program" ) )
	{
		bARBFragmentProgram = true;
	}
	else
	{
		bARBFragmentProgram = false;
		Com_Printf ("...GL_ARB_fragment_program not found\n" );
	}

	// If we support one or the other, load the shared function pointers.
	if ( bARBVertexProgram || bARBFragmentProgram )
	{
		qglProgramStringARB					= (PFNGLPROGRAMSTRINGARBPROC)  qwglGetProcAddress("glProgramStringARB");
		qglBindProgramARB					= (PFNGLBINDPROGRAMARBPROC)    qwglGetProcAddress("glBindProgramARB");
		qglDeleteProgramsARB				= (PFNGLDELETEPROGRAMSARBPROC) qwglGetProcAddress("glDeleteProgramsARB");
		qglGenProgramsARB					= (PFNGLGENPROGRAMSARBPROC)    qwglGetProcAddress("glGenProgramsARB");
		qglProgramEnvParameter4fARB			= (PFNGLPROGRAMENVPARAMETER4FARBPROC)    qwglGetProcAddress("glProgramEnvParameter4fARB");

		// Validate the functions we need.
		if ( !qglProgramStringARB || !qglBindProgramARB || !qglDeleteProgramsARB || !qglGenProgramsARB ||
			 !qglProgramEnvParameter4fARB )
		{
			bARBVertexProgram = false;
			bARBFragmentProgram = false;
			qglGenProgramsARB = NULL;	//clear ptrs that get checked
			qglProgramEnvParameter4fARB = NULL;
			Com_Printf ("...ignoring GL_ARB_vertex_program\n" );
			Com_Printf ("...ignoring GL_ARB_fragment_program\n" );
		}
	}

	// Figure out which texture rectangle extension to use.
	bool bTexRectSupported = false;
	if ( GL_CheckForExtension( "GL_NV_texture_rectangle" ) || GL_CheckForExtension( "GL_EXT_texture_rectangle" ) )
	{
		bTexRectSupported = true;
	}

	// Only allow dynamic glows/flares if they have the hardware
	if ( bTexRectSupported && bARBVertexProgram && qglActiveTextureARB && glConfig.maxActiveTextures >= 4 &&
		bARBFragmentProgram )
	{
		g_bDynamicGlowSupported = true;
		// this would overwrite any achived setting gwg
		// riptr->Cvar_Set( "r_DynamicGlow", "1" );
	}
	else
	{
		g_bDynamicGlowSupported = false;
		riptr->Cvar_Set( "r_DynamicGlow","0" );
	}
}

static qboolean GLimp_StartDriverAndSetMode(int mode, qboolean fullscreen, qboolean noborder)
{
	int colorBits;

	Com_Printf("Initializing OpenGL display\n");

	Com_Printf("...setting mode %d:", mode);

	if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, /*&glConfig.windowAspect,*/ mode ) )
	{
		Com_Printf(" invalid mode\n");
		return qfalse;
	}

	Com_Printf(" %d %d\n", glConfig.vidWidth, glConfig.vidHeight);
	colorBits = (r_colorbits->integer != 0) ? r_colorbits->integer : 24;

#ifdef __amigaos4__
	struct TagItem tags[] =
	{
		{MGLCC_Width, glConfig.vidWidth},
		{MGLCC_Height, glConfig.vidHeight},
		{MGLCC_WindowLeft, vid_xpos->integer},
		{MGLCC_WindowTop, vid_ypos->integer},
		{MGLCC_Windowed, !fullscreen},
		{MGLCC_TextureBufferSize, 65536},
		//{MGLCC_Buffers, 2},
		{MGLCC_PixelDepth, (colorBits != 32) ? colorBits : 24}, // 32-bit contexts are not supported
		{MGLCC_CloseGadget, GL_TRUE},
		{MGLCC_SizeGadget, GL_TRUE},
		{MGLCC_StencilBuffer, GL_TRUE},
		{TAG_DONE, 0}
	};

	context = (struct GLContextIFace *)CreateContext(tags);

	if (!context)
	{
		Com_Printf("Coudln't create GL context\n");
		return qfalse;
	}

	mglMakeCurrent(context);
	mglLockMode(MGL_LOCK_SMART);
	mglEnableSync(r_swapInterval->integer);

	awindow = (struct Window *)mglGetWindowHandle();
	ModifyIDCMP(awindow, IDCMP_CHANGEWINDOW | IDCMP_CLOSEWINDOW | IDCMP_EXTENDEDMOUSE);
	SetWindowTitles(awindow, (STRPTR)CLIENT_WINDOW_TITLE, (UBYTE *)~0);
#else
	/* Open new screen if needed */
	if (fullscreen)
	{
		ULONG modeid;

		modeid = BestCModeIDTags(
			CYBRBIDTG_Depth, colorBits,
			CYBRBIDTG_NominalWidth, glConfig.vidWidth,
			CYBRBIDTG_NominalHeight, glConfig.vidHeight,
			TAG_DONE);

		screen = OpenScreenTags(NULL,
			modeid != INVALID_ID ? SA_DisplayID : TAG_IGNORE, modeid,
			SA_Width, glConfig.vidWidth,
			SA_Height, glConfig.vidHeight,
			SA_Depth, colorBits,
			SA_Quiet, TRUE,
#ifdef __MORPHOS__
			SA_GammaControl, TRUE,
			SA_3DSupport, TRUE,
#endif
			TAG_DONE);

		glConfig.isFullscreen = screen ? qtrue : qfalse;
	}

	ULONG flags = WFLG_RMBTRAP | WFLG_ACTIVATE;

	if (screen)
		flags |= WFLG_BORDERLESS;
	else
		flags |= WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET;

	awindow = OpenWindowTags(NULL,
		screen ? TAG_IGNORE : WA_Left, vid_xpos->integer,
		screen ? TAG_IGNORE : WA_Top, vid_ypos->integer,
		WA_Flags, flags,
		WA_InnerWidth, glConfig.vidWidth,
		WA_InnerHeight, glConfig.vidHeight,
		screen ? WA_CustomScreen : TAG_IGNORE, (IPTR)screen,
		WA_Title, CLIENT_WINDOW_TITLE,
		WA_IDCMP, IDCMP_CHANGEWINDOW | IDCMP_CLOSEWINDOW,
		TAG_DONE);

	if (!awindow)
	{
		Com_Printf("Couldn't create window\n");
		return qfalse;
	}

#ifdef __AROS__
	context = AROSMesaCreateContextTags(
		AMA_Window,		awindow,
		AMA_Left,		screen ? 0 : awindow->BorderLeft,
		AMA_Top,		screen ? 0 : awindow->BorderTop,
		AMA_Width,		glConfig.vidWidth,
		AMA_Height,		glConfig.vidHeight,
		screen ? AMA_Screen : TAG_IGNORE, screen,
		AMA_DoubleBuf,	GL_TRUE,
		AMA_RGBMode,	GL_TRUE,
		AMA_NoAccum,	GL_TRUE,
		TAG_DONE);
#else
	__tglContext = GLInit();

	if (!__tglContext)
	{
		Com_Printf("Couldn't create GL context\n");
		return qfalse;
	}

	if (screen && !(TinyGLBase->lib_Version == 0 && TinyGLBase->lib_Revision < 4))
		context = glAInitializeContextScreen(screen);
	else
		context = glAInitializeContextWindowed(awindow);
#endif

	if (!context)
	{
		Com_Printf("Couldn't create GL context\n");
		return qfalse;
	}

#ifdef __AROS__
	AROSMesaMakeCurrent(context);
#endif
#endif

	return qtrue;
}

void GLimp_Init(void)
{
	vid_xpos = riptr->Cvar_Get("vid_xpos", "3", CVAR_ARCHIVE);
	vid_ypos = riptr->Cvar_Get("vid_ypos", "22", CVAR_ARCHIVE);

	if (!GLimp_StartDriverAndSetMode(r_mode->integer, (qboolean)r_fullscreen->integer, (qboolean)r_noborder->integer))
	{
		GLimp_Shutdown();
		Com_Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem" );
	}

	glConfig.deviceSupportsGamma = qfalse;
#ifdef __MORPHOS__
	if (screen && (IntuitionBase->LibNode.lib_Version > 50 || (IntuitionBase->LibNode.lib_Version == 50 && IntuitionBase->LibNode.lib_Revision >= 74)))
		glConfig.deviceSupportsGamma = qtrue;
#endif

	glConfig.vendor_string = (const char *)qglGetString(GL_VENDOR);
	glConfig.renderer_string = (const char *)qglGetString(GL_RENDERER);
	glConfig.version_string = (const char *)qglGetString(GL_VERSION);
	glConfig.extensions_string = (const char *)qglGetString(GL_EXTENSIONS);

#ifndef _JK2EXE
	glConfigExt.originalExtensionString = glConfig.extensions_string;
#endif

	qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &glConfig.maxTextureSize);

	GLimp_InitExtensions();

	glConfig.colorBits = GetBitMapAttr(awindow->WScreen->RastPort.BitMap, BMA_DEPTH);
	glGetIntegerv(GL_DEPTH_BITS, &glConfig.depthBits);
	glGetIntegerv(GL_STENCIL_BITS, &glConfig.stencilBits);

	glConfig.displayFrequency = 0;
	glConfig.stereoEnabled = qfalse;

	riptr->IN_Init(awindow);
}

void GLimp_Shutdown(void)
{
	riptr->IN_Shutdown();

	if (context)
	{
#ifdef __AROS__
		AROSMesaDestroyContext(context);
		context = NULL;
#elif defined(__amigaos4__)
		context->DeleteContext();
		context = NULL;

		mglMakeCurrent(NULL);
		awindow = NULL;
#else
		if (screen && !(TinyGLBase->lib_Version == 0 && TinyGLBase->lib_Revision < 4))
			glADestroyContextScreen();
		else
			glADestroyContextWindowed();

		context = 0;
#endif
	}

#ifdef __MORPHOS__
	if (__tglContext)
	{
		GLClose(__tglContext);
		__tglContext = NULL;
	}
#endif

#if defined(__AROS__) || defined(__MORPHOS__)
	if (awindow)
	{
		CloseWindow(awindow);
		awindow = NULL;
	}

	if (screen)
	{
		CloseScreen(screen);
		screen = NULL;
	}
#endif

	memset(&glConfig, 0, sizeof(glConfig));
	memset(&glState, 0, sizeof(glState));
}

#ifdef _JK2EXE
void GLimp_LogComment(const char *comment)
#else
void GLimp_LogComment(char *comment)
#endif
{
}

void GLimp_EndFrame(void)
{
#ifdef __AROS__
	AROSMesaSwapBuffers(context);
#elif defined(__amigaos4__)
	if (r_swapInterval->modified)
	{
		r_swapInterval->modified = qfalse;
		mglEnableSync(r_swapInterval->integer);
	}
	//mglUnlockDisplay();
	mglSwitchDisplay();
#else
	GLASwapBuffers(__tglContext);
#endif
}

void GLimp_SetGamma(unsigned char red[256], unsigned char green[256], unsigned char blue[256])
{
#ifdef __MORPHOS__
	if (glConfig.deviceSupportsGamma)
	{
		SetAttrs(screen,
			SA_GammaRed, red,
			SA_GammaGreen, green,
			SA_GammaBlue, blue,
			TAG_DONE);
	}
#endif
}

void GLimp_Minimize(void)
{
}
