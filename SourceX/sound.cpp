#include "all.h"
#include "../3rdParty/Storm/Source/storm.h"
#include "stubs.h"
#include <SDL.h>
#include <SDL_mixer.h>

namespace dvl {

BOOLEAN gbSndInited;
int sglMusicVolume;
int sglSoundVolume;
HANDLE sghMusic;

Mix_Music *music;
SDL_RWops *musicRw;
char *musicBuffer;

/* data */

BOOLEAN gbMusicOn = true;
BOOLEAN gbSoundOn = true;
int sgnMusicTrack = NUM_MUSIC;

char *sgszMusicTracks[NUM_MUSIC] = {
#ifdef SPAWN
	"Music\\sTowne.wav",
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
	"Music\\sintro.wav",
#else
	"Music\\DTowne.wav",
	"Music\\DLvlA.wav",
	"Music\\DLvlB.wav",
	"Music\\DLvlC.wav",
	"Music\\DLvlD.wav",
	"Music\\Dintro.wav",
#endif
};

BOOL snd_playing(TSnd *pSnd)
{
	if (pSnd == NULL || pSnd->DSB == NULL)
		return false;

	return pSnd->DSB->IsPlaying();
}

void snd_play_snd(TSnd *pSnd, int lVolume, int lPan)
{
	SoundSample *DSB;
	DWORD tc;

	if (!pSnd || !gbSoundOn) {
		return;
	}

	DSB = pSnd->DSB;
	if (DSB == NULL) {
		return;
	}

	tc = SDL_GetTicks();
	if (tc - pSnd->start_tc < 80) {
		return;
	}

	lVolume += sglSoundVolume;
	if (lVolume < VOLUME_MIN) {
		lVolume = VOLUME_MIN;
	} else if (lVolume > VOLUME_MAX) {
		lVolume = VOLUME_MAX;
	}
	DSB->Play(lVolume, lPan);
	pSnd->start_tc = tc;
}

TSnd *sound_file_load(char *path)
{
	HANDLE file;
	BYTE *wave_file;
	TSnd *pSnd;
	DWORD dwBytes;
	int error;

	WOpenFile(path, &file, false);
	pSnd = (TSnd *)DiabloAllocPtr(sizeof(TSnd));
	memset(pSnd, 0, sizeof(TSnd));
	pSnd->sound_path = path;
	pSnd->start_tc = SDL_GetTicks() - 80 - 1;

	dwBytes = SFileGetFileSize(file, NULL);
	wave_file = DiabloAllocPtr(dwBytes);
	SFileReadFile(file, wave_file, dwBytes, NULL, NULL);

	pSnd->DSB = new SoundSample();
	error = pSnd->DSB->SetChunk(wave_file, dwBytes);
	WCloseFile(file);
	mem_free_dbg(wave_file);
	if (error != 0) {
		ErrSdl();
	}

	return pSnd;
}

void sound_file_cleanup(TSnd *sound_file)
{
	if (sound_file) {
		if (sound_file->DSB) {
			sound_file->DSB->Stop();
			sound_file->DSB->Release();
			delete sound_file->DSB;
			sound_file->DSB = NULL;
		}

		mem_free_dbg(sound_file);
	}
}

void snd_init(HWND hWnd)
{
	snd_get_volume("Sound Volume", &sglSoundVolume);
	gbSoundOn = sglSoundVolume > VOLUME_MIN;
	sgbSaveSoundOn = gbSoundOn;

	snd_get_volume("Music Volume", &sglMusicVolume);
	gbMusicOn = sglMusicVolume > VOLUME_MIN;

	int result = Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024);
	if (result < 0) {
		SDL_Log(Mix_GetError());
	}
	Mix_AllocateChannels(25);
	Mix_ReserveChannels(1); // reserve one channel for naration (SFileDda*)

	gbSndInited = true;
}

