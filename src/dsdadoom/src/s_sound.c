/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:  Platform-independent sound code
 *
 *-----------------------------------------------------------------------------*/

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "s_sound.h"
#include "s_advsound.h"
#include "i_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"
#include "lprintf.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "e6y.h"

#include "hexen/sn_sonix.h"

#include "dsda/configuration.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/memory.h"
#include "dsda/music.h"
#include "dsda/settings.h"
#include "dsda/sfx.h"
#include "dsda/skip.h"

// Adjustable by menu.
#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128
#define S_STEREO_SWING (96<<FRACBITS)

const int channel_not_found = -1;

typedef struct
{
  sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  void *origin;        // origin of sound
  int handle;          // handle of the sound being played
  int pitch;

  // heretic
  int priority;

  // hexen
  int volume;

  dboolean active;
  dboolean ambient;
  dboolean loop;
  int loop_timeout;
  sfx_class_t sfx_class;
} channel_t;

// the set of channels available
static channel_t channels[MAX_CHANNELS];
static degenmobj_t sobjs[MAX_CHANNELS];

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int snd_SfxVolume;

// Derived value (not saved, accounts for muted sfx)
static int sfx_volume;

// Maximum volume of music.
int snd_MusicVolume = 15;

// whether songs are mus_paused
static dboolean mus_paused;

// music currently being played
musicinfo_t *mus_playing;

// music currently should play
static int musicnum_current;

// number of channels available
int numChannels;

//jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Internals.
//

void S_StopChannel(int cnum);

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, channel_t *channel, sfx_params_t *params);

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, sfx_params_t *params);


// heretic
int max_snd_dist = 1600;
int dist_adjust = 160;

static byte* soundCurve;
static int AmbChan = -1;

static mobj_t* GetSoundListener(void);
static void Heretic_S_StopSound(void *_origin);
static void Raven_S_StartSoundAtVolume(void *_origin, int sound_id, int volume, int loop_timeout);

void S_ResetSfxVolume(void)
{
  snd_SfxVolume = dsda_IntConfig(dsda_config_sfx_volume);

  if (nosfxparm)
    return;

  if (dsda_MuteSfx())
    sfx_volume = 0;
  else
    sfx_volume = snd_SfxVolume;
}

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(void)
{
  idmusnum = -1; //jff 3/17/98 insure idmus number is blank

  S_Stop();

  numChannels = dsda_IntConfig(dsda_config_snd_channels);

  //jff 1/22/98 skip sound init if sound not enabled
  if (!nosfxparm)
  {
    static dboolean first_s_init = true;

    // Whatever these did with DMX, these are rather dummies now.
    I_SetChannels();

    S_ResetSfxVolume();

    // Reset channel memory
    memset(channels, 0, sizeof(channels));
    memset(sobjs, 0, sizeof(sobjs));

    if (first_s_init)
    {
      int i;
      int snd_curve_lump;

      first_s_init = false;

      for (i = 1; i < num_sfx; i++)
        S_sfx[i].lumpnum = -1;

      dsda_CacheSoundLumps();

      // {
      //   int i;
      //   const int snd_curve_length = 1200;
      //   const int flat_curve_length = 160;
      //   byte* buffer = Z_Malloc(snd_curve_length);
      //   for (i = 0; i < snd_curve_length; ++i)
      //   {
      //     if (i < flat_curve_length)
      //       buffer[i] = 127;
      //     else
      //       buffer[i] = 127 * (snd_curve_length - i) / (snd_curve_length - flat_curve_length);

      //     if (!buffer[i])
      //       buffer[i] = 1;
      //   }
      //   M_WriteFile("sndcurve.lmp", buffer, snd_curve_length);
      // }

      snd_curve_lump = W_GetNumForName("SNDCURVE");
      max_snd_dist = W_LumpLength(snd_curve_lump);

      dist_adjust = max_snd_dist / 10;

      soundCurve = Z_Malloc(max_snd_dist);
      memcpy(soundCurve, (const byte *) W_LumpByNum(snd_curve_lump), max_snd_dist);
    }
  }

  // CPhipps - music init reformatted
  if (!nomusicparm) {
    void I_ResetMusicVolume(void);

    I_ResetMusicVolume();

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;
  }
}

