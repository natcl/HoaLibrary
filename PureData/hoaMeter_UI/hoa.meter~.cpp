/**
 * HoaLibrary : A High Order Ambisonics Library
 * Copyright (c) 2012-2013 Julien Colafrancesco, Pierre Guillot, Eliott Paris, CICM, Universite Paris-8.
 * All rights reserved.re Guillot, CICM - Université Paris 8
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/HoaLibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * This file is part of HOA LIBRARY.
 *
 * HOA LIBRARY is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "../hoaLibrary/hoa.library_pd.h"

#define MAX_SPEAKER 64
#define DEF_SPEAKER 8
#define OVERLED_DRAWTIME 1000

typedef struct  _meter
{
	t_jbox      j_box;
	
	t_clock*	f_clock;
	int			f_startclock;
	long        f_interval;
    
    long        f_number_of_loudspeakers;
	double      f_offset_of_loudspeakers;
	long        f_clockwise;
	
	double		f_angles_of_loudspeakers[MAX_SPEAKER];
    long		f_drawOverLedleftTime[MAX_SPEAKER];
	
	t_jrgba		f_colorBackground;
	t_jrgba		f_colorMeterBg;
	t_jrgba		f_colorBorder;
	t_jrgba		f_colorColdSignal;
	t_jrgba		f_colorTepidSignal;
	t_jrgba		f_colorWarmSignal;
	t_jrgba		f_colorHotSignal;
	t_jrgba		f_colorOverSignal;
	
	t_jrgba		f_colorEnergy;
	t_jrgba		f_colorVelocity;
	
	float		f_radius_exterior;
	float		f_radius_interior;
    long        f_drawvector;
    
    
    t_outlet*    f_vector_outlet;
    t_outlet*    f_peaks_outlet;
	
    AmbisonicsMeter*     f_meter;
} t_meter;

t_eclass *meter_class;

void *meter_new(t_symbol *s, int argc, t_atom *argv);
void meter_free(t_meter *x);
void meter_assist(t_meter *x, void *b, long m, long a, char *s);

void meter_dsp(t_meter *x, t_object *dsp, short *count, double samplerate, long maxvectorsize, long flags);
void meter_perform(t_meter *x, t_object *d, float **ins, long ni, float **outs, long no, long sf, long f,void *up);
void meter_tick(t_meter *x);
void meter_output(t_meter *x);

t_max_err meter_notify(t_meter *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
t_max_err number_of_loudspeakers_set(t_meter *x, t_object *attr, long argc, t_atom *argv);
t_max_err angles_of_loudspeakers_set(t_meter *x, void *attr, long ac, t_atom *av);

/* Paint ------------------------------------- */
void meter_getdrawparams(t_meter *x, t_object *patcherview, t_jboxdrawparams *params);
void meter_oksize(t_meter *x, t_rect *newrect);

void meter_paint(t_meter *x, t_object *view);
void draw_background(t_meter *x, t_object *view, t_rect *rect);
void draw_leds(t_meter *x,  t_object *view, t_rect *rect);
void draw_vector_energy(t_meter *x, t_object *view, t_rect *rect);
void draw_vector_velocity(t_meter *x, t_object *view, t_rect *rect);

