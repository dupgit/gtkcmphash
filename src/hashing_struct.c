/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   hashing_struct.c
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
/**
 *  Gestion du chargement, de la mise en mémoire et des comparaisons
 *  des hashs
 *  Utilisation de la structure :
 *   - Création de l'arbre/tronc : nouveau_tronc(niveau)
 *     niveau indique le nombre d'indirections (intéressant à partir de 2)
 *   - Insertion dans l'arbre/tronc : insere_dans_tronc(tronc, file_hash)
 *     Insère file_hash dans le tronc (là où il faut)
 *   - Recherche : recherche_dans_tronc(tronc, file_hash)
 *     Recherche file_hash rapidement (en quelques comparaisons) avec
 *     gestion des collisions
 */

#include "gtkcmphash.h"

static arbre_t *nouvel_arbre(guint niveau_max, guint niveau);
static guchar *choisit_le_bon_hash(file_hash_t *hash, guint hash_type);
static guint cle_de_hashage(guchar c);
static guint int_value_of_hex(guchar c);
static GSList *insere_dans_liste(GSList *list, file_hash_t *file_hash);
static void insere_dans_arbre(arbre_t *arbre, guint niveau_max, guint hash_type, file_hash_t *file_hash);
static GSList *recherche_dans_arbre(arbre_t *arbre, guint niveau_max, guint hash_type, file_hash_t *file_hash);
static GSList *recherche_dans_liste(GSList *list, file_hash_t *file_hash, guint hash_type);

static result_hash_t *copie_les_resultats(file_hash_t *file_hash, file_hash_t *result_hash);
static GSList *forme_la_liste_des_connus(GSList *found, GSList *result, file_hash_t *file_hash, options_t *opts);

static GSList *concatene_les_arbres(arbre_t *arbre, guint niveau_max);
static GSList *concatene_les_listes(arbre_t *arbre);
static GSList *transforme_arbre_en_liste(arbre_t *arbre, guint niveau_max);

static void libere_les_listes(arbre_t *arbre);
static void libere_les_arbres(arbre_t *arbre, guint niveau_max);
static void free_arbre(arbre_t *arbre, guint niveau_max);

static void comptabilise_les_listes(arbre_t *arbre, structure_stat_t *the_stats);
static void comptabilise_les_arbres(arbre_t *arbre, guint niveau_max, structure_stat_t *the_stats);
static void do_stats_arbre(arbre_t *arbre, guint niveau_max, structure_stat_t *the_stats);


/**
 *  Retourne le pointeur sur le hash adéquat en fonction du
 *  type choisit
 */
static guchar *choisit_le_bon_hash(file_hash_t *hash, guint hash_type)
{
  if (hash_type == GCH_HASH_MD5)
    {
        return hash->hash_md5;
    }
  else if (hash_type == GCH_HASH_SHA1)
    {
        return hash->hash_sha1;
    }
  else if (hash_type == GCH_HASH_RIPEMD160)
    {
        return hash->hash_ripemd;
    }
  else
    {
        return hash->hash_md5; /* la valeur par défaut */
    }
}


/**
 *  Compare les deux hash en fonction du type de hash choisit
 */
static gint compare_les_hashs(file_hash_t *hash1, file_hash_t *hash2, guint hash_type)
{
  if (hash_type == GCH_HASH_MD5)
    {
      return my_g_ascii_strcasecmp(hash1->hash_md5, hash2->hash_md5, hash2->len_md5);
    }
  else if (hash_type == GCH_HASH_SHA1)
    {
      return my_g_ascii_strcasecmp(hash1->hash_sha1, hash2->hash_sha1, hash2->len_sha1);
    }
  else if (hash_type == GCH_HASH_RIPEMD160)
    {
      return my_g_ascii_strcasecmp(hash1->hash_ripemd, hash2->hash_ripemd, hash2->len_ripemd);
    }
  else
    {
        return my_g_ascii_strcasecmp(hash1->hash_md5, hash2->hash_md5, hash2->len_md5); /* la valeur par défaut */
    }
}


/**
 *  Crée un nouvel arbre de profondeur "niveau_max", à partir de "niveau"
 */
static arbre_t *nouvel_arbre(guint niveau_max, guint niveau)
{
  arbre_t *arbre = NULL;
  guint i = 0;

  arbre = (arbre_t *) g_malloc0(sizeof(arbre_t));
  arbre->niveau = niveau;

  if (niveau >= niveau_max)
    {
      for (i=0; i<16; i++)
        {
            arbre->array[i] = NULL;
        }
      return arbre;
    }
  else
    {
      for (i=0; i<16; i++)
        {
            arbre->array[i] = nouvel_arbre(niveau_max, niveau+1);
        }
      return arbre;
    }
}


