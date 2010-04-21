/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   gtkcmphash.h
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
#ifndef _GTK_CMP_HASH_H_
#define _GTK_CMP_HASH_H_

#include <bzlib.h>
#include <glade/glade.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <dup/duptools.h>
#include <openssl/hmac.h>

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif /* HAVE_CONFIG_H */

#define ProgName "gtkcmphash"
#define ProgVersion "0.0.4"
#define ProgDate "18.10.2007"
#define ProgAuthor "Olivier Delhomme"

#define GCH_NB_INDIRECT 3      /* valeur par d�faut pour le nombre d'indirections                              */
#define GCH_NB_INDIRECT_MIN 2  /* valeurs minimales et maximales autoris�es pour la structure.                 */
#define GCH_NB_INDIRECT_MAX 5  /* d�finies ici pour �viter tout probl�me m�me en cas de modification du .glade */

#define GCH_HASH_MD5 2         /* definition des types de hash utilis�s pour la comparaison */
#define GCH_HASH_SHA1 4        /* Les valeurs doivent �tre non sign�es                      */
#define GCH_HASH_RIPEMD160 8


/**
 *  Structure regroupant le hash (en binaire) ainsi que sa longueur
 */
typedef struct
{
    guchar *hash;    /* l'�criture en binaire du hash  */
    guint hash_len;  /* la longueur de cette derni�re  */
} hex_mdp_t;


typedef struct
{
    gchar *name;  /* nom */
    guint refs;   /* Nombre de r�f�rences au nom */
} hashset_t;


/**
 * Structure pour un hash d'un chunk du fichier source (512 octets seulement)
 */
typedef struct
{
    gint64 position; /* indique le num�ro de chunk dans le fichier (soit sa position) */
                     /* -1 = tout le fichier */

    guchar *hash_md5;     /* le md5 du chunk                                           */
    guint8  len_md5;      /* longueur du md5                                           */

    guchar *hash_sha1;    /* le sha1 du chunk                                          */
    guint8  len_sha1;     /* longueur du sha1                                          */

    guchar *hash_ripemd;  /* le ripemd160 du chunk                                     */
    guint8  len_ripemd;   /* longueur du ripemd160                                     */
} chunk_t;


/**
 *  Structures de stockage des hashs issus des diff�rents fichiers
 *  composant le hashset
 *  file_hash_t : pour un seul fichier (utilis� dans une liste pour �viter les collisions)
 *  arbre_t : arbre pour g�rer l'ensemble des hash et faire tomber le
 *            nombre de comparaisons (n branches soit 16^n listes)
 *  hash_t : Super-structure permettant une simplification des appels
 *           de fonctions
 */
typedef struct
{
    hashset_t *hashset;   /* l'�ventuel nom du hashset dans lequel est situ� le fichier */
    gchar *filename;      /* Le nom du fichier                                          */

    chunk_t *file_hashs;  /* les hashs du fichier (dans sa totalit�)                    */

    /* guchar *hash_md5;      le md5 du fichier                                          */
    /* guint  len_md5;        longueur du md5                                            */

    /* guchar *hash_sha1;     le sha1 du fichier                                         */
    /* guint  len_sha1;       longueur du sha1                                           */

    /* guchar *hash_ripemd;   le ripemd160 du fichier                                    */
    /* guint  len_ripemd;     longueur du ripemd160                                      */

    GSList *chunk_hashs;  /* la liste des hashs des chuncks de 512 octets du fichier    */
                          /* de type chunk_t                                            */
} file_hash_t;


/**
 *  Structure utilis�e pour les r�sultats de la comparaison
 */
typedef struct
{
    gchar *hashset_name;          /* Le nom du hashset dans lequel on a trouv� un md5 identique */
    gchar *hashset_file_filename; /* le nom du fichier correspondant dans le hashset            */
    gchar *filename;              /* Le nom du fichier original compar� aux hashsets            */

    chunk_t *file_hashs;          /* les hashs du fichier (dans sa totalit�)                    */

    /* guchar *hash_md5;              le md5 du fichier original (et de celui du hashset)        */
    /* guint  len_md5;                longueur du md5                                            */
    /* guchar *hash_sha1;             le sha1 du fichier                                         */
    /* guint  len_sha1;               longueur du sha1                                           */
    /* guchar *hash_ripemd;           le ripemd160 du fichier                                    */
    /* guint  len_ripemd;             longueur du ripemd160                                      */
} result_hash_t;


