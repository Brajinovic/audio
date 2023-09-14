#ifndef ORQA_OPUS_ENC_H
#define ORQA_OPUS_ENC_H

#include <opus/opus.h>
#include <orqa/sound.h>
#include <orqa/code_markers.h>

#include <orqa/utility/threads.h>
#define MAX_PACKET_SIZE (3*1276)
#define MAX_FRAME_SIZE 6*960
#define SAMPLE_SIZE 160

//==========================================================
/*

*/
typedef struct orqa_encoder_config_t{
	opus_int32 sample_rate;
	int number_of_channels;
	opus_int32 bitrate;
	OpusEncoder* encoder;
	int frame_size;
}orqa_encoder_config_t;

//structure used for working with threads
typedef struct orqa_opus_context_t{
	char* raw_audio_buffer;
	char* opus_audio_buffer;
	orqa_mic_config_t *sph0645_mic;
	orqa_encoder_config_t *opus_encoder;
	orqa_thread_t *thread_handle;
}orqa_opus_context_t;

//==========================================================
/*
@return 0 if successful, -1 otherwise
*/
int orqa_create_encoder(ORQA_IN orqa_encoder_config_t *params);
int orqa_destroy_encoder(ORQA_REF OpusEncoder* encoder);

//==========================================================
/*
@params output_buffer memset to 0 before use
*/
int orqa_encode(ORQA_REF orqa_encoder_config_t *params, ORQA_IN const opus_int16 *input_buffer, ORQA_OUT char *output_buffer);
int orqa_capture_audio(ORQA_REF void *const thread_context);

//==========================================================

#endif