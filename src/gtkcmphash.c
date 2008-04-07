/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   gtkcmphash.c
   Projet gtkcmphash

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

/* fonctions utilitaires */
static void log_comparison_type(main_struct_t *main_struct);

/* les menus */
static void menu_quit(GtkWidget *widget, gpointer data);
static void menu_log(GtkWidget *widget, gpointer data);
static void menu_log(GtkWidget *widget, gpointer data);
static void menu_a_propos( GtkWidget *widget, gpointer data );
static void menu_save_hashs(GtkWidget *widget, gpointer data);
static void menu_hash_a_dir(GtkWidget *widget, gpointer data);
static void menu_load_hashsets(GtkWidget *widget, gpointer data);
static void menu_not_in_hashsets(GtkWidget *widget, gpointer data);
static void menu_in_hashsets(GtkWidget *widget, gpointer data);
static void menu_load_a_hashset(GtkWidget *widget, gpointer data);
static void menu_empty_hashsets(GtkWidget *widget, gpointer data);
static void menu_display_stats(GtkWidget *widget, gpointer data);

/* Gestion des menus */
static void make_sensitive_hashs_related(main_struct_t *main_struct);
static void make_sensitive_comparison_related(main_struct_t *main_struct);
static void make_sensitive_hashsets_related(main_struct_t *main_struct);

/* la boite de dialogue a propos */
static void a_propos_response(GtkWidget *widget, gint response, gpointer data);

/* la fen�tre principale */
static gboolean delete_main_window_event(GtkWidget *widget, GdkEvent  *event, gpointer data );
static void destroy_main_window(GtkWidget *widget, gpointer data);

/* connection des signaux */
static void connect_signaux_menus(main_struct_t *main_struct);
static void connect_signaux_about_box(main_struct_t *main_struct);
static void connect_signaux(main_struct_t *main_struct);

/* chargement de l'interface et configuration par d�faut */
static gboolean load_xml_interface(main_struct_t *main_struct);
static void set_default_gui_values(main_struct_t *main_struct);

/* initialisations */
static void init_openssl(main_struct_t *main_struct);
static void init_international_languages(void);
static main_struct_t *init_main_struct(void);
static void init_interface(main_struct_t *main_struct);
static void fin_du_programme(main_struct_t *main_struct);

/* fonctions non directement reli�es � l'interface */
static void save_the_comparison_results(main_struct_t *main_struct, gboolean in_hashsets);
static void enregistre_les_hashs(main_struct_t *main_struct);

/* Fonctions qui pourraient �tre int�gr�es � liblats */
static gchar *select_a_folder(GtkWidget *parent);
static gchar *select_a_file_to_save(GtkWidget *parent);
static gchar *select_a_file_to_load(GtkWidget *parent);

/**
 *  Fonction de s�lection d'un dossier
 */
static gchar *select_a_folder(GtkWidget *parent)
{
	GtkFileChooser *fcd = NULL;
	gchar *filename = NULL; 

	/* S�lection d'un dossier */
	fcd = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(_("Ouvrir un dossier ..."),
													   GTK_WINDOW(parent),
													   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
													   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													   GTK_STOCK_OPEN, GTK_RESPONSE_OK, 
													   NULL));

	/* Propri�t�s de la fen�tre : pr�emptive, sans multis�lections */
	gtk_window_set_modal(GTK_WINDOW(fcd), TRUE);
	gtk_file_chooser_set_select_multiple(fcd, FALSE);
   
	switch(gtk_dialog_run(GTK_DIALOG(fcd)))
		{
		case GTK_RESPONSE_OK:
			/* R�cup�ration du chemin */
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
			break;
		default:
			filename = NULL;
			break;
		}

	gtk_widget_destroy(GTK_WIDGET(fcd));

	return filename;
}

/**
 *  Permet la s�lection, par l'utilisateur d'un nom de fichier
 *  pour la sauvegarde
 */
