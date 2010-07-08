/*  
 * Copyright (C) 2010 Riccardo Cecolin 
 * rkj@playkanji.com - 8 july 2010
 */

#include "rmotion.h"

VALUE rb_cMotion;
VALUE rb_cEntity;
VALUE motion_data;

void rmotion_free_data (struct motion_data_t * mdp) 
{
	free(mdp);
}
VALUE rmotion_allocate (VALUE klass)
{
	struct motion_data_t * mdp;
	motion_data = Data_Make_Struct(klass, struct motion_data_t, 0, rmotion_free_data, mdp);

	mdp->storage = cvCreateMemStorage(0);
	mdp->point_seq = cvCreateSeq(CV_32SC2,sizeof(CvSeq),sizeof(CvPoint),mdp->storage);
		
	return motion_data;
}
VALUE rb_motion_initialize (VALUE self)
{
	int i=0;
	VALUE entities = rb_ary_new();
	VALUE e;

	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);

	for (i=0;i<ENTITIES;i++) {
		e = rb_class_new_instance (0, 0, rb_cEntity);
		rb_ary_push(entities, e);
	}
	rb_iv_set(self,"entities",entities);
	
	/* DEFAULTS */
	mdp->mode = MODECAM;
	mdp->fft = 1;
	mdp->write = NULL;
	
	mdp->show = 1;
	mdp->fill = 1; 
	mdp->rect = 0;
	mdp->point = 0;

	mdp->threshold_distance = 9.0;
	mdp->threshold_group = 20;
	mdp->threshold_fft = 1;
	mdp->threshold_direct = 8;
	
	return self;
}

int rmotion_isequal( const void* _a, const void* _b, void* userdata ) {
	CvPoint a = *(const CvPoint*)_a;
	CvPoint b = *(const CvPoint*)_b;
	double threshold = *(double*)userdata;
	return (double)((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)) <= threshold;
}

void rmotion_cellfill(IplImage *img, int x, int y, int sdv) {
	int z, w;
	CvScalar pixel;

	for(w=x;w<x+CELL;w++)
		for(z=y;z<y+CELL;z++) {
			pixel=cvGet2D(img,w,z);
			if (z==y || z==(y+CELL-1)||w==x||w==(x+CELL-1)) {
				pixel.val[0] += 100;
				pixel.val[1] += 100;
				pixel.val[2] += 100;
			} else {
				pixel.val[0] -= (int)(sdv * 10);
				pixel.val[1] += (int)(sdv * 10);
				pixel.val[2] -= (int)(sdv * 10);
			}
			if (pixel.val[0] < 0) pixel.val[0] = 0.0;
			if (pixel.val[1] > 255) pixel.val[1] = 255.0;
			if (pixel.val[1] > 255) pixel.val[1] = 255.0;
			if (pixel.val[2] < 0) pixel.val[2] = 0.0;
			if (pixel.val[2] > 255) pixel.val[2] = 255.0;
			cvSet2D(img,w,z,pixel);
		}	
}

void rmotion_spectrumdiff( struct motion_data_t * mdp, IplImage *ref, IplImage *tpl, IplImage *poc, int x, int y ) {
	int i, j, k;
	double tmp;
	int step = ref->widthStep;
	int fft_size = CELL * CELL;
	uchar *ref_data = ( uchar* ) ref->imageData;
	uchar *tpl_data = ( uchar* ) tpl->imageData;
	double *poc_data = ( double* )poc->imageData;
 
	for( i = 0, k = 0 ; i < CELL ; i++ ) {
		for( j = 0 ; j < CELL ; j++, k++ ) {
			mdp->fft_img1[k][0] = ( double )ref_data[(i+y) * step + j+x];
			mdp->fft_img1[k][1] = 0.0; 
			mdp->fft_img2[k][0] = ( double )tpl_data[(i+y) * step + j+x];
			mdp->fft_img2[k][1] = 0.0;
		}
	}

	fftw_execute_dft( mdp->fft_cell_plan, mdp->fft_img1, mdp->fft_img1);
	fftw_execute_dft( mdp->fft_cell_plan, mdp->fft_img2, mdp->fft_img2);
 
	for( i = 0; i < fft_size ; i++ ) {
		mdp->fft_res[i][0] = mdp->fft_img1[i][0] - mdp->fft_img2[i][0];
		mdp->fft_res[i][1] = mdp->fft_img1[i][1] - mdp->fft_img2[i][1];
	}

 	for( i = 0 ; i < fft_size ; i++ ) {
		poc_data[i] = mdp->fft_res[i][0] / ( double )fft_size;
	}	
}