void S_Stop(void)
{
  int cnum;

  // heretic
  AmbChan = -1;

  //jff 1/22/98 skip sound init if sound not enabled
  if (!nosfxparm)
    for (cnum=0 ; cnum<numChannels ; cnum++)
      if (channels[cnum].active)
        S_StopChannel(cnum);
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start(void)
{
  int mnum;
  int muslump;

  // kill all playing sounds at start of level
  //  (trust me - a good idea)

  S_Stop();

  // start new music for the level
  mus_paused = 0;

  dsda_MapMusic(&mnum, &muslump);

  if (muslump >= 0)
  {
    musinfo.items[0] = muslump;

    if (!dsda_StartQueuedMusic())
      S_ChangeMusInfoMusic(muslump, true);
  }
  else
  {
    memset(&musinfo, 0, sizeof(musinfo));
    musinfo.items[0] = -1;

    if (!dsda_StartQueuedMusic())
      S_ChangeMusic(mnum, true);
  }
}

void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume, int loop_timeout)
{
  int cnum;
  sfx_params_t params;
  sfxinfo_t *sfx;
  mobj_t *origin;

  if (raven) return Raven_S_StartSoundAtVolume(origin_p, sfx_id, volume, loop_timeout);

  origin = (mobj_t *) origin_p;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

  // killough 4/25/98
  if (sfx_id & PICKUP_SOUND ||
      sfx_id == sfx_oof ||
      (compatibility_level >= prboom_2_compatibility && sfx_id == sfx_noway))
    params.sfx_class = sfx_class_important;
  else
    params.sfx_class = sfx_class_none;

  params.ambient = false;
  params.loop = loop_timeout > 0;
  params.loop_timeout = loop_timeout;

  sfx_id &= ~PICKUP_SOUND;

  if (sfx_id == sfx_None)
    return;

  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > num_sfx)
    I_Error("S_StartSoundAtVolume: Bad sfx #: %d", sfx_id);

  sfx = &S_sfx[sfx_id];

  // Initialize sound parameters
  params.priority = 128 - sfx->priority;
  if (params.priority <= 0)
    params.priority = 1;
  if (sfx->pitch < 0)
    params.pitch = NORM_PITCH;
  else
    params.pitch = sfx->pitch;
  params.volume = volume;

  // Check to see if it is audible, modify the params
  // killough 3/7/98, 4/25/98: code rearranged slightly

  if (!origin || (origin == players[displayplayer].mo && walkcamera.type < 2)) {
    params.separation = NORM_SEP;
    params.volume *= 8;
    params.priority *= 10;
  } else
    if (!S_AdjustSoundParams(players[displayplayer].mo, origin, NULL, &params))
      return;
    else
      if ( origin->x == players[displayplayer].mo->x &&
           origin->y == players[displayplayer].mo->y)
        params.separation = NORM_SEP;

  if (dsda_BlockSFX(sfx)) return;

  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
    params.pitch += 8 - (M_Random()&15);
  else
    if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
      params.pitch += 16 - (M_Random()&31);

  if (params.pitch < 0)
    params.pitch = 0;

  if (params.pitch > 255)
    params.pitch = 255;

  // try to find a channel
  cnum = S_getChannel(origin, sfx, &params);

  if (cnum == channel_not_found)
    return;

  // get lumpnum if necessary
  // killough 2/28/98: make missing sounds non-fatal
  if (sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
    return;

  // Assigns the handle to one of the channels in the mix/output buffer.
  { // e6y: [Fix] Crash with zero-length sounds.
    int h = I_StartSound(sfx_id, cnum, &params);
    if (h != -1)
    {
      channels[cnum].handle = h;
      channels[cnum].pitch = params.pitch;
      channels[cnum].priority = params.priority;
      channels[cnum].ambient = params.ambient;
      channels[cnum].loop = params.loop;
      channels[cnum].loop_timeout = params.loop_timeout;
      channels[cnum].active = true;
    }
  }
}

