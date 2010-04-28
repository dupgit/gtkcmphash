/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   pref_window.c
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
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "gtkcmphash.h"

static void close_pref_window(GtkWidget *widget, gpointer data);
static void toggle_include_dir(GtkWidget *widget, gpointer data);
static void connect_signaux_pref_window(main_struct_t *main_struct);


/**
 *  Affiche ou cache la fenêtre des préférences (modale)
 */
static void show_hide_pref_window(main_struct_t *main_struct, gboolean show)
{
    if (show)
        {
            gtk_widget_show(GTK_WIDGET(glade_xml_get_widget(main_struct->xml, "pref_window")));
        }
    else
        {
            gtk_widget_hide(GTK_WIDGET(glade_xml_get_widget(main_struct->xml, "pref_window")));
        }
}


/**
 *  Pour afficher la fenêtre des préférences
 */
void show_pref_window(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;

    show_hide_pref_window(main_struct, TRUE);
}


/**
 *  Lorsqu'on clique sur le bouton "Fermer" de la fenêtre des préférences
 */
static void close_pref_window(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;

    show_hide_pref_window(main_struct, FALSE);
}


/**
 *  Lorsque l'on change l'option d'inclusion du chemin absolu
 */
static void toggle_include_dir(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkToggleButton *cb_include_dir = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_include_dir"));

    main_struct->opts->include_dir = gtk_toggle_button_get_active(cb_include_dir);
}


/**
 *  Pour inclure le nom du hashset dans les résultats des fihciers connus
 */
static void toggle_include_hashset_name(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_include_hashset_name"));

    main_struct->opts->include_hashset_name = gtk_toggle_button_get_active(button);
}


/**
 *  Pour inclure le nom du fichier ayant le même hash que celui recherché
 */
static void toggle_include_hashset_file_filename(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_include_hashset_file_filename"));

    main_struct->opts->include_hashset_file_filename = gtk_toggle_button_get_active(button);
}


/**
 *  Pour générer, ou non, les hashs des fichiers vides (de taille nulle)
 *  Par défaut on ne génère pas
 */
static void toggle_genere_hashs_vides(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_genere_hashs_vides"));

    main_struct->opts->genere_hashs_vides = gtk_toggle_button_get_active(button);
}


/**
 *  Chargement, ou non, des hashs issus de fichiers vides (de taille nulle)
 *  lors du chargement des hashsets
 */
static void toggle_charger_fv_hashsets(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_charger_fv_hashsets"));

    main_struct->opts->charger_fv_hashsets = gtk_toggle_button_get_active(button);
}


/**
 *  Changement des résultats de comparaison
 */
static void toggle_all_known(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkToggleButton *cb_all_known = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_all_known"));

    /*
      On doit supprimer les listes de comparaison, vu que la méthode est
      différente. Pas besoin de tests sur la liste elle même vu que c'est
      réalisé dans la fonction free_file_hash_list
    */
    free_dedans_ou_pas(main_struct);

    main_struct->opts->all_known = gtk_toggle_button_get_active(cb_all_known);
}


/**
 *  Option de changement du nombre d'indirections dans la structure
 */
static void value_changed_sp_nb_indirections(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkSpinButton *sp_nb_indirection = GTK_SPIN_BUTTON(glade_xml_get_widget(main_struct->xml, "sp_nb_indirections"));
    gint value = 0;

    value = gtk_spin_button_get_value_as_int(sp_nb_indirection);

    if (value >= GCH_NB_INDIRECT_MIN && value <= GCH_NB_INDIRECT_MAX)
        {
            main_struct->opts->nb_indirections = value;
        }
    else
        {
            main_struct->opts->nb_indirections = GCH_NB_INDIRECT;
        }
}


/**
 *  Option de changement du hash qui sert à la comparaison
 *  Cette fonction est appelée au moins 2 fois par clicks !
 *  Il doit y avoir une autre façon de connecter le signal
 *  adéquat...
 */
