/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   log_window.c
   Projet GtkCmpHash

   (C) Copyright 2007 - 2008 Olivier Delhomme
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
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "gtkcmphash.h"

void show_hide_log_window(main_struct_t *main_struct, gboolean show)
{

      if (show)
        {
            gtk_widget_show(GTK_WIDGET(glade_xml_get_widget(main_struct->xml, "log_dialog")));
        }
      else
        {
            gtk_widget_hide(GTK_WIDGET(glade_xml_get_widget(main_struct->xml, "log_dialog")));
        }
}


/**
 *  Lorsqu'on clique sur le bouton "Quitter" de la fenêtre de log
 */
static void quit_log_window(GtkWidget *widget, gpointer data)
{
      main_struct_t *main_struct = (main_struct_t *) data;
      GtkCheckMenuItem *cmi = GTK_CHECK_MENU_ITEM(GTK_WIDGET(glade_xml_get_widget(main_struct->xml, "menu_log")));

      gtk_check_menu_item_set_active(cmi, FALSE);
      show_hide_log_window(main_struct, gtk_check_menu_item_get_active(cmi));
}


/**
 *  Connexion des signaux correspondant à la fenêtre de log
 */
static void connect_log_window_signals(main_struct_t *main_struct)
{
      /* Bouton Quitter */
      g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "lw_quit")), "clicked",
                        G_CALLBACK(quit_log_window), main_struct);
}


/**
 *  Initialisation du système de log
 */
void init_log_window(main_struct_t *main_struct)
{
      GtkTextView *textview = GTK_TEXT_VIEW(glade_xml_get_widget(main_struct->xml, "lw_textview"));

      /* Ajoute le textview au système de log */
      main_struct->log = ldt_init_log_domain(textview, ProgName, PROGRAM_DEBUG);

      connect_log_window_signals(main_struct);
}
