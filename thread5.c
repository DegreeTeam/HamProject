#include <sys/types.h>
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <alsa/asoundlib.h>

#define SIZE 128 
#define ALSA_PCM_NEW_HW_PARAMS_API
snd_pcm_t *handle;
snd_pcm_uframes_t frames;
void *connection_handler(void *);
void *data_streaming(void *);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    pthread_t sniffer_thread1;
snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir;
	int rcm;
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons( 2008 );


    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

      snd_pcm_open(&handle, "plughw:1,0",
                    SND_PCM_STREAM_CAPTURE, 0);

  /* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
	 snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
	 snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */

  /* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_U8);

  /* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, 1);

  /* 44100 bits/second sampling rate (CD quality) */
	 val = 32768;
	 snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 32 frames. */
	 frames = SIZE;
	 snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rcm = snd_pcm_hw_params(handle, params);
  if (rcm < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rcm));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params,
                                      &frames, &dir);
 /* 2 bytes/sample, 2 channels */

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);

    while(1){

	   pthread_t sniffer_thread;
	  if(client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))
        {
                puts("Connection accepted");
                new_sock = malloc(4);
                *new_sock = client_sock;
        }
        if( pthread_create(&sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");


    if (client_sock < 0)
    {
        perror("accept failed");
    {
        perror("accept failed");
        return 1;
    }
}
}
   snd_pcm_drain(handle);
  snd_pcm_close(handle);
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
	unsigned char *buffer;
    buffer = (unsigned char *) malloc(SIZE);
	int rc;
	 while (1) {
   		rc = snd_pcm_readi(handle, buffer, frames);
		write(sock , buffer , SIZE);	
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
	 }
     
    free(socket_desc);
	free(buffer);
    return 0;
}





