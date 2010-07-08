/*  
 * Copyright (C) 2010 Riccardo Cecolin 
 * rkj@playkanji.com - 8 july 2010
 */

#include "rmotion.h"

extern VALUE motion_data;

VALUE rb_motion_show_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if(RTEST(val)) {
		mdp->show = 1;
		return Qtrue;
	} else {
		mdp->show = 0;
		return Qfalse;
	}
}
VALUE rb_motion_show_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if (mdp->show)
		return Qtrue;
	else
		return Qfalse;
}

VALUE rb_motion_fill_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if(RTEST(val)) {
		mdp->fill = 1;
		return Qtrue;
	} else {
		mdp->fill = 0;
		return Qfalse;
	}
}
VALUE rb_motion_fill_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if (mdp->fill)
		return Qtrue;
	else
		return Qfalse;
}

VALUE rb_motion_rect_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if(RTEST(val)) {
		mdp->rect = 1;
		return Qtrue;
	} else {
		mdp->rect = 0;
		return Qfalse;
	}
}
VALUE rb_motion_rect_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if (mdp->rect)
		return Qtrue;
	else
		return Qfalse;
}

VALUE rb_motion_fft_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if(RTEST(val)) {
		mdp->fft = 1;
		return Qtrue;
	} else {
		mdp->fft = 0;
		return Qfalse;
	}
}
VALUE rb_motion_fft_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if (mdp->fft)
		return Qtrue;
	else
		return Qfalse;
}

VALUE rb_motion_point_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if(RTEST(val)) {
		mdp->point = 1;
		return Qtrue;
	} else {
		mdp->point = 0;
		return Qfalse;
	}
}
VALUE rb_motion_point_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if (mdp->point)
		return Qtrue;
	else
		return Qfalse;
}

VALUE rb_motion_write_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	Check_Type(val, T_STRING);
	mdp->write = StringValuePtr(val);
}

VALUE rb_motion_write_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	if (mdp->write != NULL)
		return Qtrue;
	else
		return Qfalse;
}

VALUE rb_motion_th_distance_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	mdp->threshold_distance = NUM2DBL(val);
	return val;
}

VALUE rb_motion_th_distance_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	return rb_float_new(mdp->threshold_distance);
}

VALUE rb_motion_th_group_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	mdp->threshold_group = NUM2INT(val);
	return val;
}

VALUE rb_motion_th_group_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	return INT2FIX(mdp->threshold_group);
}

VALUE rb_motion_th_fft_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	mdp->threshold_fft = NUM2DBL(val);
	return val;
}

VALUE rb_motion_th_fft_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	return rb_float_new(mdp->threshold_fft);
}

VALUE rb_motion_th_direct_set (VALUE self, VALUE val)
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	mdp->threshold_direct = NUM2DBL(val);
	return val;
}

VALUE rb_motion_th_direct_p (VALUE self) 
{
	struct motion_data_t * mdp; 
	GetMD(motion_data,mdp);
	return rb_float_new(mdp->threshold_direct);
}
