/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   hashing_struct.h
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
#ifndef _HASHING_STRUCT_H_
#define _HASHING_STRUCT_H_

extern hash_t *nouveau_tronc(guint niveau);
extern void insere_dans_tronc(hash_t *tronc, file_hash_t *file_hash, guint hash_type);
extern void insere_la_liste_dans_tronc(hash_t *tronc, GSList *file_hash_list, guint hash_type);
extern GSList *recherche_dans_tronc(hash_t *tronc, file_hash_t *file_hash, guint hash_type);
extern guchar *transforme_le_hash_de_hex_en_binaire(guchar *hash);
extern guchar *transforme_le_hash_de_binaire_en_hex(guchar *hash, guint len);
extern gboolean is_file_hash_empty(file_hash_t *file_hash);

extern void free_file_hash(file_hash_t *file_hash);
extern void free_file_hash_list(GSList *file_hash_list);
extern void free_result_hash_list(GSList *result_hash_list);
extern void free_tronc(hash_t *tronc);
extern void free_dedans_ou_pas(main_struct_t *main_struct);

extern found_or_not_t *find_all_hashes_from_hashset(GSList *file_hash_list, hash_t *tronc, options_t *opts);
extern GSList *transforme_tronc_en_liste(hash_t *tronc);

extern gint my_g_ascii_strcasecmp(const guchar *s1, const guchar *s2, guint len);
extern guchar *my_g_strdup(const guchar *str, guint length);

extern structure_stat_t *do_stats_tronc(hash_t *tronc);

#endif /* _HASHING_STRUCT_H_ */
