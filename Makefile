CC = gcc
CXX = g++
MKDIR = mkdir -p
DEBUG = 0
OSTYPE = $(shell uname -s)
ARCH = $(shell uname -m)
ifeq ($(ARCH),powerpc)
	ARCH = ppc
endif

CFLAGS_COMMON = -Wall -Wno-comment -Wno-deprecated-declarations -fsigned-char -fno-strict-aliasing
# -MMD
CXXFLAGS_COMMON = $(CFLAGS_COMMON) -Wno-invalid-offsetof -Wno-write-strings

CFLAGS = $(CFLAGS_COMMON)

ifeq ($(DEBUG),1)
CFLAGS += -g
# -D_DEBUG
else
CFLAGS += -O2 -DNDEBUG -DFINAL_BUILD
endif
CXXFLAGS = $(CFLAGS) $(CXXFLAGS_COMMON)


.PHONY: all clean

all: openjk_sp.$(ARCH) base/jagame$(ARCH).so openjk.$(ARCH) base/jampgame$(ARCH).so base/cgame$(ARCH).so base/ui$(ARCH).so openjo_sp.$(ARCH) base/jospgame$(ARCH).so

clean:
	rm -Rf build


# SP

build/client_jasp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

build/client_jasp/%.o: %.cpp
	$(MKDIR) $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

openjk_sp.$(ARCH): CFLAGS += -D_JK2EXE -Icode/rd-vanilla -Icode/aros -Icode -Ilib
openjk_sp.$(ARCH): CXXFLAGS += -D_JK2EXE -Icode/rd-vanilla -Icode/aros -Icode -Ilib
openjk_sp.$(ARCH): LDFLAGS += -lGL

ifeq ($(OSTYPE), AmigaOS)
openjk_sp.$(ARCH): CFLAGS += -D__USE_INLINE__ -D__USE_BASETYPE__
openjk_sp.$(ARCH): CXXFLAGS += -D__USE_INLINE__ -D__USE_BASETYPE__
openjk_sp.$(ARCH): LDFLAGS += -use-dynld -ldl -lauto
endif

ifeq ($(OSTYPE), AROS)
openjk_sp.$(ARCH): LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
openjk_sp.$(ARCH): CFLAGS += -noixemul
openjk_sp.$(ARCH): CXXFLAGS += -noixemul
openjk_sp.$(ARCH): LDFLAGS += -noixemul
endif


build/game_jasp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

build/game_jasp/%.o: %.cpp
	$(MKDIR) $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

base/jagame$(ARCH).so: CFLAGS += -Icode/game
base/jagame$(ARCH).so: CXXFLAGS += -Icode/game

ifeq ($(OSTYPE), AmigaOS)
base/jagame$(ARCH).so: CFLAGS += -fPIC
base/jagame$(ARCH).so: CXXFLAGS += -fPIC
base/jagame$(ARCH).so: LDFLAGS += -shared
endif

ifeq ($(OSTYPE), AROS)
base/jagame$(ARCH).so: LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
base/jagame$(ARCH).so: CFLAGS += -noixemul
base/jagame$(ARCH).so: CXXFLAGS += -noixemul
base/jagame$(ARCH).so: LDFLAGS += -noixemul -nostartfiles
endif


# MP

build/client_jamp/%.o: %.cpp
	$(MKDIR) $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

build/client_jamp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

openjk.$(ARCH): CFLAGS += -DBOTLIB -Icodemp/rd-vanilla -Icode/aros -Icodemp -Ilib
openjk.$(ARCH): CXXFLAGS += -DBOTLIB -Icodemp/rd-vanilla -Icode/aros -Icodemp -Ilib
openjk.$(ARCH): LDFLAGS += -lGL

ifeq ($(OSTYPE), AmigaOS)
openjk.$(ARCH): CFLAGS += -D__USE_INLINE__ -D__USE_BASETYPE__ -DC_ONLY
openjk.$(ARCH): CXXFLAGS += -D__USE_INLINE__ -D__USE_BASETYPE__ -DC_ONLY
openjk.$(ARCH): LDFLAGS += -use-dynld -ldl -lauto
endif

ifeq ($(OSTYPE), AROS)
openjk.$(ARCH): LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
openjk.$(ARCH): CFLAGS += -noixemul
openjk.$(ARCH): CXXFLAGS += -noixemul
openjk.$(ARCH): LDFLAGS += -noixemul
endif


build/cgame_jamp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

base/cgame$(ARCH).so: CFLAGS += -D_CGAME -Icodemp
base/cgame$(ARCH).so: CXXFLAGS += -D_CGAME -Icodemp

ifeq ($(OSTYPE), AmigaOS)
base/cgame$(ARCH).so: CFLAGS += -fPIC -DC_ONLY
base/cgame$(ARCH).so: CXXFLAGS += -fPIC -DC_ONLY
base/cgame$(ARCH).so: LDFLAGS += -shared
endif

ifeq ($(OSTYPE), AROS)
base/cgame$(ARCH).so: LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
base/cgame$(ARCH).so: CFLAGS += -noixemul
base/cgame$(ARCH).so: CXXFLAGS += -noixemul
base/cgame$(ARCH).so: LDFLAGS += -noixemul -nostartfiles
endif


build/game_jamp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

base/jampgame$(ARCH).so: CFLAGS += -D_GAME -Icodemp
base/jampgame$(ARCH).so: CXXFLAGS += -D_GAME -Icodemp

ifeq ($(OSTYPE), AmigaOS)
base/jampgame$(ARCH).so: CFLAGS += -fPIC -DC_ONLY
base/jampgame$(ARCH).so: CXXFLAGS += -fPIC -DC_ONLY
base/jampgame$(ARCH).so: LDFLAGS += -shared
endif

ifeq ($(OSTYPE), AROS)
base/jampgame$(ARCH).so: LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
base/jampgame$(ARCH).so: CFLAGS += -noixemul
base/jampgame$(ARCH).so: CXXFLAGS += -noixemul
base/jampgame$(ARCH).so: LDFLAGS += -noixemul -nostartfiles
endif


build/ui_jamp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

base/ui$(ARCH).so: CFLAGS += -D_UI -Icodemp
base/ui$(ARCH).so: CXXFLAGS += -D_UI -Icodemp

