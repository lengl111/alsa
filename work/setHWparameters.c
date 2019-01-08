/***************************
* Opening PCM devices and setting parameters.
*
****************************/


/* operation steps:
	1. open pcm device;
	2. alloc hw parameters struct;
	3. set default hw parameters;
#	4. set access type;
#	5. set sample length(format);
#	6. set channel numbers;
#	7. set sample rate;
	8. close pcm device.
	
*/

#include <alsa/asoundlib.h>

int main(void)
{
	int rc;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int rate,val,val2;
	int dir;
	snd_pcm_uframes_t frames;

	/* snd_pcm_open *
	   mode: 
		0: the standard mode is used.       
  	   	SND_PCM_NONBLOCK: read/write access to the PCM device will return immediately. 
  	   	SND_PCM_ASYNC: SIGIO will be emitted whenever a period has been completely processed 
  				 by the soundcard.
  	*/
  
	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

	/* Alloc a hardware parameters object in stack */
	//在栈上分配内存
    snd_pcm_hw_params_alloca(&params);

	/* Fill it with default values */
    snd_pcm_hw_params_any(handle, params);

	 /* set access type */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	 

	/* signed 16 bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

	
	/* two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, 2);

	
	/* 44100 bits/second sampling rate (CD quality) */
	rate = 44100;
	/* rate == hw_support_rate, dir = 0
       rate <  hw_support_rate, dir = -1
       rate >  hw_support_rate, dir = 1
       dir 返回得到的值表明实际的rate 是高，低，等于。
       dir 根据实际硬件所能支持的采样率决定.
    */

	//printf("[0]----- %d ---\n",dir);

	//snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
	//printf("[1]----- %d ---\n",dir);

	 /* write params to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw params: %s\n", snd_strerror(rc));
        exit(1);
    }

	/* display information about PCM interface */
    printf("PCM handle name=%s\n", snd_pcm_name(handle));
    printf("PCM state=%s\n", snd_pcm_state_name(snd_pcm_state(handle)));

	snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *)&val);
    printf("access type=%s\n", snd_pcm_access_name((snd_pcm_access_t)val));

	snd_pcm_hw_params_get_format(params, (snd_pcm_format_t *)&val);
    printf("format=%s (%s)\n", snd_pcm_format_name((snd_pcm_format_t)val),
                snd_pcm_format_description((snd_pcm_format_t)val));

    snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *)&val);
    printf("subformat=%s (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t)val),
                snd_pcm_subformat_description((snd_pcm_subformat_t)val));

	
	snd_pcm_hw_params_get_channels(params, &val);
	printf("channels=%d\n", val);
	
	snd_pcm_hw_params_get_rate(params, &val, &dir);
	printf("rate=%d bps\n", val);
	
	snd_pcm_hw_params_get_period_time(params, &val, &dir);
	printf("period time=%d us\n", val);
	
	snd_pcm_hw_params_get_period_size(params, (snd_pcm_uframes_t *)&frames, &dir);
	printf("period size=%d frames\n", (int)frames);//add (int) to avoid compile warning
	
	snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
	printf("buffer time=%d us\n", val);

	snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t *)&frames);
    printf("buffer size=%d frames\n", (int)frames);//add (int) to avoid compile warning
    snd_pcm_hw_params_get_periods(params, &val, &dir);
    printf("periods per buffer=%d frames\n", val);

	snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
    printf("exact rate=%d/%d bps\n", val, val2);

    val = snd_pcm_hw_params_get_sbits(params);
    printf("significant bits=%d\n", val);

    snd_pcm_hw_params_get_tick_time(params, &val, &dir);
    printf("tick time=%d us\n", val);

    val = snd_pcm_hw_params_is_batch(params);
    printf("is batch=%d\n", val);

    val = snd_pcm_hw_params_is_block_transfer(params);
    printf("is block transfer=%d\n", val);

    val = snd_pcm_hw_params_is_double(params);
    printf("is double=%d\n", val);

    val = snd_pcm_hw_params_is_half_duplex(params);
    printf("is half duplex=%d\n", val);

    val = snd_pcm_hw_params_is_joint_duplex(params);
    printf("is jonit duplex=%d\n", val);

    val = snd_pcm_hw_params_can_overrange(params);
    printf("can overrange=%d\n", val);

    val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
    printf("can mmap=%d\n", val);

    val = snd_pcm_hw_params_can_pause(params);
    printf("can pause=%d\n", val);

    val = snd_pcm_hw_params_can_resume(params);
    printf("can resume=%d\n", val);

    val = snd_pcm_hw_params_can_sync_start(params);
    printf("can sync start=%d\n", val);
	
	snd_pcm_close(handle);
	
	return 0;
}

