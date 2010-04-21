/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   file_io.c
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

static file_hash_t *new_from_buffer_line(gchar *buf, int lus, guint n, hashset_t *hashset);
static int compute_one_block(GSList **file_hash_list, gchar *buf, int lus, hashset_t *hashset, bzip2_result_t *compte, options_t *Opts, gboolean *load_chunks);
static gchar *make_relative_filename_from_dir(gchar *dirname, gchar *filename);
static gchar *prepare_le_buffer(void *hash, gboolean save_hashset, options_t *opts);


/**
 *  Ouvre le fichier filename s'il existe
 */
FILE *open_file_if_it_exists(gchar *filename)
{
    struct stat *stat_buf = NULL;

    stat_buf = (struct stat *) g_malloc0(sizeof(struct stat));

    if (g_stat(filename, stat_buf) == 0)
        {
            return g_fopen(filename, "rb");
        }
    else
        {
            return NULL;
        }
}


/**
 *  Retourne la taille du fichier
 */
guint64 file_size(gchar *filename)
{
    struct stat *stat_buf = NULL;

    stat_buf = (struct stat *) g_malloc0(sizeof(struct stat));

    if (g_stat(filename, stat_buf) == 0)
        {
            return (guint64) stat_buf->st_size;
        }
    else
        {
            return 0;
        }
}

/**
 * @param *buffer : le buffer où les données du fichiers ont été chargées
 * @param lus : la taille de ce qui a été lus (la taille du buffer)
 * @param position : un guint64 qui est augmenté de 1 tous les 512 octets traités
 * @param chunk_hashs : la liste où l'on stocke tous les hashs issus des buffers
 */
static GSList *do_chunks_from_buffer(guchar *buffer, size_t lus, guint64 *position, GSList *chunk_hashs)
{
    const EVP_MD *md5 = NULL;
    const EVP_MD *sha1 = NULL;
    const EVP_MD *ripemd = NULL;
    EVP_MD_CTX md5_ctx;
    EVP_MD_CTX sha1_ctx;
    EVP_MD_CTX ripemd_ctx;
    unsigned int len_md5 = 0;
    unsigned int len_sha1 = 0;
    unsigned int len_ripemd = 0;

    chunk_t *a_chunk_hash = NULL;
    guchar *chunk_buffer = buffer;

    /* traitement de 512 octets dans l'incrément d'une boucle */
    while (lus > 0)
        {
            a_chunk_hash = (chunk_t *) g_malloc0(sizeof(chunk_t));

            a_chunk_hash->hash_md5 = (guchar *) g_malloc0(EVP_MAX_MD_SIZE);
            a_chunk_hash->hash_sha1 = (guchar *) g_malloc0(EVP_MAX_MD_SIZE);
            a_chunk_hash->hash_ripemd = (guchar *) g_malloc0(EVP_MAX_MD_SIZE);

            /* Initialisation des contextes */
            EVP_MD_CTX_init(&md5_ctx);
            EVP_MD_CTX_init(&sha1_ctx);
            EVP_MD_CTX_init(&ripemd_ctx);

            /* Initialisations des hash */
            md5 = EVP_get_digestbyname("md5");
            EVP_DigestInit_ex(&md5_ctx, md5, NULL);
            sha1 = EVP_get_digestbyname("sha1");
            EVP_DigestInit_ex(&sha1_ctx, sha1, NULL);
            ripemd = EVP_get_digestbyname("ripemd160");
            EVP_DigestInit_ex(&ripemd_ctx, ripemd, NULL);

            if (lus >= 512)
                {
                    EVP_DigestUpdate(&md5_ctx, chunk_buffer, 512);
                    EVP_DigestUpdate(&sha1_ctx, chunk_buffer, 512);
                    EVP_DigestUpdate(&ripemd_ctx, chunk_buffer, 512);
                    lus = lus - 512;
                    chunk_buffer = chunk_buffer + 512;
                    *position = *position + 1;
                }
            else
                {
                    EVP_DigestUpdate(&md5_ctx, chunk_buffer, lus);
                    EVP_DigestUpdate(&sha1_ctx, chunk_buffer, lus);
                    EVP_DigestUpdate(&ripemd_ctx, chunk_buffer, lus);
                    lus = 0;
                    *position = *position + 1;
                }

            /* Terminaison du calcul des hash */
            EVP_DigestFinal_ex(&md5_ctx, a_chunk_hash->hash_md5 , &len_md5);
            EVP_DigestFinal_ex(&sha1_ctx, a_chunk_hash->hash_sha1, &len_sha1);
            EVP_DigestFinal_ex(&ripemd_ctx, a_chunk_hash->hash_ripemd, &len_ripemd);

            a_chunk_hash->position = *position;
            a_chunk_hash->len_md5 = (guint8) len_md5;
            a_chunk_hash->len_sha1 = (guint8) len_sha1;
            a_chunk_hash->len_ripemd = (guint8) len_ripemd;

            chunk_hashs = g_slist_prepend(chunk_hashs, a_chunk_hash);
        }

        return chunk_hashs;

}


