<details>
<summary>

# DeviceTree

</summary>
This solution uses the SAI5 interface and ALSA to record and play audio using the SPH0645 digital microphone and I2S communication protocol.In order to use the SAI5 interface, we need to enable and configure it in the device tree file (in my case it is imx8mm-evk.dtsi). 
Our first order of business is adding the codec_dummy node and sound-sai5 interface node to /. The codec_dummy node is important for recording audio. Usually you would use a codec to convert the analog audio signals to digital form, but we have a digital microphone which does this step already so we only need to forward the data we get from the microphone. Hence, we are using the "snd-soc-dummy" codec.
	
	```
	codec_dummy: codec_dummy {
    	compatible = "asoc,snd-soc-dummy";
    	#sound-dai-cells = <0>;
    	frame-master;
    	bitclock-master;
    	status = "okay";
    	};
    ```

Secondly, we add the sound-sai5 node which represents the binding of the SAI5 interface and the dummy_codec in order to create an interface recognizable and usable by ALSA. Here we are using the "simple-audio-card" generic driver as we don't have a working sph0645 driver, the ones I have found are outdated and do not work(the stock version does not even compile but when I 'fix' the errors and it does compile, it does not work). In this node we set the interface name that will show up later in the OS, the communication format(I2S), we link the SAI5 interface and select which clock we are using.

```
sound-sai5 {
		compatible = "simple-audio-card";
	
		simple-audio-card,name="sound | sai5";
		simple-audio-card,format="i2s";
	
		simple-audio-card,frame-master=<&sai5>;
		simple-audio-card,bitclock-master=<&sai5>;
		status="okay";
	
		cpu_dai:simple-audio-card,cpu{
				sound-dai=<&sai5>;
				system-clock-frequency=<&clk IMX8MM_CLK_SAI5_ROOT>;
		};
	
		codec_dai: simple-audio-card,codec{
				sound-dai=<&codec_dummy>;
				system-clock-frequency=<&clk IMX8MM_CLK_SAI5_ROOT>;
		};
	};
```

Next, we disable the "lcdif" node. Why? Because the node uses SAI5 pins which creates collisions... check out the "pinctrl_pdm" node under "iomuxc". To disable the node, change the "status" property from "okay" to "disabled":

	status = "disabled";

Next, we toggle the ext_osc value of the "pcie0" node and disable the node as it gives us errors on bootup:

	ext_osc = <0>;
	status = "disabled";


We do the same for the "pcie0_ep" node.

Next, we edit the "sai5" node so it looks like this:

	```
	&sai5 {
		compatible="fsl,imx8mm-sai";
    	pinctrl-names = "default";
    	pinctrl-0 = <&pinctrl_sai5>;
		assigned-clocks = <&clk IMX8MM_CLK_SAI5>;
		assigned-clock-parents = <&clk IMX8MM_AUDIO_PLL1_OUT>;
		assigned-clock-rates = <24576000>;
        	
    	fsl,sai-mclk-direction-output;
    	fsl,txmasterflag = <0>;
    	fsl,mode="i2s-master";
    	status="okay";
	};
	```
The frequency can be ether 24576000 or 12288000 since the BSP PLL-s work with only those 2 frequencies.

The last thing for the device tree, we need to define which pins we are going to use. We do that in the "pinctrl_sai5" node under the "iomuxc". In order to find out the exact names I need, I searched the documentation(the biggest one, 5k+ page one), chapter 8, Chip IO and Pinmux. The addresses of the pins are all the same, namely 0xd6. Don't let it fool you.There is a header file with the correct addresses, your only job here is to specity the correct pin name and function and you are set.

	pinctrl_sai5: sai5grp {
		fsl,pins = <
			MX8MM_IOMUXC_SAI5_MCLK_SAI5_MCLK	0xd6
			MX8MM_IOMUXC_SAI5_RXD0_SAI5_RX_DATA0	0xd6
			MX8MM_IOMUXC_SAI5_RXD1_SAI5_TX_SYNC    0xd6
			MX8MM_IOMUXC_SAI5_RXD2_SAI5_TX_BCLK    0xd6
		>;
	};


Lastly, we need to edit the /sound/soc/soc-utils.c file. We need to add a few things to it, all of which is documented in the patch file. Namely, we are adding a "snd_soc_dummy" configuraton without which we will not see the microphone with ALSA.

After all of the changes, we build the project and flash it to the board.When the system is all booted up, after running the "arecord -l" command, we should see  the "sound | sai5" interface (or a different named one if you changed the name) and we should be able to interact with it.
</details>
################################################################################
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
In order to start recording, you need to setup the microphone. But firstly, you need to populate the structure *orqa_mic_config_t*. After that you pass that structure pointer to the *orqa_create_mic*. The function will write the pointer of the handle to the structure field *handle*.

###	Record
When it comes to recording, again, you only pass the *orqa_mic_config_t* structure pointer in addition with the output buffer pointer where the captured raw audio data will be stored. The size of the output buffer technically only needs to be the size of the frame you are have configured. 

###	Destroy mic
In order to destroy the microphone handle, you pass the handle pointer to the *orqa_destroy_mic* function.

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
For creating the encoder handle we need to create the *orqa_encoder_config_t* structure and populate it with our desired configuration. Later, we call the *orqa_create_encoder* function with the structure pointer being the only argument. The encoder handle is going to be placed inside the structure, in the *encoder* field of the structure.

### Encode
In order to encode raw audio data, we need to call *orqa_encoder_config_t* whilst passing the *orqa_encoder_config_t* structure pointer, raw data pointer and output buffer pointer for the encoded OPUS data.

### Record and encode - orqa_capture_audio
This function was made to be run in a different thread than the main program. It requires the *orqa_opus_context_t* structure pointer as an argument. The structure holds all the before mentioned structures and buffers: *raw_audio_buffer*, *opus_audio_buffer*, *orqa_mic_config_t*, *orqa_encoder_config_t*, *thread_handle*. This function calls the *orqa_record* function and fills the *raw_audio_buffer* after which the *orqa_encode* function is called to encode the captured data and the encoded data is saved in the *opus_audio_buffer* buffer.

</details>
##############################################################################################
<details>
<summary>

# WebRTC - OPUS extension
</summary>

## SDP
To make the whole thing work one needs to edit the SDP offer in which to pay attention to the ports and sources beeing used for sending audio and video data. The ports must be the same if using the BUNDLE policy but the SSRC must be different. 

## Debugging
On Chrome there is the webrtc-internals utility which is handy to use when debugging. If the audio packets are sent and recieved correctly, the tool will pick up on the codec information(sample rate, encoder, number of chanells). It it does not pick it up, check the SDP configuration and the RTP packet being sent. Chances are one of those two things is fucked. 

## Audio payload
The audio data must start with 0xFC and be 160 bytes in size(including the 0xFC first byte). Atop of that we have 12 bytes of RTP header data. If you recieve the audio data but it is just noise or incomprehendable, check the encoder or the way to parse and prepair the data for sending. 

</details>