void S_StartSectorSound(sector_t *sector, int sfx_id)
{
  if (sector->flags & SECF_SILENT)
    return;

  S_StartSound((mobj_t *) &sector->soundorg, sfx_id);
}

void S_LoopSectorSound(sector_t *sector, int sfx_id, int timeout)
{
  if (sector->flags & SECF_SILENT)
    return;

  S_LoopSound((mobj_t *) &sector->soundorg, sfx_id, timeout);
}

void S_StartMobjSound(mobj_t *mobj, int sfx_id)
{
  if (mobj && mobj->subsector && mobj->subsector->sector->flags & SECF_SILENT)
    return;

  S_StartSound(mobj, sfx_id);
}

void S_StartVoidSound(int sfx_id)
{
  S_StartSound(NULL, sfx_id);
}

void S_StartLineSound(line_t *line, degenmobj_t *soundorg, int sfx_id)
{
  if (line && line->frontsector && line->frontsector->flags & SECF_SILENT)
    return;

  S_StartSound((mobj_t *) soundorg, sfx_id);
}

void S_StartSound(void *origin, int sfx_id)
{
  S_StartSoundAtVolume(origin, sfx_id, raven ? 127 : sfx_volume, 0);
}

void S_LoopSound(void *origin, int sfx_id, int timeout)
{
  S_StartSoundAtVolume(origin, sfx_id, raven ? 127 : sfx_volume, timeout);
}

void S_StopSound(void *origin)
{
  int cnum;

  if (raven) return Heretic_S_StopSound(origin);

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

  for (cnum=0 ; cnum<numChannels ; cnum++)
    if (channels[cnum].active && channels[cnum].origin == origin)
      {
        S_StopChannel(cnum);
        break;
      }
}

void S_StopSoundLoops(void)
{
  int cnum;

  if (nosfxparm)
    return;

  for (cnum = 0; cnum < numChannels; ++cnum)
    if (channels[cnum].active && channels[cnum].loop)
      S_StopChannel(cnum);
}

// [FG] disable sound cutoffs
int full_sounds;

void S_UnlinkSound(void *origin)
{
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

  if (origin)
  {
    for (cnum = 0; cnum < numChannels; cnum++)
    {
      if (channels[cnum].active && channels[cnum].origin == origin)
      {
        degenmobj_t *const sobj = &sobjs[cnum];
        const mobj_t *const mobj = (mobj_t *) origin;
        sobj->x = mobj->x;
        sobj->y = mobj->y;
        sobj->z = mobj->z;
        channels[cnum].origin = (mobj_t *) sobj;
        break;
      }
    }
  }
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
  //jff 1/22/98 return if music is not enabled
  if (nomusicparm)
    return;

  if (mus_playing && !mus_paused)
    {
      I_PauseSong(mus_playing->handle);
      mus_paused = true;
    }
}

void S_ResumeSound(void)
{
  //jff 1/22/98 return if music is not enabled
  if (nomusicparm)
    return;

  if (mus_playing && mus_paused)
    {
      I_ResumeSong(mus_playing->handle);
      mus_paused = false;
    }
}


//
// Updates music & sounds
//
void S_UpdateSounds(void)
{
  mobj_t *listener;
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

#ifdef UPDATE_MUSIC
  I_UpdateMusic();
#endif

  listener = GetSoundListener();
  if (sfx_volume == 0)
    return;

  if (map_format.sndseq)
  {
    // Update any Sequences
    SN_UpdateActiveSequences();
  }

  for (cnum = 0; cnum < numChannels; cnum++)
  {
    channel_t *channel = &channels[cnum];

    if (channel->active)
    {
      if (channel->loop && --channel->loop_timeout < 0)
      {
        S_StopChannel(cnum);
      }
      else if (I_SoundIsPlaying(channel->handle))
      {
        sfx_params_t params;

        // check non-local sounds for distance clipping
        // or modify their params
        if (channel->origin && listener != channel->origin) // killough 3/20/98
        {
          if (S_AdjustSoundParams(listener, channel->origin, channel, &params))
          {
            I_UpdateSoundParams(channel->handle, &params);
            channel->priority = params.priority;
          }
          else
          {
            raven ? S_StopSound(channel->origin) : S_StopChannel(cnum);
          }
        }
      }
      else   // if channel is allocated but sound has stopped, free it
        S_StopChannel(cnum);
    }
  }
}

// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
  S_ChangeMusic(m_id, false);
}

dboolean S_ChangeMusicByName(const char *name, dboolean looping)
{
  int lump = W_CheckNumForName(name);

  if (lump == LUMP_NOT_FOUND)
  {
    S_StopMusic();
    return false;
  }

  S_ChangeMusInfoMusic(lump, looping);
  return true;
}

void S_ChangeMusic(int musicnum, int looping)
{
  musicinfo_t *music;

  // current music which should play
  musicnum_current = musicnum;
  musinfo.current_item = -1;
  S_music[mus_musinfo].lumpnum = -1;

  //jff 1/22/98 return if music is not enabled
  if (nomusicparm)
    return;

  if (musicnum <= mus_None || musicnum >= num_music)
    I_Error("S_ChangeMusic: Bad music number %d", musicnum);

  music = &S_music[musicnum];

  if (mus_playing == music)
    return;

  // shutdown old music
  S_StopMusic();

  // get lumpnum if necessary
  if (!music->lumpnum)
    music->lumpnum = dsda_MusicIndexToLumpNum(musicnum);

  // load & register it
  music->data = W_LumpByNum(music->lumpnum);
  music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;

  musinfo.current_item = -1;

  // [crispy] MUSINFO value 0 is reserved for the map's default music
  if (musinfo.items[0] == -1)
  {
     musinfo.items[0] = music->lumpnum;
     S_music[mus_musinfo].lumpnum = -1;
  }
}

void S_RestartMusic(void)
{
  if (musinfo.current_item != -1)
  {
    S_ChangeMusInfoMusic(musinfo.current_item, true);
  }
  else
  {
    if (musicnum_current > mus_None && musicnum_current < num_music)
    {
      S_ChangeMusic(musicnum_current, true);
    }
  }
}

void S_ChangeMusInfoMusic(int lumpnum, int looping)
{
  musicinfo_t *music;

  if (dsda_SkipMode())
  {
    musinfo.current_item = lumpnum;
    return;
  }

  //jff 1/22/98 return if music is not enabled
  if (nomusicparm)
    return;

  if (mus_playing && mus_playing->lumpnum == lumpnum)
    return;

  music = &S_music[mus_musinfo];

  if (music->lumpnum == lumpnum)
    return;

  // shutdown old music
  S_StopMusic();

  // save lumpnum
  music->lumpnum = lumpnum;

  // load & register it
  music->data = W_LumpByNum(music->lumpnum);
  music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;

  musinfo.current_item = lumpnum;
}

void S_StopMusic(void)
{
  //jff 1/22/98 return if music is not enabled
  if (nomusicparm)
    return;

  if (mus_playing)
    {
      if (mus_paused)
        I_ResumeSong(mus_playing->handle);

      I_StopSong(mus_playing->handle);
      I_UnRegisterSong(mus_playing->handle);

      mus_playing->data = 0;
      mus_playing = 0;
    }
}