/**
 *  Produit les hashs d'un fichier donné (filename)
 *  MD5, SHA1 et RIPEMD160 en une seule passe
 */
file_hash_t *hash_a_file(p_bar_t *pb, gchar *filename)
{
    file_hash_t *the_hash = NULL;
    const EVP_MD *md5 = NULL;
    const EVP_MD *sha1 = NULL;
    const EVP_MD *ripemd = NULL;
    EVP_MD_CTX md5_ctx;
    EVP_MD_CTX sha1_ctx;
    EVP_MD_CTX ripemd_ctx;
    unsigned int len_md5 = 0;
    unsigned int len_sha1 = 0;
    unsigned int len_ripemd = 0;
    guchar *buffer = NULL;
    FILE *fp = NULL;
    size_t lus = 0;
    guint64 position = 0;
    GSList *chunk_hashs = NULL;
    chunk_t *file_hashs = NULL;

    fp = open_file_if_it_exists(filename);

    if (fp != NULL)
        {
            pb->max_file = file_size(filename);
            pb->value_file = 0;
            refresh_file_progress_bar(pb);

            the_hash = (file_hash_t *) g_malloc0(sizeof(file_hash_t));
            file_hashs = (chunk_t *) g_malloc0(sizeof(chunk_t));
            buffer = (guchar *) g_malloc0(FILE_IO_BUFFER_SIZE);

            the_hash->hashset = NULL;
            the_hash->filename = g_strdup(filename);
            the_hash->file_hashs = file_hashs;

            the_hash->file_hashs->position = -1; /* pour l'ensemble du fichier */
            the_hash->file_hashs->hash_md5 = (guchar *) g_malloc0(EVP_MAX_MD_SIZE);
            the_hash->file_hashs->hash_sha1 = (guchar *) g_malloc0(EVP_MAX_MD_SIZE);
            the_hash->file_hashs->hash_ripemd = (guchar *) g_malloc0(EVP_MAX_MD_SIZE);

            /* Initialisation des contextes */
            EVP_MD_CTX_init(&md5_ctx);
            EVP_MD_CTX_init(&sha1_ctx);
            EVP_MD_CTX_init(&ripemd_ctx);

            /* Initialisations des hash */
            md5 = EVP_get_digestbyname("md5");
            EVP_DigestInit_ex(&md5_ctx, md5, NULL);
            sha1 = EVP_get_digestbyname("sha1");
            EVP_DigestInit_ex(&sha1_ctx, sha1, NULL);
            ripemd = EVP_get_digestbyname("ripemd160");
            EVP_DigestInit_ex(&ripemd_ctx, ripemd, NULL);

            position = 0;

            /* Lecture du fichier */
            while (!feof(fp))
                {
                    lus = fread(buffer, 1, FILE_IO_BUFFER_SIZE, fp);

                    /* on ne fait un refresh que pour les fichiers un peu gros (>100 * le buffer de lecture) */
                    if (pb->max_file > (100*FILE_IO_BUFFER_SIZE))
                        {
                            pb->value_file += lus;
                            refresh_file_progress_bar(pb);
                        }

                    chunk_hashs = do_chunks_from_buffer(buffer, lus, &position, chunk_hashs);

                    EVP_DigestUpdate(&md5_ctx, buffer, lus);
                    EVP_DigestUpdate(&sha1_ctx, buffer, lus);
                    EVP_DigestUpdate(&ripemd_ctx, buffer, lus);
                }

            /* Terminaison du calcul des hash */
            EVP_DigestFinal_ex(&md5_ctx, the_hash->file_hashs->hash_md5 , &len_md5);
            EVP_DigestFinal_ex(&sha1_ctx, the_hash->file_hashs->hash_sha1, &len_sha1);
            EVP_DigestFinal_ex(&ripemd_ctx, the_hash->file_hashs->hash_ripemd, &len_ripemd);

            the_hash->file_hashs->len_md5 = len_md5;
            the_hash->file_hashs->len_sha1 = len_sha1;
            the_hash->file_hashs->len_ripemd = len_ripemd;

            the_hash->chunk_hashs = chunk_hashs;

            /* Nettoyage des contextes */
            EVP_MD_CTX_cleanup(&md5_ctx);
            EVP_MD_CTX_cleanup(&sha1_ctx);
            EVP_MD_CTX_cleanup(&ripemd_ctx);

            fclose(fp);
            g_free(buffer);

            return the_hash;
        }
    else
        {
            return NULL;
        }
}


