/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   file_io.h
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
#ifndef _FILE_IO_H_
#define _FILE_IO_H_

/**
 *  @def  FILE_IO_BUFFER_SIZE
 * defini la taille du buffer pour l'accès aux fichiers (doit absolument être
 * un multiple de 512 ! Ici : 64 secteurs de 512 octets soit 32768 octets).
 */
#define FILE_IO_BUFFER_SIZE 512*64

extern FILE *open_file_if_it_exists(gchar *filename);
extern guint64 file_size(gchar *filename);
extern GSList *traverse_un_dossier(p_bar_t *pb, gchar *dirname, GSList *file_list);
extern file_hash_t *hash_a_file(p_bar_t *pb, gchar *filename);
extern GSList *hash_a_directory(main_struct_t *main_struct, gchar *dirname);
extern bzip2_result_t *save_the_file_hash_list(GSList *file_hash_list, gchar *filename, gboolean save_hashset, options_t *opts);
extern void load_a_complete_directory(main_struct_t *main_struct, gchar *dirname);
extern GSList *load_one_file(main_struct_t *main_struct, gchar* dirname, gchar *filename, bzip2_result_t *compte);
extern bzip2_result_t *save_the_hashsets(main_struct_t *main_struct, gchar *filename);

#endif /* _FILE_IO_H_ */
