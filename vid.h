#ifndef VIDEO_H
#define VIDEO_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct Video {
   AVFormatContext *pFormatCtx;
   AVCodecContext *pCodecCtxOrig;
   AVCodecContext *pCodecCtx;
   AVCodec *pCodec;
   AVFrame *pFrame;
   AVFrame *pFrameRGB;
   AVPacket packet;
   struct SwsContext *sws_ctx;
   uint8_t *buffer;
   int videoStream;
} Video;

Video *newVideo();
int initVideo(Video *vid, char *fileName);
int getXtermEquivalent(uint8_t r, uint8_t g, uint8_t b);
void printFrame(AVFrame *pFrame, int width, int height);
void play(Video *vid);

#endif