extern "C" void setup_hoa0x2emeter_tilde(void)
{
	t_eclass *c;
    
	c = class_new("hoa.meter~", (method)meter_new, (method)meter_free, (short)sizeof(t_meter), 0L, A_GIMME, 0);
    
	class_dspinitjbox(c);
	jbox_initclass(c, JBOX_COLOR | JBOX_FIXWIDTH);
	
	class_addmethod(c, (method) meter_dsp,           "dsp",           A_CANT, 0);
	class_addmethod(c, (method) meter_assist,		 "assist",		  A_CANT, 0);
	class_addmethod(c, (method) meter_paint,		 "paint",		  A_CANT, 0);
	class_addmethod(c, (method) meter_notify,        "notify",		  A_CANT, 0);
    class_addmethod(c, (method) meter_getdrawparams, "getdrawparams", A_CANT, 0);
    class_addmethod(c, (method) meter_oksize,        "oksize",        A_CANT, 0);
    
	CLASS_ATTR_DEFAULT			(c, "size", 0, "225 225");
	CLASS_ATTR_INVISIBLE		(c, "color", 0);
	
	CLASS_ATTR_LONG				(c, "vectors", 0, t_meter, f_drawvector);
	CLASS_ATTR_ORDER			(c, "vectors", 0, "2");
    CLASS_ATTR_LABEL            (c, "vectors", 0, "Vectors");
	CLASS_ATTR_DEFAULT			(c, "vectors", 0, "1");
	CLASS_ATTR_SAVE				(c, "vectors", 1);
    
	CLASS_ATTR_LONG				(c, "loudspeakers", 0 , t_meter, f_number_of_loudspeakers);
    CLASS_ATTR_ACCESSORS		(c, "loudspeakers", NULL, number_of_loudspeakers_set);
	CLASS_ATTR_ORDER			(c, "loudspeakers", 0, "1");
	CLASS_ATTR_LABEL			(c, "loudspeakers", 0, "Number of Loudspeakers");
	CLASS_ATTR_SAVE				(c, "loudspeakers", 1);
    CLASS_ATTR_DEFAULT          (c, "loudspeakers", 0, "8");
    CLASS_ATTR_PAINT            (c, "loudspeakers", 0);
    
	CLASS_ATTR_DOUBLE_VARSIZE	(c, "angles", 0, t_meter,f_angles_of_loudspeakers, f_number_of_loudspeakers,MAX_SPEAKER);
	CLASS_ATTR_ACCESSORS		(c, "angles", NULL, angles_of_loudspeakers_set);
	CLASS_ATTR_ORDER			(c, "angles", 0, "2");
	CLASS_ATTR_LABEL			(c, "angles", 0, "Angles of Loudspeakers");
	CLASS_ATTR_SAVE				(c, "angles", 1);
    CLASS_ATTR_PAINT            (c, "angles", 0);
    CLASS_ATTR_DEFAULT          (c, "angles", 0, "0 45 90 135 180 225 270 315");
    
	CLASS_ATTR_DOUBLE			(c, "offset", 0, t_meter, f_offset_of_loudspeakers);
	CLASS_ATTR_ORDER			(c, "offset", 0, "3");
	CLASS_ATTR_LABEL			(c, "offset", 0, "Offset of Loudspeakers");
	CLASS_ATTR_DEFAULT			(c, "offset", 0, "0");
	CLASS_ATTR_SAVE				(c, "offset", 1);
    
    CLASS_ATTR_LONG				(c, "rotation", 0, t_meter, f_clockwise);
    CLASS_ATTR_ORDER			(c, "rotation", 0, "4");
	CLASS_ATTR_LABEL			(c, "rotation", 0, "Rotation of Loudspeakers");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"rotation",0, "1");
    
    CLASS_ATTR_LONG				(c, "interval", 0, t_meter, f_interval);
	CLASS_ATTR_ORDER			(c, "interval", 0, "5");
	CLASS_ATTR_LABEL			(c, "interval", 0, "Refresh Interval in Milliseconds");
	CLASS_ATTR_FILTER_MIN		(c, "interval", 20);
	CLASS_ATTR_DEFAULT			(c, "interval", 0, "50");
	CLASS_ATTR_SAVE				(c, "interval", 1);
	
	CLASS_ATTR_RGBA				(c, "bgcolor", 0, t_meter, f_colorBackground);
	CLASS_ATTR_LABEL			(c, "bgcolor", 0, "Background Color");
	CLASS_ATTR_ORDER			(c, "bgcolor", 0, "1");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bgcolor", 0, "0.55 0.55 0.55 1.");
	
	CLASS_ATTR_RGBA				(c, "mbgcolor", 0, t_meter, f_colorMeterBg);
	CLASS_ATTR_LABEL			(c, "mbgcolor", 0, "Meter Background Color");
	CLASS_ATTR_ORDER			(c, "mbgcolor", 0, "2");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "mbgcolor", 0, "0.4 0.4 0.4 1.");
	
	CLASS_ATTR_RGBA				(c, "bordercolor", 0, t_meter, f_colorBorder);
	CLASS_ATTR_LABEL			(c, "bordercolor", 0, "Box Border Color");
	CLASS_ATTR_ORDER			(c, "bordercolor", 0, "3");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bordercolor", 0, "0.25 0.25 0.25 1");
	
	CLASS_ATTR_RGBA				(c, "coldcolor", 0, t_meter, f_colorColdSignal);
	CLASS_ATTR_LABEL			(c, "coldcolor", 0, "Cold Signal Color");
	CLASS_ATTR_ORDER			(c, "coldcolor", 0, "4");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "coldcolor", 0, "0. 0.6 0. 0.8");
	
	CLASS_ATTR_RGBA				(c, "tepidcolor", 0, t_meter, f_colorTepidSignal);
	CLASS_ATTR_LABEL			(c, "tepidcolor", 0, "Tepid Signal Color");
	CLASS_ATTR_ORDER			(c, "tepidcolor", 0, "5");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "tepidcolor", 0, "0.6 0.73 0. 0.8");
	
	CLASS_ATTR_RGBA				(c, "warmcolor", 0, t_meter, f_colorWarmSignal);
	CLASS_ATTR_LABEL			(c, "warmcolor", 0, "Warm Signal Color");
	CLASS_ATTR_ORDER			(c, "warmcolor", 0, "6");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "warmcolor", 0, ".85 .85 0. 0.8");
	
	CLASS_ATTR_RGBA				(c, "hotcolor", 0, t_meter, f_colorHotSignal);
	CLASS_ATTR_LABEL			(c, "hotcolor", 0, "Hot Signal Color");
	CLASS_ATTR_ORDER			(c, "hotcolor", 0, "7");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "hotcolor", 0, "1. 0.6 0. 0.8");
	
	CLASS_ATTR_RGBA				(c, "overcolor", 0, t_meter, f_colorOverSignal);
	CLASS_ATTR_LABEL			(c, "overcolor", 0, "Overload Signal Color");
	CLASS_ATTR_ORDER			(c, "overcolor", 0, "8");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "overcolor", 0, "1. 0. 0. 0.8");
	
	CLASS_ATTR_RGBA				(c, "energycolor", 0, t_meter, f_colorEnergy);
	CLASS_ATTR_LABEL			(c, "energycolor", 0, "Energy Vector Color");
	CLASS_ATTR_ORDER			(c, "energycolor", 0, "9");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "energycolor", 0, "0. 0. 1. 0.8");
    
    CLASS_ATTR_RGBA				(c, "velocitycolor", 0, t_meter, f_colorVelocity);
	CLASS_ATTR_LABEL			(c, "velocitycolor", 0, "Velocity Vector Color");
	CLASS_ATTR_ORDER			(c, "velocitycolor", 0, "9");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "velocitycolor", 0, "1. 0. 0. 0.8");
	
	meter_class = c;
}