static gchar *select_a_file_to_save(GtkWidget *parent)
{
	GtkFileChooser *fcd = NULL;
	gchar *filename = NULL; 

	/* S�lection d'un nom de fichier � enregistrer */
	fcd = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(_("Enregistrer sous ..."),
													   GTK_WINDOW(parent),
													   GTK_FILE_CHOOSER_ACTION_SAVE, 
													   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													   GTK_STOCK_SAVE, GTK_RESPONSE_OK, 
													   NULL));
 
	/* Propri�t�s de la fen�tre : pr�emptive, sans multis�lections, avec confirmation */
	gtk_window_set_modal(GTK_WINDOW(fcd), TRUE);
	gtk_file_chooser_set_select_multiple(fcd, FALSE);
	gtk_file_chooser_set_do_overwrite_confirmation(fcd, TRUE);

	switch(gtk_dialog_run(GTK_DIALOG(fcd)))
		{
		case GTK_RESPONSE_OK:
			/* R�cup�ration du chemin */
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
			break;
		default:
			filename = NULL;
			break;
		}

	gtk_widget_destroy(GTK_WIDGET(fcd));

	return filename;
}


/**
 *  Permet la s�lection, par l'utilisateur d'un nom de fichier
 *  pour la sauvegarde
 */
static gchar *select_a_file_to_load(GtkWidget *parent)
{
	GtkFileChooser *fcd = NULL;
	gchar *filename = NULL; 

	/* S�lection d'un nom de fichier � charger */
	fcd = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(_("Ouvrir un fichier ..."),
													   GTK_WINDOW(parent),
													   GTK_FILE_CHOOSER_ACTION_OPEN, 
													   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													   GTK_STOCK_OPEN, GTK_RESPONSE_OK, 
													   NULL));
 
	/* Propri�t�s de la fen�tre : pr�emptive, sans multis�lections */
	gtk_window_set_modal(GTK_WINDOW(fcd), TRUE);
	gtk_file_chooser_set_select_multiple(fcd, FALSE);

	switch(gtk_dialog_run(GTK_DIALOG(fcd)))
		{
		case GTK_RESPONSE_OK:
			/* R�cup�ration du chemin */
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
			break;
		default:
			filename = NULL;
			break;
		}

	gtk_widget_destroy(GTK_WIDGET(fcd));

	return filename;
}

/**
 * Gestion de l'affichage des menus reli�s aux hashs
 */
static void make_sensitive_hashs_related(main_struct_t *main_struct)
{
	/* on peut sauvegarder et/ou effacer la liste seulement s'il y a quelque chose dedans ! */
	if (main_struct->file_hash_list != NULL)
		{
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_save_hashs"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_empty_hash_list"), TRUE);
		}
	else
		{
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_save_hashs"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_empty_hash_list"), FALSE);
		}
}

/**
 *  Si l'arbre est plein (on a charg� des hashsets) et la liste 
 *  des fichiers hach�s est non vide (on a charg� un hashset ou 
 *  hach� un r�pertoire) alors on peut faire les comparaisons.
 */
static void make_sensitive_comparison_related(main_struct_t *main_struct)
{
	if (main_struct->tronc != NULL && main_struct->file_hash_list != NULL)
		{
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_do_the_comparison"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_in_hashsets"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_not_in_hashsets"), TRUE);
		}
	else
		{
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_do_the_comparison"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_in_hashsets"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_not_in_hashsets"), FALSE);
		}
}

/**
 *  Si l'arbre n'est pas charg�/rempli on ne peut pas sauvegarder ou
 *  vider l'arbre !!
 */