/**
 *  Traverse l'intégralité d'un dossier
 *  récursivement. Rempli la liste file_liste
 *  avec les noms des fichiers trouvés
 *  Avant d'appeler la fonction il faut être certain d'avoir initialisé
 *  la barre de progression
 */
GSList *traverse_un_dossier(p_bar_t *pb, gchar *dirname, GSList *file_list)
{
    GDir *my_dir = NULL;
    const gchar *filename = NULL;
    gchar *directory_name = NULL;
    GError *error = NULL ;

    my_dir = g_dir_open(dirname, 0, &error);

    if (my_dir != NULL)
        {
            filename = g_dir_read_name(my_dir);

            while (filename != NULL)
                {
                    pulse_the_progress_bar(pb);
                    directory_name = g_build_filename(dirname, filename, NULL);

                    if (g_file_test(directory_name, G_FILE_TEST_IS_DIR) &&
                        !g_file_test(directory_name, G_FILE_TEST_IS_SYMLINK))
                        {
                            file_list = traverse_un_dossier(pb, directory_name, file_list);
                            g_free(directory_name);
                        }
                    else
                        {
                            pb->max++;
                            file_list = g_slist_prepend(file_list, directory_name);
                        }

                    filename = g_dir_read_name(my_dir);
                }
            g_dir_close(my_dir);
        }

    if (error != NULL)
        {
            fprintf(stdout, "%s\n", error->message);
        }

    return file_list;
}


/**
 *  Traverse un dossier pour en hasher le contenu
 *  Retourne une liste de fichiers hashés (liste de file_hash_t)
 */
GSList *hash_a_directory(main_struct_t *main_struct, gchar *dirname)
{
    GSList *file_list = NULL;
    GSList *head = NULL;
    GSList *file_hash_list = main_struct->file_hash_list; /* Ce qui permet de hacher plusieurs répertoires */
    file_hash_t *file_hash = NULL;
    p_bar_t *pb = NULL;
    gchar *new_filename = NULL;
    guint64 max = 0;
    guint64 integres = 0;

    /* Progress bar stuff */
    init_progress_window(main_struct);
    pb = init_progress_bar(main_struct->pb, 0, 0);  /* pulse mode */

    /* récupération de la liste des fichiers à hacher */
    file_list = traverse_un_dossier(pb, dirname, file_list);
    head = file_list;

    /* Progress bar stuff */
    max = g_slist_length(file_list);
    pb = init_progress_bar(pb, max, 0); /* normal mode */

    while (file_list != NULL)
        {
            file_hash = hash_a_file(pb, (gchar *)file_list->data);

            if (file_hash != NULL)
                {
                    /* Si on ne veux que le chemin relatif */
                    if (main_struct->opts->include_dir == FALSE)
                        {
                            new_filename = make_relative_filename_from_dir(dirname, file_hash->filename);
                            g_free(file_hash->filename);
                            file_hash->filename = new_filename;
                        }

                    if (main_struct->opts->genere_hashs_vides == TRUE)
                        {
                            /* On intégre le hash dans la liste, quel qu'il soit */
                            file_hash_list = g_slist_prepend(file_hash_list, file_hash);
                            integres++;
                        }
                    else
                        {
                            /* On vire les hashs de fichiers vides */
                            if (is_file_hash_empty(file_hash) != TRUE)
                                {
                                    file_hash_list = g_slist_prepend(file_hash_list, file_hash);
                                    integres++;
                                }
                            else
                                {
                                    free_file_hash(file_hash);
                                }
                        }
                }

            file_list = g_slist_next(file_list);

            /* Refreshing the progress bar */
            pb->value++;  /* TODO : ecrire une fonction pour ça */
            refresh_progress_bar(pb);
        }

    /* Ending things with the progress bar window */
    end_progress_window(main_struct);

    ldt_log_message(main_struct->log, G_LOG_LEVEL_INFO, "Répertoire %s haché : %Ld fichier(s) et %Ld hash(s) intégrés", dirname, max, integres);
    g_slist_free(head);

    return file_hash_list;
}

/**
 *  Prepare et retourne la chaine de caractère à inscrire dans le
 *  fichier
 *  Quand save_hashset est à TRUE c'est qu'on sauvegarde la liste
 *  des fichiers connus sinon, c'est soit la liste des hashs, soit
 *  la liste des fichiers inconnus
 */