/**
 *  Construit une structure vide de niveau "niveau"
 *  maximum 16 sous niveaux pour le md5
 */
hash_t *nouveau_tronc(guint niveau_max)
{
  hash_t *tronc = NULL;

  tronc = (hash_t *) g_malloc0(sizeof(hash_t));

  tronc->arbre = nouvel_arbre(niveau_max, 1);
  tronc->nb_niveau = niveau_max;

  return tronc;
}


/**
 *  Clé de hashage (à partir d'un octet renvoi 16
 *  cas possibles)
 */
static guint cle_de_hashage(guchar c)
{
  return ((guint)(c >>4));
}


/**
 *  Retourne la valeur du caractère héxadécimal en décimal
 *  0->0 ... 9->9, a->10, ... f->15 sinon 0
 */
static guint int_value_of_hex(guchar c)
{
  switch (c)
    {
    case '0':
      return 0;
      break;
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
    case 'a':
      return 10;
      break;
    case 'b':
      return 11;
      break;
    case 'c':
      return 12;
      break;
    case 'd':
      return 13;
      break;
    case 'e':
      return 14;
      break;
    case 'f':
      return 15;
      break;
    default:
      return 0;
      break;
    }
}


/**
 *  Insère le file_hash d'un fichier dans une liste
 *  crée éventuellement cette liste
 */
static GSList *insere_dans_liste(GSList *list, file_hash_t *file_hash)
{
  list = g_slist_prepend(list, file_hash);

  return list;
}


/**
 *  Insère un file_hash d'un fichier dans l'arbre (en fonction de son md5)
 *  TODO : laisser le choix à l'utilisateur du hash servant de comparaison
 */
static void insere_dans_arbre(arbre_t *arbre, guint niveau_max, guint hash_type, file_hash_t *file_hash)
{
    guint numero_liste = 0;
    guchar *hash = NULL;

    hash = choisit_le_bon_hash(file_hash, hash_type);

    numero_liste = cle_de_hashage(hash[arbre->niveau-1]);

    if (arbre->niveau >= niveau_max)
    {
        arbre->array[numero_liste] = insere_dans_liste(arbre->array[numero_liste], file_hash);
    }
    else
    {
        insere_dans_arbre(arbre->array[numero_liste], niveau_max, hash_type, file_hash);
    }
}


/**
 *  Insère un file_hash d'un fichier dans la structure
 */
void insere_dans_tronc(hash_t *tronc, file_hash_t *file_hash, guint hash_type)
{

    if (tronc != NULL)
    {
        if (tronc->arbre != NULL && tronc->nb_niveau > 0)
        {
            insere_dans_arbre(tronc->arbre, tronc->nb_niveau, hash_type, file_hash);
        }
    }
}


/**
 *  Insère toute une liste de file_hash dans la structure tronc
 */
void insere_la_liste_dans_tronc(hash_t *tronc, GSList *file_hash_list, guint hash_type)
{
    file_hash_t *file_hash = NULL;

    while (file_hash_list != NULL)
    {
        file_hash = (file_hash_t *) file_hash_list->data;

        if (file_hash != NULL)
        {
            insere_dans_tronc(tronc, file_hash, hash_type);
        }

        file_hash_list = g_slist_next(file_hash_list);
    }
}


/**
 *  Procédure copiée de la glib, mais dans laquelle
 *  j'ai enlevé les TOLOWER dont je n'ai absolument
 *  pas besoin... donc gain de rapidité (c'est plus rapide que memcmp)
 */
gint my_g_ascii_strcasecmp(const guchar *s1, const guchar *s2, guint len)
{

    while (len)
    {
        if ((gint)(guchar)(*s1) != (gint)(guchar)(*s2))
        {
            return (((gint)(guchar) *s1) - ((gint)(guchar) *s2));
        }

        s1++;
        s2++;
        len--;
    }

    return (((gint)(guchar) *s1) - ((gint)(guchar) *s2));
}


/**
 *  Procédure récupérée de la glib et transformée pour
 *  qu'elle ait en entrée et en sortie le type guchar
 *  et qu'on puisse copier une chaine binaire (avec des 0x00)
 */
