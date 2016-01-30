#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include "vid.h"

int main(int argc, char**argv){
   
   av_register_all();
   Video *vid = newVideo();
   initVideo(vid, argv[1]);
   initscr();
   start_color();
   play(vid);
   endwin();
   

}