VALUE rmotion_process (VALUE self)
{ 

	int i, j, k, w, z, key = 0;
	CvScalar avg, sdv;
	IplImage * swap;

	int count;
	CvPoint coord;
	CvSeq* labels = 0;

	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);

	/* SETUP */
	if (mdp->mode == MODEVID  && (strlen(mdp->vid) > 0)) {
		mdp->capture = cvCaptureFromAVI(mdp->vid);
	} else {
		mdp->capture = cvCaptureFromCAM(mdp->camid);
	}

	if(!mdp->capture) {
		printf("error mdp capture\n");
	}

	mdp->frame = cvQueryFrame(mdp->capture);

	/* ALLOCATE WORKING BUFFERS */
	mdp->last =cvCreateImage(cvSize(mdp->frame->width,mdp->frame->height), mdp->frame->depth, 1);
	mdp->cur =cvCreateImage(cvSize(mdp->frame->width,mdp->frame->height), mdp->frame->depth, 1);

	mdp->cell1 = cvCreateImage(cvSize(CELL, CELL), mdp->frame->depth, 1);
	mdp->cell2 = cvCreateImage(cvSize(CELL, CELL), mdp->frame->depth, 1);
	mdp->tmp = cvCreateImage(cvSize(CELL, CELL), IPL_DEPTH_64F, 1);

    mdp->fft_img1 = ( fftw_complex* )fftw_malloc( sizeof( fftw_complex ) * CELL * CELL );
	mdp->fft_img2 = ( fftw_complex* )fftw_malloc( sizeof( fftw_complex ) * CELL * CELL );
	mdp->fft_res  = ( fftw_complex* )fftw_malloc( sizeof( fftw_complex ) * CELL * CELL );
	mdp->fft_cell_plan = fftw_plan_dft_1d( CELL * CELL, mdp->fft_img1, mdp->fft_img1, FFTW_FORWARD,  FFTW_ESTIMATE );

	if(mdp->write != NULL) {
		mdp->writer=cvCreateVideoWriter(mdp->write,CV_FOURCC('P','I','M','1'),25,cvSize(mdp->frame->width,mdp->frame->height),1);
	}

	if (mdp->show) {
		cvNamedWindow(LABEL,CV_WINDOW_AUTOSIZE );
		cvShowImage(LABEL,mdp->frame);
	}

	cvCvtColor(mdp->frame,mdp->last,CV_RGB2GRAY);

	/* MAIN LOOP */
	mdp->quit = 0;

	int n = 0;
	while (!mdp->quit && key != 'b') {

		mdp->frame= cvQueryFrame(mdp->capture);
		if(!mdp->frame) break;
		
		cvCvtColor(mdp->frame,mdp->cur,CV_RGB2GRAY);

		/* FORCELLS*/
		for (i= 0; i < mdp->frame->height; i+= CELL) {
			for (k= 0; k < mdp->frame->width; k+= CELL) {

				if(mdp->fft) {

					rmotion_spectrumdiff( mdp, mdp->last, mdp->cur, mdp->tmp, k, i);
					cvAvgSdv(mdp->tmp, &avg, &sdv, 0);

					if(sdv.val[0] > mdp->threshold_fft) {

						if(mdp->show && mdp->fill) 
							rmotion_cellfill(mdp->frame, i, k, sdv.val[0]);
						coord.x= k/CELL;
						coord.y= i/CELL;
						cvSeqPush( mdp->point_seq, &coord );

					}
				} else {
					cvSetImageROI(mdp->last,cvRect(k, i, CELL, CELL));
					cvSetImageROI(mdp->cur,cvRect(k, i, CELL, CELL));
					cvCopy(mdp->last,mdp->cell1, 0);
					cvCopy(mdp->cur,mdp->cell2, 0);
					cvResetImageROI(mdp->cur);
					cvResetImageROI(mdp->last);

					cvAbsDiff(mdp->cell1,mdp->cell2,mdp->cell1);
					cvAvgSdv(mdp->cell1, &avg, &sdv, 0);
					if(sdv.val[0] > mdp->threshold_direct) {
						if(mdp->show && mdp->fill) 
							rmotion_cellfill(mdp->frame, i, k, sdv.val[0]);
						coord.x= k/CELL;
						coord.y= i/CELL;
						cvSeqPush( mdp->point_seq, &coord );
					}
				}
			}
		}
		/* ENTITIES */
		count = cvSeqPartition( mdp->point_seq,0,&labels,rmotion_isequal, &mdp->threshold_distance );
		// DBG(count);
		struct entity groups [ENTITIES] = { 0 };
		memset(groups, 0, sizeof(struct entity) * ENTITIES);
		for( i = 0; i < labels->total; i++ ){
			CvPoint pt = *(CvPoint*)cvGetSeqElem( mdp->point_seq, i );
			int seq = *(int*)cvGetSeqElem( labels, i );
			if (seq < ENTITIES) {
				if (pt.x <= groups[seq].tl.x || groups[seq].init == 0)
					groups[seq].tl.x = pt.x;
				if (pt.y <= groups[seq].tl.y || groups[seq].init == 0)
					groups[seq].tl.y = pt.y;
				if (pt.x >= groups[seq].br.x || groups[seq].init == 0)
					groups[seq].br.x = pt.x; 
				if (pt.y >= groups[seq].br.y || groups[seq].init == 0)
					groups[seq].br.y = pt.y;
				groups[seq].tot +=1;
				groups[seq].init = 1;
			}
		}

		for (i=0;i<ENTITIES;i++) {
			VALUE ary = rb_iv_get(self,"entities");
			VALUE e = rb_ary_entry(ary,i);
			if (groups[i].tot > mdp->threshold_group) {
				groups[i].tl.x *= CELL;
				groups[i].tl.y *= CELL;
				groups[i].br.x *= CELL;
				groups[i].br.y *= CELL;
				
				rb_iv_set(e,"@tlx", INT2FIX((int)groups[i].tl.x));
				rb_iv_set(e,"@tly", INT2FIX((int)groups[i].tl.y));
				rb_iv_set(e,"@brx", INT2FIX((int)groups[i].br.x));
				rb_iv_set(e,"@bry", INT2FIX((int)groups[i].br.y));

				if (mdp->rect) cvRectangle(mdp->frame,groups[i].tl,groups[i].br,cvScalar(255,112,61,0),2,0,0);
				if (mdp->point) {
					cvCircle(mdp->frame, cvPoint((groups[i].tl.x + groups[i].br.x)/2,(groups[i].tl.y + groups[i].br.y)/2), 10, cvScalar(255,112,61,0), 2, 0,0);					
				}
			} else {
				rb_iv_set(e,"@tlx", Qnil);
				rb_iv_set(e,"@tly", Qnil);
				rb_iv_set(e,"@brx", Qnil);
				rb_iv_set(e,"@bry", Qnil);
			}
		}
		cvClearSeq(mdp->point_seq);
		cvClearSeq(labels);
	
		if (mdp->show) {
			cvShowImage(LABEL,mdp->frame);
			key = cvWaitKey( 10 );
		}
		
		if(mdp->write != NULL) {
			cvWriteFrame(mdp->writer,mdp->frame);
		}

		if(rb_block_given_p()) 
			rb_yield(rb_iv_get(self,"entities"));

		swap = mdp->last;
		mdp->last = mdp->cur;
		mdp->cur=swap;
			
	}

	/* FREE */
	if(mdp->write != NULL) cvReleaseVideoWriter(&mdp->writer);

	fftw_free ( mdp->fft_img1 );
	fftw_free ( mdp->fft_img2 );
	fftw_free ( mdp->fft_res );
	fftw_destroy_plan( mdp->fft_cell_plan );

	cvReleaseCapture( &mdp->capture);
	cvReleaseImage( &mdp->cur );
	cvReleaseImage( &mdp->last );
	cvReleaseImage( &mdp->frame );
	cvReleaseImage( &mdp->cell1 );
	cvReleaseImage( &mdp->cell2 );
	cvReleaseImage( &mdp->tmp );

}
VALUE rb_motion_cam (int argc, VALUE *argv, VALUE self)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	VALUE camid;

	rb_scan_args(argc, argv, "01", &camid);
	if(NIL_P(camid)) {
		camid = INT2FIX(-1);
	}
	mdp->mode = MODECAM;
	mdp->camid = NUM2INT(camid);
	rmotion_process(self);
	return Qtrue;
}