static void make_sensitive_hashsets_related(main_struct_t *main_struct)
{
	if (main_struct->tronc != NULL)
		{
			/* Gestion du menu de sauvegarde et d'oubli des hashsets */
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_save_hashsets"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_empty_hashsets"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_display_stats"), TRUE);

			/* d�s qu'on a charg� des hashsets, on ne doit plus pouvoir modifier la clef */
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "frame_hash_type"), FALSE); 
			/*
			  gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "rb_md5"), FALSE);
			  gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "rb_sha1"), FALSE);
			  gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "rb_ripemd160"), FALSE);
			*/
		}
	else
		{
			/* Gestion du menu de sauvegarde et d'oubli des hashsets */
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_save_hashsets"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_empty_hashsets"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_display_stats"), FALSE);

			/* Si on n'a plus de hashsets, on peut a nouveau changer la clef */
			gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "frame_hash_type"), TRUE);
			/*
			  gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "rb_md5"), TRUE);
			  gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "rb_sha1"), TRUE);
			  gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "rb_ripemd160"), TRUE);
			*/
		}
}


/**
 *  Enregistre les hash dans un fichier que l'utilisateur
 *  s�lectionne.
 */
static void enregistre_les_hashs(main_struct_t *main_struct)
{ 
	gchar *filename = NULL; 
	bzip2_result_t *res = NULL;

	filename = select_a_file_to_save(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	if (filename != NULL)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fichier � sauvegarder %s", filename);
      
			res = save_the_file_hash_list(main_struct->file_hash_list, filename, FALSE, main_struct->opts);
      
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "%d octets compress�s en %d octets dans le fichier %s (%d fichiers hach�s)", res->uncompressed, res->compressed, filename, res->nb_hash);
      

			g_free(filename);
			g_free(res);
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Pas de fichier � sauvegarder !!!");
		}
}

/** 
 *  Menu quitter
 */
static void menu_quit(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;

	fin_du_programme(main_struct);		
}

/** 
 *  Sous menu Log du menu Affichage. Affiche ou cache la fen�tre de log
 */
static void menu_log(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
	GtkCheckMenuItem *cmi = GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_struct->xml, "menu_log"));

	show_hide_log_window(main_struct, gtk_check_menu_item_get_active(cmi));
}


static void menu_a_propos( GtkWidget *widget, gpointer data )
{
	main_struct_t *main_struct = (main_struct_t *) data;
	
	gtk_widget_show(glade_xml_get_widget(main_struct->xml, "about_dialog"));
}

/**
 *  Sous menu pour sauvegarder dans un fichier les
 *  hash calcul�s
 */
static void menu_save_hashs(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;

	if (main_struct->file_hash_list != NULL)
		{
			enregistre_les_hashs(main_struct);
		}
}

/**
 *  Sous menu Hacher un r�pertoire du menu Fichier
 */
static void menu_hash_a_dir(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
	gchar *dirname = NULL;

	dirname = select_a_folder(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	if (dirname != NULL)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Dossier s�lectionn� %s", dirname);
			main_struct->file_hash_list = hash_a_directory(main_struct, dirname);
			g_free(dirname);
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Pas de dossier s�lectionn� !!!");
		}

	make_sensitive_hashs_related(main_struct);
	make_sensitive_comparison_related(main_struct);
}

/**
 *  Sous menu Charger un ensemble de hashsets
 */
static void menu_load_hashsets(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
	gchar *dirname = NULL; 

	dirname = select_a_folder(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	if (dirname != NULL)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Dossier s�lectionn� %s", dirname);
			load_a_complete_directory(main_struct, dirname);

			/* Gestion du menu de sauvegarde et d'oubli des hashsets */
			make_sensitive_hashsets_related(main_struct);

			g_free(dirname);
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Pas de dossier s�lectionn� !!!");
		}

	make_sensitive_comparison_related(main_struct);
}

/**
 *  Sous-menu enregistrer les hashsets
 */
static void menu_save_hashsets(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
	gchar *filename = NULL; 
	bzip2_result_t *res = NULL;

	filename = select_a_file_to_save(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	if (filename != NULL)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fichier � sauvegarder %s", filename);
      
			res = save_the_hashsets(main_struct, filename);
      
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "%d octets compress�s en %d octets dans le fichier %s (%d fichiers hach�s)", res->uncompressed, res->compressed, filename, res->nb_hash);
      
			g_free(filename);
			g_free(res);
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Pas de fichier � sauvegarder !!!");
		}
}


