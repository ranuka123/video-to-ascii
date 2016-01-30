/**
 *This program uses ffmpeg and ncurses to take any video supported
 *by the ffmpeg api and converts each pixel to ascii. The ascii value is printed
 *to commandline.
 *Dependencies: ffmpeg, ncurses, c
 *Tested on Mac OSX 10.8 with gcc 4.9.2
 *author: snjt@github.com
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include "vid.h"
char *str = " .,/c(@#8";
uint8_t points[] = {47, 115, 155, 195, 235};

/**
 *Mallocs a new video struct that will
 *be used to hold all the metadata for the video
 *@return A malloced video struct
 */
Video *newVideo(){
   Video *vid = malloc(sizeof(vid));
   vid->pFormatCtx = NULL;
   vid->pCodecCtxOrig = NULL;
   vid->pCodecCtx = NULL;
   vid->pCodec = NULL;
   vid->pFrame = NULL;
   vid->pFrameRGB = NULL;
   vid->sws_ctx = NULL;
   vid->buffer = NULL;
   return vid;
}

/**
 *Boilerplate code that takes the name of the video and
 *an video and populates the the neccessary metadata that ffmpeg
 *requires. Refer to http://dranger.com/ffmpeg/tutorial01.html
 *@param vid Malloced struct used for metadata population
 *@param videoName Video to open
 *@return Success or faliure code of 1 or -1 respectively
 */
int initVideo(Video *vid, char *videoName){

   int i, t;
   if(avformat_open_input(&(vid->pFormatCtx), videoName, NULL,  NULL) != 0){
      fprintf(stderr, "Couldn't open file\n");
      return -1;
   }

   if(avformat_find_stream_info(vid->pFormatCtx, NULL)<0){
      fprintf(stderr, "Couldn't find stream info\n");
      return -1;
   }


   //av_dump_format(pFormatCtx, 0, argv[1], 0);
   for (i=0; i<vid->pFormatCtx->nb_streams; i++){
      t = vid->pFormatCtx->streams[i]->codec->codec_type;
      if(t==AVMEDIA_TYPE_VIDEO){
         vid->videoStream = i;
         break;
      }
   }

   if(vid->videoStream == -1)
      return -1;

   vid->pCodecCtxOrig = vid->pFormatCtx->streams[vid->videoStream]->codec;
   vid->pCodec = avcodec_find_decoder(vid->pCodecCtxOrig->codec_id);
   if(vid->pCodec==NULL){
      fprintf(stderr, "Unsupported codec!\n");
      return -1;
   }

   vid->pCodecCtx = avcodec_alloc_context3(vid->pCodec);
   avcodec_copy_context(vid->pCodecCtx, vid->pCodecCtxOrig);
   if(avcodec_copy_context(vid->pCodecCtx, vid->pCodecCtxOrig) != 0) {
      fprintf(stderr, "Couldn't copy codec context");
      return -1; // Error copying codec context
   }

   if(avcodec_open2(vid->pCodecCtx, vid->pCodec, NULL)<0)
      return -1;

   vid->pFrame = av_frame_alloc();
   vid->pFrameRGB = av_frame_alloc();
   if(vid->pFrameRGB==NULL)
      return -1;

   int numBytes = avpicture_get_size(PIX_FMT_RGB24, vid->pCodecCtx->width, vid->pCodecCtx->height);
   vid->buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
   avpicture_fill((AVPicture *)vid->pFrameRGB, vid->buffer, PIX_FMT_RGB24, vid->pCodecCtx->width, vid->pCodecCtx->height);

   vid->sws_ctx = sws_getContext( vid->pCodecCtx->width, vid->pCodecCtx->height, vid->pCodecCtx->pix_fmt, vid->pCodecCtx->width,
                            vid->pCodecCtx->height, PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

   return 1;
}

/**
 *Used to play the video. Takes a frame from av_read_frame
 *and checks if it's a video frame. If it is it is decoded and then scaled
 *using sws_scale. printFrame takes the RGB frame and prints it to terminal.
 *Currently just using video parameters to scale but
 *might allow an option to let the user scale using their own parameters.
 *ncurse refresh is called to output buffer to screen.
 *@param vid 
 */
void play(Video *vid){
   int frameFinished;

   while(av_read_frame(vid->pFormatCtx, &(vid->packet))>=0){

      if(vid->packet.stream_index==vid->videoStream){
         avcodec_decode_video2(vid->pCodecCtx, vid->pFrame, &frameFinished, &(vid->packet));

         if(frameFinished) {
            sws_scale(vid->sws_ctx, (uint8_t const * const *)vid->pFrame->data,
                     vid->pFrame->linesize, 0, vid->pCodecCtx->height,
                     vid->pFrameRGB->data, vid->pFrameRGB->linesize);

            printFrame(vid->pFrameRGB, vid->pCodecCtx->width,
                         vid->pCodecCtx->height);
            refresh();
         }
      }

      av_free_packet(&(vid->packet));

   }
}
  
/**
 *Takes the AVFrame and prints it to screen.
 *The width and height are required as this is used to calculate
 *the x,y of the ncurse pointer to place the ascii value.
 *Each pixel is 3 bytes and consists of r,g,b. I'm not exactly sure
 *of the order in memory. The pointer is incremented by 3 bytes after every
 *iteration to get the next pixel. p is reseted to the beginning
 *after a full traversal.
 *@param pFrame
 *@param width
 *@param height
 */
void printFrame(AVFrame *pFrame, int width, int height){

   uint8_t r = 0, g = 0, b = 0;
   int y = 0;
   int x = 0;
   int val;

   //pixel data
   int heightOffset = 0;
   uint8_t *p = pFrame->data[0];
   int index = 0;
   
   for(y=0; y<height; y++){
      heightOffset = y*(pFrame->linesize[0]);
      for(x=0; x< width; x++){
         r = (p+heightOffset)[0];
         g = (p+heightOffset)[1];
         b = (p+heightOffset)[2];
         val = b * 0.333 + g * 0.333 + r * 0.333;
         index = (val/(256.0/9));
         //xtermColor = getXtermEquivalent(r,g,b);
         //attron(COLOR_PAIR(color));
         //printw(" ");
         
         addch(str[index]);
         move(y, x);
         p += 3;
      }
      p = pFrame->data[0];
   }

}

/**
 *Something I tried to get working. Used for colored output instead of
 *ascii but too slow and too ugly.
 *Used to convert an rgb range of 16million down to just 256 colors
 *supported by xterm.
 *Refer to https://gist.github.com/MicahElliott/719710
 */
int getXtermEquivalent(uint8_t r, uint8_t g, uint8_t b){
   uint8_t rgbHolder[] = {r,g,b};
   uint8_t index[] = {0,0,0};
   int x, i;
   for (x = 0; x < 3; x++)
      for (i = 0; i < 5; i++)
         if (points[i] < rgbHolder[x])
            index[x] ++;

   return index[0]*36 + index[1]*6 + index[2] + 16;
}


