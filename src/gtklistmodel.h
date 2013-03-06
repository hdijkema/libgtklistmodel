/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * lib.h
 * Copyright (C) 2013 Hans Oesterholt <debian@oesterholt.net>
 * 
 * libgtklistmodel is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libgtklistmodel is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */

#ifndef GTK_LIST_MODEL_HOD
#define GTK_LIST_MODEL_HOD
 
#include <gtk/gtk.h>
 
#define GTK_LIST_TYPE_MODEL             (gtk_list_model_get_type ())
#define GTK_LIST_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_LIST_TYPE_MODEL, GtkListModel))
#define GTK_LIST_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GTK_LIST_TYPE_MODEL, GtkListModelClass))
#define GTK_LIST_IS_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_LIST_TYPE_MODEL))
#define GTK_LIST_IS_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GTK_LIST_TYPE_MODEL))
#define GTK_LIST_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GTK_LIST_TYPE_MODEL, GtkListModelClass))
 
typedef struct _GtkListModel        GtkListModel;
typedef struct _GtkListModelClass   GtkListModelClass;
 
struct _GtkListModel
{
  GObject         parent;      /* this MUST be the first member */
 
  int     (*n_columns)  (void* data);
  GType   (*column_type)(void* data, int col);
  int     (*n_rows)     (void* data);
  void    (*cell_value) (void* data, int row, int col, GValue* val);
 
  void* data;
  
  gint            stamp;       /* Random integer to check whether an iter belongs to our model */
};
 
 
struct _GtkListModelClass
{
  GObjectClass parent_class;
};
 
 
GType             gtk_list_model_get_type (void);
GtkListModel     *gtk_list_model_new (void* data,
                                      int (*n_columns)(void* data),
                                      GType (*column_type)(void* data, int col),
                                      int (*n_rows)(void* data),
                                      void (*cell_value)(void* data, int row, int col, GValue* val)
                                      );

void gtk_list_model_destroy(GtkListModel* model);

void gtk_list_model_row_changed(GtkListModel* model, int row);
void gtk_list_model_row_deleted(GtkListModel* model, int row);
void gtk_list_model_row_inserted(GtkListModel* model, int row);
 
#endif