/**
 *  Sauvegarde les r�sultats des comparaisons
 *  in_hashsets indique si on doit sauvegarder 
 *    - la liste de ceux qui sont dans les hashsets (TRUE)
 *    - la liste de ceux qui ne sont PAS dans les hashsets (FALSE)
 */
static void save_the_comparison_results(main_struct_t *main_struct, gboolean in_hashsets)
{
	gchar *filename = NULL; 
	bzip2_result_t *res = NULL;

	filename = select_a_file_to_save(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	if (filename != NULL)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fichier � sauvegarder %s", filename);
      
			if (in_hashsets == TRUE)
				res = save_the_file_hash_list(main_struct->dedans_ou_pas->found, filename, TRUE, main_struct->opts);
			else
				res = save_the_file_hash_list(main_struct->dedans_ou_pas->not_found, filename, FALSE, main_struct->opts);
      
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "%d octets compress�s en %d octets dans le fichier %s (%d hashs)", res->uncompressed, res->compressed, filename, res->nb_hash);
     
			g_free(filename);
			g_free(res);
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Pas de fichier � sauvegarder !!!");
		}
}

/**
 *  Log le type de hash utilis� pour la comparaison
 */
static void log_comparison_type(main_struct_t *main_struct)
{
	if (main_struct->opts->hash_type == GCH_HASH_MD5)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Hash de comparaison : MD5");
		}
	else if (main_struct->opts->hash_type == GCH_HASH_SHA1)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Hash de comparaison : SHA1");
		}
	else if (main_struct->opts->hash_type == GCH_HASH_RIPEMD160)
		{
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Hash de comparaison : RIPEMD160");
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Hash de comparaison inconnu");
		}
}

/**
 *  Sous menu "Pas dans les hashsets" (menu comparaisons)
 */
static void menu_not_in_hashsets(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;

	if (main_struct->tronc != NULL && main_struct->file_hash_list != NULL)
		{
			if (main_struct->dedans_ou_pas->not_found == NULL)
				{	  
					log_message(main_struct->log, G_LOG_LEVEL_INFO, "D�but de la comparaison. Soyez patient !");
					log_comparison_type(main_struct);
					main_struct->dedans_ou_pas = find_all_hashes_from_hashset(main_struct->file_hash_list, main_struct->tronc, main_struct->opts);
					log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fin de la comparaison");
				}
			save_the_comparison_results(main_struct, FALSE);
		}
}


/**
 *  Sous menu "Dans les hashsets" (menu comparaisons)
 */
static void menu_in_hashsets(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
 
	if (main_struct->tronc != NULL && main_struct->file_hash_list != NULL)
		{
			if (main_struct->dedans_ou_pas->found == NULL)
				{
					log_message(main_struct->log, G_LOG_LEVEL_INFO, "D�but de la comparaison. Soyez patient !");
					log_comparison_type(main_struct);
					main_struct->dedans_ou_pas = find_all_hashes_from_hashset(main_struct->file_hash_list, main_struct->tronc, main_struct->opts);
					log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fin de la comparaison");
				}
			save_the_comparison_results(main_struct, TRUE);
		}
}

/**
 * Menu qui r�alise effectivement la comparaison (ou la recommence)
 * Si la comparaison avait d�j� �t� effectu�e, la fonction supprime
 * le r�sultat et recommence l'op�ration
 */
static void menu_do_the_comparison(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
 
	if (main_struct->tronc != NULL && main_struct->file_hash_list != NULL)
		{
			if (main_struct->dedans_ou_pas->found != NULL || main_struct->dedans_ou_pas->not_found != NULL)
				{
					free_dedans_ou_pas(main_struct); 
				}
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "D�but de la comparaison. Soyez patient !");
			log_comparison_type(main_struct);
			main_struct->dedans_ou_pas = find_all_hashes_from_hashset(main_struct->file_hash_list, main_struct->tronc, main_struct->opts);
			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fin de la comparaison");
		}
}