ifeq ($(OSTYPE), AmigaOS)
base/ui$(ARCH).so: CFLAGS += -fPIC
base/ui$(ARCH).so: CXXFLAGS += -fPIC
base/ui$(ARCH).so: LDFLAGS += -shared
endif

ifeq ($(OSTYPE), AROS)
base/ui$(ARCH).so: LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
base/ui$(ARCH).so: CFLAGS += -noixemul
base/ui$(ARCH).so: CXXFLAGS += -noixemul
base/ui$(ARCH).so: LDFLAGS += -noixemul -nostartfiles
endif


# JOSP

build/client_josp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

build/client_josp/%.o: %.cpp
	$(MKDIR) $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

openjo_sp.$(ARCH): CFLAGS += -D_JK2EXE -DJK2_MODE -Icode/rd-vanilla -Icode/aros -Icode -Ilib
openjo_sp.$(ARCH): CXXFLAGS += -D_JK2EXE -DJK2_MODE -Icode/rd-vanilla -Icode/aros -Icode -Ilib
openjo_sp.$(ARCH): LDFLAGS += -lGL

ifeq ($(OSTYPE), AmigaOS)
openjo_sp.$(ARCH): CFLAGS += -D__USE_INLINE__ -D__USE_BASETYPE__
openjo_sp.$(ARCH): CXXFLAGS += -D__USE_INLINE__ -D__USE_BASETYPE__
openjo_sp.$(ARCH): LDFLAGS += -use-dynld -ldl -lauto
endif

ifeq ($(OSTYPE), AROS)
openjo_sp.$(ARCH): LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
openjk_sp.$(ARCH): CFLAGS += -noixemul
openjk_sp.$(ARCH): CXXFLAGS += -noixemul
openjk_sp.$(ARCH): LDFLAGS += -noixemul
endif


build/game_josp/%.o: %.c
	$(MKDIR) $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

build/game_josp/%.o: %.cpp
	$(MKDIR) $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

base/jospgame$(ARCH).so: CFLAGS += -IcodeJK2/game -DJK2_MODE
base/jospgame$(ARCH).so: CXXFLAGS += -IcodeJK2/game -DJK2_MODE

ifeq ($(OSTYPE), AmigaOS)
base/jospgame$(ARCH).so: CFLAGS += -fPIC
base/jospgame$(ARCH).so: CXXFLAGS += -fPIC
base/jospgame$(ARCH).so: LDFLAGS += -shared
endif

ifeq ($(OSTYPE), AROS)
base/jospgame$(ARCH).so: LDFLAGS += -ldll
endif

ifeq ($(OSTYPE), MorphOS)
base/jospgame$(ARCH).so: CFLAGS += -noixemul
base/jospgame$(ARCH).so: CXXFLAGS += -noixemul
base/jospgame$(ARCH).so: LDFLAGS += -noixemul -nostartfiles
endif


# common libs

JPEG_OBJ = \
	lib/jpeg-8c/jaricom.o \
	lib/jpeg-8c/jcapimin.o \
	lib/jpeg-8c/jcapistd.o \
	lib/jpeg-8c/jcarith.o \
	lib/jpeg-8c/jccoefct.o \
	lib/jpeg-8c/jccolor.o \
	lib/jpeg-8c/jcdctmgr.o \
	lib/jpeg-8c/jchuff.o \
	lib/jpeg-8c/jcinit.o \
	lib/jpeg-8c/jcmainct.o \
	lib/jpeg-8c/jcmarker.o \
	lib/jpeg-8c/jcmaster.o \
	lib/jpeg-8c/jcomapi.o \
	lib/jpeg-8c/jcparam.o \
	lib/jpeg-8c/jcprepct.o \
	lib/jpeg-8c/jcsample.o \
	lib/jpeg-8c/jctrans.o \
	lib/jpeg-8c/jdapimin.o \
	lib/jpeg-8c/jdapistd.o \
	lib/jpeg-8c/jdarith.o \
	lib/jpeg-8c/jdatadst.o \
	lib/jpeg-8c/jdatasrc.o \
	lib/jpeg-8c/jdcoefct.o \
	lib/jpeg-8c/jdcolor.o \
	lib/jpeg-8c/jddctmgr.o \
	lib/jpeg-8c/jdhuff.o \
	lib/jpeg-8c/jdinput.o \
	lib/jpeg-8c/jdmainct.o \
	lib/jpeg-8c/jdmarker.o \
	lib/jpeg-8c/jdmaster.o \
	lib/jpeg-8c/jdmerge.o \
	lib/jpeg-8c/jdpostct.o \
	lib/jpeg-8c/jdsample.o \
	lib/jpeg-8c/jdtrans.o \
	lib/jpeg-8c/jerror.o \
	lib/jpeg-8c/jfdctflt.o \
	lib/jpeg-8c/jfdctfst.o \
	lib/jpeg-8c/jfdctint.o \
	lib/jpeg-8c/jidctflt.o \
	lib/jpeg-8c/jidctfst.o \
	lib/jpeg-8c/jidctint.o \
	lib/jpeg-8c/jmemmgr.o \
	lib/jpeg-8c/jmemnobs.o \
	lib/jpeg-8c/jquant1.o \
	lib/jpeg-8c/jquant2.o \
	lib/jpeg-8c/jutils.o
	
PNG_OBJ = \
	lib/libpng/png.o \
	lib/libpng/pngerror.o \
	lib/libpng/pngget.o \
	lib/libpng/pngmem.o \
	lib/libpng/pngpread.o \
	lib/libpng/pngread.o \
	lib/libpng/pngrio.o \
	lib/libpng/pngrtran.o \
	lib/libpng/pngrutil.o \
	lib/libpng/pngset.o \
	lib/libpng/pngtrans.o \
	lib/libpng/pngwio.o \
	lib/libpng/pngwrite.o \
	lib/libpng/pngwtran.o \
	lib/libpng/pngwutil.o