static void rb_hash_toggled(GtkWidget *widget, gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GtkRadioButton *rb = GTK_RADIO_BUTTON(widget);
    GtkWidget *activated = NULL;
    const gchar *widget_name = NULL;

    activated = ldt_gtk_radio_button_get_active_from_widget(rb);
    widget_name = gtk_widget_get_name(activated);

    if (g_ascii_strcasecmp(widget_name, "rb_sha1") == 0)
        {
            main_struct->opts->hash_type = GCH_HASH_SHA1;
        }
    else if (g_ascii_strcasecmp(widget_name, "rb_ripemd160") == 0)
        {
            main_struct->opts->hash_type = GCH_HASH_RIPEMD160;
        }
    else  /* cas par défaut */
        {
            main_struct->opts->hash_type = GCH_HASH_MD5;
        }
}


/**
 *  Connexion des signaux correspondant à la fenêtre des préférences
 */
static void connect_signaux_pref_window(main_struct_t *main_struct)
{
    /* Bouton Fermer */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "pw_quit")), "clicked",
                     G_CALLBACK(close_pref_window), main_struct);

    /* Option d'inclusion des répertoires */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "cb_include_dir")), "toggled",
                     G_CALLBACK(toggle_include_dir), main_struct);

    /* Option d'inclusion de tous les hashset_name dans lesquel apparait le fichier (forcément connu) */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "cb_all_known")), "toggled",
                     G_CALLBACK(toggle_all_known), main_struct);

    /* Option d'inclusion du nom du hashset pour les fichiers connus */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "cb_include_hashset_name")), "toggled",
                     G_CALLBACK(toggle_include_hashset_name), main_struct);

    /* Option d'inclusion du nom du fichier (celui de l'ensemble des hashsets) pour lequel le hash est identique */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "cb_include_hashset_file_filename")), "toggled",
                     G_CALLBACK(toggle_include_hashset_file_filename), main_struct);

    /* Option pour hasher les fihciers vides (de taille nulle) */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "cb_genere_hashs_vides")), "toggled",
                     G_CALLBACK(toggle_genere_hashs_vides), main_struct);

    /* Option de chargement des hashs issus de fichiers vides */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "cb_charger_fv_hashsets")), "toggled",
                     G_CALLBACK(toggle_charger_fv_hashsets), main_struct);

    /* Option de changement du nombre d'indirections dans la structure */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "sp_nb_indirections")), "value-changed",
                     G_CALLBACK(value_changed_sp_nb_indirections), main_struct);

    /* Option de changement du hash */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "rb_md5")), "toggled",
                     G_CALLBACK(rb_hash_toggled), main_struct);

    /* Option de changement du hash */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "rb_sha1")), "toggled",
                     G_CALLBACK(rb_hash_toggled), main_struct);

    /* Option de changement du hash */
    g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "rb_ripemd160")), "toggled",
                     G_CALLBACK(rb_hash_toggled), main_struct);
}


/**
 *  Initialise la fenêtre de préférences avec les valeurs par défaut
 *  Connecte les signaux
 */
void init_pref_window_with_defaults(main_struct_t *main_struct)
{
    GtkToggleButton *button = NULL;

    button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_include_dir"));
    gtk_toggle_button_set_active(button, main_struct->opts->include_dir);

    button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_all_known"));
    gtk_toggle_button_set_active(button, main_struct->opts->all_known);

    button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_include_hashset_name"));
    gtk_toggle_button_set_active(button, main_struct->opts->include_hashset_name);

    button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_include_hashset_file_filename"));
    gtk_toggle_button_set_active(button, main_struct->opts->include_hashset_file_filename);

    button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_genere_hashs_vides"));
    gtk_toggle_button_set_active(button, main_struct->opts->genere_hashs_vides);

    button = GTK_TOGGLE_BUTTON(glade_xml_get_widget(main_struct->xml, "cb_charger_fv_hashsets"));
    gtk_toggle_button_set_active(button, main_struct->opts->charger_fv_hashsets);

    connect_signaux_pref_window(main_struct);
}