/**
 *  Menu qui vide la liste des hashs charg�s et aussi celles 
 *  des �ventuels r�sultats
 */
static void menu_empty_hash_list(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;

	if (main_struct->file_hash_list != NULL)
		{
			free_file_hash_list(main_struct->file_hash_list);
			main_struct->file_hash_list = NULL;

			if (main_struct->dedans_ou_pas->found  != NULL || main_struct->dedans_ou_pas->not_found != NULL)
				{
					free_dedans_ou_pas(main_struct);
				}

			make_sensitive_hashs_related(main_struct);
			make_sensitive_comparison_related(main_struct);
		}
}

/**
 *  Menu "Oublier les hashsets"
 */
static void menu_empty_hashsets(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;

	if (main_struct != NULL)
		{
			free_tronc(main_struct->tronc);
		}
  
	main_struct->tronc = NULL;
   
	if (main_struct->dedans_ou_pas->found  != NULL || main_struct->dedans_ou_pas->not_found != NULL)
		{
			free_dedans_ou_pas(main_struct);
		}

	make_sensitive_hashsets_related(main_struct);
	make_sensitive_comparison_related(main_struct);
}

/**
 *  Menu "Charger un hashset"
 */
static void menu_load_a_hashset(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data; 
	gchar *filename = NULL;
	gchar *dirname = NULL;
	bzip2_result_t *compte = NULL;
	p_bar_t *pb = NULL;
	GSList *file_hash_list = NULL;

	filename = select_a_file_to_load(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	if (filename != NULL)
		{
      
			compte = (bzip2_result_t *) g_malloc0(sizeof(bzip2_result_t));
			compte->nb_hash = 0;
			compte->compressed = 0;
			compte->uncompressed = 0;

			dirname = g_path_get_dirname(filename);

			/* Progress bar stuff */
			init_progress_window(main_struct);
			pb = init_progress_bar(main_struct->pb, 1, 0); /* normal mode */

			file_hash_list = load_one_file(main_struct, dirname, filename, compte);
			main_struct->file_hash_list = g_slist_concat(main_struct->file_hash_list, file_hash_list);

			/* Ending things with the progress bar window */
			end_progress_window(main_struct);


			log_message(main_struct->log, G_LOG_LEVEL_INFO, "Fichier %s charg� (%d hashs int�gr�s)", filename, compte->nb_hash);
			g_free(filename);
			g_free(dirname);
		}
	else
		{
			log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Pas de dossier s�lectionn� !!!");
		}

	make_sensitive_hashs_related(main_struct);
	make_sensitive_comparison_related(main_struct);
}

/**
 *  Menu d'affichage de la fen�tre des statistiques
 */
static void menu_display_stats(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
	structure_stat_t *the_stats = NULL;
	GtkLabel *label = NULL;
	gchar *the_text = NULL;
  
	if (main_struct->tronc != NULL)
		{
			the_stats = do_stats_tronc(main_struct->tronc);
			if (the_stats != NULL)
				{
					label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "label_nb_lists"));
					the_text = g_strdup_printf("Nombre total de listes : %d", the_stats->nb_lists); 
					gtk_label_set_text(label, the_text);
					g_free(the_text);
					label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "label_nb_lists_ne"));
					the_text = g_strdup_printf("Nombre total de listes non vides : %d", the_stats->nb_lists_ne); 
					gtk_label_set_text(label, the_text);
					g_free(the_text);
					label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "label_nb_lists_e"));
					the_text = g_strdup_printf("Nombre total de listes vides : %d", the_stats->nb_lists-the_stats->nb_lists_ne); 
					gtk_label_set_text(label, the_text);
					g_free(the_text);
					label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "label_max_lists_len"));
					the_text = g_strdup_printf("Taille de la liste la plus longue : %d", the_stats->max_len_lists); 
					gtk_label_set_text(label, the_text);
					g_free(the_text);
					label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "label_min_lists_len"));
					the_text = g_strdup_printf("Taille de la liste la plus courte (>0) : %d", the_stats->min_len_lists); 
					gtk_label_set_text(label, the_text);
					g_free(the_text);
					label = GTK_LABEL(glade_xml_get_widget(main_struct->xml, "label_moy_lists_len"));
					the_text = g_strdup_printf("Taille moyenne des listes : %d", the_stats->moy_len_lists); 
					gtk_label_set_text(label, the_text);
					g_free(the_text);

					gtk_widget_show(glade_xml_get_widget(main_struct->xml, "stat_window"));
				}
		}
}

