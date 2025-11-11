/* petrock-window.c
 *
 * Copyright 2025 Luca Murdoch
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include <cairo.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <math.h>

#include "petrock-window.h"

struct _PetrockWindow
{
	AdwApplicationWindow  parent_instance;

	/* Template widgets */
	GtkPicture           *rock_picture;
	GtkLabel             *compliment_label;
	GtkLabel             *fact_label;
	GtkButton            *compliment_button;

	/* State */
	guint                 compliment_count;
	guint                 fact_index;
};

G_DEFINE_FINAL_TYPE (PetrockWindow, petrock_window, ADW_TYPE_APPLICATION_WINDOW)

static const char *rock_facts[] = {
	N_("Fun fact: Pet rocks were the original low-maintenance companions."),
	N_("Fun fact: This rock's favorite hobby is sedimentary contemplation."),
	N_("Fun fact: A pet rock never rolls away unless you help it."),
	N_("Fun fact: Rocks make great listeners; sediment keeps secrets."),
	N_("Fun fact: Pebbles appreciate compliments even if they do not show it.")
};

static void
petrock_window_update_counter_label (PetrockWindow *self)
{
	g_autofree char *label = NULL;

	label = g_strdup_printf (_("Compliments received: %u"), self->compliment_count);
	gtk_label_set_text (self->compliment_label, label);
}

static void
petrock_window_show_fact (PetrockWindow *self,
                          guint          index)
{
	if (G_N_ELEMENTS (rock_facts) == 0)
		return;

	self->fact_index = index % G_N_ELEMENTS (rock_facts);
	gtk_label_set_text (self->fact_label, _(rock_facts[self->fact_index]));
}

static void
petrock_window_advance_fact (PetrockWindow *self)
{
	if (G_N_ELEMENTS (rock_facts) == 0)
		return;

	petrock_window_show_fact (self, self->fact_index + 1);
}

static cairo_t *
petrock_window_begin_surface (int width,
                              int height,
                              cairo_surface_t **surface_out)
{
	cairo_surface_t *surface = NULL;
	cairo_t *cr = NULL;

	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	cr = cairo_create (surface);

	*surface_out = surface;
	return cr;
}

static double
petrock_window_get_mood_ratio (PetrockWindow *self)
{
	double ratio = 0.0;

	if (self->compliment_count == 0)
		return 0.0;

	ratio = (double) self->compliment_count / 12.0;
	return CLAMP (ratio, 0.0, 1.0);
}

