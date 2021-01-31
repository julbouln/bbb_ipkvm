#include "ipkvm.h"

void audio_open(ipkvm_t *ipkvm, char *device_name)
{
  int err;
  unsigned int rate = AUDIO_RATE;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

  if ((err = snd_pcm_open (&ipkvm->audio_handle, device_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             device_name,
             snd_strerror (err));
    return;
  }
	   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    return;
  }

  if ((err = snd_pcm_hw_params_any (ipkvm->audio_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    return;
  }

  if ((err = snd_pcm_hw_params_set_access (ipkvm->audio_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    return;
  }

  if ((err = snd_pcm_hw_params_set_format (ipkvm->audio_handle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    return;
  }
	
  if ((err = snd_pcm_hw_params_set_rate_near (ipkvm->audio_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    return;
  }
	
  if ((err = snd_pcm_hw_params_set_channels (ipkvm->audio_handle, hw_params, AUDIO_CHANNELS)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    return;
  }

  if ((err = snd_pcm_hw_params (ipkvm->audio_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    return;
  }

  snd_pcm_hw_params_free (hw_params);


  if ((err = snd_pcm_prepare (ipkvm->audio_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    return;
  }

  ipkvm->audio_frames = AUDIO_BUFFER_FRAMES;
  ipkvm->audio_size = AUDIO_BUFFER_FRAMES * snd_pcm_format_width(format) / 8 * AUDIO_CHANNELS;
  ipkvm->audio_buffer = malloc(ipkvm->audio_size);
  printf("audio size %d\n", ipkvm->audio_size);
}

static int xrun_recovery(snd_pcm_t *handle, int err)
{
        if (err == -EPIPE) {    /* under-run */
                err = snd_pcm_prepare(handle);
                if (err < 0)
                        fprintf(stderr, "Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
                return 0;
        } else if (err == -ESTRPIPE) {
                while ((err = snd_pcm_resume(handle)) == -EAGAIN)
                        sleep(1);       /* wait until the suspend flag is released */
                if (err < 0) {
                        err = snd_pcm_prepare(handle);
                        if (err < 0)
                                fprintf(stderr, "Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
                }
                return 0;
        }
        return err;
}

void audio_capture(ipkvm_t *ipkvm) {
	if(ipkvm->audio_enabled) {
    int err;

	if ((err = snd_pcm_readi (ipkvm->audio_handle, ipkvm->audio_buffer, ipkvm->audio_frames)) != ipkvm->audio_frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
               snd_strerror (err));
		  xrun_recovery(ipkvm->audio_handle, err);

	      return;
    	}
	}
}

void audio_close(ipkvm_t *ipkvm) {
	snd_pcm_close (ipkvm->audio_handle);
}