/**
 *  Bouton de fermeture de la fen�tre de statistiques
 */
static void close_stat_window(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
  
	gtk_widget_hide(glade_xml_get_widget(main_struct->xml, "stat_window"));
}


/**
 *  To close the A propos dialog box (with the "close" button)
 */
static void a_propos_response(GtkWidget *widget, gint response, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
 	
	gtk_widget_hide(glade_xml_get_widget(main_struct->xml, "about_dialog"));
}

static void a_propos_close(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
	
	gtk_widget_hide(glade_xml_get_widget(main_struct->xml, "about_dialog"));
}

static gboolean a_propos_delete(GtkWidget *widget, GdkEvent  *event, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;
 	
	gtk_widget_hide(glade_xml_get_widget(main_struct->xml, "about_dialog"));
 	
	return TRUE;
}


/** 
 *  fermeture de la fen�tre 
 */
static gboolean delete_main_window_event(GtkWidget *widget, GdkEvent  *event, gpointer data )
{
	gtk_widget_destroy(widget);

	return TRUE;
}

/** 
 *  destruction de la fen�tre principale 
 */
static void destroy_main_window(GtkWidget *widget, gpointer data)
{
	main_struct_t *main_struct = (main_struct_t *) data;

	fin_du_programme(main_struct);
}

/**
 *  Connecte les signaux correspondant aux menus
 */
static void connect_signaux_menus(main_struct_t *main_struct)
{

	/* Quitter le programme (menu Quitter ou Ctrl+q) */
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "menu_quit")), "activate",
					 G_CALLBACK(menu_quit), main_struct);

	/* Afficher la fen�tre de log */
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "menu_log")), "activate",
					 G_CALLBACK(menu_log), main_struct);

	/* about dialog box */		
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_a_propos")), "activate",  
					 G_CALLBACK(menu_a_propos), main_struct);
 
	/* menu hacher un r�pertoire */		
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_hash_a_dir")), "activate",  
					 G_CALLBACK(menu_hash_a_dir), main_struct);

	/* menu enregistrer des hash d'un r�pertoire */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_save_hashs")), "activate",  
					 G_CALLBACK(menu_save_hashs), main_struct); 

	/* menu chargement des hashsets */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_load_hashsets")), "activate",  
					 G_CALLBACK(menu_load_hashsets), main_struct);

	/* menu enregistrement des hahsets */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_save_hashsets")), "activate",  
					 G_CALLBACK(menu_save_hashsets), main_struct);

	/* menu comparaison : ceux qui ne sont PAS dans les hashsets */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_not_in_hashsets")), "activate",  
					 G_CALLBACK(menu_not_in_hashsets), main_struct); 

	/* menu comparaison : ceux qui sont dans les hashsets */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_in_hashsets")), "activate",  
					 G_CALLBACK(menu_in_hashsets), main_struct);

	/* menu effectuer la comparaison */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_do_the_comparison")), "activate",  
					 G_CALLBACK(menu_do_the_comparison), main_struct); 

	/* menu Oublier les hashs d�j� r�alis�s */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_empty_hash_list")), "activate",  
					 G_CALLBACK(menu_empty_hash_list), main_struct);

	/* menu pour les pr�f�rences */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_preferences")), "activate",  
					 G_CALLBACK(show_pref_window), main_struct);

	/* menu pour oublier les hashsets charg�s */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_empty_hashsets")), "activate",  
					 G_CALLBACK(menu_empty_hashsets), main_struct);

	/* menu pour charger un seul hashset pour comparaison */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_load_a_hashset")), "activate",  
					 G_CALLBACK(menu_load_a_hashset), main_struct);

	/* menu pour afficher la fen�tre de statistiques */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "menu_display_stats")), "activate",  
					 G_CALLBACK(menu_display_stats), main_struct);
 
	/* bouton de fermeture de la fen�tre des statistiques */
	g_signal_connect(G_OBJECT (glade_xml_get_widget(main_struct->xml, "statw_close")), "clicked",  
					 G_CALLBACK(close_stat_window), main_struct);
}