void S_StopChannel(int cnum)
{
  channel_t *c = &channels[cnum];

  if (AmbChan == cnum)
    AmbChan = -1;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

  if (c->active)
  {
    // stop the sound playing
    if (I_SoundIsPlaying(c->handle))
      I_StopSound(c->handle);

    c->active = false;
    c->sfxinfo = NULL;
    c->origin = NULL;
    c->handle = 0;
  }
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, channel_t *channel, sfx_params_t *params)
{
  fixed_t adx, ady;
  ufixed_t approx_dist;
  angle_t angle;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return 0;

  // e6y
  if (!listener)
    return 0;

  if (channel)
  {
    params->ambient = channel->ambient;
    params->loop = channel->loop;
    params->loop_timeout = channel->loop_timeout;
  }

  // calculate the distance to sound origin
  //  and clip it if necessary
  if (walkcamera.type > 1)
  {
    adx = D_abs(walkcamera.x - source->x);
    ady = D_abs(walkcamera.y - source->y);
  }
  else
  {
    adx = D_abs(listener->x - source->x);
    ady = D_abs(listener->y - source->y);
  }

  approx_dist = P_AproxDistance(adx, ady);
  approx_dist >>= FRACBITS;

  if (approx_dist >= max_snd_dist)
    return 0;

  // angle of source to listener
  angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

  if (angle <= listener->angle)
    angle += 0xffffffff;
  angle -= listener->angle;
  angle >>= ANGLETOFINESHIFT;

  // stereo separation
  params->separation = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

  // volume calculation
  if (raven)
  {
    if (channel)
    {
      params->volume =
        (soundCurve[approx_dist] * sfx_volume * 8 * channel->volume) >> 14;
    }
    else
    {
      // currently raven only adjusts on update (channel exists)
    }
  }
  else
  {
    params->volume = (soundCurve[approx_dist] * sfx_volume * 8) >> 7;
  }

  if (channel)
  {
    params->pitch = channel->pitch;
    params->priority = channel->sfxinfo->priority;
    if (!raven)
      params->priority = 128 - params->priority;
  }

  // heretic_note: divides by 256 instead of the dist_adjust
  params->priority *= (10 - approx_dist / dist_adjust);

  return (params->volume > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int S_ChannelScore(channel_t *channel)
{
  return channel->priority;
}

static int S_LowestScoreChannel(void)
{
  int cnum;
  int lowest_score = INT_MAX;
  int lowest_cnum = channel_not_found;

  for (cnum = 0; cnum < numChannels; ++cnum)
  {
    int score = S_ChannelScore(&channels[cnum]);

    if (score < lowest_score)
    {
      lowest_score = score;
      lowest_cnum = cnum;
    }
  }

  return lowest_cnum;
}

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, sfx_params_t *params)
{
  // channel number to use
  int cnum;
  channel_t *c;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return channel_not_found;

  // Only allow one sound per origin
  for (cnum = 0; cnum < numChannels; cnum++)
    if (channels[cnum].active && channels[cnum].origin == origin &&
        (comp[comp_sound] || channels[cnum].sfx_class == params->sfx_class))
    {
      // The sound is already playing
      if (channels[cnum].sfxinfo == sfxinfo && channels[cnum].loop && params->loop) {
        channels[cnum].loop_timeout = params->loop_timeout;

        return channel_not_found;
      }

      S_StopChannel(cnum);
      break;
    }

  // Find an open channel
  for (cnum = 0; cnum < numChannels; cnum++)
    if (!channels[cnum].active)
      break;

  // None available
  if (cnum == numChannels)
  {      // Look for lower priority
    channel_t temp_channel;

    memset(&temp_channel, 0, sizeof(temp_channel));
    temp_channel.priority = params->priority;
    temp_channel.volume = params->volume;

    cnum = S_LowestScoreChannel();

    if (cnum == channel_not_found)
      return channel_not_found;

    if (S_ChannelScore(&temp_channel) > S_ChannelScore(&channels[cnum]))
      S_StopChannel(cnum);
    else
      return channel_not_found;
  }

  c = &channels[cnum];              // channel is decided to be cnum.
  c->sfxinfo = sfxinfo;
  c->origin = origin;
  c->sfx_class = params->sfx_class;
  return cnum;
}

// heretic

static dboolean S_StopSoundInfo(sfxinfo_t* sfx, sfx_params_t *params)
{
  int i;
  int priority;
  int least_priority;
  int found;

  if (sfx->numchannels == -1)
    return true;

  priority = params->priority;
  least_priority = -1;
  found = 0;

  for (i = 0; i < numChannels; i++)
  {
    if (channels[i].active && channels[i].sfxinfo == sfx && channels[i].origin)
    {
      found++;            //found one.  Now, should we replace it??
      if (priority >= channels[i].priority)
      {                   // if we're gonna kill one, then this'll be it
        if (!channels[i].loop || priority > channels[i].priority)
        {
          least_priority = i;
          priority = channels[i].priority;
        }
      }
    }
  }

  if (found < sfx->numchannels)
    return true;

  if (least_priority >= 0)
  {
    S_StopChannel(least_priority);

    return true;
  }

  return false; // don't replace any sounds
}

static int Raven_S_getChannel(mobj_t *origin, sfxinfo_t *sfx, sfx_params_t *params)
{
  int i;
  static int sndcount = 0;

  for (i = 0; i < numChannels; i++)
  {
    // The sound is already playing
    if (channels[i].active &&
        channels[i].sfxinfo == sfx &&
        channels[i].origin == origin &&
        channels[i].loop && params->loop)
    {
      channels[i].loop_timeout = params->loop_timeout;

      return channel_not_found;
    }
  }

  if (!S_StopSoundInfo(sfx, params))
    return channel_not_found; // other sounds have greater priority

  for (i = 0; i < numChannels; i++)
  {
    if (gamestate != GS_LEVEL || origin == players[displayplayer].mo)
    {
      i = numChannels;
      break;              // let the player have more than one sound.
    }
    if (origin == channels[i].origin)
    {                       // only allow other mobjs one sound
      S_StopSound(channels[i].origin);
      break;
    }
  }

  if (i >= numChannels)
  {
    // TODO: can ambient sounds even reach this flow?
    if (params->ambient)
    {
      if (AmbChan != -1 && sfx->priority <= channels[AmbChan].sfxinfo->priority)
        return channel_not_found;         //ambient channel already in use

      AmbChan = -1;
    }

    for (i = 0; i < numChannels; i++)
      if (!channels[i].active)
        break;

    if (i >= numChannels)
    {
      int chan;

      //look for a lower priority sound to replace.
      sndcount++;
      if (sndcount >= numChannels)
        sndcount = 0;

      for (chan = 0; chan < numChannels; chan++)
      {
        i = (sndcount + chan) % numChannels;
        if (params->priority >= channels[i].priority)
        {
          chan = -1;  //denote that sound should be replaced.
          break;
        }
      }

      if (chan != -1)
        return channel_not_found;  //no free channels.

      S_StopChannel(i);
    }
  }

  return i;
}

static mobj_t* GetSoundListener(void)
{
  static degenmobj_t dummy_listener;

  // If we are at the title screen, the display player doesn't have an
  // object yet, so return a pointer to a static dummy listener instead.

  if (players[displayplayer].mo != NULL)
  {
    return players[displayplayer].mo;
  }
  else
  {
    dummy_listener.x = 0;
    dummy_listener.y = 0;
    dummy_listener.z = 0;

    return (mobj_t *) &dummy_listener;
  }
}

static void Raven_S_StartSoundAtVolume(void *_origin, int sound_id, int volume, int loop_timeout)
{
  sfxinfo_t *sfx;
  mobj_t *origin;
  mobj_t *listener;
  sfx_params_t params;
  int dist;
  int cnum;
  angle_t angle;
  fixed_t absx;
  fixed_t absy;

  origin = (mobj_t *)_origin;
  listener = GetSoundListener();

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

  if (sound_id == heretic_sfx_None)
    return;

  if (origin == NULL)
    origin = listener;

  sfx = &S_sfx[sound_id];

  params.ambient = heretic && sound_id >= heretic_sfx_wind;
  params.loop = loop_timeout > 0;
  params.loop_timeout = loop_timeout;

  // calculate the distance before other stuff so that we can throw out
  // sounds that are beyond the hearing range.
  absx = abs(origin->x - listener->x);
  absy = abs(origin->y - listener->y);
  dist = P_AproxDistance(absx, absy);
  dist >>= FRACBITS;

  if (dist >= max_snd_dist)
    return; //sound is beyond the hearing range...
  if (dist < 0)
    dist = 0;

  params.priority = sfx->priority;
  params.priority *= (10 - (dist / dist_adjust));

  params.sfx_class = sfx_class_none;

  cnum = Raven_S_getChannel(origin, sfx, &params);
  if (cnum == channel_not_found)
    return;

  if (sfx->lumpnum <= 0)
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

  params.volume = (soundCurve[dist] * volume * sfx_volume * 8) >> 14;

  if (origin == listener)
    params.separation = 128;
  else
  {
    angle = R_PointToAngle2(listener->x, listener->y, origin->x, origin->y);
    if (angle <= listener->angle)
      angle += 0xffffffff;
    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    params.separation = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);
  }

  if (!hexen || sfx->pitch)
  {
    params.pitch = (byte) (NORM_PITCH + (M_Random() & 7) - (M_Random() & 7));
  }
  else
  {
    params.pitch = NORM_PITCH;
  }

  channels[cnum].pitch = params.pitch;
  channels[cnum].handle = I_StartSound(sound_id, cnum, &params);
  channels[cnum].origin = origin;
  channels[cnum].sfxinfo = sfx;
  channels[cnum].priority = params.priority;
  channels[cnum].volume = volume; // original volume, not attenuated volume
  channels[cnum].ambient = params.ambient;
  channels[cnum].loop = params.loop;
  channels[cnum].loop_timeout = params.loop_timeout;
  channels[cnum].active = true;
  if (channels[cnum].ambient) // TODO: can ambient sounds even reach this flow?
    AmbChan = cnum;
}

