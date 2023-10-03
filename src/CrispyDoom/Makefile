include ../common.mak
include ../commonp.mak

GENOBJS=libdoom/m_config.o libdoom/i_sound.o libdoom/i_system.o libdoom/i_video.o libdoom/m_argv.o libdoom/m_misc.o libdoom/aes_prng.o libdoom/d_event.o libdoom/d_iwad.o libdoom/d_loop.o libdoom/d_mode.o libdoom/deh_str.o libdoom/i_endoom.o libdoom/i_glob.o libdoom/i_input.o libdoom/i_timer.o libdoom/m_bbox.o libdoom/m_cheat.o libdoom/m_controls.o libdoom/m_fixed.o libdoom/net_client.o libdoom/net_common.o libdoom/net_io.o libdoom/net_loop.o libdoom/net_packet.o libdoom/net_petname.o libdoom/net_query.o libdoom/net_server.o libdoom/net_structrw.o libdoom/sha1.o libdoom/memio.o libdoom/tables.o libdoom/v_video.o libdoom/v_trans.o libdoom/w_checksum.o libdoom/w_main.o libdoom/w_wad.o libdoom/w_file.o libdoom/w_file_stdc.o libdoom/w_file_posix.o libdoom/w_file_win32.o libdoom/w_merge.o libdoom/z_zone.o libdoom/deh_main.o libdoom/deh_io.o libdoom/deh_mapping.o libdoom/deh_text.o libdoom/a11y.o libdoom/crispy.o libdoom/quit.o libdoom/palette.o libdoom/main.o

TEXTOBJS=libdoom/txt_desktop.o libdoom/txt_gui.o libdoom/txt_io.o libdoom/txt_label.o libdoom/txt_sdl.o libdoom/txt_separator.o libdoom/txt_strut.o libdoom/txt_table.o libdoom/txt_utf8.o libdoom/txt_widget.o libdoom/txt_window.o libdoom/txt_window_action.o

LIBOBJS=$(GENOBJS) $(TEXTOBJS)
CUSTOMPFLAGS=-I. -I./libdoom

DOOM=Doom
DOOMCREATOR=Doom
DOOMPRC=$(VFS)/app_install/$(DOOM).prc
DOOMOBJS=doom/am_map.o doom/d_items.o doom/d_main.o doom/d_net.o doom/d_pwad.o doom/deh_ammo.o doom/deh_bexincl.o doom/deh_bexpars.o doom/deh_bexptr.o doom/deh_bexstr.o doom/deh_cheat.o doom/deh_doom.o doom/deh_frame.o doom/deh_misc.o doom/deh_ptr.o doom/deh_sound.o doom/deh_thing.o doom/deh_weapon.o doom/doomdef.o doom/doomstat.o doom/dstrings.o doom/f_finale.o doom/f_wipe.o doom/g_game.o doom/hu_lib.o doom/hu_stuff.o doom/info.o doom/m_crispy.o doom/m_menu.o doom/m_random.o doom/p_bexptr.o doom/p_blockmap.o doom/p_ceilng.o doom/p_doors.o doom/p_enemy.o doom/p_extnodes.o doom/p_extsaveg.o doom/p_floor.o doom/p_inter.o doom/p_lights.o doom/p_map.o doom/p_maputl.o doom/p_mobj.o doom/p_plats.o doom/p_pspr.o doom/p_saveg.o doom/p_setup.o doom/p_sight.o doom/p_spec.o doom/p_switch.o doom/p_telept.o doom/p_tick.o doom/p_user.o doom/r_bmaps.o doom/r_bsp.o doom/r_data.o doom/r_draw.o doom/r_main.o doom/r_plane.o doom/r_segs.o doom/r_sky.o doom/r_swirl.o doom/r_things.o doom/s_musinfo.o doom/s_sound.o doom/sounds.o doom/st_lib.o doom/st_stuff.o doom/v_snow.o doom/wi_stuff.o doom/doomgame.o
DOOMFLAGS=-I./doom -DDOOM