/**
 *  Connecte les signaux correspondant � la boite de dialogue A propos
 */
static void connect_signaux_about_box(main_struct_t *main_struct)
{
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "about_dialog")), "close",
					 G_CALLBACK(a_propos_close), main_struct);

	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "about_dialog")), "response",
					 G_CALLBACK(a_propos_response), main_struct);

	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "about_dialog")), "delete-event",
					 G_CALLBACK(a_propos_delete), main_struct);
}

/** 
 *  connection des diff�rents signaux g�n�r�s par les widgets
 *  aux bonnes fonctions                                   
 */
static void connect_signaux(main_struct_t *main_struct)
{
	/* Supression de la fen�tre */
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window")), "delete-event", 
					 G_CALLBACK(delete_main_window_event), NULL);

	/* Destruction de la fen�tre */
	g_signal_connect(G_OBJECT(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window")), "destroy", 
					 G_CALLBACK(destroy_main_window), main_struct);

	/* Connexion des signaux du menu de la fen�tre principale */
	connect_signaux_menus(main_struct);

	/* Connexion des signaux de la boite de dialogue a propos */
	connect_signaux_about_box(main_struct);
}


/**
 *  Chargement de l'interface Glade (en XML) 
 */
static gboolean load_xml_interface(main_struct_t *main_struct)
{
	main_struct->xml = load_glade_xml_file(main_struct->location_list, "gtkcmphash.glade");

	if (main_struct->xml == NULL)
		return FALSE;
	else
		return TRUE;
}

/**
 *  Initialisation de l'internationalisation
 */
static void init_international_languages(void)
{
	gchar *result = NULL;

#ifdef WINDOWS
	gchar *locale_dir = NULL;
	locale_dir = g_strdup_printf("%s\\locale", g_get_current_dir());
	result = bindtextdomain(GETTEXT_PACKAGE, locale_dir);
	g_free(locale_dir);
#else
	result = bindtextdomain(GETTEXT_PACKAGE, PROGRAM_LOCALE_DIR);
#endif
  
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
}


/**
 *  Initialisation des structures 
 */
static main_struct_t *init_main_struct(void)
{
	main_struct_t *main_struct = NULL;
	options_t *opts = NULL;
	found_or_not_t *dedans_ou_pas = NULL;

	main_struct = (main_struct_t *) g_malloc0 (sizeof(main_struct_t));
	opts = (options_t *) g_malloc0 (sizeof(options_t));
	dedans_ou_pas = (found_or_not_t *) g_malloc0(sizeof(found_or_not_t));

	if (main_struct != NULL && opts != NULL)
		{
			main_struct->xml = NULL;    /* le XML de l'interface                           */

			main_struct->location_list = init_location_list(NULL, "GtkCmpHash"); /* la liste de localisations  */

			main_struct->log = NULL;    /* Le syst�me de log                               */

			main_struct->pb = NULL;     /* La barre de progression                         */

			main_struct->file_hash_list = NULL; /* la liste des hash (pour un r�pertoire ) */

			opts->include_dir = TRUE;   /* par d�faut on veux inclure les r�pertoires          */
			opts->all_known = FALSE;    /* par d�faut on ne prend que le premier dans la liste */
			opts->include_hashset_name = TRUE;  /* par d�faut on veux le nom du hashset        */
			opts->include_hashset_file_filename = TRUE;  /* idem pour le nom du fichier        */
			opts->genere_hashs_vides = FALSE;   /* par d�faut on ne g�n�re pas les hashs de fichiers vides */
			opts->charger_fv_hashsets = FALSE;  /* par d�faut on ne charge pas les hashs de fichiers vides */
			opts->nb_indirections = GCH_NB_INDIRECT; /* valeur par d�faut */
			opts->hash_type = GCH_HASH_MD5; /* type de hash utilis� pour la comparaison */
			main_struct->opts = opts;

			main_struct->tronc = NULL;  /* le tronc sera g�r� � part */

			dedans_ou_pas->found = NULL;     /* les listes de r�sultats de la comparaison */
			dedans_ou_pas->not_found = NULL;
			main_struct->dedans_ou_pas = dedans_ou_pas;

			return main_struct;
		}
	else
		return NULL;
}