void snd_get_volume(char *value_name, int *value)
{
	int v = *value;
	if (!SRegLoadValue("Diablo", value_name, 0, &v)) {
		v = VOLUME_MAX;
	}
	*value = v;

	if (*value < VOLUME_MIN) {
		*value = VOLUME_MIN;
	} else if (*value > VOLUME_MAX) {
		*value = VOLUME_MAX;
	}
	*value -= *value % 100;
}

void sound_cleanup()
{
	SFileDdaDestroy();

	if (gbSndInited) {
		gbSndInited = false;
		snd_set_volume("Sound Volume", sglSoundVolume);
		snd_set_volume("Music Volume", sglMusicVolume);
	}
}

void snd_set_volume(char *key, int value)
{
	SRegSaveValue("Diablo", key, 0, value);
}

void music_stop()
{
	if (sghMusic) {
		Mix_HaltMusic();
		SFileCloseFile(sghMusic);
		sghMusic = NULL;
		Mix_FreeMusic(music);
		music = NULL;
		musicRw = NULL;
		mem_free_dbg(musicBuffer);
		sgnMusicTrack = NUM_MUSIC;
	}
}

#ifdef _OGG_MUSIC
struct sMusicOgg
{
std::string strOggFile;
int iTrack;
};

sMusicOgg OGGTracks[6] = 
{
	{"dtowne.ogg", TMUSIC_TOWN},
	{"dlvla.ogg", TMUSIC_L1},
	{"dlvlb.ogg", TMUSIC_L2},
	{"dlvlc.ogg", TMUSIC_L3},
	{"dlvld.ogg", TMUSIC_L4},
	{"dintro.ogg", TMUSIC_INTRO}
};
#endif

void music_start(int nTrack)
{
#ifdef _OGG_MUSIC
	assert((DWORD)nTrack < NUM_MUSIC);
	music_stop();

	if (gbMusicOn)
	{
		for(int i = 0; i < sizeof(OGGTracks)/sizeof(OGGTracks[0]); i++)
		{
			if(OGGTracks[i].iTrack == nTrack)
			{
				std::string strPath("D:\\assets\\music\\");
				strPath += OGGTracks[i].strOggFile;
				music = Mix_LoadMUS(strPath.c_str());
				Mix_VolumeMusic(MIX_MAX_VOLUME - MIX_MAX_VOLUME * sglMusicVolume / VOLUME_MIN);
				Mix_PlayMusic(music, -1);
				break;
			}
		}
		sgnMusicTrack = nTrack;
	}
#else
	BOOL success;

	assert((DWORD)nTrack < NUM_MUSIC);
	music_stop();
	if (gbMusicOn) {
		success = SFileOpenFile(sgszMusicTracks[nTrack], &sghMusic);
		if (!success) {
			sghMusic = NULL;
		} else {
			int bytestoread = SFileGetFileSize(sghMusic, 0);
			musicBuffer = (char *)DiabloAllocPtr(bytestoread);
			SFileReadFile(sghMusic, musicBuffer, bytestoread, NULL, 0);

			musicRw = SDL_RWFromConstMem(musicBuffer, bytestoread);
			if (musicRw == NULL) {
				ErrSdl();
			}
			music = Mix_LoadMUSType_RW(musicRw, MUS_NONE, 1);
			Mix_VolumeMusic(MIX_MAX_VOLUME - MIX_MAX_VOLUME * sglMusicVolume / VOLUME_MIN);
			Mix_PlayMusic(music, -1);

			sgnMusicTrack = nTrack;
		}
	}
#endif
}

void sound_disable_music(BOOL disable)
{
	if (disable) {
		music_stop();
	} else if (sgnMusicTrack != NUM_MUSIC) {
		music_start(sgnMusicTrack);
	}
}

int sound_get_or_set_music_volume(int volume)
{
	if (volume == 1)
		return sglMusicVolume;

	sglMusicVolume = volume;

	if (sghMusic)
		SFileDdaSetVolume(sghMusic, volume, 0);

	return sglMusicVolume;
}

int sound_get_or_set_sound_volume(int volume)
{
	if (volume == 1)
		return sglSoundVolume;

	sglSoundVolume = volume;

	return sglSoundVolume;
}

} // namespace dvl