VALUE rb_motion_vid (int argc, VALUE *argv, VALUE self)
{
	VALUE vid;
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);

	rb_scan_args(argc, argv, "01", &vid);
	if(TYPE(vid) == T_STRING) {
		mdp->mode = MODEVID;
		mdp->vid = StringValuePtr(vid);
	} else {
		return Qfalse;
	}
	rmotion_process(self);
	return Qtrue;
}

VALUE rb_motion_quit (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	mdp->quit = 1;
	return Qtrue;
}

void
Init_rmotion ()
{
	rb_cMotion = rb_define_class ("Motion", rb_cObject);
	rb_cEntity = rb_define_class ("Entity", rb_cObject);

	rb_define_alloc_func(rb_cMotion, rmotion_allocate);
	rb_define_method (rb_cMotion, "initialize", rb_motion_initialize, 0);
	rb_define_method (rb_cMotion, "cam", rb_motion_cam, -1);
	rb_define_method (rb_cMotion, "vid", rb_motion_vid, -1);

	rb_define_method (rb_cMotion, "show=", rb_motion_show_set, 1);
	rb_define_method (rb_cMotion, "show?", rb_motion_show_p, 0);

	rb_define_method (rb_cMotion, "fft=", rb_motion_fft_set, 1);
	rb_define_method (rb_cMotion, "fft?", rb_motion_fft_p, 0);

	rb_define_method (rb_cMotion, "fill=", rb_motion_fill_set, 1);
	rb_define_method (rb_cMotion, "fill?", rb_motion_fill_p, 0);

	rb_define_method (rb_cMotion, "rect=", rb_motion_rect_set, 1);
	rb_define_method (rb_cMotion, "rect?", rb_motion_rect_p, 0);
	
	rb_define_method (rb_cMotion, "point=", rb_motion_point_set, 1);
	rb_define_method (rb_cMotion, "point?", rb_motion_point_p, 0);
	
	rb_define_method (rb_cMotion, "write=", rb_motion_write_set, 1);
	rb_define_method (rb_cMotion, "write?", rb_motion_write_p, 0);

	rb_define_method (rb_cMotion, "threshold_distance=", rb_motion_th_distance_set, 1);
	rb_define_method (rb_cMotion, "threshold_distance?", rb_motion_th_distance_p, 0);

	rb_define_method (rb_cMotion, "threshold_group=", rb_motion_th_group_set, 1);
	rb_define_method (rb_cMotion, "threshold_group?", rb_motion_th_group_p, 0);

	rb_define_method (rb_cMotion, "threshold_fft=", rb_motion_th_fft_set, 1);
	rb_define_method (rb_cMotion, "threshold_fft?", rb_motion_th_fft_p, 0);

	rb_define_method (rb_cMotion, "threshold_direct=", rb_motion_th_direct_set, 1);
	rb_define_method (rb_cMotion, "threshold_direct?", rb_motion_th_direct_p, 0);

	rb_define_method (rb_cMotion, "quit", rb_motion_quit, 0);
	

}
