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

#include "../MaxConverter.h"

typedef struct  _space
{
	t_jbox		j_box;
	void*		f_out;
	void*		f_outInfos;

	long		f_mode;
	long		f_order;

	t_jrgba		f_color_background;
	t_jrgba		f_color_border_box;
	t_jrgba		f_color_circleInner;
	t_jrgba		f_color_harmonics;
	t_jrgba		f_color_points;

	t_rect		f_center;
    double		f_nbCircleToDraw;
	double		f_rayonGlobal;
	double		f_rayonCircle;
    double      f_rayonExtCircle;
    
    t_jsurface* f_cursor_gain;
    t_jsurface* f_cursor_rotation;
    
    t_pt        f_mousepos;
    int         f_cursorType;

	AmbisonicViewer*        f_viewer;
    AmbisonicRecomposer*    f_recomposer;

	double                  f_harmonicsValues[MAX_SPEAKER];
    double                  f_mode_values[MAX_SPEAKER];
    double                  f_microphonesValues[MAX_SPEAKER];
    t_atom                  f_tempory_values[MAX_SPEAKER];
    long                    f_number_of_harmonics;
    t_atom_long             f_number_of_microphones;
    double                  f_reference_angle;
    double                  f_rotation;
    double                  f_rotation_max;
    double                  f_retractation;
} t_space;

t_class *space_class;

void *space_new(t_symbol *s, int argc, t_atom *argv);
void space_free(t_space *x);
void space_assist(t_space *x, void *b, long m, long a, char *s);
void space_preset(t_space *x);
void space_bang(t_space *x);

