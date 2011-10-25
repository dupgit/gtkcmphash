/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   genhashs.c
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

#include "genhashs.h"

static hash_context_t *init_hash_algorithms(void);
static int init_libgcrypt(void);
static void init_one_hash(gcry_md_hd_t *handle, int algo);


/**
 * Initializes the library libgcrypt. We do not have to protect the memory in
 * any special way because we will only hash some files and we will not use any
 * secret key or cert.
 * @return 0 upon success and 1 upon failure
 */
static int init_libgcrypt(void)
{
    int ret = 0;

    ret = gcry_control(GCRYCTL_SET_THREAD_CBS, NULL);

    if (!gcry_check_version(NULL))
        {
            return 1;
        }
    else
        {
            /* Disable secure memory.  */
            gcry_control (GCRYCTL_DISABLE_SECMEM, 0);

            /* Tell Libgcrypt that initialization has completed. */
            gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

            return 0;
        }
}

/**
 * inits one hash handle and adds it to the available algorithms
 * @param handle : the handle to be initialized
 * @param algo : the libgcrypt constant for the desired algorithm ie
 *               GCRY_MD_MD5 for instance
 */
static init_one_hash(gcry_md_hd_t *handle, int algo)
{
    gcry_md_open(handle, algo, 0);
    gcry_md_enable(*handle, algo);
}


/**
 * Inits all hash algorithms that we will need in the program
 * Here we use TIGER because our version of the libgcrypt is 1.4.4 and does
 * not support TIGER2 algorithm. It will change in the future.
 * @return the initialised structure that contains contexts
 */
static hash_context_t *init_hash_algorithms(void)
{
    hash_context_t *hashs = NULL;


    hashs = (hash_context_t *) g_malloc0(sizeof(hash_context_t));

    init_one_hash(&hashs->md5, GCRY_MD_MD5);
    init_one_hash(&hashs->sha1, GCRY_MD_SHA1);
    init_one_hash(&hashs->rmd160, GCRY_MD_RMD160);
    init_one_hash(&hashs->tiger, GCRY_MD_TIGER);
    init_one_hash(&hashs->sha384, GCRY_MD_SHA384);
    init_one_hash(&hashs->whirlpool, GCRY_MD_WHIRLPOOL);

    return hashs;
}


/**
 * Main program with command line arguments
 */
int main(int argc, char **argv)
{
    int exit_value = 0;
    hash_context_t *hashs = NULL;

    /* First of all things initializing the library libgcrypt */
    exit_value = init_libgcrypt();

    hashs = init_hash_algorithms();

    return exit_value;
}