typedef struct
{
    void *array[16]; /* un tableau d'indirections 0 -> 9 + a -> f qui pointe soit sur un structure arbre_t soit sur une liste de compte_t  */
    guint niveau;    /* un indicateur du niveau dans lequel on se trouve (g�n�ralement entre 1 et 3)                                       */
} arbre_t;


typedef struct
{
    guint nb_niveau; /* Nombre de niveaux (1 si on n'a qu'un seul niveau d'indirection) */
    arbre_t *arbre;  /* l'arbre des hash  */
} hash_t;


/**
 *  Structure pour la barre de progression
 */
typedef struct
{
    guint64 max;               /* maximum value                                                        */
    guint64 value;             /* current value                                                        */
    guint64 max_file;          /* maximum value for the file progress bar                              */
    guint64 value_file;        /* current value for the file progress bar                              */
    GtkProgressBar *pb_global; /* la barre de progression principale                                   */
    GtkProgressBar *pb_file;   /* la barre de progression secondaire                                   */
    GtkLabel *pb_label;        /* le label situ� juste au dessus de la barre de progression principale */
} p_bar_t;


/**
 *  Structure pour la gestion des options
 */
typedef struct
{
    gboolean include_dir;  /* indique si l'on doit inclure le r�pertoire dans le nom du fichier dans file_hash_t  */
    gboolean all_known;    /* indique si l'on doit inclure tous les noms des hashset o� le fichier appara�t       */
    gboolean include_hashset_name; /* indique si l'on doit inclure le nom du hashset o� est connu le hash cherch� */
    gboolean include_hashset_file_filename; /* idem mais avec le nom du fichier du hashset o� le hash est connu   */
    gboolean genere_hashs_vides;  /* Pour la g�n�ration des hashs de fichiers vides (par d�faut == FALSE)         */
    gboolean charger_fv_hashsets; /* Charge les hashs des fichiers vides contenus dans les hashsets               */
    guint nb_indirections; /* Nombre d'indirections pour la structure (de 2 � 5)                                  */
    guint hash_type;       /* Indique le type de hash : GCH_HASH_MD5 GCH_HASH_SHA1 GCH_HASH_RIPEMD160             */
} options_t;


typedef struct
{
    guint nb_lists;      /* Nombre total de listes            */
    guint nb_lists_ne;   /* Nombre total de listes non vides  */
    guint max_len_lists; /* Longueur maximale des listes      */
    guint min_len_lists; /* Longueur minimale des listes (>0) */
    guint moy_len_lists; /* Longueur moyenne des listes       */
} structure_stat_t;


/**
 *  Structure pour la gestion des r�sultats des comparaisons
 *  Attention :
 *   - found est de type result_hash_t *
 *   - not_found est de type file_hash_t *
 */
typedef struct
{
    GSList *found;      /* les hashs de la liste qui sont pr�sent dans le hashset ; result_hash_t *  */
    GSList *not_found;  /* les hashs de la liste qui ne sont PAS dans le hashset  ; file_hash_t *    */
} found_or_not_t;


/**
 *  Structure permettant de retourner le nombre d'octets
 *  non compress�s trait�s ainsi que le nombre d'octets
 *  compress�s que cela repr�sente ainsi que le nombre
 *  de hashs
 */
typedef struct
{
    unsigned int uncompressed;
    unsigned int compressed;
    unsigned int nb_hash;
} bzip2_result_t;


/**
 *  Structure principale
 */
typedef struct
{
    GladeXML *xml;          /* la d�finition XML de l'interface glade                               */
    GList *location_list;   /* la liste des localisation o� l'on peut trouver les fichiers de conf  */
    log_t *log;             /* pour pouvoir logguer les messages                                    */
    p_bar_t *pb;            /* pour la gestion de la fen�tre de progression                         */
    GSList *file_hash_list; /* Liste des fichiers hash� (d'un r�pertoire)                           */
    options_t *opts;        /* Options ou pr�f�rences                                               */
    hash_t *tronc;          /* Le tronc o� l'on va stocker le hashset                               */
    found_or_not_t *dedans_ou_pas; /* dans le hashset ou pas.                                       */
} main_struct_t;


#include "log_window.h"
#include "file_io.h"
#include "hashing_struct.h"
#include "progress_window.h"
#include "pref_window.h"

#endif /* _GTK_CMP_HASH_H_ */