HERETIC=Heretic
HERETICCREATOR=Hrtc
HERETICPRC=$(VFS)/app_install/$(HERETIC).prc
HERETICOBJS=heretic/am_map.o heretic/ct_chat.o heretic/d_main.o heretic/d_net.o heretic/deh_ammo.o heretic/deh_frame.o heretic/deh_htext.o heretic/deh_htic.o heretic/deh_sound.o heretic/deh_thing.o heretic/deh_weapon.o heretic/f_finale.o heretic/g_game.o heretic/in_lude.o heretic/info.o heretic/m_random.o heretic/mn_menu.o heretic/p_ceilng.o heretic/p_doors.o heretic/p_enemy.o heretic/p_extnodes.o heretic/p_floor.o heretic/p_inter.o heretic/p_lights.o heretic/p_map.o heretic/p_maputl.o heretic/p_mobj.o heretic/p_plats.o heretic/p_pspr.o heretic/p_saveg.o heretic/p_setup.o heretic/p_sight.o heretic/p_spec.o heretic/p_switch.o heretic/p_telept.o heretic/p_tick.o heretic/p_user.o heretic/r_bmaps.o heretic/r_bsp.o heretic/r_data.o heretic/r_draw.o heretic/r_main.o heretic/r_plane.o heretic/r_segs.o heretic/r_swirl.o heretic/r_things.o heretic/s_sound.o heretic/sb_bar.o heretic/sounds.o heretic/hereticgame.o
HERETICFLAGS=-I./heretic -DHERETIC

HEXEN=Hexen
HEXENCREATOR=Hexe
HEXENPRC=$(VFS)/app_install/$(HEXEN).prc
HEXENOBJS=hexen/a_action.o hexen/am_map.o hexen/ct_chat.o hexen/d_net.o hexen/f_finale.o hexen/g_game.o hexen/h2_main.o hexen/in_lude.o hexen/info.o hexen/m_random.o hexen/mn_menu.o hexen/p_acs.o hexen/p_anim.o hexen/p_ceilng.o hexen/p_doors.o hexen/p_enemy.o hexen/p_floor.o hexen/p_inter.o hexen/p_lights.o hexen/p_map.o hexen/p_maputl.o hexen/p_mobj.o hexen/p_plats.o hexen/p_pspr.o hexen/p_setup.o hexen/p_sight.o hexen/p_spec.o hexen/p_switch.o hexen/p_telept.o hexen/p_things.o hexen/p_tick.o hexen/p_user.o hexen/po_man.o hexen/r_bmaps.o hexen/r_bsp.o hexen/r_data.o hexen/r_draw.o hexen/r_main.o hexen/r_plane.o hexen/r_segs.o hexen/r_things.o hexen/s_sound.o hexen/sb_bar.o hexen/sc_man.o hexen/sn_sonix.o hexen/sounds.o hexen/st_start.o hexen/sv_save.o hexen/hexengame.o
HEXENFLAGS=-I./hexen -DHEXEN

STRIFE=Strife
STRIFECREATOR=Strf
STRIFEPRC=$(VFS)/app_install/$(STRIFE).prc
STRIFEOBJS=strife/am_map.o strife/d_items.o strife/d_main.o strife/d_net.o strife/deh_ammo.o strife/deh_cheat.o strife/deh_frame.o strife/deh_misc.o strife/deh_ptr.o strife/deh_sound.o strife/deh_strife.o strife/deh_thing.o strife/deh_weapon.o strife/doomdef.o strife/doomstat.o strife/dstrings.o strife/f_finale.o strife/f_wipe.o strife/g_game.o strife/hu_lib.o strife/hu_stuff.o strife/info.o strife/m_crispy.o strife/m_menu.o strife/m_random.o strife/m_saves.o strife/p_ceilng.o strife/p_dialog.o strife/p_doors.o strife/p_enemy.o strife/p_floor.o strife/p_inter.o strife/p_lights.o strife/p_map.o strife/p_maputl.o strife/p_mobj.o strife/p_plats.o strife/p_pspr.o strife/p_saveg.o strife/p_setup.o strife/p_sight.o strife/p_spec.o strife/p_switch.o strife/p_telept.o strife/p_tick.o strife/p_user.o strife/r_bsp.o strife/r_data.o strife/r_draw.o strife/r_main.o strife/r_plane.o strife/r_segs.o strife/r_sky.o strife/r_things.o strife/s_sound.o strife/sounds.o strife/st_lib.o strife/st_stuff.o strife/wi_stuff.o strife/strifegame.o
STRIFEFLAGS=-I./strife -DSTRIFE

all:
	@echo Run one the these commands:
	@echo make Doom.prc
	@echo make Heretic.prc
	@echo make Hexen.prc
	@echo make Strife.prc