void *meter_new(t_symbol *s, int argc, t_atom *argv)
{
	t_meter *x =  NULL;
	t_dictionary *d;
	long flags;
	if (!(d = object_dictionaryarg(argc,argv)))
		return NULL;
    
	x = (t_meter *)object_alloc(meter_class);
    
	flags = 0
    | JBOX_DRAWFIRSTIN
    | JBOX_DRAWINLAST
    | JBOX_TRANSPARENT
    | JBOX_DRAWBACKGROUND
    | JBOX_GROWY
    ;
    
    x->f_number_of_loudspeakers = 1;
	jbox_new((t_jbox *)x, flags, argc, argv);
	x->j_box.b_firstin = (t_object *)x;
    
	dsp_setupjbox((t_jbox *)x, x->f_number_of_loudspeakers, 0);
    
    x->f_peaks_outlet = listout(x);
    x->f_vector_outlet = listout(x);
    
    x->f_meter = new AmbisonicsMeter(x->f_number_of_loudspeakers, sys_getblksize(), sys_getsr());
    x->f_number_of_loudspeakers = x->f_meter->getNumberOfInputs();
    
    x->f_clock = clock_new(x,(t_method)meter_tick);
	x->f_startclock = 0;
    
	attr_dictionary_process(x, d);	
	jbox_ready((t_jbox *)x);
    
	return (x);
}

void meter_oksize(t_meter *x, t_rect *newrect)
{
    newrect->width = pd_clip_min(newrect->width, 15.);
    newrect->height = pd_clip_min(newrect->height, 15.);
    
    if(newrect->height > newrect->width)
        newrect->width = newrect->height;
    else
        newrect->height = newrect->width;
}

