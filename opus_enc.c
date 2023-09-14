#include <opus_enc.h>
#include <opus/opus.h>
#include <stdio.h>
#include <sound.h>
#include <threads.h>


int create_encoder(IN encoder_config_t *params){
	OpusEncoder *encoder;
	int error;
	//configure the encoder by setting the sample rate, number of channels, mode
	//the OPUS_APPLICATION_VOIP is the best one for VoIP and videoconferencing
	encoder = opus_encoder_create(params->sample_rate, params->number_of_channels, OPUS_APPLICATION_AUDIO, &error);
	if (error<0)
   	{
      fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(error));
      return -1;
   	}
   	//add the bitrate to the configuration
   	error = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(params->bitrate));
   	if (error<0)
   	{
      fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(error));
      return -1;
  	}
  	//write the encoder pointer to the structure
  	params->encoder = encoder;

	return 0;
}

int destroy_encoder(REF OpusEncoder* encoder){
	opus_encoder_destroy(encoder);

	return 0;
}

int encode(REF encoder_config_t *params, IN const opus_int16 *input_buffer, OUT char *output_buffer){
	int number_of_bytes= 0;
	//encode the data from the input_buffer and place it in the output_buffer
	//maybe needs to be converted from litle endian formating
	number_of_bytes = opus_encode(params->encoder, input_buffer, params->frame_size, output_buffer, 160);
  //fprintf(stderr, "Number of encoded bytes: %d\n", number_of_bytes);
	if (number_of_bytes<0)
    {
    	fprintf(stderr, "encode failed: %s\n", opus_strerror(number_of_bytes));
    	return -1;
    }

	return number_of_bytes;
}

int capture_audio(REF void *const thread_context){
    int index = 0;
    opus_context_t *context =(opus_context_t *)thread_context; 
    thread_set_alive_flag(context->thread_handle, 1);

    while(!thread_is_stopping(context->thread_handle)){
        memset(context->raw_audio_buffer, 0, SAMPLE_SIZE);
        if(record(context->sph0645_mic, context->raw_audio_buffer) <= 0){
        	printf("Error whilst recording!\n");
        	return -1;
        }
        
        memset(context->opus_audio_buffer + (SAMPLE_SIZE * index), 0, SAMPLE_SIZE);
        encode(context->opus_encoder, context->raw_audio_buffer, context->opus_audio_buffer + (SAMPLE_SIZE * index));
        index = (index + 1) % 10;
    }


    return 0;
}