UNZIP_OBJ = \
	lib/minizip/unzip.o \
	lib/minizip/ioapi.o \
	lib/zlib/adler32.o \
	lib/zlib/compress.o \
	lib/zlib/crc32.o \
	lib/zlib/deflate.o \
	lib/zlib/gzclose.o \
	lib/zlib/gzlib.o \
	lib/zlib/gzread.o \
	lib/zlib/gzwrite.o \
	lib/zlib/infback.o \
	lib/zlib/inffast.o \
	lib/zlib/inflate.o \
	lib/zlib/inftrees.o \
	lib/zlib/trees.o \
	lib/zlib/uncompr.o \
	lib/zlib/zutil.o


# JASP exe

CLIENT_SP_OBJS_ = \
	code/aros/aros_glimp.o \
	code/aros/aros_in.o \
	code/aros/aros_snddma.o \
	code/client/cl_cgame.o \
	code/client/cl_cin.o \
	code/client/cl_console.o \
	code/client/cl_input.o \
	code/client/cl_keys.o \
	code/client/cl_main.o \
	code/client/cl_mp3.o \
	code/client/cl_parse.o \
	code/client/cl_scrn.o \
	code/client/cl_ui.o \
	code/client/snd_ambient.o \
	code/client/snd_dma.o \
	code/client/snd_mem.o \
	code/client/snd_mix.o \
	code/client/snd_music.o \
	code/client/vmachine.o \
	code/game/genericparser2.o \
	code/mp3code/cdct.o \
	code/mp3code/csbt.o \
	code/mp3code/csbtb.o \
	code/mp3code/csbtl3.o \
	code/mp3code/cup.o \
	code/mp3code/cupini.o \
	code/mp3code/cupl1.o \
	code/mp3code/cupl3.o \
	code/mp3code/cwin.o \
	code/mp3code/cwinb.o \
	code/mp3code/cwinm.o \
	code/mp3code/hwin.o \
	code/mp3code/l3dq.o \
	code/mp3code/l3init.o \
	code/mp3code/mdct.o \
	code/mp3code/mhead.o \
	code/mp3code/msis.o \
	code/mp3code/towave.o \
	code/mp3code/uph.o \
	code/mp3code/upsf.o \
	code/mp3code/wavep.o \
	code/qcommon/cm_load.o \
	code/qcommon/cm_patch.o \
	code/qcommon/cm_polylib.o \
	code/qcommon/cm_test.o \
	code/qcommon/cm_trace.o \
	code/qcommon/cmd.o \
	code/qcommon/common.o \
	code/qcommon/cvar.o \
	code/qcommon/files_common.o \
	code/qcommon/files_pc.o \
	code/qcommon/matcomp.o \
	code/qcommon/md4.o \
	code/qcommon/msg.o \
	code/qcommon/net_chan.o \
	code/qcommon/persistence.o \
	code/qcommon/q_math.o \
	code/qcommon/q_shared.o \
	code/qcommon/stringed_ingame.o \
	code/qcommon/stringed_interface.o \
	code/qcommon/strip.o \
	code/qcommon/tri_coll_test.o \
	code/qcommon/z_memman_pc.o \
	code/server/sv_ccmds.o \
	code/server/sv_client.o \
	code/server/sv_game.o \
	code/server/sv_init.o \
	code/server/sv_main.o \
	code/server/sv_savegame.o \
	code/server/sv_snapshot.o \
	code/server/sv_world.o \
	code/sys/sys_main.o \
	code/sys/sys_unix.o \
	code/ui/ui_atoms.o \
	code/ui/ui_connect.o \
	code/ui/ui_debug.o \
	code/ui/ui_main.o \
	code/ui/ui_saber.o \
	code/ui/ui_shared.o \
	code/ui/ui_syscalls.o \
	code/rd-common/tr_font.o \
	code/rd-common/tr_image_jpg.o \
	code/rd-common/tr_image_load.o \
	code/rd-common/tr_image_png.o \
	code/rd-common/tr_image_tga.o \
	code/rd-common/tr_noise.o \
	code/rd-vanilla/G2_API.o \
	code/rd-vanilla/G2_bolts.o \
	code/rd-vanilla/G2_bones.o \
	code/rd-vanilla/G2_misc.o \
	code/rd-vanilla/G2_surfaces.o \
	code/rd-vanilla/tr_backend.o \
	code/rd-vanilla/tr_bsp.o \
	code/rd-vanilla/tr_cmds.o \
	code/rd-vanilla/tr_curve.o \
	code/rd-vanilla/tr_draw.o \
	code/rd-vanilla/tr_flares.o \
	code/rd-vanilla/tr_ghoul2.o \
	code/rd-vanilla/tr_image.o \
	code/rd-vanilla/tr_init.o \
	code/rd-vanilla/tr_light.o \
	code/rd-vanilla/tr_main.o \
	code/rd-vanilla/tr_marks.o \
	code/rd-vanilla/tr_mesh.o \
	code/rd-vanilla/tr_model.o \
	code/rd-vanilla/tr_quicksprite.o \
	code/rd-vanilla/tr_scene.o \
	code/rd-vanilla/tr_shade.o \
	code/rd-vanilla/tr_shader.o \
	code/rd-vanilla/tr_shade_calc.o \
	code/rd-vanilla/tr_shadows.o \
	code/rd-vanilla/tr_sky.o \
	code/rd-vanilla/tr_stl.o \
	code/rd-vanilla/tr_surface.o \
	code/rd-vanilla/tr_surfacesprites.o \
	code/rd-vanilla/tr_world.o \
	code/rd-vanilla/tr_WorldEffects.o \
	$(JPEG_OBJ) $(PNG_OBJ) $(UNZIP_OBJ)


# JASP game DLL