static gchar *prepare_le_buffer(void *hash, gboolean save_hashset, options_t *opts)
{
    gchar *buf = NULL;
    guchar *md5 = NULL;
    guchar *sha1 = NULL;
    guchar *ripemd = NULL;

    if (save_hashset == TRUE)
        {
            /* La liste des fichiers connus */
            result_hash_t *file_hash = NULL;
            file_hash = (result_hash_t *) hash;
            md5 = transforme_le_hash_de_binaire_en_hex(file_hash->file_hashs->hash_md5, file_hash->file_hashs->len_md5);
            sha1 = transforme_le_hash_de_binaire_en_hex(file_hash->file_hashs->hash_sha1, file_hash->file_hashs->len_sha1);
            ripemd = transforme_le_hash_de_binaire_en_hex(file_hash->file_hashs->hash_ripemd, file_hash->file_hashs->len_ripemd);

            /* Enregistrement en fonction des options */
            if (opts->include_hashset_name == TRUE && opts->include_hashset_file_filename == TRUE)
                {
                    buf = g_strdup_printf("%s%c%s%c%s%c%Ld%c%s%c%s%c%s%c", file_hash->hashset_name, '\t', file_hash->hashset_file_filename, '\t', file_hash->filename, '\t', file_hash->file_hashs->position, '\t', md5, '\t', sha1, '\t', ripemd, '\n');
                }
            else if (opts->include_hashset_name == TRUE)
                {
                    buf = g_strdup_printf("%s%c%s%c%Ld%c%s%c%s%c%s%c", file_hash->hashset_name, '\t', file_hash->filename, '\t', file_hash->file_hashs->position, '\t', md5, '\t', sha1, '\t', ripemd, '\n');
                }
            else if (opts->include_hashset_file_filename == TRUE)
                {
                    buf = g_strdup_printf("%s%c%s%c%Ld%c%s%c%s%c%s%c", file_hash->hashset_file_filename, '\t', file_hash->filename, '\t', file_hash->file_hashs->position, '\t', md5, '\t', sha1, '\t', ripemd, '\n');
                }
            else
                {
                    buf = g_strdup_printf("%s%c%Ld%c%s%c%s%c%s%c", file_hash->filename, '\t', file_hash->file_hashs->position, '\t',  md5, '\t', sha1, '\t', ripemd, '\n');
                }

            g_free(md5);
            g_free(sha1);
            g_free(ripemd);
        }
    else
        {
            /* La liste des fichiers inconnus */
            file_hash_t *file_hash = (file_hash_t *) hash;
            chunk_t *chunk_hash = NULL;
            GSList *list = NULL;
            gchar *buf2 = NULL;
            gchar *buf3 = NULL;

            md5 = transforme_le_hash_de_binaire_en_hex(file_hash->file_hashs->hash_md5, file_hash->file_hashs->len_md5);
            sha1 = transforme_le_hash_de_binaire_en_hex(file_hash->file_hashs->hash_sha1, file_hash->file_hashs->len_sha1);
            ripemd = transforme_le_hash_de_binaire_en_hex(file_hash->file_hashs->hash_ripemd, file_hash->file_hashs->len_ripemd);

            buf = g_strdup_printf("%s%c%Ld%c%s%c%s%c%s%c", file_hash->filename, '\t', file_hash->file_hashs->position, '\t', md5, '\t', sha1, '\t', ripemd, '\n');

            g_free(md5);
            g_free(sha1);
            g_free(ripemd);

            list = file_hash->chunk_hashs;

            while (list)
                {
                    chunk_hash = (chunk_t *) list->data;
                    md5 = transforme_le_hash_de_binaire_en_hex(chunk_hash->hash_md5, chunk_hash->len_md5);
                    sha1 = transforme_le_hash_de_binaire_en_hex(chunk_hash->hash_sha1, chunk_hash->len_sha1);
                    ripemd = transforme_le_hash_de_binaire_en_hex(chunk_hash->hash_ripemd, chunk_hash->len_ripemd);

                    buf2 = g_strdup_printf("%s%c%Ld%c%s%c%s%c%s%c", file_hash->filename, '\t', chunk_hash->position, '\t', md5, '\t', sha1, '\t', ripemd, '\n');

                    buf3 = g_strconcat(buf, buf2, NULL);

                    g_free(buf);
                    g_free(buf2);
                    g_free(md5);
                    g_free(sha1);
                    g_free(ripemd);

                    buf = buf3;
                    list = g_slist_next(list);
                }
        }

    return buf;
}


