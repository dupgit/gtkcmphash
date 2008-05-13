/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */
/*
   progress_window.h
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
#ifndef _PROGRESS_WINDOW_H_
#define _PROGRESS_WINDOW_H_

extern void refresh_progress_bar(p_bar_t *pb);
extern void refresh_file_progress_bar(p_bar_t *pb);
extern p_bar_t *init_progress_bar(p_bar_t *pb, guint64 max, guint64 max_file);
extern void init_progress_window(main_struct_t *main_struct);
extern void end_progress_window(main_struct_t *main_struct);
extern void pulse_the_progress_bar(p_bar_t *pb);

#endif /* _PROGRESS_WINDOW_H_ */
