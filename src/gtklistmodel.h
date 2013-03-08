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
#include <elementals.h>
 
#define GTK_LIST_TYPE_MODEL             (gtk_list_model_get_type ())
#define GTK_LIST_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_LIST_TYPE_MODEL, GtkListModel))
#define GTK_LIST_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GTK_LIST_TYPE_MODEL, GtkListModelClass))
#define GTK_LIST_IS_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_LIST_TYPE_MODEL))
#define GTK_LIST_IS_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GTK_LIST_TYPE_MODEL))
#define GTK_LIST_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GTK_LIST_TYPE_MODEL, GtkListModelClass))
 
typedef struct _GtkListModel        GtkListModel;
typedef struct _GtkListModelClass   GtkListModelClass;
 
DECLARE_EL_ARRAY(gtklistmodel_index, int);

struct _GtkListModel
{
  GObject         parent;      /* this MUST be the first member */
 
  gtklistmodel_index index;
  gboolean (*filter_out_func) (void* data, int row);
  gboolean change_transaction;
  int (*sort_cmp)(void* data, int row1, int row2);
  

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

void gtk_list_model_begin_change(GtkListModel* model);
void gtk_list_model_commit_change(GtkListModel* model);

void gtk_list_model_set_filter(GtkListModel* model,
                               gboolean (*filter_out_func)(void* data, int row)
                               );
void gtk_list_model_refilter(GtkListModel* model);

int gtk_list_model_iter_to_row(GtkListModel* model, GtkTreeIter iter);
void gtk_list_model_row_to_iter(GtkListModel* model, int row, GtkTreeIter *iter);

typedef void(*GtkListModelIterFunc)(void*, int, void*);

void gtk_list_model_iterate_with_func(GtkListModel* model, void (*f)(void* data, int row, void* f_data), void* f_data);

typedef int(*GtkListModelSortFunc)(void*, int, int);
void gtk_list_model_sort(GtkListModel* model, int (*cmp)(void* data, int row1, int row2));

#endif