/**
 *  Sauvegarde les hashs. Format du fichier :
 *  nom_du_fichier md5 sha1 ripemd160
 *  séparés par une tabulation et terminé par un caractère \n
 *  Limitation : un fichier ne doit pas avoir de tabulation
 *               ni le caractère \n dans son nom.
 *  Lors de la sauvegarde du résultat de la comparaison, on aimerai
 *  notamment savoir de quel hashset ils sont issus. Donc lorsque
 *  save_hashset == TRUE on sauvegarde, en début de ligne le nom du hashset et
 *                       le nom du fichier
 */
bzip2_result_t *save_the_file_hash_list(GSList *file_hash_list, gchar *filename, gboolean save_hashset, options_t *opts)
{
    FILE *fp = NULL;
    file_hash_t *file_hash = NULL;
    int bzerror = 0;
    BZFILE *compressed = NULL;
    gchar *buf = NULL;
    guint buf_len = 0;
    unsigned int nbytes_in = 0;
    unsigned int nbytes_out = 0;
    unsigned int nb_hash = 0;
    bzip2_result_t *res = NULL;

    fp = g_fopen(filename, "wb");

    compressed = BZ2_bzWriteOpen(&bzerror, fp, 9, 0, 30);

    while (file_hash_list != NULL)
        {
            file_hash = (file_hash_t *) file_hash_list->data;
            if (file_hash != NULL)
                {
                    buf = prepare_le_buffer(file_hash, save_hashset, opts);

                    buf_len = strlen(buf);
                    BZ2_bzWrite(&bzerror, compressed, buf, buf_len);
                    nb_hash++;

                    g_free(buf);
                }
            file_hash_list = g_slist_next(file_hash_list);
        }

    BZ2_bzWriteClose(&bzerror, compressed, 0, &nbytes_in, &nbytes_out );

    res = (bzip2_result_t *) g_malloc0(sizeof(bzip2_result_t));
    res->uncompressed = nbytes_in;
    res->compressed = nbytes_out;
    res->nb_hash = nb_hash;

    fclose(fp);

    return res;
}


/**
 *  Sauvegarde l'arbre en entier
 *  TODO : libérer de la mémoire la liste !!
 */
bzip2_result_t *save_the_hashsets(main_struct_t *main_struct, gchar *filename)
{
    GSList *file_hash_list = NULL;

    file_hash_list = transforme_tronc_en_liste(main_struct->tronc);

    return save_the_file_hash_list(file_hash_list, filename, FALSE, main_struct->opts);
}


/**
 *  Rempli la structure file_hash_t en lisant une seule ligne
 *  dans le buffer
 */
static file_hash_t *new_from_buffer_line(gchar *buf, int lus, guint n, hashset_t *hashset)
{
    file_hash_t *file_hash = NULL;

    guint j = 0;  /* position des \t dans la chaine            */
    guint l = 0;  /* position des \t précédente dans la chaine */
    guchar *md5 = NULL;
    guchar *sha1 = NULL;
    guchar *ripemd = NULL;
    gchar *filename = NULL;
    gchar *str_pos = NULL;
    long long int pos = 0;

    file_hash = (file_hash_t *) g_malloc0(sizeof(file_hash_t));
    file_hash->file_hashs = (chunk_t *) g_malloc0(sizeof(chunk_t));

    file_hash->hashset = hashset;

    /* récupération du nom de fichier */
    j = 0;
    while (buf[n+j] != '\t' && n+j <lus)
        {
            j++;
        }
    filename = (gchar *) g_malloc0(j+1);
    memcpy(filename, buf+n, j);
    filename[j] = (gchar) 0;
    file_hash->filename = filename;

    /* Récupération de la position */
    l = j+1;
    j = 0;
    while (buf[n+l+j] != '\t' && n+l+j <lus)
        {
            j++;
        }
    str_pos = (gchar *) g_malloc0(j+1);
    memcpy(str_pos, buf+n+l, j);
    str_pos[j] = (gchar) 0;
    sscanf(str_pos, "%Ld", &pos);
    file_hash->file_hashs->position = (gint64) pos;
    g_free(str_pos);

    /* récupération du md5 */
    l += j+1;
    j = 0;
    while (buf[n+l+j] != '\t' && n+l+j <lus)
        {
            j++;
        }
    md5 = (guchar *) g_malloc0(j+1);
    memcpy(md5, buf+n+l, j);
    md5[j] = (guchar) 0;
    file_hash->file_hashs->hash_md5 = transforme_le_hash_de_hex_en_binaire(md5);
    file_hash->file_hashs->len_md5 = j/2;
    g_free(md5);

    /* récupération du sha1 */
    l += j+1;
    j = 0;
    while (buf[n+l+j] != '\t' && n+l+j <lus)
        {
            j++;
        }
    sha1 = (guchar *) g_malloc0(j+1);
    memcpy(sha1, buf+n+l, j);
    sha1[j] = (guchar) 0;
    file_hash->file_hashs->hash_sha1 = transforme_le_hash_de_hex_en_binaire(sha1);
    file_hash->file_hashs->len_sha1 = j/2;
    g_free(sha1);

    /* récupération du ripemd */
    l += j+1;
    j = 0;
    while (buf[n+l+j] != '\n' && n+l+j <lus)
        {
            j++;
        }
    ripemd = (guchar *) g_malloc0(j+1);
    memcpy(ripemd, buf+n+l, j);
    ripemd[j] = (guchar) 0;
    file_hash->file_hashs->hash_ripemd = transforme_le_hash_de_hex_en_binaire(ripemd);
    file_hash->file_hashs->len_ripemd = j/2;
    g_free(ripemd);

    return file_hash;
}