GAME_JASP_OBJS_ = \
	code/cgame/FX_ATSTMain.o \
	code/cgame/FX_Blaster.o \
	code/cgame/FX_Bowcaster.o \
	code/cgame/FX_BryarPistol.o \
	code/cgame/FX_Concussion.o \
	code/cgame/FX_DEMP2.o \
	code/cgame/FX_Disruptor.o \
	code/cgame/FX_Emplaced.o \
	code/cgame/FX_Flechette.o \
	code/cgame/FX_HeavyRepeater.o \
	code/cgame/FX_NoghriShot.o \
	code/cgame/FX_RocketLauncher.o \
	code/cgame/FX_TuskenShot.o \
	code/cgame/FxPrimitives.o \
	code/cgame/FxScheduler.o \
	code/cgame/FxSystem.o \
	code/cgame/FxTemplate.o \
	code/cgame/FxUtil.o \
	code/cgame/cg_camera.o \
	code/cgame/cg_consolecmds.o \
	code/cgame/cg_credits.o \
	code/cgame/cg_draw.o \
	code/cgame/cg_drawtools.o \
	code/cgame/cg_effects.o \
	code/cgame/cg_ents.o \
	code/cgame/cg_event.o \
	code/cgame/cg_headers.o \
	code/cgame/cg_info.o \
	code/cgame/cg_lights.o \
	code/cgame/cg_localents.o \
	code/cgame/cg_main.o \
	code/cgame/cg_marks.o \
	code/cgame/cg_players.o \
	code/cgame/cg_playerstate.o \
	code/cgame/cg_predict.o \
	code/cgame/cg_scoreboard.o \
	code/cgame/cg_servercmds.o \
	code/cgame/cg_snapshot.o \
	code/cgame/cg_syscalls.o \
	code/cgame/cg_text.o \
	code/cgame/cg_view.o \
	code/cgame/cg_weapons.o \
	code/game/AI_Animal.o \
	code/game/AI_AssassinDroid.o \
	code/game/AI_Atst.o \
	code/game/AI_BobaFett.o \
	code/game/AI_Civilian.o \
	code/game/AI_Default.o \
	code/game/AI_Droid.o \
	code/game/AI_GalakMech.o \
	code/game/AI_Grenadier.o \
	code/game/AI_HazardTrooper.o \
	code/game/AI_Howler.o \
	code/game/AI_ImperialProbe.o \
	code/game/AI_Interrogator.o \
	code/game/AI_Jedi.o \
	code/game/AI_Mark1.o \
	code/game/AI_Mark2.o \
	code/game/AI_MineMonster.o \
	code/game/AI_Rancor.o \
	code/game/AI_Remote.o \
	code/game/AI_RocketTrooper.o \
	code/game/AI_SaberDroid.o \
	code/game/AI_SandCreature.o \
	code/game/AI_Seeker.o \
	code/game/AI_Sentry.o \
	code/game/AI_Sniper.o \
	code/game/AI_Stormtrooper.o \
	code/game/AI_Tusken.o \
	code/game/AI_Utils.o \
	code/game/AI_Wampa.o \
	code/game/AnimalNPC.o \
	code/game/FighterNPC.o \
	code/game/G_Timer.o \
	code/game/NPC.o \
	code/game/NPC_behavior.o \
	code/game/NPC_combat.o \
	code/game/NPC_goal.o \
	code/game/NPC_misc.o \
	code/game/NPC_move.o \
	code/game/NPC_reactions.o \
	code/game/NPC_senses.o \
	code/game/NPC_sounds.o \
	code/game/NPC_spawn.o \
	code/game/NPC_stats.o \
	code/game/NPC_utils.o \
	code/game/Q3_Interface.o \
	code/game/SpeederNPC.o \
	code/game/WalkerNPC.o \
	code/game/bg_misc.o \
	code/game/bg_pangles.o \
	code/game/bg_panimate.o \
	code/game/bg_pmove.o \
	code/game/bg_slidemove.o \
	code/game/bg_vehicleLoad.o \
	code/game/g_active.o \
	code/game/g_breakable.o \
	code/game/g_camera.o \
	code/game/g_client.o \
	code/game/g_cmds.o \
	code/game/g_combat.o \
	code/game/g_emplaced.o \
	code/game/g_functions.o \
	code/game/g_fx.o \
	code/game/g_inventory.o \
	code/game/g_itemLoad.o \
	code/game/g_items.o \
	code/game/g_main.o \
	code/game/g_mem.o \
	code/game/g_misc.o \
	code/game/g_misc_model.o \
	code/game/g_missile.o \
	code/game/g_mover.o \
	code/game/g_nav.o \
	code/game/g_navigator.o \
	code/game/g_navnew.o \
	code/game/g_object.o \
	code/game/g_objectives.o \
	code/game/g_rail.o \
	code/game/g_ref.o \
	code/game/g_roff.o \
	code/game/g_savegame.o \
	code/game/g_session.o \
	code/game/g_spawn.o \
	code/game/g_svcmds.o \
	code/game/g_target.o \
	code/game/g_trigger.o \
	code/game/g_turret.o \
	code/game/g_usable.o \
	code/game/g_utils.o \
	code/game/g_vehicles.o \
	code/game/g_weapon.o \
	code/game/g_weaponLoad.o \
	code/game/genericparser2.o \
	code/qcommon/q_math.o \
	code/qcommon/q_shared.o \
	code/game/wp_atst.o \
	code/game/wp_blaster_pistol.o \
	code/game/wp_blaster_rifle.o \
	code/game/wp_bot_laser.o \
	code/game/wp_bowcaster.o \
	code/game/wp_concussion.o \
	code/game/wp_demp2.o \
	code/game/wp_det_pack.o \
	code/game/wp_disruptor.o \
	code/game/wp_emplaced_gun.o \
	code/game/wp_flechette.o \
	code/game/wp_melee.o \
	code/game/wp_noghri_stick.o \
	code/game/wp_repeater.o \
	code/game/wp_rocket_launcher.o \
	code/game/wp_saber.o \
	code/game/wp_saberLoad.o \
	code/game/wp_stun_baton.o \
	code/game/wp_thermal.o \
	code/game/wp_trip_mine.o \
	code/game/wp_tusken.o \
	code/icarus/BlockStream.o \
	code/icarus/IcarusImplementation.o \
	code/icarus/Sequence.o \
	code/icarus/Sequencer.o \
	code/icarus/TaskManager.o \
	code/qcommon/tri_coll_test.o \
	code/ui/gameinfo.o \
	code/Ratl/ratl.o \
	code/Ravl/CBounds.o \
	code/Ravl/CVec.o \
	code/Rufl/hfile.o \
	code/Rufl/hstring.o

ifeq ($(OSTYPE), AROS)
GAME_JASP_OBJS_ += \
	code/aros/aros_exports.o
endif

ifeq ($(OSTYPE), MorphOS)
GAME_JASP_OBJS_ += \
	code/aros/libnix_so.o
endif


# JOSP game DLL