guchar *my_g_strdup(const guchar *str, guint length)
{
    guchar *new_str = NULL;

    if (str)
    {
        new_str = g_new(unsigned char, length);
        memcpy(new_str, str, length);
    }
    else
    {
        new_str = NULL;
    }

    return new_str;
}


/**
 *  Recherche dans la liste d'éventuels fichiers ayant un hash
 *  identique à celui de file_hash (utilise le md5)
 */
static GSList *recherche_dans_liste(GSList *list, file_hash_t *file_hash, guint hash_type)
{
    file_hash_t *tmp_hash = NULL;
    GSList *head = list;
    GSList *file_hash_list = NULL;

    while (list != NULL)
    {
        tmp_hash = list->data;

        if (tmp_hash != NULL)
        {
            if (compare_les_hashs(file_hash, tmp_hash, hash_type) == 0)
            {
                file_hash_list = g_slist_prepend(file_hash_list, tmp_hash);
            }
        }

        list = list->next;
    }

    list = head;

    return file_hash_list;
}


/**
 *  Effectue la recherche dans un arbre
 */
static GSList *recherche_dans_arbre(arbre_t *arbre, guint niveau_max, guint hash_type, file_hash_t *file_hash)
{
    guint numero_liste = 0;
    guchar *hash = NULL;

    hash = choisit_le_bon_hash(file_hash, hash_type);

    /* il faut utiliser la même clef que celle de l'insertion */
    /* TODO : donner le choix de la clef à l'utilisateur      */
    numero_liste = cle_de_hashage(hash[arbre->niveau-1]);

    if (arbre->niveau >= niveau_max)
    {
        return recherche_dans_liste(arbre->array[numero_liste], file_hash, hash_type);
    }
    else
    {
        return recherche_dans_arbre(arbre->array[numero_liste], niveau_max, hash_type, file_hash);
    }
}


/**
 *  Recherche si le hash "hash" est présent dans la structure de données
 *  la fonction retourne la liste des hash correspondant
 *  NULL dans les autres cas
 */
GSList *recherche_dans_tronc(hash_t *tronc, file_hash_t *file_hash, guint hash_type)
{
    if (tronc != NULL && tronc->arbre != NULL && tronc->nb_niveau > 0)
    {
        return recherche_dans_arbre(tronc->arbre, tronc->nb_niveau, hash_type, file_hash);
    }
    else
    {
        return NULL;
    }
}


/**
 *  Concatène les sous-listes
 */
static GSList *concatene_les_listes(arbre_t *arbre)
{
    guint i = 0;
    GSList *en_cours = NULL;

    for (i = 0; i < 16; i++)
    {
        if (arbre->array[i] != NULL)
        {
            en_cours = g_slist_concat(arbre->array[i], en_cours);
        }
    }

    return en_cours;
}


/**
 *  Concatène le résultat des différents sous-arbres
 */
static GSList *concatene_les_arbres(arbre_t *arbre, guint niveau_max)
{
    guint i = 0;
    GSList *en_cours = NULL;
    GSList *resultats = NULL;

    for (i = 0; i < 16; i++)
    {
        resultats = transforme_arbre_en_liste(arbre->array[i], niveau_max);
        if (resultats != NULL)
        {
            en_cours = g_slist_concat(resultats, en_cours);
        }
    }

    return en_cours;
}


/**
 *  Remet l'arbre à plat
 */
static GSList *transforme_arbre_en_liste(arbre_t *arbre, guint niveau_max)
{
    if (arbre->niveau >= niveau_max)
    {
        return concatene_les_listes(arbre); /* ici c'est le niveau des listes */
    }
    else
    {
        return concatene_les_arbres(arbre, niveau_max);
    }
}


/**
 *  Remet le tronc à plat
 */
GSList *transforme_tronc_en_liste(hash_t *tronc)
{
    if (tronc != NULL && tronc->arbre != NULL && tronc->nb_niveau > 0)
    {
        return transforme_arbre_en_liste(tronc->arbre, tronc->nb_niveau);
    }
    else
    {
        return NULL;
    }
}


/**
 *  Procédure qui transforme le hash b1f548... en son
 *  équivalent binaire; Réalise un g_malloc0 qu'on pourra
 *  libérer avec un g_free par la suite.
 */
