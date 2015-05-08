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
void *connection_handler(void *);
void *data_streaming(void *);
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char *buffer;
int flag = 0;
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    pthread_t sniffer_thread1;
    buffer = (unsigned char *) malloc(SIZE);

	
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

    if(pthread_create(&sniffer_thread1 , NULL , data_streaming , (void*) new_sock) < 0)
       {
            perror("could not create thread");
            return 1;
        }

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
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;

        while(1)
	{
		if (flag) {
                 write(sock , buffer , SIZE);
		flag = 0;}
	}
    //Free the socket pointer
    free(socket_desc);

    return 0;
}

void *data_streaming(void *socket_desc)
{
  int rc;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;


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
 /* 2 bytes/sample, 2 channels */

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);

printf("frmes :%d ", frames);
  while (1) {
        pthread_mutex_lock(&mutex);
   	 rc = snd_pcm_readi(handle, buffer, frames);
	flag = 1;
        pthread_mutex_unlock(&mutex);
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

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

    return 0;
}