GAME_JOSP_OBJS_ = \
	codejk2/cgame/cg_camera.o \
	codejk2/cgame/cg_consolecmds.o \
	codejk2/cgame/cg_credits.o \
	codejk2/cgame/cg_draw.o \
	codejk2/cgame/cg_drawtools.o \
	codejk2/cgame/cg_effects.o \
	codejk2/cgame/cg_ents.o \
	codejk2/cgame/cg_event.o \
	codejk2/cgame/cg_info.o \
	codejk2/cgame/cg_lights.o \
	codejk2/cgame/cg_localents.o \
	codejk2/cgame/cg_main.o \
	codejk2/cgame/cg_marks.o \
	codejk2/cgame/cg_players.o \
	codejk2/cgame/cg_playerstate.o \
	codejk2/cgame/cg_predict.o \
	codejk2/cgame/cg_scoreboard.o \
	codejk2/cgame/cg_servercmds.o \
	codejk2/cgame/cg_snapshot.o \
	codejk2/cgame/cg_syscalls.o \
	codejk2/cgame/cg_text.o \
	codejk2/cgame/cg_view.o \
	codejk2/cgame/cg_weapons.o \
	codejk2/cgame/FxPrimitives.o \
	codejk2/cgame/FxScheduler.o \
	codejk2/cgame/FxSystem.o \
	codejk2/cgame/FxTemplate.o \
	codejk2/cgame/FxUtil.o \
	codejk2/cgame/FX_ATSTMain.o \
	codejk2/cgame/FX_Blaster.o \
	codejk2/cgame/FX_Bowcaster.o \
	codejk2/cgame/FX_BryarPistol.o \
	codejk2/cgame/FX_DEMP2.o \
	codejk2/cgame/FX_Disruptor.o \
	codejk2/cgame/FX_Emplaced.o \
	codejk2/cgame/FX_Flechette.o \
	codejk2/cgame/FX_HeavyRepeater.o \
	codejk2/cgame/FX_RocketLauncher.o \
	codejk2/game/AI_Atst.o \
	codejk2/game/AI_Default.o \
	codejk2/game/AI_Droid.o \
	codejk2/game/AI_GalakMech.o \
	codejk2/game/AI_Grenadier.o \
	codejk2/game/AI_Howler.o \
	codejk2/game/AI_ImperialProbe.o \
	codejk2/game/AI_Interrogator.o \
	codejk2/game/AI_Jedi.o \
	codejk2/game/AI_Mark1.o \
	codejk2/game/AI_Mark2.o \
	codejk2/game/AI_MineMonster.o \
	codejk2/game/AI_Remote.o \
	codejk2/game/AI_Seeker.o \
	codejk2/game/AI_Sentry.o \
	codejk2/game/AI_Sniper.o \
	codejk2/game/AI_Stormtrooper.o \
	codejk2/game/AI_Utils.o \
	codejk2/game/bg_misc.o \
	codejk2/game/bg_pangles.o \
	codejk2/game/bg_panimate.o \
	codejk2/game/bg_pmove.o \
	codejk2/game/bg_slidemove.o \
	codejk2/game/genericparser2.o \
	codejk2/game/g_active.o \
	codejk2/game/g_breakable.o \
	codejk2/game/g_camera.o \
	codejk2/game/g_client.o \
	codejk2/game/g_cmds.o \
	codejk2/game/g_combat.o \
	codejk2/game/g_functions.o \
	codejk2/game/g_fx.o \
	codejk2/game/g_ICARUS.o \
	codejk2/game/g_inventory.o \
	codejk2/game/g_itemLoad.o \
	codejk2/game/g_items.o \
	codejk2/game/g_main.o \
	codejk2/game/g_mem.o \
	codejk2/game/g_misc.o \
	codejk2/game/g_misc_model.o \
	codejk2/game/g_missile.o \
	codejk2/game/g_mover.o \
	codejk2/game/g_nav.o \
	codejk2/game/g_navigator.o \
	codejk2/game/g_navnew.o \
	codejk2/game/g_object.o \
	codejk2/game/g_objectives.o \
	codejk2/game/g_ref.o \
	codejk2/game/g_roff.o \
	codejk2/game/g_savegame.o \
	codejk2/game/g_session.o \
	codejk2/game/g_spawn.o \
	codejk2/game/g_svcmds.o \
	codejk2/game/g_target.o \
	codejk2/game/G_Timer.o \
	codejk2/game/g_trigger.o \
	codejk2/game/g_turret.o \
	codejk2/game/g_usable.o \
	codejk2/game/g_utils.o \
	codejk2/game/g_weapon.o \
	codejk2/game/g_weaponLoad.o \
	codejk2/game/NPC.o \
	codejk2/game/NPC_behavior.o \
	codejk2/game/NPC_combat.o \
	codejk2/game/NPC_goal.o \
	codejk2/game/NPC_misc.o \
	codejk2/game/NPC_move.o \
	codejk2/game/NPC_reactions.o \
	codejk2/game/NPC_senses.o \
	codejk2/game/NPC_sounds.o \
	codejk2/game/NPC_spawn.o \
	codejk2/game/NPC_stats.o \
	codejk2/game/NPC_utils.o \
	codejk2/game/Q3_Interface.o \
	codejk2/game/Q3_Registers.o \
	codejk2/game/wp_atst.o \
	codejk2/game/wp_blaster_rifle.o \
	codejk2/game/wp_bot_laser.o \
	codejk2/game/wp_bowcaster.o \
	codejk2/game/wp_bryar_pistol.o \
	codejk2/game/wp_demp2.o \
	codejk2/game/wp_det_pack.o \
	codejk2/game/wp_disruptor.o \
	codejk2/game/wp_emplaced_gun.o \
	codejk2/game/wp_flechette.o \
	codejk2/game/wp_melee.o \
	codejk2/game/wp_repeater.o \
	codejk2/game/wp_rocket_launcher.o \
	codejk2/game/wp_saber.o \
	codejk2/game/wp_stun_baton.o \
	codejk2/game/wp_thermal.o \
	codejk2/game/wp_trip_mine.o \
	codejk2/icarus/BlockStream.o \
	codejk2/icarus/Instance.o \
	codejk2/icarus/Sequence.o \
	codejk2/icarus/Sequencer.o \
	codejk2/icarus/TaskManager.o \
	code/ui/gameinfo.o \
	code/qcommon/tri_coll_test.o \
	code/qcommon/q_math.o \
	code/qcommon/q_shared.o \
	code/Rufl/hstring.o \

