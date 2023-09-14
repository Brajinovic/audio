#include <sound.h>
#include <alsa/asoundlib.h>
#include <stdio.h>


int create_mic(REF mic_config_t *mic_config){
	snd_pcm_hw_params_t *params;
	snd_pcm_t *handle;
	int rc;
	int dir;
	
	rc = snd_pcm_open(&handle, mic_config->device, SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		exit(1);
	}
	mic_config->handle = handle;
	/* Allocate a hardware parameters object. */
  	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(mic_config->handle, params);

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(mic_config->handle, params, mic_config->access);

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(mic_config->handle, params, mic_config->format);

	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(mic_config->handle, params, mic_config->number_of_channels);

	/* Sampling rate */
	snd_pcm_hw_params_set_rate_near(mic_config->handle, params, &mic_config->sample_rate, &dir);

	/* Set period size */
  	snd_pcm_hw_params_set_period_size_near(mic_config->handle, params, &mic_config->number_of_frames, &dir);

  	/* Write the parameters to the driver */
  	rc = snd_pcm_hw_params(mic_config->handle, params);
	if (rc < 0) {
		fprintf(stderr,"unable to set hw parameters: %s\n",
		snd_strerror(rc));
		return -1;
	}

	/* Use a buffer large enough to hold one period */
  	snd_pcm_hw_params_get_period_size(params, &mic_config->number_of_frames, &dir);

  	snd_pcm_hw_params_get_period_time(params, &mic_config->period_size, &dir);

  	return 0;
}

int destroy_mic(REF snd_pcm_t *handle){
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	return 0;
}

int record(IN  mic_config_t *mic_config, OUT char *output_buffer){
	int characters_read = 0;
	characters_read = snd_pcm_readi(mic_config->handle, output_buffer, mic_config->number_of_frames);

	if (characters_read == -EPIPE) {
	  /* EPIPE means overrun */
	  fprintf(stderr, "overrun occurred\n");
	  snd_pcm_prepare(mic_config->handle);
	  return -1;

	} else if (characters_read < 0) {
	  fprintf(stderr, "error from read: %s\n", snd_strerror(characters_read));
	  return -1;

	} else if (characters_read != (int)mic_config->number_of_frames) {
	  fprintf(stderr, "short read, read %d frames\n", characters_read);
	  return -1;
	}

	return characters_read;
}