static void
petrock_window_draw_rock (cairo_t       *cr,
                          int            width,
                          int            height,
                          double         mood_ratio)
{
	const double padding = 24.0;
	const double rock_width = MAX (width - padding * 2.0, 20.0);
	const double rock_height = MAX (height * 0.45, 20.0);
	const double rock_center_x = width / 2.0;
	const double rock_center_y = height * 0.55;
	const double shadow_width = rock_width * 0.8;
	const double shadow_height = rock_height * 0.4;
	const double shadow_center_y = height * 0.78;
	const double sparkle_radius = 6.0;
	double base_r = 0.45 + 0.2 * mood_ratio;
	double base_g = 0.44 + 0.15 * mood_ratio;
	double base_b = 0.48 + 0.1 * mood_ratio;
	double glow_strength = 0.12 + 0.25 * mood_ratio;
	cairo_pattern_t *sky = NULL;
	cairo_pattern_t *glow = NULL;

	/* Background gradient */
	sky = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgb (sky, 0.0, 0.92, 0.95, 0.99);
	cairo_pattern_add_color_stop_rgb (sky, 1.0, 0.94 - 0.1 * mood_ratio, 0.93, 0.96);
	cairo_set_source (cr, sky);
	cairo_paint (cr);
	cairo_pattern_destroy (sky);

	/* Ground shadow */
	cairo_save (cr);
	cairo_translate (cr, rock_center_x, shadow_center_y);
	cairo_scale (cr, shadow_width / 2.0, shadow_height / 2.0);
	cairo_arc (cr, 0, 0, 1.0, 0, 2 * G_PI);
	cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 0.15);
	cairo_fill (cr);
	cairo_restore (cr);

	/* Rock body */
	/* Rock body - slightly lopsided blob */
	cairo_save (cr);
	cairo_translate (cr, rock_center_x, rock_center_y);
	cairo_scale (cr, rock_width / 2.0, rock_height / 2.0);
	cairo_move_to (cr, 0.0, -1.0);
	cairo_curve_to (cr,
	                0.65, -0.9,
	                1.0, -0.2,
	                0.9, 0.5);
	cairo_curve_to (cr,
	                0.8, 1.0,
	                0.1, 1.1,
	                -0.2, 1.0);
	cairo_curve_to (cr,
	                -0.9, 0.9,
	                -1.1, 0.2,
	                -0.8, -0.6);
	cairo_curve_to (cr,
	                -0.6, -1.0,
	                -0.1, -1.1,
	                0.0, -1.0);
	cairo_close_path (cr);
	cairo_restore (cr);

	cairo_set_source_rgba (cr, base_r, base_g, base_b, 1.0);
	cairo_fill_preserve (cr);

	cairo_set_source_rgba (cr, base_r - 0.17, base_g - 0.18, base_b - 0.14, 1.0);
	cairo_set_line_width (cr, 2.0);
	cairo_stroke (cr);

	/* Glow rim */
	glow = cairo_pattern_create_radial (rock_center_x - rock_width * 0.2,
	                                    rock_center_y - rock_height * 0.4,
	                                    rock_width * 0.05,
	                                    rock_center_x,
	                                    rock_center_y,
	                                    rock_width * 0.9);
	cairo_pattern_add_color_stop_rgba (glow, 0.0, 1.0, 1.0, 1.0, glow_strength);
	cairo_pattern_add_color_stop_rgba (glow, 1.0, 1.0, 1.0, 1.0, 0.0);
	cairo_set_source (cr, glow);
	cairo_save (cr);
	cairo_translate (cr, rock_center_x, rock_center_y);
	cairo_scale (cr, rock_width / 2.0, rock_height / 2.0);
	cairo_arc (cr, 0, 0, 1.0, 0, 2 * G_PI);
	cairo_restore (cr);
	cairo_set_line_width (cr, 4.0);
	cairo_stroke (cr);
	cairo_pattern_destroy (glow);

	/* Facets */
	cairo_set_source_rgba (cr, base_r - 0.08, base_g - 0.08, base_b - 0.05, 0.6);
	cairo_set_line_width (cr, 2.0);
	cairo_move_to (cr, rock_center_x - rock_width * 0.35, rock_center_y - rock_height * 0.1);
	cairo_line_to (cr, rock_center_x - rock_width * 0.2, rock_center_y - rock_height * 0.3);
	cairo_line_to (cr, rock_center_x, rock_center_y - rock_height * 0.2);
	cairo_move_to (cr, rock_center_x + rock_width * 0.3, rock_center_y - rock_height * 0.05);
	cairo_line_to (cr, rock_center_x + rock_width * 0.15, rock_center_y + rock_height * 0.15);
	cairo_line_to (cr, rock_center_x - rock_width * 0.05, rock_center_y + rock_height * 0.08);
	cairo_stroke (cr);

	/* Highlight streak */
	cairo_save (cr);
	cairo_translate (cr, rock_center_x - rock_width * 0.18, rock_center_y - rock_height * 0.2);
	cairo_scale (cr, rock_width * 0.12, rock_height * 0.22);
	cairo_arc (cr, 0, 0, 1.0, -G_PI / 4, G_PI / 2);
	cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.2 + 0.2 * mood_ratio);
	cairo_set_line_width (cr, 3.0);
	cairo_stroke (cr);
	cairo_restore (cr);

	/* Sparkle buddy */
	cairo_save (cr);
	cairo_translate (cr,
	                 rock_center_x + rock_width * 0.35,
	                 rock_center_y - rock_height * 0.55);
	cairo_rotate (cr, G_PI / 6);
	cairo_move_to (cr, 0, -sparkle_radius);
	cairo_line_to (cr, 0, sparkle_radius);
	cairo_move_to (cr, -sparkle_radius, 0);
	cairo_line_to (cr, sparkle_radius, 0);
	cairo_move_to (cr, -sparkle_radius * 0.7, -sparkle_radius * 0.7);
	cairo_line_to (cr, sparkle_radius * 0.7, sparkle_radius * 0.7);
	cairo_move_to (cr, sparkle_radius * 0.7, -sparkle_radius * 0.7);
	cairo_line_to (cr, -sparkle_radius * 0.7, sparkle_radius * 0.7);
	cairo_set_source_rgba (cr, 1.0, 0.95, 0.75, 0.6 + 0.3 * mood_ratio);
	cairo_set_line_width (cr, 1.6);
	cairo_stroke (cr);
	cairo_restore (cr);

	/* Pet tag pebble */
	cairo_save (cr);
	cairo_arc (cr,
	           rock_center_x + rock_width * 0.25,
	           rock_center_y + rock_height * 0.45,
	           rock_width * 0.08,
	           0,
	           2 * G_PI);
	cairo_set_source_rgba (cr, 0.9, 0.78, 0.32, 0.9);
	cairo_fill (cr);
	cairo_set_source_rgba (cr, 0.6, 0.4, 0.1, 1.0);
	cairo_set_line_width (cr, 1.2);
	cairo_arc (cr,
	           rock_center_x + rock_width * 0.25,
	           rock_center_y + rock_height * 0.45,
	           rock_width * 0.08,
	           0,
	           2 * G_PI);
	cairo_stroke (cr);
	cairo_restore (cr);
}