guchar *transforme_le_hash_de_hex_en_binaire(guchar *hash)
{
    guchar c = (guchar) 0;
    guint i = 0;
    guint j = 0;
    guint len = strlen((const char *)hash);
    guchar *aux = NULL;

    aux = (guchar *) g_malloc0(sizeof(guchar)*((len/2)+2));

    while (i < len)
    {
        c = 16*int_value_of_hex(hash[i]) + int_value_of_hex(hash[i+1]);
        aux[j] = c;
        j++;
        i += 2;
    }

    aux[j] = (guchar) 0;

    return aux;
}


/**
 *  Fonction qui transforme un hash sous forme binaire
 *  en un hash sous forme hexadécimale affichable "12f5cda5..."
 *  Réalise un g_malloc0 qu'on pourra libérer par la suite avec
 *  un g_free
 */
guchar *transforme_le_hash_de_binaire_en_hex(guchar *hash, guint len)
{
    guint i = 0;
    guchar *aux = NULL;
    guchar *md5 = NULL;

    aux = (guchar *) g_malloc0(sizeof(gchar)*3);
    md5 = (guchar *) g_malloc0(sizeof(guchar)*((len*2)+2));

    for(i = 0; i < len; i++)
    {
        sprintf((char *) aux, "%02x", hash[i]);
        memcpy(md5+(i*2), aux, 2);
    }

    md5[i*2+1] = '\0';
    g_free(aux);

    return md5;
}

/**
 *  Dit si l'un des hashs de file_hash est un hash de fichier vide
 */
