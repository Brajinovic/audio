<details>
<summary>

# ALSA

</summary>

## Device selection
If you wish to record using the ALSA library, the first orderof business is to select your recording device. Even if you have only your microphone pluggd in, there will probably be at least one more option to choose from and the ALSA library will choose the default one which is probably not the one you want. In order to list available recording devices, run the command `arecord -l`. The output is as follows:

```
**** List of CAPTURE Hardware Devices ****
card 0: PCH  [HDA Intel PCH], device 0: ALC3204 Analog [ALC3204 Analog]
		Subdevices: 1/1
		Subdevice #0: subdevice #0
```

## Recording
Here we can see the card index and the device index. We will need those 2 indexes to select the recording device.
The following command is used to record the audio using the above mentioned card:
```arecord -D hw:0,0 -f s16_le -r 48000 -c 1 recording.raw```	
		
What you need to know when recording:
### Format
The format tells the computer which number format to use when encoding the analog audio signal to numerals out of which the digital signal consists of.

### Sample rate
As we are trying to capture audio which are analog sounds and convert them to digital, we need to tell the microphone how often will he sample the analog signal(wave). Those samples combined create the digital audio representation of the analog sound. In this application, the sample rate unit is 48000 which means every 1/48000 seconds a sample is taken of the analog audio signal.

### Number of channels
The number of channels property tells us exactly that, the number of channels we will use to store our sound in and later reproduce. If 1 that is Mono, one channel is used and if you try to listen to your captured audio over your headphones, the sound will emmit only from one headphone. 2 means Stereo that is 2 channels are used and you will hear sound comming from both headphones. If you want stereo you need 2 sph0645 microphones(they are the ones I used, you connect them together), I cannot tell for sure about other microphones.
		
This is where you determine the parameters of your audio file. You cannot record the audio with 1 channel, sample rate 48000 and then when it comes to encoding change the channel count to 2, the sample rate to 96000 etc. I mean, you can but it will not affect the encoded audio in any or good way.
	
Be carefull about selecting the sample rate(or any other parameter) as not all of them are supported by your desired encoder. Check your encoder capabilities before deciding the settings for capturing audio.

## ALSA recording - sound.c 

Depends on: asoundlib.h, stdio.h

### Create mic
In order to start recording, you need to setup the microphone. But firstly, you need to populate the structure *mic_config_t*. After that you pass that structure pointer to the *create_mic*. The function will write the pointer of the handle to the structure field *handle*.

###	Record
When it comes to recording, again, you only pass the *mic_config_t* structure pointer in addition with the output buffer pointer where the captured raw audio data will be stored. The size of the output buffer technically only needs to be the size of the frame you are have configured. 

###	Destroy mic
In order to destroy the microphone handle, you pass the handle pointer to the *destroy_mic* function.

</details>
##############################################################################
<details>
<summary>

# Encoding audio

</summary>

##	Sample rate
As we are trying to capture audio which are analog sounds and convert them to digital, we need to tell the microphone how often will he sample the analog signal(wave). Those samples combined create the digital audio representation of the analog sound. In this application, the sample rate unit is 48000 which means every 1/48000 seconds a sample is taken of the analog audio signal.

##	Number of channels
The number of channels property tells us exactly that, the number of channels we will use to store our sound in and later reproduce. If 1 that is Mono, one channel is used and if you try to listen to your captured audio over your headphones, the sound will emmit only from one headphone. 2 means Stereo that is 2 channels are used and you will hear sound comming from both headphones. If you want stereo you need 2 sph0645 microphones(they are the ones I used, you connect them together), I cannot tell for sure about other microphones.

When encoding audio, the number of channels is determent by the raw audio, so if you record it in Mono mode, you can try to encode it in Stereo but it will always be Mono.

##	Bitrate
Bitrate represents the amount of data being transferred into audio. That is how fast does the computer shoot out sampled data out to your speakers. As you can notice, higher bitrate generally equals to better audio but there are a few limitations. You cannot put the bitrate to the max value if other parameters are not up for it. Hence, everything needs to be balanced. 

##	Frame size 
Number of samples in a frame, can be confusing as it is refered to my the timestamp increment in the code so 960 timestamp increment is really 160 samples. This is very important as it gave me quite a few headaces before I got the hang of it whilst implementing audio in WebRTC. The default size of ALSA is 160 samples per frame but you can change it if needed.

My configuration:
	Sample rate: 48000
	Number of channels: 2
	Bitrate: 64000
	Frame size: 160

## OPUS encoder - opus_enc.h

Depends on: opus.h, sound.h, stdio.h, threads.h


### Create encoder handle
For creating the encoder handle we need to create the *encoder_config_t* structure and populate it with our desired configuration. Later, we call the *orqa_create_encoder* function with the structure pointer being the only argument. The encoder handle is going to be placed inside the structure, in the *encoder* field of the structure.

### Encode
In order to encode raw audio data, we need to call *encoder_config_t* whilst passing the *orqa_encoder_config_t* structure pointer, raw data pointer and output buffer pointer for the encoded OPUS data.

### Record and encode - orqa_capture_audio
This function was made to be run in a different thread than the main program. It requires the *opus_context_t* structure pointer as an argument. The structure holds all the before mentioned structures and buffers: *raw_audio_buffer*, *opus_audio_buffer*, *mic_config_t*, *encoder_config_t*, *thread_handle*. This function calls the *record* function and fills the *raw_audio_buffer* after which the *encode* function is called to encode the captured data and the encoded data is saved in the *opus_audio_buffer* buffer.

</details>
##############################################################################################