t_max_err space_notify(t_space *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void space_getdrawparams(t_space *x, t_object *patcherview, t_jboxdrawparams *params);
t_max_err number_of_microphones_set(t_space *x, t_object *attr, long argc, t_atom *argv);
t_max_err coefficients_set(t_space *x, t_object *attr, long ac, t_atom *av);

t_max_err space_setvalueof(t_space *x, long ac, t_atom *av);
t_max_err space_getvalueof(t_space *x, long *ac, t_atom **av);

/* Interaction ***************************************/
void space_mouse_move(t_space *x, t_object *patcherview, t_pt pt, long modifiers);
void space_mouse_down(t_space *x, t_object *patcherview, t_pt pt, long modifiers);
void space_mouse_drag(t_space *x, t_object *patcherview, t_pt pt, long modifiers);
void space_mouse_enddrag(t_space *x, t_object *patcherview, t_pt pt, long modifiers);

void space_draw_points(t_space *x, t_object *patcherview, t_pt pt, long modifiers);
void space_rotate_points(t_space *x, t_object *patcherview, t_pt pt, long modifiers);
void space_retract_points(t_space *x, t_object *patcherview, t_pt pt, long modifiers);

void space_compute(t_space *x);
void space_output(t_space *x);

/* Paint *********************************************/
void space_paint(t_space *x, t_object *view);
void draw_background(t_space *x, t_object *view, t_rect *rect);
void draw_harmonics(t_space *x,  t_object *view, t_rect *rect);
void draw_microphones_points(t_space *x, t_object *view, t_rect *rect);
void draw_cursor(t_space *x, t_object *view, t_rect *rect);

int C74_EXPORT main()
{
	t_class *c;
    
	c = class_new("hoa.space", (method)space_new, (method)space_free, (short)sizeof(t_space), 0L, A_GIMME, 0);

	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
	jbox_initclass(c, JBOX_COLOR | JBOX_FIXWIDTH);

	class_addmethod(c, (method)space_assist,          "assist",         A_CANT,	0);
	class_addmethod(c, (method)space_paint,           "paint",          A_CANT,	0);
	class_addmethod(c, (method)space_notify,          "notify",         A_CANT, 0);
    class_addmethod(c, (method)space_bang,            "bang",           A_CANT, 0);
	class_addmethod(c, (method)space_getdrawparams,   "getdrawparams",  A_CANT, 0);
	class_addmethod(c, (method)space_mouse_down,      "mousedown",      A_CANT, 0);
    class_addmethod(c, (method)space_mouse_move,      "mousemove",      A_CANT, 0);
	class_addmethod(c, (method)space_mouse_drag,      "mousedrag",      A_CANT, 0);
    class_addmethod(c, (method)space_mouse_enddrag,   "mouseup",        A_CANT, 0);
    class_addmethod(c, (method)space_preset,          "preset",         0);
    class_addmethod(c, (method)space_getvalueof,      "getvalueof",     A_CANT, 0);
	class_addmethod(c, (method)space_setvalueof,      "setvalueof",     A_CANT, 0);
    class_addmethod(c, (method)coefficients_set,      "list",           A_GIMME, 0);
    
	CLASS_ATTR_DEFAULT				(c, "patching_rect", 0, "0 0 225 225");
	CLASS_ATTR_INVISIBLE			(c, "color", 0);
	CLASS_ATTR_INVISIBLE			(c, "textcolor", 0);

    CLASS_ATTR_LONG					(c, "channels", 0, t_space, f_number_of_microphones);
	CLASS_ATTR_CATEGORY				(c, "channels", 0, "Behavior");
	CLASS_ATTR_ORDER				(c, "channels", 0, "1");
	CLASS_ATTR_LABEL				(c, "channels", 0, "Number of Channels");
	CLASS_ATTR_ACCESSORS			(c, "channels", NULL, number_of_microphones_set);
	CLASS_ATTR_DEFAULT_SAVE_PAINT   (c, "channels", 0,"8");
	CLASS_ATTR_ALIAS                (c, "channels", "loudspeakers");
    
    CLASS_ATTR_DOUBLE_VARSIZE       (c, "coeffs", 0, t_space, f_microphonesValues, f_number_of_microphones, MAX_SPEAKER);
	CLASS_ATTR_CATEGORY             (c, "coeffs", 0, "Behavior");
	CLASS_ATTR_ORDER                (c, "coeffs", 0, "2");
	CLASS_ATTR_LABEL                (c, "coeffs", 0, "Channels coefficients");
    CLASS_ATTR_ACCESSORS			(c, "coeffs", NULL, coefficients_set);
	CLASS_ATTR_DEFAULT              (c, "coeffs", 0, "666.");
	CLASS_ATTR_SAVE                 (c, "coeffs", 0);
    
	CLASS_ATTR_RGBA					(c, "bgcolor", 0, t_space, f_color_background);
	CLASS_ATTR_CATEGORY				(c, "bgcolor", 0, "Color");
	CLASS_ATTR_STYLE				(c, "bgcolor", 0, "rgba");
	CLASS_ATTR_LABEL				(c, "bgcolor", 0, "Background Color");
	CLASS_ATTR_DEFAULT_SAVE_PAINT	(c, "bgcolor", 0, "1. 1. 1. 1.");
	
	CLASS_ATTR_RGBA					(c, "harmocolor", 0, t_space, f_color_harmonics);
	CLASS_ATTR_CATEGORY				(c, "harmocolor", 0, "Color");
	CLASS_ATTR_STYLE				(c, "harmocolor", 0, "rgba");
	CLASS_ATTR_LABEL				(c, "harmocolor", 0, "Harmonics color");
	CLASS_ATTR_DEFAULT_SAVE_PAINT	(c, "harmocolor", 0, "0. 0. 1. 0.25");
	
	CLASS_ATTR_RGBA					(c, "miccolor", 0, t_space, f_color_points);
	CLASS_ATTR_CATEGORY				(c, "miccolor", 0, "Color");
	CLASS_ATTR_STYLE				(c, "miccolor", 0, "rgba");
	CLASS_ATTR_LABEL				(c, "miccolor", 0, "Virtuals microphones color");
	CLASS_ATTR_DEFAULT_SAVE_PAINT	(c, "miccolor", 0, "0. 0. 0. 1.");
	
	CLASS_ATTR_RGBA					(c, "cicolorin", 0, t_space, f_color_circleInner);
	CLASS_ATTR_CATEGORY				(c, "cicolorin", 0, "Color");
	CLASS_ATTR_STYLE_LABEL			(c, "cicolorin", 0, "rgba", "Circle Inner Color");
	CLASS_ATTR_DEFAULTNAME_SAVE		(c, "cicolorin", 0, "0.7 0.7 0.7 1.");
	
	CLASS_ATTR_RGBA					(c, "borderboxcolor", 0, t_space, f_color_border_box);
	CLASS_ATTR_CATEGORY				(c, "borderboxcolor", 0, "Color");
	CLASS_ATTR_STYLE_LABEL			(c, "borderboxcolor", 0, "rgba", "Border Box Color");
	CLASS_ATTR_DEFAULTNAME_SAVE		(c, "borderboxcolor", 0, "0.5 0.5 0.5 1.");
	
	CLASS_ATTR_ORDER				(c, "bgcolor", 0, "1");
	CLASS_ATTR_ORDER				(c, "borderboxcolor", 0, "2");
	CLASS_ATTR_ORDER				(c, "cicolor", 0, "3");
	CLASS_ATTR_ORDER				(c, "cicolorin", 0, "4");
	CLASS_ATTR_ORDER				(c, "cishadcolor", 0, "5");
	CLASS_ATTR_ORDER				(c, "harmocolor", 0, "6");
	CLASS_ATTR_ORDER				(c, "miccolor", 0, "7");
	
	class_register(CLASS_BOX, c);
	space_class = c;
	
	class_findbyname(CLASS_BOX, gensym("hoa.encoder~"));
	return 0;
}

void *space_new(t_symbol *s, int argc, t_atom *argv)
{
	t_space *x =  NULL; 
	t_dictionary *d;
	long flags;
	
	if (!(d = object_dictionaryarg(argc,argv)))
		return NULL;

	x = (t_space *)object_alloc(space_class);
	flags = 0 
			| JBOX_DRAWFIRSTIN 
			| JBOX_DRAWINLAST
			| JBOX_TRANSPARENT	
			| JBOX_GROWY
			| JBOX_DRAWBACKGROUND
			;
    
	x->f_viewer				= new AmbisonicViewer(1);
    x->f_recomposer         = new AmbisonicRecomposer(1, 3, Hoa_Fixe);
    
	jbox_new((t_jbox *)x, flags, argc, argv);
	x->j_box.b_firstin = (t_object *)x;

    x->f_outInfos   = outlet_new(x, NULL);
    x->f_out        = listout(x);

	attr_dictionary_process(x, d);
	jbox_ready((t_jbox *)x);

	return (x);
}			

void space_free(t_space *x)
{
	jbox_free((t_jbox *)x);

	delete x->f_viewer;
    delete x->f_recomposer;
}

void space_assist(t_space *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
	{
		sprintf(s,"(Anything) Behavior and appearance");
	}
	else
	{
		if (a == 0)
			sprintf(s,"(List) Virtual microphones coefficients");
        else if (a == 1)
			sprintf(s,"(int) Number of Virtual microphones");
	}
}

t_max_err space_notify(t_space *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	t_symbol *name;
	if (msg == gensym("attr_modified"))
	{
		name = (t_symbol *)object_method((t_object *)data, gensym("getname"));
		
		if(name == gensym("bgcolor") || name == gensym("borderboxcolor") || name == gensym("cicolor") || name == gensym("cicolorin") )
		{
			jbox_invalidate_layer((t_object *)x, NULL, gensym("background_layer"));
		}
		else if(name == gensym("harmocolor"))
		{
			jbox_invalidate_layer((t_object *)x, NULL, gensym("harmonics_layer"));
		}
		else if(name == gensym("miccolor"))
		{
			jbox_invalidate_layer((t_object *)x, NULL, gensym("microphones_points_layer"));
		}
		jbox_redraw((t_jbox *)x);
	}
	return jbox_notify((t_jbox *)x, s, msg, sender, data);
}

void space_getdrawparams(t_space *x, t_object *patcherview, t_jboxdrawparams *params)
{
    params->d_boxfillcolor = x->f_color_background;
    params->d_bordercolor = x->f_color_border_box;
	params->d_borderthickness = 1;
	params->d_cornersize = 6;
}

void space_bang(t_space *x)
{
    space_compute(x);
}

/**********************************************************/
/*                      Preset et Pattrs                  */
/**********************************************************/

void space_preset(t_space *x)
{
    
    void* z;
    if(!(z = gensym("_preset")->s_thing))
        return;
    
    for(int i = 0; i < x->f_number_of_microphones; i++)
        binbuf_vinsert(z, gensym("osslf")->s_name, x, object_classname(x), gensym("coeffs"), i, (float)x->f_microphonesValues[i]);
}

t_max_err space_setvalueof(t_space *x, long ac, t_atom *av)
{
	if (ac && av)
    {
        if(ac > MAX_SPEAKER)
            ac = MAX_SPEAKER;
        for (int i = 0; i < ac; i++)
        {
            if(atom_gettype(av+i) == A_FLOAT)
                x->f_microphonesValues[i] = Tools::clip((double)atom_getfloat(av + i), 0., 1.);
        }
        space_compute(x);
	}
	return MAX_ERR_NONE;
}

t_max_err space_getvalueof(t_space *x, long *ac, t_atom **av)
{
	if (ac && av)
    {
		if (*ac && *av)
        {
            int limit = *ac;
            if (limit > MAX_SPEAKER)
                limit = MAX_SPEAKER;
            
            for (int i = 0; i < limit; i++)
                atom_setfloat(*av+i, (float)x->f_microphonesValues[i]);
		}
        else
        {
			*av = (t_atom *)getbytes(x->f_number_of_microphones * sizeof(t_atom));
            *ac = x->f_number_of_microphones;
        }
        for (int i = 0; i < *ac; i++)
            atom_setfloat(*av+i, (float)x->f_microphonesValues[i]);
    }
	return MAX_ERR_NONE;
}

/**********************************************************/
/*                      Dessin                            */
/**********************************************************/

void space_paint(t_space *x, t_object *view)
{
	t_rect rect;
	jbox_get_rect_for_view((t_object *)x, view, &rect);
	
	x->f_center.x = rect.width * .5;
	x->f_center.y = rect.height * .5;
	
	x->f_rayonGlobal = rect.width * .5;
	if(rect.width > rect.height)
		x->f_rayonGlobal = rect.height * .5;
	
    x->f_nbCircleToDraw = 5;
    x->f_rayonExtCircle = x->f_rayonGlobal - 5;
    x->f_rayonCircle = x->f_rayonExtCircle / x->f_nbCircleToDraw;
	
	draw_background(x, view, &rect);
	draw_harmonics(x, view, &rect);
	draw_microphones_points(x, view, &rect);
    draw_cursor(x, view, &rect);
}

void draw_cursor(t_space *x, t_object *view, t_rect *rect)
{
	t_jmatrix transform;
    
    t_jrgba black = {0,0,0,1.};
    
    double size;
    
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("cursor_layer"), rect->width, rect->height);
	
	if (g)
	{
        if (x->f_cursorType != 0)
        {
            jgraphics_matrix_init(&transform, 1, 0, 0, -1, x->f_center.x, x->f_center.y);
            jgraphics_set_matrix(g, &transform);
            jgraphics_set_source_jrgba(g, &black);
            jgraphics_rotate(g, x->f_mousepos.y);
            jgraphics_translate(g, 0, x->f_mousepos.x*rect->width*0.5);
            
            if (x->f_cursorType == 1) // gain cursor
            {
                size = 10;
                
                jgraphics_move_to(g, 0, -size*0.6);
                jgraphics_line_to(g, 0, size*0.6);
                jgraphics_set_line_width(g, 2);
                jgraphics_stroke(g);
                
                jgraphics_move_to(g, 0, -size);
                jgraphics_line_to(g, -(size*0.4), -size*0.5);
                jgraphics_line_to(g, (size*0.4), -size*0.5);
                jgraphics_line_to(g, 0, -size);
                jgraphics_fill(g);
                
                jgraphics_move_to(g, 0, size);
                jgraphics_line_to(g, -(size*0.6), size*0.5);
                jgraphics_line_to(g, (size*0.6), size*0.5);
                jgraphics_line_to(g, 0, size);
                jgraphics_fill(g);
                
                jmouse_setcursor(view, (t_object *)x, JMOUSE_CURSOR_NONE);
            }
            
            if (x->f_cursorType == 2) // rotation cursor
            {
                size = 6;
                jgraphics_translate(g, size*1.5, 0);
                jgraphics_ovalarc(g, -size*1.5, 0, size*1.5, size*0.5, CICM_2PI*0.1, CICM_2PI*0.4);
                jgraphics_set_line_width(g, 2);
                jgraphics_stroke(g);
                
                jgraphics_translate(g, -size*1.5, 0);
                
                jgraphics_translate(g, -size*0.4, size*0.7);
                jgraphics_rotate(g, 0.4);
                jgraphics_move_to(g, -size*2, 0);
                jgraphics_line_to(g, -size*1, size*0.6);
                jgraphics_line_to(g, -size*1, -size*0.6);
                jgraphics_fill(g);
                
                jgraphics_translate(g, size*0.4, -size*0.7);
                jgraphics_rotate(g, -0.4);
                
                jgraphics_translate(g, size*0.2, size*0.5);
                jgraphics_rotate(g, -0.4);
                jgraphics_move_to(g, size*2, 0);
                jgraphics_line_to(g, size*1, size*0.6);
                jgraphics_line_to(g, size*1, -size*0.6);
                jgraphics_fill(g);
                
                jmouse_setcursor(view, (t_object *)x, JMOUSE_CURSOR_NONE);
            }
        }
        else
        {
            jmouse_setcursor(view, (t_object*)x, JMOUSE_CURSOR_ARROW);
        }
        jbox_end_layer((t_object*)x, view, gensym("cursor_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("cursor_layer"), 0., 0.);
}

void draw_background(t_space *x,  t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("background_layer"), rect->width, rect->height);
	
	if (g) 
	{
        t_jmatrix transform;
        jgraphics_matrix_init(&transform, 1, 0, 0, -1, x->f_center.x, x->f_center.y);
		jgraphics_set_matrix(g, &transform);
        
        t_jrgba black  = CicmMax::jrgba_addContrast(x->f_color_circleInner, -0.12);
        t_jrgba white  = CicmMax::jrgba_addContrast(x->f_color_circleInner, 0.08);
        
        /* Gros cercle ***********************************/
		jgraphics_arc(g, 0., 0., x->f_rayonExtCircle,  0., JGRAPHICS_2PI);
		jgraphics_set_source_jrgba(g, &x->f_color_circleInner);
		jgraphics_fill(g);
		
		/* Petits cercles ********************************/
		for(int i = x->f_nbCircleToDraw; i > 0; i--)
		{
            /* Ombres */
            jgraphics_set_line_width(g, 2);
            jgraphics_set_source_jrgba(g, &white);
            jgraphics_arc(g, 0.5, 0.5, (double)i * x->f_rayonCircle,  0., JGRAPHICS_2PI);
            jgraphics_stroke(g);
            
			/* Circle color */
			jgraphics_set_line_width(g, 1);
			jgraphics_set_source_jrgba(g, &black);
			jgraphics_arc(g, 0, 0, (double)i * x->f_rayonCircle,  0., JGRAPHICS_2PI);
			jgraphics_stroke(g);
		}

		/* Axes *******************************************/
		jgraphics_set_source_jrgba(g, &black);
		
		for(int i = 0; i < x->f_number_of_microphones; i++)
		{
            double y1, y2, rotateAngle;
			rotateAngle = (double)i / (double)x->f_number_of_microphones * CICM_2PI - CICM_PI / (double)x->f_number_of_microphones;
			jgraphics_rotate(g, rotateAngle);
            
            y1 = 1. / x->f_nbCircleToDraw * x->f_rayonExtCircle;
			y2 = x->f_rayonExtCircle;
			
            /* Inner shadow */
            if ( Tools::isInsideDeg( Tools::radToDeg(rotateAngle), 46., -135.) )
            {
                jgraphics_move_to(g, -0.5, y1-0.5);
                jgraphics_line_to(g, -0.5, y2-0.5);
            }
            else
            {
                jgraphics_move_to(g, 0.5, y1+0.5);
                jgraphics_line_to(g, 0.5, y2+0.5);
            }
            jgraphics_set_line_width(g, 2);
            jgraphics_set_source_jrgba(g, &white);
            jgraphics_stroke(g);
			
			jgraphics_move_to(g, 0, y1);
			jgraphics_line_to(g, 0, y2);
			jgraphics_set_source_jrgba(g, &black);
			jgraphics_set_line_width(g, 1);
			jgraphics_stroke(g);
			
			jgraphics_rotate(g, -rotateAngle);
		}
		jbox_end_layer((t_object*)x, view, gensym("background_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("background_layer"), 0., 0.);
}

void draw_harmonics(t_space *x,  t_object *view, t_rect *rect)
{
	int pathLength = 0;
	t_pt beginCoord;
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("harmonics_layer"), rect->width, rect->height);

	if (g)
	{
		t_jmatrix transform;
		jgraphics_matrix_init(&transform, 1, 0, 0, -1, x->f_center.x, x->f_center.y);
		jgraphics_set_matrix(g, &transform);
		jgraphics_set_line_width(g, 2);
        
		if(x->f_viewer->getBiggestContribution() != 0.)
		{
            double max = 0.;
            
            double normalize = 1;
            for(int i = 0 ; i < x->f_number_of_microphones; i++)
            {
                if(x->f_microphonesValues[i] > max)
                    max = x->f_microphonesValues[i];
            }
            normalize = max;
            
            
            if(x->f_mode == 1)
                normalize = x->f_rotation_max;
            
            double factor = x->f_rayonExtCircle * normalize;
            
			jgraphics_set_source_jrgba(g, &x->f_color_harmonics);
			for(int i = 0; i < NUMBEROFCIRCLEPOINTS; i++)
			{
				
				if (i == NUMBEROFCIRCLEPOINTS-1) {
					jgraphics_line_to(g, beginCoord.x, beginCoord.y );
				}
				else if(x->f_viewer->getColor(i) == 1)
				{
					if (pathLength == 0) 
					{
						beginCoord.x = x->f_viewer->getAbscisseValue(i) * factor;
						beginCoord.y = x->f_viewer->getOrdinateValue(i) * factor;
						jgraphics_move_to(g, beginCoord.x, beginCoord.y );
						pathLength++;
					}
                    else
                    {
						jgraphics_line_to(g, x->f_viewer->getAbscisseValue(i) * factor, 
										  x->f_viewer->getOrdinateValue(i) * factor);
					}
				}
			}
			if (pathLength) {
				jgraphics_close_path(g);
				jgraphics_fill_preserve(g);
				jgraphics_stroke(g);
			}
		}
		jbox_end_layer((t_object*)x, view, gensym("harmonics_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("harmonics_layer"), 0., 0.);
}

void draw_microphones_points(t_space *x,  t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("microphones_points_layer"), rect->width, rect->height);

	if (g) 
	{
		t_jmatrix transform;
		jgraphics_matrix_init(&transform, 1, 0, 0, -1, x->f_center.x, x->f_center.y);
		jgraphics_set_matrix(g, &transform);

		jgraphics_set_source_jrgba(g, &x->f_color_points);
        double loudspeaker_angle = CICM_2PI / (double)x->f_number_of_microphones;
        double factor1 = (x->f_nbCircleToDraw-1) * x->f_rayonCircle;
        double factor2 = x->f_rayonCircle;
        for(int i = 0; i < x->f_number_of_microphones; i++)
        {
            jgraphics_set_source_jrgba(g, &x->f_color_points);
            double angle = loudspeaker_angle * (double)(i) + CICM_PI2;
            jgraphics_arc(g, Tools::abscissa(x->f_microphonesValues[i] * factor1 + factor2, angle), Tools::ordinate(x->f_microphonesValues[i] * factor1 + factor2, angle), 3.,  0., JGRAPHICS_2PI);
            jgraphics_fill(g);
        }
		jbox_end_layer((t_object*)x, view, gensym("microphones_points_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("microphones_points_layer"), 0., 0.);
}

/**********************************************************/
/*                      Souris                            */
/**********************************************************/

void space_mouse_move(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    double mapped_x = (pt.x - x->f_center.x) / x->f_center.x;
    double mapped_y = (pt.y - x->f_center.y) / x->f_center.y * -1.;
    double radius   = Tools::radius(mapped_x, mapped_y);
    double angle    = Tools::angle(mapped_x, mapped_y) - CICM_PI2;
    
    x->f_mousepos.x = radius;
    x->f_mousepos.y = angle;
    
    if(modifiers == 132 || modifiers == 5) // ctrl : rotation
    {
        x->f_cursorType = 2;
    }
    else if(modifiers == 2) // shift : gain
    {
        x->f_cursorType = 1;
    }
    else
    {
        x->f_cursorType = 0;
    }
    
    jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
    jbox_redraw((t_jbox *)x);
}

void space_mouse_down(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    // cmd = 17
    // ctrl = 148
    double mapped_x = (pt.x - x->f_center.x) / x->f_center.x;
    double mapped_y = (pt.y - x->f_center.y) / x->f_center.y * -1.;
    double radius   = Tools::radius(mapped_x, mapped_y);
    double angle    = Tools::angle(mapped_x, mapped_y) - CICM_PI2;
    
    if(modifiers == 18) // shift
    {
        x->f_mode = 2;
        x->f_retractation = radius;
        for(int i =0; i < x->f_number_of_microphones; i++)
        {
            x->f_mode_values[i] = x->f_microphonesValues[i];
        }
    }
    else if(modifiers == 148 || modifiers == 21) // ctrl
    {
        x->f_mode = 1;
        x->f_rotation_max = 0.;
        for(int i = 0 ; i < x->f_number_of_microphones; i++)
        {
            if(x->f_microphonesValues[i] > x->f_rotation_max)
                x->f_rotation_max = x->f_microphonesValues[i];
        }
        x->f_reference_angle = angle;
        for(int i =0; i < x->f_number_of_microphones; i++)
            x->f_mode_values[i] = x->f_microphonesValues[i];
    }
    else
    {
        x->f_mode = 0;
        space_mouse_drag(x, patcherview, pt, modifiers);
    }
}

void space_mouse_drag(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    double mapped_x = (pt.x - x->f_center.x) / x->f_center.x;
    double mapped_y = (pt.y - x->f_center.y) / x->f_center.y * -1.;
    double radius   = Tools::radius(mapped_x, mapped_y);
    double angle    = Tools::angle(mapped_x, mapped_y) - CICM_PI2;
    
    x->f_mousepos.x = radius;
    x->f_mousepos.y = angle;
    
    if(x->f_mode == 0)
    {
        x->f_cursorType = 0;
        space_draw_points(x, patcherview, pt, modifiers);
    }
    else if(x->f_mode == 1)
    {
        x->f_cursorType = 2;
        space_rotate_points(x, patcherview, pt, modifiers);
    }
    else if(x->f_mode == 2)
    {
        x->f_cursorType = 1;
        space_retract_points(x, patcherview, pt, modifiers);
    }
    
    jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
    jbox_redraw((t_jbox *)x);
}

void space_mouse_enddrag(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    space_compute(x);
}

void space_draw_points(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    double loudspeaker_angle = CICM_2PI / (double)x->f_number_of_microphones;
    double loudspeaker_angle_mid = loudspeaker_angle / 2.;
    
    double mapped_x = (pt.x - x->f_center.x);
    double mapped_y = (pt.y - x->f_center.y);
    
    double angle    = Tools::angle(mapped_x, -mapped_y) - CICM_PI2;
    if(angle < 0.)
        angle += CICM_2PI;
    
    if(Tools::radius(mapped_x, mapped_y) < x->f_rayonCircle)
    {
        if(angle > CICM_2PI - loudspeaker_angle_mid || angle < loudspeaker_angle_mid)
            atom_setfloat(x->f_tempory_values, 0.);
        else
        {
            for(int i = 1; i < x->f_number_of_microphones; i++)
            {
                if(angle > i * loudspeaker_angle - loudspeaker_angle_mid && angle < i * loudspeaker_angle + loudspeaker_angle_mid)
                    atom_setfloat(x->f_tempory_values+i, 0.);
            }
        }
    }
    else
    {
        double center_x = Tools::abscissa(x->f_rayonCircle, angle - CICM_PI2 + CICM_PI);
        mapped_x = mapped_x - center_x;
        double center_y = Tools::ordinate(x->f_rayonCircle, angle - CICM_PI2 + CICM_PI);
        mapped_y = -mapped_y - center_y;
        double radius  = Tools::radius(mapped_x, mapped_y) / (4. * x->f_rayonCircle);
    
        if(angle > CICM_2PI - loudspeaker_angle_mid || angle < loudspeaker_angle_mid)
            atom_setfloat(x->f_tempory_values, radius);
        else
        {
            for(int i = 1; i < x->f_number_of_microphones; i++)
            {
                if(angle > i * loudspeaker_angle - loudspeaker_angle_mid && angle < i * loudspeaker_angle + loudspeaker_angle_mid)
                    atom_setfloat(x->f_tempory_values+i, radius);
            }
        }
    }
    object_method(x, gensym("coeffs"), x->f_number_of_microphones, x->f_tempory_values);
}

void space_rotate_points(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    double loudspeaker_angle = CICM_2PI / (double)x->f_number_of_microphones;
    double mapped_x = (pt.x - x->f_center.x) / x->f_center.x;
    double mapped_y = (pt.y - x->f_center.y) / x->f_center.y * -1.;
    x->f_rotation  = Tools::angle(mapped_x, mapped_y) - CICM_PI2;
    double offset = x->f_rotation - x->f_reference_angle;
    offset *= -1.;
    while(offset < 0.)
        offset += CICM_2PI;
    int nbDecalage = offset / loudspeaker_angle;
    double decimal = (offset - loudspeaker_angle * nbDecalage) / loudspeaker_angle;
    
    double newcoeff;
    int index1, index2;
    
    for(int i = 0; i < x->f_number_of_microphones; i++)
    {
        index1 = ((i + nbDecalage) + x->f_number_of_microphones) % x->f_number_of_microphones;
        index2 = ((i + 1 + nbDecalage) + x->f_number_of_microphones) % x->f_number_of_microphones;
        newcoeff = x->f_mode_values[index2] * decimal + x->f_mode_values[index1] * (1. - decimal);
        atom_setfloat(x->f_tempory_values+i ,newcoeff);
    }

    object_method(x, gensym("coeffs"), x->f_number_of_microphones, x->f_tempory_values);
}

void space_retract_points(t_space *x, t_object *patcherview, t_pt pt, long modifiers)
{
    double mapped_x = (pt.x - x->f_center.x) / x->f_center.x;
    double mapped_y = (pt.y - x->f_center.y) / x->f_center.y * -1.;
    double radius   = (Tools::radius(mapped_x, mapped_y) - (1. / 6.)) * (31. / 20.);
    double offset = x->f_retractation - radius;
    
    for(int i = 0; i < x->f_number_of_microphones; i++)
        atom_setfloat(x->f_tempory_values+i ,x->f_mode_values[i] - offset);
    
    object_method(x, gensym("coeffs"), x->f_number_of_microphones, x->f_tempory_values);
}

void space_compute(t_space *x)
{
    x->f_recomposer->processFixe(x->f_microphonesValues, x->f_harmonicsValues);
    x->f_viewer->processContribAndRep(x->f_harmonicsValues);

    jbox_invalidate_layer((t_object *)x, NULL, gensym("rotation_layer"));
    jbox_invalidate_layer((t_object *)x, NULL, gensym("harmonics_layer"));
    jbox_invalidate_layer((t_object *)x, NULL, gensym("microphones_points_layer"));
    jbox_redraw((t_jbox *)x);
    
    space_output(x);
}

void space_output(t_space *x)
{
    outlet_int(x->f_outInfos, x->f_number_of_microphones);
    
    for(int i = 0; i < x->f_number_of_microphones; i++)
        atom_setfloat(x->f_tempory_values+i, x->f_microphonesValues[i]);
    
    outlet_list(x->f_out, 0L, x->f_number_of_microphones, x->f_tempory_values);
}

t_max_err number_of_microphones_set(t_space *x, t_object *attr, long argc, t_atom *argv)
{
    if (argc && argv)
    {
        if(atom_gettype(argv) == A_LONG)
        {
            if(atom_getlong(argv) != x->f_number_of_microphones)
            {
				
                delete x->f_viewer;
                delete x->f_recomposer;
                
                x->f_number_of_microphones  = Tools::clip(long(atom_getlong(argv)), (long)3, (long)MAX_SPEAKER);
				
                if(x->f_number_of_microphones % 2 == 0)
                    x->f_order              = (x->f_number_of_microphones - 2) / 2;
                else
                    x->f_order              = (x->f_number_of_microphones - 1) / 2;
                x->f_number_of_harmonics    = x->f_order * 2 + 1;
               
                x->f_viewer         = new AmbisonicViewer(x->f_order);
                x->f_recomposer		= new AmbisonicRecomposer(x->f_order, x->f_number_of_microphones, Hoa_Fixe);
                
                jbox_invalidate_layer((t_object*)x, NULL, gensym("background_layer"));
                space_compute(x);
            }
        }
    }
	return 0;
}

t_max_err coefficients_set(t_space *x, t_object *attr, long ac, t_atom *av)
{
    if (ac && av)
    {
        if(atom_gettype(av) == A_FLOAT)
        {
            if(x->f_mode == 0 || x->f_mode == 2)
            {
                for (int i = 0; i < ac && i < MAX_SPEAKER; i++)
                {
                    if(atom_gettype(av+i) == A_FLOAT)
                        x->f_microphonesValues[i] = Tools::clip((double)atom_getfloat(av + i), 0., 1.);
                }
                
            }
            else
            {
                for (int i = 0; i < ac && i < MAX_SPEAKER; i++)
                {   
                    x->f_microphonesValues[i] = atom_getfloat(av + i);
                }
            }
            
        }
        else if (atom_gettype(av) == A_LONG && ac >= 2)
        {
            if(atom_gettype(av+1) == A_FLOAT && atom_getlong(av) < x->f_number_of_microphones)
                x->f_microphonesValues[atom_getlong(av)] = Tools::clip((double)atom_getfloat(av+1), 0., 1.);
        }
        
        object_notify(x, gensym("modified"), NULL);
    }
    
    space_compute(x);
    
    return MAX_ERR_NONE;
}


