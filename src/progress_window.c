/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   progress_window.c
   Projet GtkCmpHash

   (C) Copyright 2007 Olivier Delhomme
   e-mail : olivierdelhomme@gmail.com
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.           
*/

#include "gtkcmphash.h"


static p_bar_t *set_progress_bar_values(p_bar_t *pb, GtkProgressBar *pb_global, GtkProgressBar *pb_file, GtkLabel *pb_label, guint64 max, guint64 value, guint64 max_file, guint64 value_file);
static p_bar_t *new_progress_bar_struct(GtkProgressBar *pb_global, GtkProgressBar *pb_file, GtkLabel *pb_label);
static gboolean delete_pb_window_event(GtkWidget *widget, GdkEvent  *event, gpointer data );
static void destroy_pb_window(GtkWidget *widget, gpointer data);


static p_bar_t *set_progress_bar_values(p_bar_t *pb, GtkProgressBar *pb_global, GtkProgressBar *pb_file, GtkLabel *pb_label, guint64 max, guint64 value, guint64 max_file, guint64 value_file)
{
	pb->max = max;
	pb->value = value;
	pb->max_file = max_file;
	pb->value_file = value_file;
	pb->pb_global = pb_global;
	pb->pb_file = pb_file;
	pb->pb_label = pb_label;

	return pb;
}

/**
 *  Créer la structure pour la barre de progression
 *  Détruite dans la fin de la fenêtre
 */
static p_bar_t *new_progress_bar_struct(GtkProgressBar *pb_global, GtkProgressBar *pb_file, GtkLabel *pb_label)
{
	p_bar_t *pb = NULL;

	pb = (p_bar_t *) g_malloc0(sizeof(p_bar_t));

	set_progress_bar_values(pb, pb_global, pb_file, pb_label, 0, 0, 0, 0);
  
	return pb;
}


/**
 *  Gère l'affichage de la barre de progression
 */
void refresh_progress_bar(p_bar_t *pb)
{
	gdouble fraction = 0.0;
	gchar *label = NULL;

	if (pb->max != 0)
		{
			fraction = (gdouble) pb->value / (gdouble) pb->max;
			if (fraction >=0 && fraction <=1)
				{
					gtk_progress_bar_set_fraction(pb->pb_global, fraction);
					g_main_context_iteration(NULL, FALSE);
				}

			label = g_strdup_printf("%Ld / %Ld", pb->value, pb->max);
			gtk_label_set_text(pb->pb_label, label);
			g_main_context_iteration(NULL, FALSE);
			g_free(label);
		}
	else
		{
			gtk_progress_bar_set_fraction(pb->pb_global, 0.0);
			g_main_context_iteration(NULL, FALSE);
		}
}

/**
 *  Ne rafraîchit que la barre de progression secondaire
 */
void refresh_file_progress_bar(p_bar_t *pb)
{
	gdouble fraction = 0.0;

	if (pb->max_file != 0)
		{
			fraction = (gdouble) pb->value_file / (gdouble) pb->max_file;
			if (fraction >=0 && fraction <=1)
				{
					gtk_progress_bar_set_fraction(pb->pb_file, fraction);
					g_main_context_iteration(NULL, FALSE);
				}
		}
	else
		{
			gtk_progress_bar_set_fraction(pb->pb_file, 0.0);
			g_main_context_iteration(NULL, FALSE);
		}
}



/**
 *  Fonction permettant de faire aller et venir la barre de
 *  progression (car on ne connait pas à l'avance le nombre
 *  de choses a réaliser
 */
void pulse_the_progress_bar(p_bar_t *pb)
{
	gchar *label = NULL;

	label = g_strdup_printf("%Ld / %Ld", pb->max, pb->max);
	gtk_label_set_text(pb->pb_label, label);
	g_free(label);

	gtk_progress_bar_pulse(pb->pb_global);
	g_main_context_iteration(NULL, FALSE);
}

/**
 *  Initialise la barre de progression
 *  Si max = 0 alors en est en mode pulse
 *  sinon, en mode normal
 */
p_bar_t *init_progress_bar(p_bar_t *pb, guint64 max, guint64 max_file)
{
	gdouble fraction = 0.0;
	gchar *label = NULL;

	pb = set_progress_bar_values(pb, pb->pb_global, pb->pb_file, pb->pb_label, max, 0, max_file, 0);

	if (pb->max != 0)
		{
			fraction = (gdouble) pb->value / (gdouble) pb->max;
			if (fraction >=0 && fraction <=1)
				gtk_progress_bar_set_fraction(pb->pb_global, fraction);

			label = g_strdup_printf("%Ld / %Ld", pb->value, pb->max);
			gtk_label_set_text(pb->pb_label, label);
			g_free(label);
		}
	else
		{
			gtk_label_set_text(pb->pb_label, "? / ?");
			gtk_progress_bar_set_pulse_step(pb->pb_global, 0.01);
			gtk_progress_bar_set_fraction(pb->pb_file, 0.0);
		}

	return pb;
}

/** 
 *  fermeture de la fenêtre de la barre de progression : on ne veux pas !
 */
static gboolean delete_pb_window_event(GtkWidget *widget, GdkEvent  *event, gpointer data )
{
  
	return TRUE;
}

/** 
 *  destruction de la fenêtre principale : on ne veux pas !
 */
static void destroy_pb_window(GtkWidget *widget, gpointer data)
{
	/* ne fait rien ! */
}


/**
 *  initialise la fenêtre de la barre de progression et gtk+
 */
void init_progress_window(main_struct_t *main_struct)
{
	GtkWidget *pbw = glade_xml_get_widget(main_struct->xml, "progress_window");
	GtkProgressBar *pb_global = GTK_PROGRESS_BAR(glade_xml_get_widget(main_struct->xml, "progress_bar"));
	GtkProgressBar *pb_file = GTK_PROGRESS_BAR(glade_xml_get_widget(main_struct->xml, "file_progress_bar"));
	GtkLabel *pb_label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "progress_label"));

	main_struct->pb = new_progress_bar_struct(pb_global, pb_file, pb_label);

	/* Supression de la fenêtre */
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "progress_window")), "delete-event", 
					 G_CALLBACK(delete_pb_window_event), NULL);

	/* Destruction de la fenêtre */
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "progress_window")), "destroy", 
					 G_CALLBACK(destroy_pb_window), main_struct);


	gtk_widget_show_all(pbw);
}

/**
 *  Remet la fenêtre de la barre de progression et gtk+
 *  dans un état "normal"
 */
void end_progress_window(main_struct_t *main_struct)
{

	GtkWidget *pbw = glade_xml_get_widget(main_struct->xml, "progress_window");

	gtk_widget_hide(pbw);

	g_free(main_struct->pb);
	main_struct->pb = NULL;
}