ifeq ($(OSTYPE), AROS)
GAME_JOSP_OBJS_ += \
	code/aros/aros_exports.o
endif

ifeq ($(OSTYPE), MorphOS)
GAME_JOSP_OBJS_ += \
	code/aros/libnix_so.o
endif


# JAMP exe

CLIENT_MP_OBJS_ = \
	code/aros/aros_glimp.o \
	code/aros/aros_in.o \
	code/aros/aros_snddma.o \
	codemp/botlib/be_aas_bspq3.o \
	codemp/botlib/be_aas_cluster.o \
	codemp/botlib/be_aas_debug.o \
	codemp/botlib/be_aas_entity.o \
	codemp/botlib/be_aas_file.o \
	codemp/botlib/be_aas_main.o \
	codemp/botlib/be_aas_move.o \
	codemp/botlib/be_aas_optimize.o \
	codemp/botlib/be_aas_reach.o \
	codemp/botlib/be_aas_route.o \
	codemp/botlib/be_aas_routealt.o \
	codemp/botlib/be_aas_sample.o \
	codemp/botlib/be_ai_char.o \
	codemp/botlib/be_ai_chat.o \
	codemp/botlib/be_ai_gen.o \
	codemp/botlib/be_ai_goal.o \
	codemp/botlib/be_ai_move.o \
	codemp/botlib/be_ai_weap.o \
	codemp/botlib/be_ai_weight.o \
	codemp/botlib/be_ea.o \
	codemp/botlib/be_interface.o \
	codemp/botlib/l_crc.o \
	codemp/botlib/l_libvar.o \
	codemp/botlib/l_log.o \
	codemp/botlib/l_memory.o \
	codemp/botlib/l_precomp.o \
	codemp/botlib/l_script.o \
	codemp/botlib/l_struct.o \
	codemp/qcommon/cm_load.o \
	codemp/qcommon/cm_patch.o \
	codemp/qcommon/cm_polylib.o \
	codemp/qcommon/cm_test.o \
	codemp/qcommon/cm_trace.o \
	codemp/qcommon/cmd.o \
	codemp/qcommon/common.o \
	codemp/qcommon/cvar.o \
	codemp/qcommon/files.o \
	codemp/qcommon/GenericParser2.o \
	codemp/qcommon/huffman.o \
	codemp/qcommon/md4.o \
	codemp/qcommon/md5.o \
	codemp/qcommon/msg.o \
	codemp/qcommon/matcomp.o \
	codemp/qcommon/net_chan.o \
	codemp/qcommon/net_ip.o \
	codemp/qcommon/persistence.o \
	codemp/qcommon/q_math.o \
	codemp/qcommon/q_shared.o \
	codemp/qcommon/RoffSystem.o \
	codemp/qcommon/stringed_ingame.o \
	codemp/qcommon/stringed_interface.o \
	codemp/qcommon/vm.o \
	codemp/qcommon/z_memman_pc.o \
	codemp/icarus/BlockStream.o \
	codemp/icarus/GameInterface.o \
	codemp/icarus/Instance.o \
	codemp/icarus/Interface.o \
	codemp/icarus/Memory.o \
	codemp/icarus/Q3_Interface.o \
	codemp/icarus/Q3_Registers.o \
	codemp/icarus/Sequence.o \
	codemp/icarus/Sequencer.o \
	codemp/icarus/TaskManager.o \
	codemp/server/NPCNav/navigator.o \
	codemp/server/sv_bot.o \
	codemp/server/sv_ccmds.o \
	codemp/server/sv_client.o \
	codemp/server/sv_game.o \
	codemp/server/sv_init.o \
	codemp/server/sv_main.o \
	codemp/server/sv_net_chan.o \
	codemp/server/sv_snapshot.o \
	codemp/server/sv_world.o \
	codemp/server/sv_gameapi.o \
	codemp/sys/snapvector.o \
	codemp/client/cl_avi.o \
	codemp/client/cl_cgame.o \
	codemp/client/cl_cgameapi.o \
	codemp/client/cl_cin.o \
	codemp/client/cl_console.o \
	codemp/client/cl_input.o \
	codemp/client/cl_keys.o \
	codemp/client/cl_lan.o \
	codemp/client/cl_main.o \
	codemp/client/cl_net_chan.o \
	codemp/client/cl_parse.o \
	codemp/client/cl_scrn.o \
	codemp/client/cl_ui.o \
	codemp/client/cl_uiapi.o \
	codemp/client/FXExport.o \
	codemp/client/FxPrimitives.o \
	codemp/client/FxScheduler.o \
	codemp/client/FxSystem.o \
	codemp/client/FxTemplate.o \
	codemp/client/FxUtil.o \
	codemp/client/snd_ambient.o \
	codemp/client/snd_dma.o \
	codemp/client/snd_mem.o \
	codemp/client/snd_mix.o \
	codemp/client/snd_mp3.o \
	codemp/client/snd_music.o \
	codemp/mp3code/cdct.o \
	codemp/mp3code/csbt.o \
	codemp/mp3code/csbtb.o \
	codemp/mp3code/csbtl3.o \
	codemp/mp3code/cup.o \
	codemp/mp3code/cupini.o \
	codemp/mp3code/cupl1.o \
	codemp/mp3code/cupl3.o \
	codemp/mp3code/cwin.o \
	codemp/mp3code/cwinb.o \
	codemp/mp3code/cwinm.o \
	codemp/mp3code/hwin.o \
	codemp/mp3code/l3dq.o \
	codemp/mp3code/l3init.o \
	codemp/mp3code/mdct.o \
	codemp/mp3code/mhead.o \
	codemp/mp3code/msis.o \
	codemp/mp3code/towave.o \
	codemp/mp3code/uph.o \
	codemp/mp3code/upsf.o \
	codemp/mp3code/wavep.o \
	codemp/sys/sys_main.o \
	codemp/sys/sys_unix.o \
	codemp/rd-vanilla/G2_API.o \
	codemp/rd-vanilla/G2_bolts.o \
	codemp/rd-vanilla/G2_bones.o \
	codemp/rd-vanilla/G2_misc.o \
	codemp/rd-vanilla/G2_surfaces.o \
	codemp/rd-vanilla/tr_backend.o \
	codemp/rd-vanilla/tr_bsp.o \
	codemp/rd-vanilla/tr_cmds.o \
	codemp/rd-vanilla/tr_curve.o \
	codemp/rd-vanilla/tr_decals.o \
	codemp/rd-vanilla/tr_ghoul2.o \
	codemp/rd-vanilla/tr_image.o \
	codemp/rd-vanilla/tr_init.o \
	codemp/rd-vanilla/tr_light.o \
	codemp/rd-vanilla/tr_main.o \
	codemp/rd-vanilla/tr_marks.o \
	codemp/rd-vanilla/tr_mesh.o \
	codemp/rd-vanilla/tr_model.o \
	codemp/rd-vanilla/tr_quicksprite.o \
	codemp/rd-vanilla/tr_scene.o \
	codemp/rd-vanilla/tr_shade.o \
	codemp/rd-vanilla/tr_shade_calc.o \
	codemp/rd-vanilla/tr_shader.o \
	codemp/rd-vanilla/tr_shadows.o \
	codemp/rd-vanilla/tr_skin.o \
	codemp/rd-vanilla/tr_sky.o \
	codemp/rd-vanilla/tr_surface.o \
	codemp/rd-vanilla/tr_surfacesprites.o \
	codemp/rd-vanilla/tr_world.o \
	codemp/rd-vanilla/tr_WorldEffects.o \
	codemp/ghoul2/G2_gore.o \
	codemp/rd-common/tr_font.o \
	codemp/rd-common/tr_image_load.o \
	codemp/rd-common/tr_image_jpg.o \
	codemp/rd-common/tr_image_tga.o \
	codemp/rd-common/tr_image_png.o \
	codemp/rd-common/tr_noise.o \
	$(JPEG_OBJ) $(PNG_OBJ) $(UNZIP_OBJ)