$(DOOM).prc: $(DOOM).dlib doom/res
	cp $(DOOM).dlib doom/$(DLIB)
	$(PRCDUILD) -f $(DOOM).prc -t appl -c $(DOOMCREATOR) -n $(DOOM) doom/*.bin
	rm -f doom/$(DLIB)
	cp $(DOOM).prc $(DOOMPRC)

$(DOOM).dlib: $(DOOMOBJS) libdoom/libdoom.a
	$(CC) -shared -o $(DOOM).dlib $(DOOMOBJS) libdoom/libdoom.a -L$(BIN) -lpumpkin -lpit

$(HERETIC).prc: $(HERETIC).dlib heretic/res
	cp $(HERETIC).dlib heretic/$(DLIB)
	$(PRCDUILD) -f $(HERETIC).prc -t appl -c $(HERETICCREATOR) -n $(HERETIC) heretic/*.bin
	rm -f heretic/$(DLIB)
	cp $(HERETIC).prc $(HERETICPRC)

$(HERETIC).dlib: $(HERETICOBJS) libdoom/libdoom.a
	$(CC) -shared -o $(HERETIC).dlib $(HERETICOBJS) libdoom/libdoom.a -L$(BIN) -lpumpkin -lpit

$(HEXEN).prc: $(HEXEN).dlib hexen/res
	cp $(HEXEN).dlib hexen/$(DLIB)
	$(PRCDUILD) -f $(HEXEN).prc -t appl -c $(HEXENCREATOR) -n $(HEXEN) hexen/*.bin
	rm -f hexen/$(DLIB)
	cp $(HEXEN).prc $(HEXENPRC)

$(HEXEN).dlib: $(HEXENOBJS) libdoom/libdoom.a
	$(CC) -shared -o $(HEXEN).dlib $(HEXENOBJS) libdoom/libdoom.a -L$(BIN) -lpumpkin -lpit

$(STRIFE).prc: $(STRIFE).dlib strife/res
	cp $(STRIFE).dlib strife/$(DLIB)
	$(PRCDUILD) -f $(STRIFE).prc -t appl -c $(STRIFECREATOR) -n $(STRIFE) strife/*.bin
	rm -f strife/$(DLIB)
	cp $(STRIFE).prc $(STRIFEPRC)

$(STRIFE).dlib: $(STRIFEOBJS) libdoom/libdoom.a
	$(CC) -shared -o $(STRIFE).dlib $(STRIFEOBJS) libdoom/libdoom.a -L$(BIN) -lpumpkin -lpit

libdoom/libdoom.a: $(LIBOBJS)
	ar r libdoom/libdoom.a $(LIBOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

doom/%.o: doom/%.c
	$(CC) $(CFLAGS) $(DOOMFLAGS) -c -o $@ $<

heretic/%.o: heretic/%.c
	$(CC) $(CFLAGS) $(HERETICFLAGS) -c -o $@ $<

hexen/%.o: hexen/%.c
	$(CC) $(CFLAGS) $(HEXENFLAGS) -c -o $@ $<

strife/%.o: strife/%.c
	$(CC) $(CFLAGS) $(STRIFEFLAGS) -c -o $@ $<

doom/res: doom/resource.rcp
	$(PILRC) doom/resource.rcp doom; touch doom/res

heretic/res: heretic/resource.rcp
	$(PILRC) heretic/resource.rcp heretic; touch heretic/res

hexen/res: hexen/resource.rcp
	$(PILRC) hexen/resource.rcp hexen; touch hexen/res

strife/res: strife/resource.rcp
	$(PILRC) strife/resource.rcp strife; touch strife/res

clean:
	rm -f $(DOOMPRC) $(DOOM).prc $(DOOM).dlib $(DOOMOBJS)
	rm -f $(HERETICPRC) $(HERETIC).prc $(HERETIC).dlib $(HERETICOBJS)
	rm -f $(HEXENPRC) $(HEXEN).prc $(HEXEN).dlib $(HEXENOBJS)
	rm -f $(STRIFEPRC) $(STRIFE).prc $(STRIFE).dlib $(STRIFEOBJS)
	rm -f $(LIBOBJS) libdoom/libdoom.a
	rm -f doom/*.bin doom/res hexen/*.bin hexen/res heretic/res heretic/*.bin strife/res strife/*.bin