/**
 *  Rempli la structure chunk_t en lisant une seule ligne dans le buffer
 */
static chunk_t *new_chunk_from_buffer_line(gchar *buf, int lus, guint n)
{
    chunk_t *chunk_hash = NULL;

    guint j = 0;  /* position des \t dans la chaine            */
    guint l = 0;  /* position des \t précédente dans la chaine */
    guchar *md5 = NULL;
    guchar *sha1 = NULL;
    guchar *ripemd = NULL;
    gchar *str_pos = NULL;
    long long int pos = 0;

    chunk_hash = (chunk_t *) g_malloc0(sizeof(chunk_t));

    /* récupération du nom de fichier */
    j = 0;
    while (buf[n+j] != '\t' && n+j <lus)
        {
            j++;
        }

    /* Récupération de la position */
    l = j+1;
    j = 0;
    while (buf[n+l+j] != '\t' && n+l+j <lus)
        {
            j++;
        }
    str_pos = (gchar *) g_malloc0(j+1);
    memcpy(str_pos, buf+n+l, j);
    str_pos[j] = (gchar) 0;
    sscanf(str_pos, "%Ld", &pos);
    chunk_hash->position = (gint64) pos;
    g_free(str_pos);

    /* récupération du md5 */
    l += j+1;
    j = 0;
    while (buf[n+l+j] != '\t' && n+l+j <lus)
        {
            j++;
        }
    md5 = (guchar *) g_malloc0(j+1);
    memcpy(md5, buf+n+l, j);
    md5[j] = (guchar) 0;
    chunk_hash->hash_md5 = transforme_le_hash_de_hex_en_binaire(md5);
    chunk_hash->len_md5 = j/2;
    g_free(md5);

    /* récupération du sha1 */
    l += j+1;
    j = 0;
    while (buf[n+l+j] != '\t' && n+l+j <lus)
        {
            j++;
        }
    sha1 = (guchar *) g_malloc0(j+1);
    memcpy(sha1, buf+n+l, j);
    sha1[j] = (guchar) 0;
    chunk_hash->hash_sha1 = transforme_le_hash_de_hex_en_binaire(sha1);
    chunk_hash->len_sha1 = j/2;
    g_free(sha1);

    /* récupération du ripemd */
    l += j+1;
    j = 0;
    while (buf[n+l+j] != '\n' && n+l+j <lus)
        {
            j++;
        }
    ripemd = (guchar *) g_malloc0(j+1);
    memcpy(ripemd, buf+n+l, j);
    ripemd[j] = (guchar) 0;
    chunk_hash->hash_ripemd = transforme_le_hash_de_hex_en_binaire(ripemd);
    chunk_hash->len_ripemd = j/2;
    g_free(ripemd);

    return chunk_hash;
}


/**
 *  Ajoute dans la liste file_hash_list les éléments
 *  contenus dans un bloc de buffer "buf"
 *  retourne le premier caractère d'une chaine non entière
 */
