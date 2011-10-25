/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   genhashs.h
   Projet GtkCmpHash

   (C) Copyright 2011 Olivier Delhomme
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

#include <stdio.h>
#include <gcrypt.h>
#include <glib.h>


typedef struct
{
    gcry_md_hd_t md5;        /* Message is 16 bytes long */
    gcry_md_hd_t sha1;       /* Message is 20 bytes long */
    gcry_md_hd_t rmd160;     /* Message is 20 bytes long */
    gcry_md_hd_t tiger;      /* Message is 24 bytes long */
    gcry_md_hd_t sha384;     /* Message is 48 bytes long */
    gcry_md_hd_t whirlpool;  /* Message is 64 bytes long */
                              /* Total is  192 bytes long */
} hash_context_t;