CGAME_JAMP_OBJS_ = \
	codemp/game/AnimalNPC.o \
	codemp/game/bg_g2_utils.o \
	codemp/game/bg_misc.o \
	codemp/game/bg_panimate.o \
	codemp/game/bg_pmove.o \
	codemp/game/bg_saber.o \
	codemp/game/bg_saberLoad.o \
	codemp/game/bg_saga.o \
	codemp/game/bg_slidemove.o \
	codemp/game/bg_vehicleLoad.o \
	codemp/game/bg_weapons.o \
	codemp/game/FighterNPC.o \
	codemp/game/SpeederNPC.o \
	codemp/game/WalkerNPC.o \
	codemp/cgame/cg_consolecmds.o \
	codemp/cgame/cg_cvar.o \
	codemp/cgame/cg_draw.o \
	codemp/cgame/cg_drawtools.o \
	codemp/cgame/cg_effects.o \
	codemp/cgame/cg_ents.o \
	codemp/cgame/cg_event.o \
	codemp/cgame/cg_info.o \
	codemp/cgame/cg_light.o \
	codemp/cgame/cg_localents.o \
	codemp/cgame/cg_main.o \
	codemp/cgame/cg_marks.o \
	codemp/cgame/cg_newDraw.o \
	codemp/cgame/cg_players.o \
	codemp/cgame/cg_playerstate.o \
	codemp/cgame/cg_predict.o \
	codemp/cgame/cg_saga.o \
	codemp/cgame/cg_scoreboard.o \
	codemp/cgame/cg_servercmds.o \
	codemp/cgame/cg_snapshot.o \
	codemp/cgame/cg_spawn.o \
	codemp/cgame/cg_syscalls.o \
	codemp/cgame/cg_turret.o \
	codemp/cgame/cg_view.o \
	codemp/cgame/cg_weaponinit.o \
	codemp/cgame/cg_weapons.o \
	codemp/cgame/fx_blaster.o \
	codemp/cgame/fx_bowcaster.o \
	codemp/cgame/fx_bryarpistol.o \
	codemp/cgame/fx_demp2.o \
	codemp/cgame/fx_disruptor.o \
	codemp/cgame/fx_flechette.o \
	codemp/cgame/fx_force.o \
	codemp/cgame/fx_heavyrepeater.o \
	codemp/cgame/fx_rocketlauncher.o \
	codemp/qcommon/q_math.o \
	codemp/qcommon/q_shared.o \
	codemp/ui/ui_shared.o

ifeq ($(OSTYPE), AROS)
CGAME_JAMP_OBJS_ += \
	codemp/aros/aros_exports.o
endif

ifeq ($(OSTYPE), MorphOS)
CGAME_JAMP_OBJS_ += \
	code/aros/libnix_so.o
endif


