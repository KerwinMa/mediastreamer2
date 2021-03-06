/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006-2013 Belledonne Communications, Grenoble

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "mediastreamer2_tester.h"
#include "mediastreamer2/msmediaplayer.h"

static int tester_init() {
	ms_init();
	return 0;
}

static int tester_cleanup() {
	ms_exit();
	return 0;
}

typedef struct _Eof {
	bool_t eof;
	int time_ms;
	ms_mutex_t mutex;
} Eof;

static void eof_init(Eof *obj) {
	obj->eof = FALSE;
	obj->time_ms = 0;
	ms_mutex_init(&obj->mutex, NULL);
}

static void eof_callback(void *user_data) {
	Eof *obj = (Eof *)user_data;
	ms_mutex_lock(&obj->mutex);
	obj->eof = TRUE;
	ms_mutex_unlock(&obj->mutex);
}

static void wait_for_eof(Eof *obj, int refresh_time_ms, int timeout_ms) {
	ms_mutex_lock(&obj->mutex);
	while(obj->time_ms < timeout_ms && !obj->eof) {
		ms_mutex_unlock(&obj->mutex);
		ms_usleep((useconds_t)(refresh_time_ms) * 1000U);
		obj->time_ms += refresh_time_ms;
		ms_mutex_lock(&obj->mutex);
	}
	ms_mutex_unlock(&obj->mutex);
}

static void play_file(const char *filepath, bool_t unsupported_format, bool_t seeking_test, bool_t play_twice) {
	bool_t succeed;
	Eof eof;
	MSMediaPlayer *file_player = NULL;
	MSSndCard *snd_card = ms_snd_card_manager_get_default_playback_card(ms_snd_card_manager_get());
	const char *display_name = video_stream_get_default_video_renderer();
	int duration, timeout;
	const int seek_time = 6100;
	const double timeout_prec = 0.05;

	eof_init(&eof);

	CU_ASSERT_PTR_NOT_NULL(snd_card);
	file_player = ms_media_player_new(snd_card, display_name, 0);
	CU_ASSERT_PTR_NOT_NULL(file_player);
	if(file_player == NULL) return;

	CU_ASSERT_EQUAL(ms_media_player_get_state(file_player), MSPlayerClosed);
	ms_media_player_set_eof_callback(file_player, eof_callback, &eof);

	succeed = ms_media_player_open(file_player, filepath);
	if(unsupported_format) {
		CU_ASSERT_FALSE(succeed);
		CU_ASSERT_EQUAL(ms_media_player_get_state(file_player), MSPlayerClosed);
	} else {
		CU_ASSERT_TRUE(succeed);
		CU_ASSERT_EQUAL(ms_media_player_get_state(file_player), MSPlayerPaused);
	}
	if(!succeed) {
		ms_media_player_free(file_player);
		return;
	}

	duration = ms_media_player_get_duration(file_player);
	if(ms_media_player_get_file_format(file_player) == MS_FILE_FORMAT_WAVE) {
		CU_ASSERT_TRUE(duration == -1);
		duration = 20000;
	} else {
		CU_ASSERT_TRUE(duration >= 0);
	}

	if(seeking_test) {
		timeout = (duration - seek_time) * (1 + timeout_prec);
	} else {
		timeout = duration * (1 + timeout_prec);
	}

	succeed = ms_media_player_start(file_player);
	CU_ASSERT_TRUE(succeed);
	CU_ASSERT_EQUAL(ms_media_player_get_state(file_player), MSPlayerPlaying);

	if(seeking_test) {
		CU_ASSERT_TRUE(ms_media_player_seek(file_player, seek_time));
	}

	if(succeed) {
		wait_for_eof(&eof, 100, timeout);
		CU_ASSERT_TRUE(eof.eof);
	}
	ms_media_player_close(file_player);
	CU_ASSERT_EQUAL(ms_media_player_get_state(file_player), MSPlayerClosed);

	if(play_twice) {
		eof_init(&eof);
		CU_ASSERT_TRUE(ms_media_player_open(file_player, filepath));
		CU_ASSERT_TRUE(ms_media_player_start(file_player));
		wait_for_eof(&eof, 100, timeout);
		ms_media_player_close(file_player);
		CU_ASSERT_TRUE(eof.eof);
	}
	ms_media_player_free(file_player);
}

static void play_root_file(const char *filepath, bool_t unsupported_format, bool_t seeking_test, bool_t play_twice){
    char* file = ms_strdup_printf("%s/%s", mediastreamer2_tester_get_file_root(), filepath);
	play_file(file, unsupported_format, seeking_test, play_twice);
    ms_free(file);
}

static void play_hello_8000_wav(void) {
	play_root_file("sounds/hello8000.wav", FALSE, FALSE, FALSE);
}

static void play_hello_16000_wav(void) {
	play_root_file("sounds/hello16000.wav", FALSE, FALSE, FALSE);
}

static void play_hello_pcmu_mka(void) {
	play_root_file("sounds/hello_pcmu.mka", !ms_media_player_matroska_supported(), FALSE, FALSE);
}

static void play_hello_opus_mka(void) {
	play_root_file("sounds/hello_opus.mka", !ms_media_player_matroska_supported(), FALSE, FALSE);
}

static void play_sintel_trailer_pcmu_h264_mkv(void) {
	play_root_file("sounds/sintel_trailer_pcmu_h264.mkv", !ms_media_player_matroska_supported(), FALSE, FALSE);
}

static void play_sintel_trailer_opus_h264_mkv(void) {
	play_root_file("sounds/sintel_trailer_opus_h264.mkv", !ms_media_player_matroska_supported(), FALSE, FALSE);
}

static void seeking_test(void) {
	play_root_file("sounds/sintel_trailer_opus_h264.mkv", !ms_media_player_matroska_supported(), TRUE, FALSE);
}

static void playing_twice_test(void) {
	play_root_file("./sounds/sintel_trailer_opus_h264.mkv", !ms_media_player_matroska_supported(), FALSE, TRUE);
}

static test_t tests[] = {
	{	"Play hello8000.wav"                 ,	play_hello_8000_wav                },
	{	"Play hello16000.wav"                ,	play_hello_16000_wav               },
	{	"Play hello_pcmu.mka"                ,	play_hello_pcmu_mka                },
	{	"Play hello_opus.mka"                ,	play_hello_opus_mka                },
	{	"Play sintel_trailer_pcmu_h264.mkv"  ,	play_sintel_trailer_pcmu_h264_mkv  },
	{	"Play sintel_trailer_opus_h264.mkv"  ,	play_sintel_trailer_opus_h264_mkv  },
	{	"Seeking"                            ,	seeking_test                       },
	{	"Playing twice"                      ,	playing_twice_test                 }
};

test_suite_t player_test_suite = {
	"Player",
	tester_init,
	tester_cleanup,
	sizeof(tests)/sizeof(test_t),
	tests
};