gboolean is_file_hash_empty(file_hash_t *file_hash)
{
    guchar *md5_vide = transforme_le_hash_de_hex_en_binaire((guchar *) "d41d8cd98f00b204e9800998ecf8427e");
    guchar *sha1_vide = transforme_le_hash_de_hex_en_binaire((guchar *) "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    guchar *ripemd_vide = transforme_le_hash_de_hex_en_binaire((guchar *) "9c1185a5c5e9fc54612808977ee8f548b2258d31");
    gint result = FALSE;

    if (file_hash != NULL)
    {
        result = my_g_ascii_strcasecmp(file_hash->hash_md5, md5_vide, file_hash->len_md5);

        if (result != 0)
        {
            result = my_g_ascii_strcasecmp(file_hash->hash_sha1, sha1_vide, file_hash->len_sha1);
        }

        if (result != 0)
        {
            result = my_g_ascii_strcasecmp(file_hash->hash_ripemd, ripemd_vide, file_hash->len_ripemd);
        }
    }

    g_free(md5_vide);
    g_free(sha1_vide);
    g_free(ripemd_vide);

    if (result == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/**
 *  Fonction de libération d'un file_hash
 *  On ne supprime pas le hashset_name qui n'est pas inclus dans
 *  chaque structure (donc si on le libère en cours de route,
 *  on le perd définitivement)
 */
void free_file_hash(file_hash_t *file_hash)
{
    if (file_hash->hash_md5 != NULL)
    {
        g_free(file_hash->hash_md5);
    }

    if (file_hash->hash_sha1 != NULL)
    {
        g_free(file_hash->hash_sha1);
    }

    if (file_hash->hash_ripemd != NULL)
    {
        g_free(file_hash->hash_ripemd);
    }

    if (file_hash->filename != NULL)
    {
        g_free(file_hash->filename);
    }
}


/**
 * Libération du hashset->name qui n'est stocké en mémoire
 * qu'une seule fois (ou du moins, le moins possible)
 */
static void free_file_hash_hashset(hashset_t *hashset)
{
  /* Il peut y avoir un double appel a free pour un même pointeur */

    if (hashset != NULL)
    {
        if (hashset->refs > 0)
        {
            hashset->refs--;
        }
        else
        {
            g_free(hashset->name);
            g_free(hashset);
        }
    }
}


/**
 *  Fonction de libération de la liste (libération totale)
 *  perte du hashset_name !
 */
void free_file_hash_list(GSList *file_hash_list)
{
    GSList *head = file_hash_list;
    file_hash_t *file_hash = NULL;

    while (file_hash_list != NULL)
    {
        file_hash = (file_hash_t *) file_hash_list->data;

        if (file_hash != NULL)
        {
            free_file_hash(file_hash);
            free_file_hash_hashset(file_hash->hashset);
            g_free(file_hash);
        }

        file_hash_list = g_slist_next(file_hash_list);
    }

    g_slist_free(head);
}


/**
 *  Fonction de libération de la liste des résultats (libération totale)
 *  La mémoire est gérée différemment de la la liste file_hash_list
 */
void free_result_hash_list(GSList *result_hash_list)
{
    GSList *head = result_hash_list;
    result_hash_t *result_hash = NULL;

    while (result_hash_list != NULL)
    {
        result_hash = (result_hash_t *) result_hash_list->data;

        if (result_hash != NULL)
        {
            g_free(result_hash->hashset_name);
            g_free(result_hash->hashset_file_filename);
            g_free(result_hash->filename);
            g_free(result_hash->hash_md5);
            g_free(result_hash->hash_sha1);
            g_free(result_hash->hash_ripemd);
            g_free(result_hash);
        }

        result_hash_list = g_slist_next(result_hash_list);
    }

    g_slist_free(head);
}


/**
 *  Libération de la liste des résultats
 *  Attention aucun test n'est effectué sur la structure
 */
void free_dedans_ou_pas(main_struct_t *main_struct)
{
    /* on peut tout libérer, normalement cette liste est formée par copie */
    free_result_hash_list(main_struct->dedans_ou_pas->found);

    /* Si on libère la structure -> la liste d'origine sera libérée ! */
    g_slist_free(main_struct->dedans_ou_pas->not_found);

    main_struct->dedans_ou_pas->found = NULL;
    main_struct->dedans_ou_pas->not_found = NULL;
}


/**
 *  Libere les listes. arbre doit être non NULL
 */
static void libere_les_listes(arbre_t *arbre)
{
    guint i = 0;

    for (i = 0; i < 16; i++)
    {
        free_file_hash_list(arbre->array[i]);
        arbre->array[i] = NULL;
    }

    g_free(arbre);
}

/**
 *  Libere les sous arbres. arbre doit être non NULL
 */
static void libere_les_arbres(arbre_t *arbre, guint niveau_max)
{
    guint i = 0;

    for (i = 0; i < 16; i++)
    {
        free_arbre(arbre->array[i], niveau_max);
    }

    g_free(arbre);  /* la sous branche est libérée */

}

/**
 *  Vide l'intégralité des arbres (récursif)
 */
static void free_arbre(arbre_t *arbre, guint niveau_max)
{

    if (arbre != NULL)
    {
        if (arbre->niveau >= niveau_max)
        {
            libere_les_listes(arbre);
        }
        else
        {
            libere_les_arbres(arbre, niveau_max);
        }
    }
}

/**
 *   Libère l'intégralité des listes de la structure
 */
void free_tronc(hash_t *tronc)
{

    if (tronc != NULL && tronc->arbre != NULL && tronc->nb_niveau > 0)
    {
        free_arbre(tronc->arbre, tronc->nb_niveau);

        g_free(tronc);
    }
}


/**
 *  Copie les informations adéquates dans une nouvelle
 *  structure qui est retournée
 */
static result_hash_t *copie_les_resultats(file_hash_t *file_hash, file_hash_t *result_hash)
{
    result_hash_t *all_known_hash = NULL;

    all_known_hash = (result_hash_t *) g_malloc0(sizeof(result_hash_t));

    all_known_hash->hashset_name = g_strdup(result_hash->hashset->name);
    all_known_hash->hashset_file_filename = g_strdup(result_hash->filename);

    all_known_hash->filename = g_strdup(file_hash->filename);
    all_known_hash->hash_md5 = my_g_strdup(file_hash->hash_md5, file_hash->len_md5);
    all_known_hash->len_md5 = file_hash->len_md5;
    all_known_hash->hash_sha1 = my_g_strdup(file_hash->hash_sha1, file_hash->len_sha1);
    all_known_hash->len_sha1 = file_hash->len_sha1;
    all_known_hash->hash_ripemd = my_g_strdup(file_hash->hash_ripemd, file_hash->len_ripemd);
    all_known_hash->len_ripemd = file_hash->len_ripemd;

    return all_known_hash;
}

/**
 *  Insère, en fonction des options, les fichiers connus dans la liste
 *  Attention la liste contient des éléments de type result_hash_t *
 */
static GSList *forme_la_liste_des_connus(GSList *found, GSList *result, file_hash_t *file_hash, options_t *opts)
{
    file_hash_t *result_hash = NULL;
    result_hash_t *all_known_hash = NULL;

    if (opts->all_known == TRUE)
    {
        /* intègre tous les fichiers connus */
        while (result != NULL)
        {
            result_hash = (file_hash_t *) result->data;

            all_known_hash = copie_les_resultats(file_hash, result_hash);

            found = g_slist_prepend(found, all_known_hash);

            result = g_slist_next(result);

            /* On ne libère pas de mémoire car tout est dans la liste pour une   */
            /* utilisation ultérieure. La libération intervient dans la fonction */
            /* free_file_hash_list(GSList *file_hash_list) plus haut             */
        }
    }
    else
    {
        result_hash = (file_hash_t *) result->data;

        all_known_hash = copie_les_resultats(file_hash, result_hash);

        found = g_slist_prepend(found, all_known_hash);
    }

    return found;
}

/**
 *  Fonction pour trouver les différences entre une liste de fichiers
 *  hachés et un hashset chargés en mémoire
 */
found_or_not_t *find_all_hashes_from_hashset(GSList *file_hash_list, hash_t *tronc, options_t *opts)
{
    file_hash_t *file_hash = NULL; /* pour parcourir la liste des fichiers hachés */
    GSList *result = NULL;  /* résultat de la recherche dans l'arbre */
    found_or_not_t *dedans_ou_pas = NULL;

    dedans_ou_pas = (found_or_not_t *) g_malloc0(sizeof(found_or_not_t));
    dedans_ou_pas->found = NULL;
    dedans_ou_pas->not_found = NULL;

    while (file_hash_list != NULL)
    {
        file_hash = (file_hash_t *) file_hash_list->data;

        if (file_hash != NULL)
        {
            /* retourne la liste de tous les fichiers qui ont le même hash */
            result = recherche_dans_tronc(tronc, file_hash, opts->hash_type);

            if (result != NULL)
            {
                dedans_ou_pas->found = forme_la_liste_des_connus(dedans_ou_pas->found, result, file_hash, opts);
            }
            else
            {
                dedans_ou_pas->not_found = g_slist_prepend(dedans_ou_pas->not_found, file_hash);
            }

            g_slist_free(result);
        }

        file_hash_list = g_slist_next(file_hash_list);
    }

    return dedans_ou_pas;
}


/**
 *  Appel terminal qui réalise la comptabilité pour les listes
 */
static void comptabilise_les_listes(arbre_t *arbre, structure_stat_t *the_stats)
{
    guint i = 0;
    guint len = 0;
    for (i = 0; i < 16; i++)
    {
        the_stats->nb_lists++;
        if (arbre->array[i] != NULL)
        {
            the_stats->nb_lists_ne++;
            len = g_slist_length(arbre->array[i]);
            if (len > the_stats->max_len_lists)
            {
                the_stats->max_len_lists = len;
            }
            else if (len <the_stats->min_len_lists)
                {
                    the_stats->min_len_lists = len;
                }

            if (the_stats->nb_lists == 1)
            {
                the_stats->moy_len_lists = len;
            }
            else
            {
                the_stats->moy_len_lists = (the_stats->moy_len_lists*(the_stats->nb_lists-1)+ len) / the_stats->nb_lists;
            }
        }
    }
}


/**
 *  Appel récursif permettant la prise en compte des sous-arbres
 */
static void comptabilise_les_arbres(arbre_t *arbre, guint niveau_max, structure_stat_t *the_stats)
{
    guint i = 0;

    for (i = 0; i < 16; i++)
    {
        do_stats_arbre(arbre->array[i], niveau_max, the_stats);
    }
}


/**
 *  Statistiques sur les arbres ou les listes en fonction du
 *  niveau
 */
static void do_stats_arbre(arbre_t *arbre, guint niveau_max, structure_stat_t *the_stats)
{
    if (arbre != NULL)
    {
        if (arbre->niveau >= niveau_max)
        {
            comptabilise_les_listes(arbre, the_stats);
        }
        else
        {
            comptabilise_les_arbres(arbre, niveau_max, the_stats);
        }
    }
}


/**
 *   Statistiques sur la structure
 */
structure_stat_t *do_stats_tronc(hash_t *tronc)
{
    structure_stat_t *the_stats = NULL;

    if (tronc != NULL && tronc->arbre != NULL && tronc->nb_niveau > 0)
    {
        the_stats = (structure_stat_t *) g_malloc0(sizeof(structure_stat_t));
        the_stats->nb_lists = 0;
        the_stats->nb_lists_ne = 0;
        the_stats->max_len_lists = 0;
        the_stats->min_len_lists = G_MAXUINT;
        the_stats->moy_len_lists = 0;
        do_stats_arbre(tronc->arbre, tronc->nb_niveau, the_stats);
        return the_stats;
    }
    else
    {
        return NULL;
    }
}
