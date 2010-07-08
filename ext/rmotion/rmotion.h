/*  
 * Copyright (C) 2010 Riccardo Cecolin 
 * rkj@playkanji.com - 8 july 2010
 */

#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fftw3.h>

#define CELL 8 /* 2FIX */
#define ENTITIES 8
#define LABEL "RMotion"

#define MODECAM 0
#define MODEVID 1

#define DBG(x) printf("%s: %d\n",#x,x)

struct motion_data_t {
	/* FFTW */
	fftw_complex *fft_img1;
	fftw_complex *fft_img2;
	fftw_complex *fft_res;
	fftw_plan fft_cell_plan;

	/* OPENCV */
	CvCapture *capture;
	CvVideoWriter *writer;

    IplImage *last;
    IplImage *cur;
	IplImage *frame;

	IplImage *cell1;
	IplImage *cell2;
	IplImage *tmp;

	CvMemStorage* storage;
	CvSeq* point_seq;

	int quit;

	/* GENERAL CONFIG */
	int mode;
	int camid;
	char * vid;
	int fft;
	char * write;

	/* DISPLAY */
	int show;
	int fill;
	int rect;
	int point;
	
	/* ALGORITHM */
	double threshold_distance;
	int threshold_group;
	double threshold_fft;
	double threshold_direct;

};

struct entity {
	int init;
	int tot;
	CvPoint tl;
	CvPoint br;
};

#define GetMD(obj, p) Data_Get_Struct(obj, struct motion_data_t, p);


VALUE rb_motion_show_set (VALUE self, VALUE val);
VALUE rb_motion_show_p (VALUE self);
VALUE rb_motion_fill_set (VALUE self, VALUE val);
VALUE rb_motion_fill_p (VALUE self);
VALUE rb_motion_rect_set (VALUE self, VALUE val);
VALUE rb_motion_rect_p (VALUE self);
VALUE rb_motion_fft_set (VALUE self, VALUE val);
VALUE rb_motion_fft_p (VALUE self);
VALUE rb_motion_point_set (VALUE self, VALUE val);
VALUE rb_motion_point_p (VALUE self);
VALUE rb_motion_write_set (VALUE self, VALUE val);
VALUE rb_motion_write_p (VALUE self) ;
VALUE rb_motion_th_distance_set (VALUE self, VALUE val);
VALUE rb_motion_th_distance_p (VALUE self) ;
VALUE rb_motion_th_group_set (VALUE self, VALUE val);
VALUE rb_motion_th_group_p (VALUE self);
VALUE rb_motion_th_fft_set (VALUE self, VALUE val);
VALUE rb_motion_th_fft_p (VALUE self);
VALUE rb_motion_th_direct_set (VALUE self, VALUE val);
VALUE rb_motion_th_direct_p (VALUE self);