/**
 *  Fonction permettant de d�finir les valeurs par d�faut
 *  Utilis�es dans l'interface utilisateur (GUI)
 */
static void set_default_gui_values(main_struct_t *main_struct)
{
	make_sensitive_hashs_related(main_struct);
	make_sensitive_comparison_related(main_struct);
  
	/* La sauvegarde de l'ensemble des hashsets */
	gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_save_hashsets"), FALSE);

	/* Le menu pour oublier les hashsets charg�s */
	gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_empty_hashsets"), FALSE);

	/* Le menu pour charger un hashset (pour comparaison) */
	gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_load_a_hashset"), TRUE);

	/* le menu d'affichage de la fen�tre de statistiques */
	gtk_widget_set_sensitive(glade_xml_get_widget(main_struct->xml, "menu_display_stats"), FALSE);

}

/**
 *  Initialisation de l'interface pr�alablement charg�e
 */
static void init_interface(main_struct_t *main_struct)
{

	/* fen�tre principale */
	connect_signaux(main_struct); 

	/* initialisation du syst�me de log */
	init_log_window(main_struct);

	/* initialisation de la fen�tre des pr�f�rences */
	init_pref_window_with_defaults(main_struct);

	/* initialisation aux valeurs par d�faut de l'ensemble des widgets du GUI */
	set_default_gui_values(main_struct);

	/* Affichage de tous les widgets qui doivent �tre affich�s */
	gtk_widget_show_all(glade_xml_get_widget(main_struct->xml, "gtkcmphash_window"));

	log_message(main_struct->log, G_LOG_LEVEL_MESSAGE, "Version %s - Date %s - %s", ProgVersion, ProgDate, ProgAuthor);

}

/**
 *  Inits the openssl library
 */
static void init_openssl(main_struct_t *main_struct)
{

	OpenSSL_add_all_digests();
	log_message(main_struct->log, G_LOG_LEVEL_MESSAGE, "OpenSSL Digest messages loaded !");
}


/** 
 *  Terminaison normale du programme 
 *  C'est l� que tout fini !
 */
static void fin_du_programme(main_struct_t *main_struct)
{
	/* Il faudrait lib�rer un peu de m�moire !! */

	gtk_main_quit();
}


/**
 *  C'est l� que tout commence !
 */
int main(int argc, char **argv)
{
	gboolean exit_value = TRUE;
	main_struct_t *main_struct;

	/* Initialisation des threads au plus t�t */
	if (!g_thread_supported ())
		g_thread_init (NULL);

	/* Initialisation de gtk */
	exit_value = gtk_init_check(&argc, &argv);

	init_international_languages();

	main_struct = init_main_struct();

	if (main_struct != NULL)
		{	
			if (load_xml_interface(main_struct))
				{
	  
					init_interface(main_struct);
					init_openssl(main_struct);

					/* le loop principal de gtk+ */
					gtk_main();

				}
			else
				{
					fprintf(stderr, _("Erreur dans le chargement de l'interface"));
					exit_value = FALSE;
				}
		}
	else
		{
			fprintf(stderr, _("Impossible d'initialiser la structure principale"));
			exit_value = FALSE;
		}

	return exit_value;
}