static int compute_one_block(GSList **file_hash_list, gchar *buf, int lus, hashset_t *hashset, bzip2_result_t *compte, options_t *opts, gboolean *load_chunks)
{
    guint n = 0;  /* position du début de la chaine            */
    guint i = 0;  /* position de la fin de la chaine           */
    file_hash_t *file_hash = NULL;
    chunk_t *a_chunk = NULL;

    while (n+i < lus-1) /* < */
        {
            /* recherche de la fin de la première ligne (TODO : détecter un \n dans un nom de fichier) */
            while (buf[n+i] != '\n' && n+i < lus-1)  /* < */
                {
                    i++;
                }


            if (buf[n+i] == '\n' && n+i <= lus-1)  /* Si on est arrivé avant la fin du bloc */
                {
                    if (*load_chunks == TRUE)
                        {
                            a_chunk = new_chunk_from_buffer_line(buf, lus, n);
                            file_hash->chunk_hashs = g_slist_prepend(file_hash->chunk_hashs, a_chunk); /* lecture d'une ligne */
                            n += i+1;
                            i = 0;
                            if (a_chunk->position == 1)
                                {
                                        *load_chunks = FALSE;
                                }
                        }
                    else
                        {

                            file_hash = new_from_buffer_line(buf, lus, n, hashset); /* lecture d'une ligne */
                            n += i+1; /* la position du \n +1 */
                            i = 0;
                            if (file_hash->file_hashs->position == -1)
                                {
                                    *load_chunks = TRUE;
                                    file_hash->chunk_hashs = NULL;
                                }

                            /* insertion dans la liste des fichiers qui sera retournée */

                            if (opts->charger_fv_hashsets == TRUE)
                                {
                                    /* On charge tous les hashs indistinctement */
                                    *file_hash_list =  g_slist_prepend(*file_hash_list, file_hash);
                                    compte->nb_hash++;
                                    hashset->refs++;
                                }
                            else
                                {
                                    /* On ne charge que les hashs issus des fichiers non vides */
                                    if (is_file_hash_empty(file_hash) != TRUE)
                                        {
                                            /* Si les hash ne sont pas issus d'un fichier vide */
                                            *file_hash_list =  g_slist_prepend(*file_hash_list, file_hash);
                                            compte->nb_hash++;
                                            hashset->refs++;
                                        }
                                    else
                                        {
                                            free_file_hash(file_hash);
                                        }
                                }
                        }
                }
        }

    if (buf[n+i] != '\n')
        {
            return n;
        }
    else
        {
            return lus;
        }
}


/**
 *  Extrait dirname de filename pour retourner uniquement la
 *  partie basse (relative à dirname)
 */
static gchar *make_relative_filename_from_dir(gchar *dirname, gchar *filename)
{
    guint dirname_len = 0;
    guint filename_len = 0;
    guint rel_len = 0;
    gchar *relative = NULL;

    dirname_len = strlen(dirname);
    filename_len = strlen(filename);
    rel_len = filename_len - dirname_len;

    if (filename_len > dirname_len)
        {
            relative = (gchar *) g_malloc0(rel_len + 1);
            memcpy(relative, filename + dirname_len + 1, rel_len);
            relative[rel_len] = '\0';

            return relative;
        }
    else
        {
            return g_strdup(filename);
        }
}


/**
 *  Charge un seul fichier (au format bzip2) dans une
 *  liste qu'il retourne
 */
GSList *load_one_file(main_struct_t *main_struct, gchar* dirname, gchar *filename, bzip2_result_t *compte)
{
    FILE *fp = NULL;
    int bzerror;
    BZFILE *compressed = NULL;
    gchar *buf = NULL;
    guint buf_len = FILE_IO_BUFFER_SIZE;
    int lus = 0;
    int faits = 0; /* pour détecter les effets de bords */
    int deja_lus = 0; /* les octets du buffer précédent */
    hashset_t *hashset = NULL;
    GSList *file_hash_list = NULL;
    gboolean stop = FALSE;
    p_bar_t *pb = main_struct->pb;
    gboolean load_chunks = FALSE;

    buf = (gchar *) g_malloc0(buf_len);
    hashset = (hashset_t *) g_malloc0(sizeof(hashset_t));

    fp = open_file_if_it_exists(filename);

    if (fp != NULL)
        {
            pb->max_file = file_size(filename);
            pb->value_file = 0;

            hashset->name = make_relative_filename_from_dir(dirname, filename);
            hashset->refs = 1;

            compressed  = BZ2_bzReadOpen(&bzerror, fp, 0, 0, NULL, 0);
            lus = BZ2_bzRead (&bzerror, compressed, buf, buf_len);

            if (lus > 0)
                {
                    pb->value_file += lus;
                    faits = compute_one_block(&file_hash_list, buf, lus, hashset, compte, main_struct->opts, &load_chunks);
                    stop = FALSE;
                    refresh_file_progress_bar(pb);
                }
            else
                {
                    faits = lus; /* <= 0 */
                    stop = FALSE;
                }

            /* Gestion des effets de bords */
            if (faits < lus)
                {
                    deja_lus = lus - faits;
                    memmove(buf, buf+faits, deja_lus);
                }

            while (stop == FALSE)
                {
                    lus = BZ2_bzRead(&bzerror, compressed, buf+deja_lus, buf_len-deja_lus);

                    if (bzerror == BZ_STREAM_END || bzerror == BZ_OK)
                        {
                            if (lus > 0)
                                {
                                    pb->value_file += lus;
                                    faits = compute_one_block(&file_hash_list, buf, lus+deja_lus, hashset, compte, main_struct->opts, &load_chunks);
                                    /* Gestion des effets de bords */
                                    if (faits < lus+deja_lus)
                                        {
                                            deja_lus = (lus+deja_lus) - faits;
                                            memmove(buf, buf+faits, deja_lus);
                                        }
                                    refresh_file_progress_bar(pb);
                                }
                            else
                                {
                                    stop = TRUE;
                                }
                        }
                    else
                        {
                            if (bzerror != BZ_SEQUENCE_ERROR)
                                {
                                    ldt_log_message(main_struct->log, G_LOG_LEVEL_WARNING, "Erreur %d  dans le hashset %s", bzerror, hashset->name);
                                }
                            stop = TRUE;
                        }
                }

            BZ2_bzReadClose(&bzerror, compressed);

            fclose(fp);
            ldt_log_message(main_struct->log, G_LOG_LEVEL_INFO, "Hashset %s intégré (%d hashs)", hashset->name, compte->nb_hash);

            /* on ne libère pas hashset car il est intégré dans la structure file_hash_list ! */
        }

    g_free(buf);

    return file_hash_list;
}