GAME_JAMP_OBJS_ = \
	codemp/game/ai_main.o \
	codemp/game/ai_util.o \
	codemp/game/ai_wpnav.o \
	codemp/game/AnimalNPC.o \
	codemp/game/bg_g2_utils.o \
	codemp/game/bg_misc.o \
	codemp/game/bg_panimate.o \
	codemp/game/bg_pmove.o \
	codemp/game/bg_saber.o \
	codemp/game/bg_saberLoad.o \
	codemp/game/bg_saga.o \
	codemp/game/bg_slidemove.o \
	codemp/game/bg_vehicleLoad.o \
	codemp/game/bg_weapons.o \
	codemp/game/FighterNPC.o \
	codemp/game/g_active.o \
	codemp/game/g_bot.o \
	codemp/game/g_client.o \
	codemp/game/g_cmds.o \
	codemp/game/g_combat.o \
	codemp/game/g_cvar.o \
	codemp/game/g_exphysics.o \
	codemp/game/g_ICARUScb.o \
	codemp/game/g_items.o \
	codemp/game/g_log.o \
	codemp/game/g_main.o \
	codemp/game/g_mem.o \
	codemp/game/g_misc.o \
	codemp/game/g_missile.o \
	codemp/game/g_mover.o \
	codemp/game/g_nav.o \
	codemp/game/g_navnew.o \
	codemp/game/g_object.o \
	codemp/game/g_saga.o \
	codemp/game/g_session.o \
	codemp/game/g_spawn.o \
	codemp/game/g_svcmds.o \
	codemp/game/g_syscalls.o \
	codemp/game/g_target.o \
	codemp/game/g_team.o \
	codemp/game/g_timer.o \
	codemp/game/g_trigger.o \
	codemp/game/g_turret.o \
	codemp/game/g_turret_G2.o \
	codemp/game/g_utils.o \
	codemp/game/g_vehicles.o \
	codemp/game/g_vehicleTurret.o \
	codemp/game/g_weapon.o \
	codemp/game/NPC.o \
	codemp/game/NPC_AI_Atst.o \
	codemp/game/NPC_AI_Default.o \
	codemp/game/NPC_AI_Droid.o \
	codemp/game/NPC_AI_GalakMech.o \
	codemp/game/NPC_AI_Grenadier.o \
	codemp/game/NPC_AI_Howler.o \
	codemp/game/NPC_AI_ImperialProbe.o \
	codemp/game/NPC_AI_Interrogator.o \
	codemp/game/NPC_AI_Jedi.o \
	codemp/game/NPC_AI_Mark1.o \
	codemp/game/NPC_AI_Mark2.o \
	codemp/game/NPC_AI_MineMonster.o \
	codemp/game/NPC_AI_Rancor.o \
	codemp/game/NPC_AI_Remote.o \
	codemp/game/NPC_AI_Seeker.o \
	codemp/game/NPC_AI_Sentry.o \
	codemp/game/NPC_AI_Sniper.o \
	codemp/game/NPC_AI_Stormtrooper.o \
	codemp/game/NPC_AI_Utils.o \
	codemp/game/NPC_AI_Wampa.o \
	codemp/game/NPC_behavior.o \
	codemp/game/NPC_combat.o \
	codemp/game/NPC_goal.o \
	codemp/game/NPC_misc.o \
	codemp/game/NPC_move.o \
	codemp/game/NPC_reactions.o \
	codemp/game/NPC_senses.o \
	codemp/game/NPC_sounds.o \
	codemp/game/NPC_spawn.o \
	codemp/game/NPC_stats.o \
	codemp/game/NPC_utils.o \
	codemp/game/SpeederNPC.o \
	codemp/game/tri_coll_test.o \
	codemp/game/w_force.o \
	codemp/game/w_saber.o \
	codemp/game/WalkerNPC.o \
	codemp/qcommon/q_math.o \
	codemp/qcommon/q_shared.o

ifeq ($(OSTYPE), AROS)
GAME_JAMP_OBJS_ += \
	codemp/aros/aros_exports.o
endif

ifeq ($(OSTYPE), MorphOS)
GAME_JAMP_OBJS_ += \
	code/aros/libnix_so.o
endif

#set(MPUIDefines ${MPSharedDefines} "_UI")

UI_JAMP_OBJS_ = \
	codemp/game/bg_misc.o \
	codemp/game/bg_saberLoad.o \
	codemp/game/bg_saga.o \
	codemp/game/bg_vehicleLoad.o \
	codemp/game/bg_weapons.o \
	codemp/qcommon/q_math.o \
	codemp/qcommon/q_shared.o \
	codemp/ui/ui_atoms.o \
	codemp/ui/ui_cvar.o \
	codemp/ui/ui_force.o \
	codemp/ui/ui_gameinfo.o \
	codemp/ui/ui_main.o \
	codemp/ui/ui_saber.o \
	codemp/ui/ui_shared.o \
	codemp/ui/ui_syscalls.o

ifeq ($(OSTYPE), AROS)
UI_JAMP_OBJS_ += \
	codemp/aros/aros_exports.o
endif

ifeq ($(OSTYPE), MorphOS)
UI_JAMP_OBJS_ += \
	code/aros/libnix_so.o
endif


# path substitition

# SP
GAME_JASP_OBJS = $(patsubst %,build/game_jasp/%,$(GAME_JASP_OBJS_))
CLIENT_JASP_OBJS = $(patsubst %,build/client_jasp/%,$(CLIENT_SP_OBJS_))
GAME_JOSP_OBJS = $(patsubst %,build/game_josp/%,$(GAME_JOSP_OBJS_))
CLIENT_JOSP_OBJS = $(patsubst %,build/client_josp/%,$(CLIENT_SP_OBJS_))

# MP
CLIENT_JAMP_OBJS = $(patsubst %,build/client_jamp/%,$(CLIENT_MP_OBJS_))
CGAME_JAMP_OBJS = $(patsubst %,build/cgame_jamp/%,$(CGAME_JAMP_OBJS_))
GAME_JAMP_OBJS = $(patsubst %,build/game_jamp/%,$(GAME_JAMP_OBJS_))
UI_JAMP_OBJS = $(patsubst %,build/ui_jamp/%,$(UI_JAMP_OBJS_))


# dependencies

#GAME_JASP_DEPS = $(GAME_JASP_OBJS:.o=.d)
#CLIENT_JASP_DEPS = $(CLIENT_JASP_OBJS:.o=.d)
#GAME_JOSP_DEPS = $(GAME_JOSP_OBJS:.o=.d)
#CLIENT_JOSP_DEPS = $(CLIENT_JOSP_OBJS:.o=.d)

#-include $(GAME_JASP_DEPS)
#-include $(CLIENT_JASP_DEPS)
#-include $(GAME_JOSP_DEPS)
#-include $(CLIENT_JOSP_DEPS)


# targets

# SP
openjk_sp.$(ARCH): $(CLIENT_JASP_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

base/jagame$(ARCH).so: $(GAME_JASP_OBJS)
	$(MKDIR) $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@	

openjo_sp.$(ARCH): $(CLIENT_JOSP_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

base/jospgame$(ARCH).so: $(GAME_JOSP_OBJS)
	$(MKDIR) $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@

# MP
openjk.$(ARCH): $(CLIENT_JAMP_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

base/cgame$(ARCH).so: $(CGAME_JAMP_OBJS)
	$(MKDIR) $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@	

base/jampgame$(ARCH).so: $(GAME_JAMP_OBJS)
	$(MKDIR) $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@	
	
base/ui$(ARCH).so: $(UI_JAMP_OBJS)
	$(MKDIR) $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@	
