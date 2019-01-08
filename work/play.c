#include <alsa/asoundlib.h>

#define IS_PLAY 0

#if IS_PLAY
static char *device = "hw:0,2";			/* playback/arecord device */
#define STREAM_DIEC   SND_PCM_STREAM_PLAYBACK
#else
static char *device = "hw:0,3";	
#define STREAM_DIEC   SND_PCM_STREAM_CAPTURE
#endif


int main(void)
{
	int rc;
	snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir;
	snd_pcm_uframes_t frames;
	unsigned int size;
    char *buffer;
	long loops;

	
	rc = snd_pcm_open(&handle, device, STREAM_DIEC, 0);
	if (rc < 0) {
        fprintf(stderr, "unable to open PCM device: %s\n",
                snd_strerror(rc));
        exit(1);

    }

	/* alloc hardware params object */
    snd_pcm_hw_params_alloca(&params);

    /* fill it with default values */
    snd_pcm_hw_params_any(handle, params);

	/*set access type: interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	/*sample format: signed 16 bit little ending format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	
	/* two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, 2);
	
	/* 44100 bits/second sampling rate (CD quality) */
	val = 44100;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

	/* set period size t 32 frames */
    frames = 32;    
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

	 /* write params to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw params: %s\n",
                snd_strerror(rc));
        exit(1);
    }

	
	/* use buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	printf("period size: %lu frames \n",frames);
	size = frames * 4; //2 //bytes/sample, 2 channels
	buffer = (char *)malloc(size);

	/* we want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    /* 5 second in micro seconds divided by period time */
    loops = 5000000 / val;

	#if IS_PLAY
	while (loops > 0) {
        loops--;
        rc = read(0, buffer, size);
        if (rc == 0) {
            fprintf(stderr, "end of file on input\n");
            break;
        } else if (rc != size) {
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }

        rc = snd_pcm_writei(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* -EPIPE means underrun */
            fprintf(stderr, "underrun occured\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short write, write %d frames\n", rc);
        }
    }
	#else
	while (loops > 0) {
		loops--;
		rc = snd_pcm_readi(handle, buffer, frames);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			fprintf(stderr, "overrun occured\n");
			snd_pcm_prepare(handle);
		} else if (rc < 0) {
			fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
		} else if (rc != (int)frames) {
			fprintf(stderr, "short read, read %d frames\n", rc);
		}
	
		rc = write(1, buffer, size);
		if (rc != size) {
			fprintf(stderr, "short write: wrote %d bytes\n", rc);
		}
	}
	#endif
	/* allow any pending sound samples to be transferred */
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
	free(buffer);
	
	return 0;
}