/**
 *  Charge un hashset : un ensemble de fichiers bzip2 contenus
 *  dans un répertoire (et ses sous répertoires)
 *  Chaque fichier est considéré comme un hashset
 */
void load_a_complete_directory(main_struct_t *main_struct, gchar *dirname)
{
    GSList *file_list = NULL;
    GSList *head = NULL;
    guint64 max = 0;                 /* Nombre de fichiers à charger */
    guint64 total = 0;               /* Nombre total de hashs chargés */
    GSList *file_hash_list = NULL;
    gchar *filename = NULL;
    p_bar_t *pb = NULL;
    bzip2_result_t *compte = NULL; /* pour éviter deux appels couteux à g_slist_length(file_hash_list) */

    compte = (bzip2_result_t *) g_malloc0(sizeof(bzip2_result_t));
    compte->nb_hash = 0;
    compte->compressed = 0;
    compte->uncompressed = 0;

    /* Progress bar stuff */
    init_progress_window(main_struct);
    pb = init_progress_bar(main_struct->pb, 0, 0);  /* pulse mode */

    /* récupération de la liste des fichiers à charger */
    file_list = traverse_un_dossier(pb, dirname, file_list);
    head = file_list;

    /* Progress bar */
    max = g_slist_length(file_list);
    pb = init_progress_bar(pb, max, 0); /* normal mode */

    if (main_struct->tronc == NULL &&
        main_struct->opts->nb_indirections >= GCH_NB_INDIRECT_MIN &&
        main_struct->opts->nb_indirections <= GCH_NB_INDIRECT_MAX)
        {
            ldt_log_message(main_struct->log, G_LOG_LEVEL_INFO, "Création de la structure avec %d niveaux", main_struct->opts->nb_indirections);
            main_struct->tronc = nouveau_tronc(main_struct->opts->nb_indirections); /* n niveaux d'indirection -> 256^(n-1) listes !*/
        }
    else if (main_struct->tronc == NULL)
        {
            ldt_log_message(main_struct->log, G_LOG_LEVEL_INFO, "Création de la structure avec %d niveaux", GCH_NB_INDIRECT);
            main_struct->tronc = nouveau_tronc(GCH_NB_INDIRECT); /* valeur par défaut */
        }

    while (file_list != NULL)
        {
            filename = (gchar *) file_list->data;

            if (filename != NULL)
                {
                    file_hash_list = load_one_file(main_struct, dirname, filename, compte); /* g_strdup(filename) */

                    total += compte->nb_hash;
                    compte->nb_hash = 0;

                    insere_la_liste_dans_tronc(main_struct->tronc, file_hash_list, main_struct->opts->hash_type);

                    g_slist_free(file_hash_list);

                    /* Refreshing the progress bar */
                    pb->value++;
                    refresh_progress_bar(pb);
                }

            file_list = g_slist_next(file_list);
        }

    /* Ending things with the progress bar window */
    end_progress_window(main_struct);

    ldt_log_message(main_struct->log, G_LOG_LEVEL_INFO, "%Ld fichier(s) intégré(s) soit %Ld hashs", max, total);

    /* Libération de mémoire */
    g_slist_free(head); /* Ici ça peut prendre du temps */
    g_free(compte);
}
