#define ALSA_PCM_NEW_HW_PARAMS_API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <alsa/asoundlib.h>

 main() {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
 unsigned char *buffer;
  int i=0;

/* socket setting */
   int sd;
   struct sockaddr_in s_addr;
   int n, n_send, status;

   int on = 1;


   sd = socket (AF_INET, SOCK_DGRAM, 0);

   bzero(&s_addr, sizeof(s_addr));
   s_addr.sin_family = AF_INET;
   s_addr.sin_addr.s_addr = inet_addr("192.168.42.255");
   s_addr.sin_port = htons(2007);

   if((status = setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))) != 0 )
   {
      printf("setsockopt error\n");
      exit(-1);
   }


  /* Open PCM device for recording (capture). */
  rc = snd_pcm_open(&handle, "plughw:1,0",
                    SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,SND_PCM_FORMAT_U8);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 1);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 32 frames. */
  frames =4800;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params,
                                      &frames, &dir);
  size = frames *1; /* 2 bytes/sample, 2 channels */

  buffer = (unsigned char*) malloc(size);
	
  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);
  while (1) {
    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(rc));
    } else if (rc != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }

if ((n_send = sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *)&s_addr, sizeof(s_addr))) < 0 ) {
	fprintf(stderr, "sendto() error");
         exit(-3);
      }
   }
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);
   close(sd);
}