void S_StartAmbientSound(void *_origin, int sound_id, int volume)
{
  sfxinfo_t *sfx;
  sfx_params_t params;
  mobj_t *origin;
  mobj_t *listener;
  int i;

  origin = (mobj_t *)_origin;
  listener = GetSoundListener();

  if (nosfxparm)
    return;

  if (sound_id == heretic_sfx_None || volume == 0)
    return;

  if (origin == NULL)
    origin = listener;

  sfx = &S_sfx[sound_id];

  params.volume = (volume * (sfx_volume + 1) * 8) >> 7;
  params.pitch = (byte) (NORM_PITCH - (M_Random() & 3) + (M_Random() & 3));
  params.priority = 1; // super low priority
  params.separation = 128;
  params.sfx_class = sfx_class_none;
  params.ambient = true;
  params.loop = false;
  params.loop_timeout = 0;

  // no priority checking, as ambient sounds would be the LOWEST.
  for (i = 0; i < numChannels; i++)
    if (channels[i].origin == NULL)
      break;

  if (i >= numChannels)
    return;

  if (sfx->lumpnum <= 0)
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

  channels[i].pitch = params.pitch;
  channels[i].handle = I_StartSound(sound_id, i, &params);
  channels[i].origin = origin;
  channels[i].sfxinfo = sfx;
  channels[i].priority = params.priority;
  channels[i].ambient = params.ambient;
  channels[i].loop = params.loop;
  channels[i].loop_timeout = params.loop_timeout;
  channels[i].active = true;
}