static GdkTexture *
petrock_window_create_rock_texture (int    width,
                                    int    height,
                                    double mood_ratio)
{
	cairo_surface_t *surface = NULL;
	cairo_t *cr = NULL;
	GdkTexture *texture = NULL;
	GBytes *bytes = NULL;
	guchar *copy = NULL;
	gsize stride = 0;
	gsize data_size = 0;
	unsigned char *surface_data = NULL;

	cr = petrock_window_begin_surface (width, height, &surface);
	petrock_window_draw_rock (cr, width, height, mood_ratio);

	cairo_destroy (cr);

	surface_data = cairo_image_surface_get_data (surface);
	stride = cairo_image_surface_get_stride (surface);
	data_size = stride * height;
	copy = g_memdup2 (surface_data, data_size);
	bytes = g_bytes_new_take (copy, data_size);

	texture = gdk_memory_texture_new (width,
	                                  height,
	                                  GDK_MEMORY_DEFAULT,
	                                  bytes,
	                                  stride);

	g_bytes_unref (bytes);
	cairo_surface_destroy (surface);

	return texture;
}

static void
petrock_window_update_paintable (PetrockWindow *self,
                                 int            width,
                                 int            height)
{
	int target_width = width;
	int target_height = height;
	double mood_ratio = 0.0;
	GdkTexture *texture = NULL;

	if (target_width <= 0)
		target_width = MAX (gtk_widget_get_width (GTK_WIDGET (self->rock_picture)), 220);

	if (target_height <= 0)
		target_height = MAX (gtk_widget_get_height (GTK_WIDGET (self->rock_picture)), 220);

	target_width = MAX (target_width, 220);
	target_height = MAX (target_height, 220);

	mood_ratio = petrock_window_get_mood_ratio (self);
	texture = petrock_window_create_rock_texture (target_width, target_height, mood_ratio);
	gtk_picture_set_paintable (self->rock_picture, GDK_PAINTABLE (texture));
	g_object_unref (texture);
}

static void
petrock_window_on_picture_size_allocate (GtkWidget *widget,
                                         int        width,
                                         int        height,
                                         int        baseline,
                                         gpointer   user_data)
{
	PetrockWindow *self = PETROCK_WINDOW (user_data);

	petrock_window_update_paintable (self, width, height);
}

static void
petrock_window_on_compliment_clicked (GtkButton *button,
                                      gpointer   user_data)
{
	PetrockWindow *self = PETROCK_WINDOW (user_data);

	self->compliment_count++;
	petrock_window_update_counter_label (self);
	petrock_window_advance_fact (self);
	petrock_window_update_paintable (self, -1, -1);
}

static void
petrock_window_class_init (PetrockWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lucamurdoch/PetRock/petrock-window.ui");
	gtk_widget_class_bind_template_child (widget_class, PetrockWindow, rock_picture);
	gtk_widget_class_bind_template_child (widget_class, PetrockWindow, compliment_label);
	gtk_widget_class_bind_template_child (widget_class, PetrockWindow, fact_label);
	gtk_widget_class_bind_template_child (widget_class, PetrockWindow, compliment_button);
}

static void
petrock_window_init (PetrockWindow *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));

	self->compliment_count = 0;
	self->fact_index = 0;

	g_signal_connect (self->compliment_button,
	                  "clicked",
	                  G_CALLBACK (petrock_window_on_compliment_clicked),
	                  self);

	g_signal_connect (self->rock_picture,
	                  "size-allocate",
	                  G_CALLBACK (petrock_window_on_picture_size_allocate),
	                  self);

	petrock_window_update_counter_label (self);
	petrock_window_show_fact (self, 0);
	petrock_window_update_paintable (self, -1, -1);
}
