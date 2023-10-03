include ../common.mak
include ../commonp.mak

CUSTOMPFLAGS=-I. -I./src -I./data -I$(SRC)/libzip

DOOM=DSDA
DOOMCREATOR=DSDA
DOOMPRC=$(VFS)/app_install/$(DOOM).prc
DOOMOBJS=data/rd_graphic.o data/rd_main.o data/rd_output.o data/rd_palette.o data/rd_sound.o data/rd_util.o src/am_map.o src/d_client.o src/d_deh.o src/d_items.o src/d_main.o src/doomdef.o src/doomstat.o src/dsda.o src/dsda/analysis.o src/dsda/args.o src/dsda/brute_force.o src/dsda/build.o src/dsda/compatibility.o src/dsda/configuration.o src/dsda/console.o src/dsda/cr_table.o src/dsda/data_organizer.o src/dsda/death.o src/dsda/deh_hash.o src/dsda/demo.o src/dsda/destructible.o src/dsda/endoom.o src/dsda/episode.o src/dsda/excmd.o src/dsda/exdemo.o src/dsda/exhud.o src/dsda/features.o src/dsda/font.o src/dsda/ghost.o src/dsda/global.o src/dsda/hud_components/ammo_text.o src/dsda/hud_components/armor_text.o src/dsda/hud_components/attempts.o src/dsda/hud_components/base.o src/dsda/hud_components/big_ammo.o src/dsda/hud_components/big_armor.o src/dsda/hud_components/big_armor_text.o src/dsda/hud_components/big_artifact.o src/dsda/hud_components/big_health.o src/dsda/hud_components/big_health_text.o src/dsda/hud_components/color_test.o src/dsda/hud_components/command_display.o src/dsda/hud_components/composite_time.o src/dsda/hud_components/coordinate_display.o src/dsda/hud_components/event_split.o src/dsda/hud_components/fps.o src/dsda/hud_components/free_text.o src/dsda/hud_components/health_text.o src/dsda/hud_components/keys.o src/dsda/hud_components/level_splits.o src/dsda/hud_components/line_display.o src/dsda/hud_components/line_distance_tracker.o src/dsda/hud_components/line_tracker.o src/dsda/hud_components/local_time.o src/dsda/hud_components/map_coordinates.o src/dsda/hud_components/map_time.o src/dsda/hud_components/map_title.o src/dsda/hud_components/map_totals.o src/dsda/hud_components/message.o src/dsda/hud_components/minimap.o src/dsda/hud_components/mobj_tracker.o src/dsda/hud_components/null.o src/dsda/hud_components/player_tracker.o src/dsda/hud_components/ready_ammo_text.o src/dsda/hud_components/render_stats.o src/dsda/hud_components/secret_message.o src/dsda/hud_components/sector_tracker.o src/dsda/hud_components/speed_text.o src/dsda/hud_components/stat_totals.o src/dsda/hud_components/tracker.o src/dsda/hud_components/weapon_text.o src/dsda/id_list.o src/dsda/input.o src/dsda/key_frame.o src/dsda/map_format.o src/dsda/mapinfo.o src/dsda/mapinfo/doom.o src/dsda/mapinfo/doom/parser.o src/dsda/mapinfo/hexen.o src/dsda/mapinfo/legacy.o src/dsda/mapinfo/u.o src/dsda/memory.o src/dsda/messenger.o src/dsda/mobjinfo.o src/dsda/mouse.o src/dsda/msecnode.o src/dsda/music.o src/dsda/name.o src/dsda/options.o src/dsda/palette.o src/dsda/pause.o src/dsda/pclass.o src/dsda/playback.o src/dsda/preferences.o src/dsda/quake.o src/dsda/render_stats.o src/dsda/save.o src/dsda/settings.o src/dsda/sfx.o src/dsda/skill_info.o src/dsda/skip.o src/dsda/sndinfo.o src/dsda/spawn_number.o src/dsda/split_tracker.o src/dsda/sprite.o src/dsda/state.o src/dsda/stretch.o src/dsda/text_color.o src/dsda/text_file.o src/dsda/thing_id.o src/dsda/time.o src/dsda/tracker.o src/dsda/tranmap.o src/dsda/udmf.o src/dsda/utility.o src/dsda/utility/string_view.o src/dsda/wad_stats.o src/dsda/zipfile.o src/dstrings.o src/e6y.o src/f_finale.o src/f_wipe.o src/g_game.o src/g_overflow.o src/heretic/d_main.o src/heretic/f_finale.o src/heretic/in_lude.o src/heretic/info.o src/heretic/level_names.o src/heretic/mn_menu.o src/heretic/sb_bar.o src/heretic/sounds.o src/hexen/a_action.o src/hexen/f_finale.o src/hexen/h2_main.o src/hexen/in_lude.o src/hexen/info.o src/hexen/p_acs.o src/hexen/p_anim.o src/hexen/p_things.o src/hexen/po_man.o src/hexen/sn_sonix.o src/hexen/sounds.o src/hexen/sv_save.o src/hu_lib.o src/hu_stuff.o src/i_glob.o src/info.o src/lprintf.o src/m_argv.o src/m_bbox.o src/m_cheat.o src/m_file.o src/m_fixed.o src/m_menu.o src/m_misc.o src/m_random.o src/md5.o src/memio.o src/mus2mid.o src/p_ceilng.o src/p_doors.o src/p_enemy.o src/p_floor.o src/p_genlin.o src/p_inter.o src/p_lights.o src/p_map.o src/p_maputl.o src/p_mobj.o src/p_plats.o src/p_pspr.o src/p_saveg.o src/p_setup.o src/p_sight.o src/p_spec.o src/p_switch.o src/p_telept.o src/p_tick.o src/p_user.o src/r_bsp.o src/r_data.o src/r_draw.o src/r_fps.o src/r_main.o src/r_patch.o src/r_plane.o src/r_segs.o src/r_sky.o src/r_things.o src/s_advsound.o src/s_sound.o src/sc_man.o src/smooth.o src/sounds.o src/st_lib.o src/st_stuff.o src/tables.o src/umapinfo.o src/v_video.o src/w_wad.o src/wadtbl.o src/wi_stuff.o src/z_bmalloc.o src/z_zone.o src/scanner.o src/i_main.o src/i_system.o src/i_sound.o src/i_video.o src/w_memcache.o src/quit.o src/doomgame.o src/main.o

$(DOOM).prc: $(DOOM).dlib res
	cp $(DOOM).dlib $(DLIB)
	$(PRCDUILD) -f $(DOOM).prc -t appl -c $(DOOMCREATOR) -n $(DOOM) *.bin
	rm -f $(DLIB)
	cp $(DOOM).prc $(DOOMPRC)

$(DOOM).dlib: $(DOOMOBJS)
	g++ -shared -o $(DOOM).dlib $(DOOMOBJS) -L$(BIN) -lpumpkin -lpit -lzip

src/dsda/mapinfo/doom/parser.o: src/dsda/mapinfo/doom/parser.cpp
	g++ $(CFLAGS) -c -o $@ $<

src/dsda/udmf.o: src/dsda/udmf.cpp
	g++ $(CFLAGS) -c -o $@ $<

src/umapinfo.o: src/umapinfo.cpp
	g++ $(CFLAGS) -c -o $@ $<

src/scanner.o: src/scanner.cpp
	g++ $(CFLAGS) -c -o $@ $<

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(DOOMPRC) $(DOOM).prc $(DOOM).dlib $(DOOMOBJS)
	rm -f *.bin res