static void Heretic_S_StopSound(void *_origin)
{
  mobj_t *origin = _origin;
  int i;

  //jff 1/22/98 return if sound is not enabled
  if (nosfxparm)
    return;

  for (i = 0; i < numChannels; i++)
  {
    if (channels[i].active && channels[i].origin == origin)
    {
      S_StopChannel(i);
    }
  }
}

// hexen

dboolean S_GetSoundPlayingInfo(void * origin, int sound_id)
{
    int i;
    sfxinfo_t *sfx;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return false;

    sfx = &S_sfx[sound_id];

    for (i = 0; i < numChannels; i++)
    {
        if (channels[i].active && channels[i].sfxinfo == sfx && channels[i].origin == origin)
        {
            if (I_SoundIsPlaying(channels[i].handle))
            {
                return true;
            }
        }
    }
    return false;
}

int S_GetSoundID(const char *name)
{
    int i;

    for (i = 0; i < num_sfx; i++)
    {
        if (!strcmp(S_sfx[i].tagname, name))
        {
            return i;
        }
    }
    return 0;
}

void S_StartSongName(const char *songLump, dboolean loop)
{
    int musicnum;

    // lazy shortcut hack - this is a unique character
    switch (songLump[1])
    {
      case 'e':
        musicnum = hexen_mus_hexen;
        break;
      case 'u':
        musicnum = hexen_mus_hub;
        break;
      case 'a':
        musicnum = hexen_mus_hall;
        break;
      case 'r':
        musicnum = hexen_mus_orb;
        break;
      case 'h':
        musicnum = hexen_mus_chess;
        break;
      default:
        musicnum = hexen_mus_hub;
        break;
    }

    S_ChangeMusic(musicnum, loop);
}
