#ifndef SOUND_H
#define SOUND_H

#include <alsa/asoundlib.h>
#include <stdio.h>
//==========================================================
/*
@params number_of_frames is measured in timestamp increase so 960 is equal to 160 samples
*/
typedef struct mic_config_t{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	char device[7];
	snd_pcm_stream_t stream;
	snd_pcm_access_t access;
	snd_pcm_format_t format;
	unsigned int number_of_channels;
	unsigned int sample_rate;
	snd_pcm_uframes_t number_of_frames;
	unsigned int period_size;
} mic_config_t;

//==========================================================
/*
@return 0 if success, -1 if not
*/
int create_mic(REF orqa_mic_config_t *params);
int destroy_mic(REF snd_pcm_t *handle);

//==========================================================
/*
@params output_buffer memset it to 0 before use
*/
int record(IN orqa_mic_config_t *sph0645_mic, OUT char *output_buffer);

//==========================================================

#endif //SOUND_H