void meter_getdrawparams(t_meter *x, t_object *patcherview, t_jboxdrawparams *params)
{
	params->d_borderthickness = 2;
	params->d_cornersize = 8;
    params->d_bordercolor = x->f_colorBorder;
    params->d_boxfillcolor = x->f_colorBackground;
}

t_max_err number_of_loudspeakers_set(t_meter *x, t_object *attr, long argc, t_atom *argv)
{
    if(argc && argv)
    {
        if(atom_gettype(argv) == A_FLOAT)
        {
            long d = Tools::clip(atom_getfloat(argv), 1, MAX_SPEAKER);
            if(d != x->f_meter->getNumberOfLoudspeakers())
            {
                int dspState = canvas_suspend_dsp();
                x->f_meter->setNumberOfLoudspeakers(d);
                
                x->f_number_of_loudspeakers = x->f_meter->getNumberOfLoudspeakers();
                
                angles_of_loudspeakers_set(x, NULL, NULL, NULL);
                jbox_resize_inputs((t_jbox *)x, x->f_number_of_loudspeakers);
                jbox_resize_inputs((t_jbox *)x, x->f_number_of_loudspeakers);
                
                canvas_resume_dsp(dspState);
            }
        }
    }
    return NULL;
}

t_max_err angles_of_loudspeakers_set(t_meter *x, void *attr, long ac, t_atom *av)
{
    double angles[MAX_SPEAKER];
    if(ac > MAX_SPEAKER)
        ac = MAX_SPEAKER;
    if(ac && av)
    {
        for(int i = 0; i < ac; i++)
        {
            if(atom_gettype(av+i) == A_FLOAT)
            {
                angles[i] = atom_getfloat(av+i);
            }
        }
        x->f_meter->setLoudspeakerAnglesDegrees(ac, angles);
    }
    for(int i = 0; i < x->f_meter->getNumberOfLoudspeakers(); i++)
    {
        x->f_angles_of_loudspeakers[i] = x->f_meter->getLoudspeakerAngle(i);
    }

	jbox_invalidate_layer((t_object *)x, NULL, gensym("background_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("leds_layers"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("energy_vector_layer"));
    jbox_invalidate_layer((t_object *)x, NULL, gensym("velocity_vector_layer"));
    
	jbox_redraw((t_jbox *)x);
    return 0;
}

void meter_dsp(t_meter *x, t_object *dsp, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->f_meter->setVectorSize(maxvectorsize);
   
    object_method(dsp, gensym("dsp_add"), x, (method)meter_perform, 0, NULL);
    
	x->f_startclock = 1;
}

void meter_perform(t_meter *x, t_object *dsp, float **ins, long ni, float **outs, long no, long sf, long f,void *up)
{
	x->f_meter->process(ins);
	if (x->f_startclock)
	{
		x->f_startclock = 0;
		clock_delay(x->f_clock,0);
	}
}

void meter_tick(t_meter *x)
{
    x->f_meter->processEnergy();
    x->f_meter->processVectors();
    meter_output(x);
    
    double peak;
    for (int i = 0; i<x->f_number_of_loudspeakers; i++)
    {
        peak = x->f_meter->getLoudspeakerEnergy(i); // dB (negatif) de -240 à 0;
        if ( peak >= 0. ) x->f_drawOverLedleftTime[i] = OVERLED_DRAWTIME;
        else x->f_drawOverLedleftTime[i] -= x->f_interval;
        if (x->f_drawOverLedleftTime[i] < 0) x->f_drawOverLedleftTime[i] = 0;
    }
    
	jbox_invalidate_layer((t_object *)x, NULL, gensym("leds_layers"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("energy_vector_layer"));
    jbox_invalidate_layer((t_object *)x, NULL, gensym("velocity_vector_layer"));
  	jbox_redraw((t_jbox *)x);
    
	if (sys_getdspstate())
		clock_delay(x->f_clock, x->f_interval);
}

void meter_output(t_meter *x)
{
    long nbChannels = x->f_meter->getNumberOfInputs();
    t_atom*	av_peaks = new t_atom[nbChannels];
    t_atom	av_vectors[4];
    
    for (int i=0; i<nbChannels; i++)
        atom_setfloat(av_peaks+i, fabs(x->f_meter->getLoudspeakerPeaks(i)));
    outlet_list(x->f_peaks_outlet, NULL, nbChannels, av_peaks);
    
    atom_setfloat(av_vectors,   x->f_meter->getEnergyVectorAbscissa());
    atom_setfloat(av_vectors+1, x->f_meter->getEnergyVectorOrdinate());
    atom_setfloat(av_vectors+2, x->f_meter->getVelocityVectorAbscissa());
    atom_setfloat(av_vectors+3, x->f_meter->getVelocityVectorOrdinate());
    outlet_list(x->f_vector_outlet, NULL, 4, av_vectors);
	free(av_peaks);
}

void meter_free(t_meter *x)
{
	dsp_freejbox((t_jbox *)x);
    clock_free(x->f_clock);
    delete x->f_meter;
}

void meter_assist(t_meter *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
        sprintf(s,"(Signal) %s", x->f_meter->getChannelName(a).c_str());
    else
    {
        if (a == 0)
            sprintf(s,"(list) Vectors Values For Each Metering Interval : Energy(x/y) and Velocity(x/y)");
        else if (a == 1)
            sprintf(s,"(list) Peak Values For Each Metering Interval");
    }
}

t_max_err meter_notify(t_meter *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if (msg == gensym("attr_modified"))
	{
		jbox_invalidate_layer((t_object *)x, NULL, gensym("background_layer"));
        jbox_invalidate_layer((t_object *)x, NULL, gensym("leds_layers"));
        jbox_invalidate_layer((t_object *)x, NULL, gensym("energy_vector_layer"));
        jbox_invalidate_layer((t_object *)x, NULL, gensym("velocity_vector_layer"));
		jbox_redraw((t_jbox *)x);
	}
	return 0;
}

void meter_paint(t_meter *x, t_object *view)
{
	t_rect rect;
	jbox_get_rect_for_view((t_object *)x, view, &rect);
	
	x->f_radius_exterior = rect.width * .5 - 2.5;
	x->f_radius_interior = x->f_radius_exterior * 0.2;
	
	draw_background(x, view, &rect);
    draw_leds(x, view, &rect);

    if (x->f_drawvector == 2 || x->f_drawvector == 3)
		draw_vector_velocity(x, view, &rect);
	if (x->f_drawvector == 1 || x->f_drawvector == 3)
		draw_vector_energy(x, view, &rect);
}

void draw_background(t_meter *x,  t_object *view, t_rect *rect)
{
	int i;
    float coso, sino, angle, x1, y1, x2, y2;

	t_jmatrix transform;
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("background_layer"), rect->width, rect->height);
	
    t_jrgba black = cicm_rgba_addContrast(x->f_colorMeterBg,  -0.12);
    t_jrgba white = cicm_rgba_addContrast(x->f_colorMeterBg,  0.08);
    
	if (g)
	{
        jgraphics_matrix_init(&transform, 1, 0, 0, -1, rect->width * .5, rect->width * .5);
        jgraphics_set_matrix(g, &transform);
        
		jgraphics_set_line_width(g, 1.);

		jgraphics_set_source_jrgba(g, &x->f_colorMeterBg);
		jgraphics_arc(g, 0.f, 0.f, x->f_radius_exterior, 0., JGRAPHICS_2PI);
		jgraphics_fill(g);

		jgraphics_set_source_jrgba(g, &x->f_colorBackground);
		jgraphics_arc(g, 0.f, 0.f, x->f_radius_interior, 0., JGRAPHICS_2PI);
		jgraphics_fill(g);
		
        jgraphics_set_source_jrgba(g, &white);
        jgraphics_set_line_width(g, 1.f);
        jgraphics_arc(g, 1, 1, x->f_radius_exterior,  0., JGRAPHICS_2PI);
        jgraphics_stroke(g);
        jgraphics_arc(g, 1, 1, x->f_radius_interior,  0., JGRAPHICS_2PI);
        jgraphics_stroke(g);
        
        jgraphics_set_source_jrgba(g, &black);
        jgraphics_set_line_width(g, 1.f);
        jgraphics_arc(g, 0.f, 0.f, x->f_radius_exterior,  0., JGRAPHICS_2PI);
        jgraphics_stroke(g);
        jgraphics_arc(g, 0.f, 0.f, x->f_radius_interior,  0., JGRAPHICS_2PI);
        jgraphics_stroke(g);
        
        if(x->f_number_of_loudspeakers != 1)
        {
            for(i = 0; i < x->f_number_of_loudspeakers; i++)
            {
                angle = x->f_meter->getLoudspeakerAngleMappedRadian(i) - x->f_meter->getLoudspeakerWidthRadian(i) * 0.5f + x->f_offset_of_loudspeakers / 360. * CICM_2PI + CICM_PI2;
                if(x->f_clockwise)
                    angle = -angle + CICM_PI;
                jgraphics_set_line_width(g, 1.f);
               
                coso = cosf(angle);
                sino = sinf(angle);
                x1 = x->f_radius_interior * coso;
                y1 = x->f_radius_interior * sino;
                x2 = x->f_radius_exterior * coso;
                y2 = x->f_radius_exterior * sino;
                if(Tools::isInsideRad(angle, CICM_PI4, CICM_PI + CICM_PI4))
                {
                    jgraphics_move_to(g, x1 - 0.5, y1 - 0.5);
                    jgraphics_line_to(g, x2 - 0.5, y2 - 0.5);
                }
                else
                {
                    jgraphics_move_to(g, x1 + 0.5, y1 + 0.5);
                    jgraphics_line_to(g, x2 + 0.5, y2 + 0.5);
                }
                
                jgraphics_set_source_jrgba(g, &white);
                jgraphics_stroke(g);
                
                
				jgraphics_move_to(g, x1, y1);
				jgraphics_line_to(g, x2, y2);
                
                jgraphics_set_source_jrgba(g, &black);
				jgraphics_stroke(g);
			}
        }
		jbox_end_layer((t_object*)x, view, gensym("background_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("background_layer"), 0., 0.);
}

void draw_leds(t_meter *x, t_object *view, t_rect *rect)
{
    int i;
    float j, dB;
    float angle_start, angle, angle_end, radius, center_x, center_y;
    float led_width = 0.5 / 16. * rect->width;
    float offset = x->f_offset_of_loudspeakers / 360. * CICM_2PI + CICM_PI2;
    
    t_jrgba white = cicm_rgba_addContrast(x->f_colorMeterBg,  0.08);
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("leds_layers"), rect->width, rect->height);
    
	if (g)
	{
        for(i = 0; i < x->f_meter->getNumberOfLoudspeakers(); i++)
        {
            angle = x->f_meter->getLoudspeakerAngleMappedRadian(i) + offset;
            angle_start = angle - x->f_meter->getLoudspeakerWidthRadian(i) * 0.5f + 6.f / rect->width;
            angle_end   = angle + x->f_meter->getLoudspeakerWidthRadian(i) * 0.5f - 6.f / rect->width;
            
            if(x->f_clockwise)
            {
                angle_start = -angle_start + CICM_PI;
                angle_end = -angle_end + CICM_PI;
                angle = -angle + CICM_PI;
            }
            for(j = 11, dB = -34.5; j > -1; j--, dB += 3.)
            {
                center_x = pd_abscissa(led_width * (2 - (j / 11.)), angle);
                center_y = -pd_ordinate(led_width * (2 - (j / 11.)), angle);
                radius    = (j + 3.33) * led_width;
                
                if(x->f_meter->getLoudspeakerEnergy(i) > dB)
                {
                    if(j > 8)
                        jgraphics_set_source_jrgba(g, &x->f_colorColdSignal);
                    else if(j > 5)
                        jgraphics_set_source_jrgba(g, &x->f_colorTepidSignal);
                    else if(j > 2)
                        jgraphics_set_source_jrgba(g, &x->f_colorWarmSignal);
                    else
                        jgraphics_set_source_jrgba(g, &x->f_colorHotSignal);
                        
                    
                    jgraphics_set_line_width(g, led_width - pd_clip_min(360. / rect->width, 2.));
                    jgraphics_arc(g, center_x + rect->width * 0.5f, center_y + rect->width * 0.5f, radius, angle_start, angle_end);
                    jgraphics_stroke(g);
                }
                else if(j != -1)
                {
                    jgraphics_set_source_jrgba(g, &white);
                    jgraphics_set_line_width(g, led_width - pd_clip_min(360. / rect->width, 2.));
                    jgraphics_arc(g, center_x + rect->width * 0.5f, center_y + rect->width * 0.5f, radius,  angle_start, angle_end);
                    jgraphics_stroke(g);
                }
            }
            if(x->f_drawOverLedleftTime[i])
            {
                center_x = pd_abscissa(led_width * (2 - (-1 / 11.)), angle);
                center_y = -pd_ordinate(led_width * (2 - (-1 / 11.)), angle);
                radius    = (2.33) * led_width;
                jgraphics_set_source_jrgba(g, &x->f_colorOverSignal);
                jgraphics_set_line_width(g, led_width - pd_clip_min(360. / rect->width, 2.));
                jgraphics_arc(g, center_x + rect->width * 0.5f, center_y + rect->width * 0.5f, radius, angle_start, angle_end);
                jgraphics_stroke(g);
            }
        }
		jbox_end_layer((t_object*)x, view, gensym("leds_layers"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("leds_layers"), 0., 0.);
}

void draw_vector_energy(t_meter *x, t_object *view, t_rect *rect)
{
	double angle, rayon;
	t_jmatrix transform;
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("energy_vector_layer"), rect->width, rect->height);
	
	if (g)
	{
		jgraphics_matrix_init(&transform, 1, 0, 0, -1, rect->width / 2., rect->width / 2.);
		jgraphics_set_matrix(g, &transform);
		
		/* Energy */
		jgraphics_set_source_jrgba(g, &x->f_colorEnergy);
        
        rayon = x->f_radius_interior * 0.85;
        angle = x->f_meter->getEnergyVectorAngle() + (x->f_offset_of_loudspeakers / 180.) * JGRAPHICS_PI;
		double x1;
		double y1;
        
        if(!x->f_clockwise)
        {
            x1 = Tools::abscissa(rayon, angle + CICM_PI2);
            y1 = Tools::ordinate(rayon, angle + CICM_PI2);
        }
        else
        {
            x1 = Tools::abscissa(rayon, -angle + CICM_PI2);
            y1 = Tools::ordinate(rayon, -angle + CICM_PI2);
        }
		jgraphics_move_to(g, 0, 0);
		jgraphics_line_to(g, x1, y1);
        jgraphics_stroke(g);
		
        jgraphics_arc(g, x1, y1, 2, 0., JGRAPHICS_2PI);
		jgraphics_fill(g);
		
		/* Center */
		jgraphics_arc(g, 0, 0, 3., 0., JGRAPHICS_2PI);
		jgraphics_fill(g);
        
		jbox_end_layer((t_object*)x, view, gensym("energy_vector_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("energy_vector_layer"), 0., 0.);
}

void draw_vector_velocity(t_meter *x, t_object *view, t_rect *rect)
{
	double angle, rayon;
	t_jmatrix transform;
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("velocity_vector_layer"), rect->width, rect->height);
	
	if (g)
	{
		jgraphics_matrix_init(&transform, 1, 0, 0, -1, rect->width / 2., rect->width / 2.);
		jgraphics_set_matrix(g, &transform);
		
		/* Velocity */
		jgraphics_set_source_jrgba(g, &x->f_colorVelocity);
        
        rayon = x->f_radius_interior * 0.85;
        angle = x->f_meter->getEnergyVectorAngle() + (x->f_offset_of_loudspeakers / 180.) * JGRAPHICS_PI;
		double x1;
		double y1;
        
		if(!x->f_clockwise)
        {
            x1 = Tools::abscissa(rayon, angle + CICM_PI2);
            y1 = Tools::ordinate(rayon, angle + CICM_PI2);
        }
        else
        {
            x1 = Tools::abscissa(rayon, -angle + CICM_PI2);
            y1 = Tools::ordinate(rayon, -angle + CICM_PI2);
        }
		jgraphics_move_to(g, 0, 0);
		jgraphics_line_to(g, x1, y1);
        jgraphics_stroke(g);
		
        jgraphics_arc(g, x1, y1, 2, 0., JGRAPHICS_2PI);
		jgraphics_fill(g);
		
		/* Center */
		jgraphics_arc(g, 0, 0, 3., 0., JGRAPHICS_2PI);
		jgraphics_fill(g);
        
		jbox_end_layer((t_object*)x, view, gensym("velocity_vector_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("velocity_vector_layer"), 0., 0.);
}

