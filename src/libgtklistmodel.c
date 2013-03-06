/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * lib.c
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

#include <gtk/gtk.h>
#include <gtklistmodel.h>

/* Declarations of local functions */
 
static void         gtk_list_model_init            (GtkListModel* pkg_tree);
static void         gtk_list_model_class_init      (GtkListModelClass* klass);
static void         gtk_list_model_tree_model_init (GtkTreeModelIface* iface);

static void         gtk_list_model_finalize        (GObject* object);
 
static GtkTreeModelFlags gtk_list_model_get_flags  (GtkTreeModel* tree_model);
 
static gint         gtk_list_model_get_n_columns   (GtkTreeModel* tree_model);
static GType        gtk_list_model_get_column_type (GtkTreeModel* tree_model, gint index);
 
static gboolean     gtk_list_model_get_iter        (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreePath* path);
static GtkTreePath* gtk_list_model_get_path        (GtkTreeModel* tree_model, GtkTreeIter* iter);
 
static void         gtk_list_model_get_value       (GtkTreeModel* tree_model, GtkTreeIter* iter, gint column, GValue* value);
 
static gboolean     gtk_list_model_iter_next       (GtkTreeModel* tree_model, GtkTreeIter* iter);
static gboolean     gtk_list_model_iter_children   (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* parent);
static gboolean     gtk_list_model_iter_has_child  (GtkTreeModel* tree_model, GtkTreeIter* iter);
static gint         gtk_list_model_iter_n_children (GtkTreeModel* tree_model, GtkTreeIter* iter);
 
static gboolean     gtk_list_model_iter_nth_child  (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* parent, gint n);
static gboolean     gtk_list_model_iter_parent     (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* child);
 
static GObjectClass *parent_class = NULL;  /* GObject stuff - nothing to worry about */
 
#define ITER_SET(iter, row) { char* b = (char*) &iter->user_data; b[0]='A'; \
                              int* p = (int*) &b[1]; *p = row; \
                            }
#define ITER_VALID(iter) (*((char*) &iter->user_data) = 'A')
#define VALID_ITER(iter) ITER_VALID(iter)
#define ITER_GET(iter, row) row = *((int*) &((char*) &iter->user_data)[1])

/*****************************************************************************
 *
 *  gtk_list_model_get_type: here we register our new type and its interfaces
 *                        with the type system. If you want to implement
 *                        additional interfaces like GtkTreeSortable, you
 *                        will need to do it here.
 *
 *****************************************************************************/
 
GType gtk_list_model_get_type (void)
{
  static GType gtk_list_model_type = 0;
 
  /* Some boilerplate type registration stuff */
  if (gtk_list_model_type == 0)
  {
    static const GTypeInfo gtk_list_model_info =
    {
      sizeof (GtkListModelClass),
      NULL,                                         /* base_init */
      NULL,                                         /* base_finalize */
      (GClassInitFunc) gtk_list_model_class_init,
      NULL,                                         /* class finalize */
      NULL,                                         /* class_data */
      sizeof (GtkListModel),
      0,                                           /* n_preallocs */
      (GInstanceInitFunc) gtk_list_model_init
    };
    static const GInterfaceInfo tree_model_info =
    {
      (GInterfaceInitFunc) gtk_list_model_tree_model_init,
      NULL,
      NULL
    };
 
    /* First register the new derived type with the GObject type system */
    gtk_list_model_type = g_type_register_static (G_TYPE_OBJECT, "GtkListModel",
                                               &gtk_list_model_info, (GTypeFlags)0);
 
    /* Now register our GtkTreeModel interface with the type system */
    g_type_add_interface_static (gtk_list_model_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
  }
 
  return gtk_list_model_type;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_class_init: more boilerplate GObject/GType stuff.
 *                          Init callback for the type system,
 *                          called once when our new class is created.
 *
 *****************************************************************************/
 
static void
gtk_list_model_class_init (GtkListModelClass *klass)
{
  GObjectClass *object_class;
 
  parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
  object_class = (GObjectClass*) klass;
 
  object_class->finalize = gtk_list_model_finalize;
}
 
/*****************************************************************************
 *
 *  gtk_list_model_tree_model_init: init callback for the interface registration
 *                               in gtk_list_model_get_type. Here we override
 *                               the GtkTreeModel interface functions that
 *                               we implement.
 *
 *****************************************************************************/
 
static void
gtk_list_model_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags       = gtk_list_model_get_flags;
  iface->get_n_columns   = gtk_list_model_get_n_columns;
  iface->get_column_type = gtk_list_model_get_column_type;
  iface->get_iter        = gtk_list_model_get_iter;
  iface->get_path        = gtk_list_model_get_path;
  iface->get_value       = gtk_list_model_get_value;
  iface->iter_next       = gtk_list_model_iter_next;
  iface->iter_children   = gtk_list_model_iter_children;
  iface->iter_has_child  = gtk_list_model_iter_has_child;
  iface->iter_n_children = gtk_list_model_iter_n_children;
  iface->iter_nth_child  = gtk_list_model_iter_nth_child;
  iface->iter_parent     = gtk_list_model_iter_parent;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_init: this is called everytime a new custom list object
 *                    instance is created (we do that in gtk_list_model_new).
 *                    Initialise the list structure's fields here.
 *
 *****************************************************************************/
 
static int n_columns(void* data) {
  return 0;
}

static GType column_type(void* data, int col) {
  return G_TYPE_INT;
}

static int n_rows(void* data) {
  return 0;
}

void cell_value(void* data, int row, int col, GValue* value)
{
}
 
static void gtk_list_model_init (GtkListModel* model)
{
  model->n_columns = n_columns;
  model->column_type = column_type;
  model->n_rows = n_rows;
  model->cell_value = cell_value;
  model->data = NULL;
  model->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_finalize: this is called just before a custom list is
 *                        destroyed. Free dynamically allocated memory here.
 *
 *****************************************************************************/
 
static void gtk_list_model_finalize (GObject *object)
{
  /* must chain up - finalize parent */
  (* parent_class->finalize) (object);
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_get_flags: tells the rest of the world whether our tree model
 *                         has any special characteristics. In our case,
 *                         we have a list model (instead of a tree), and each
 *                         tree iter is valid as long as the row in question
 *                         exists, as it only contains a pointer to our struct.
 *
 *****************************************************************************/
 
static GtkTreeModelFlags gtk_list_model_get_flags (GtkTreeModel *tree_model)
{
  g_return_val_if_fail (GTK_LIST_IS_MODEL(tree_model), (GtkTreeModelFlags)0);
 
  return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_get_n_columns: tells the rest of the world how many data
 *                             columns we export via the tree model interface
 *
 *****************************************************************************/
 
static gint gtk_list_model_get_n_columns (GtkTreeModel *tree_model)
{
  g_return_val_if_fail (GTK_LIST_IS_MODEL(tree_model), 0);
  return GTK_LIST_MODEL(tree_model)->n_columns(GTK_LIST_MODEL(tree_model)->data);
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_get_column_type: tells the rest of the world which type of
 *                               data an exported model column contains
 *
 *****************************************************************************/
 
static GType gtk_list_model_get_column_type (GtkTreeModel *tree_model, gint index)
{
  g_return_val_if_fail (GTK_LIST_IS_MODEL(tree_model), G_TYPE_INVALID);
  GtkListModel* model = GTK_LIST_MODEL(tree_model);
  g_return_val_if_fail (index < model->n_columns(model->data) && index >= 0, G_TYPE_INVALID);
 
  return model->column_type(model->data, index);
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_get_iter: converts a tree path (physical position) into a
 *                        tree iter structure (the content of the iter
 *                        fields will only be used internally by our model).
 *                        We simply store a pointer to our CustomRecord
 *                        structure that represents that row in the tree iter.
 *
 *****************************************************************************/
 
static gboolean gtk_list_model_get_iter (GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreePath  *path)
{
  GtkListModel* model;
  gint          *indices, n, depth;
 
  g_assert(GTK_LIST_IS_MODEL(tree_model));
  g_assert(path!=NULL);
 
  model = GTK_LIST_MODEL(tree_model);
 
  indices = gtk_tree_path_get_indices(path);
  depth   = gtk_tree_path_get_depth(path);
 
  /* we do not allow children */
  g_assert(depth == 1); /* depth 1 = top level; a list only has top level nodes and no children */
 
  n = indices[0]; /* the n-th top level row */
 
  if ( n >= model->n_rows(model->data) || n < 0 )
    return FALSE;
 
  iter->stamp = model->stamp;
  ITER_SET(iter, n);
 
  return TRUE;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_get_path: converts a tree iter into a tree path (ie. the
 *                        physical position of that row in the list).
 *
 *****************************************************************************/
 
static GtkTreePath* gtk_list_model_get_path (GtkTreeModel *tree_model, GtkTreeIter  *iter)
{
  GtkTreePath*  path;
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL(tree_model), NULL);
  g_return_val_if_fail (iter != NULL, NULL);
  g_return_val_if_fail (VALID_ITER(iter), NULL);
 
  int row;
  ITER_GET(iter, row);
 
  path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, row);
 
  return path;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_get_value: Returns a row's exported data columns
 *                         (_get_value is what gtk_tree_model_get uses)
 *
 *****************************************************************************/
 
static void gtk_list_model_get_value (GtkTreeModel* tree_model, GtkTreeIter* iter, gint column, GValue* value)
{
  GtkListModel* gtk_list_model;
 
  g_return_if_fail (GTK_LIST_IS_MODEL (tree_model));

  gtk_list_model = GTK_LIST_MODEL(tree_model);
  
  g_return_if_fail (iter != NULL);
  g_return_if_fail (column < gtk_list_model->n_columns(gtk_list_model->data));
  g_return_if_fail (VALID_ITER(iter));
 
  g_value_init (value, gtk_list_model->column_type(gtk_list_model->data, column));
 
  int row;
  ITER_GET(iter, row);
 
  g_return_if_fail ( row >= 0 && row < gtk_list_model->n_rows(gtk_list_model->data) );
 
  if (row >= gtk_list_model->n_rows(gtk_list_model->data))
   g_return_if_reached();
 
  gtk_list_model->cell_value(gtk_list_model->data, row, column, value);
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_iter_next: Takes an iter structure and sets it to point
 *                         to the next row.
 *
 *****************************************************************************/
 
static gboolean gtk_list_model_iter_next (GtkTreeModel  *tree_model, GtkTreeIter* iter)
{
  GtkListModel* gtk_list_model;
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), FALSE);
 
  if (iter == NULL || !VALID_ITER(iter))
    return FALSE;
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  int row;
  ITER_GET(iter, row);
 
  /* Is this the last record in the list? */
  if ((row + 1) >= gtk_list_model->n_rows(gtk_list_model->data))
    return FALSE;
 
  row += 1;
 
  iter->stamp = gtk_list_model->stamp;
  ITER_SET(iter, row);
 
  return TRUE;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_iter_children: Returns TRUE or FALSE depending on whether
 *                             the row specified by 'parent' has any children.
 *                             If it has children, then 'iter' is set to
 *                             point to the first child. Special case: if
 *                             'parent' is NULL, then the first top-level
 *                             row should be returned if it exists.
 *
 *****************************************************************************/
 
static gboolean gtk_list_model_iter_children (GtkTreeModel *tree_model, GtkTreeIter* iter, GtkTreeIter* parent)
{
  GtkListModel* gtk_list_model;
 
  g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);
 
  /* this is a list, nodes have no children */
  if (parent)
    return FALSE;
 
  /* parent == NULL is a special case; we need to return the first top-level row */
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), FALSE);
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  /* No rows => no first row */
  if (gtk_list_model->n_rows(gtk_list_model->data) == 0)
    return FALSE;
 
  /* Set iter to first item in list */
  iter->stamp = gtk_list_model->stamp;
  ITER_SET(iter, 0);
 
  return TRUE;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_iter_has_child: Returns TRUE or FALSE depending on whether
 *                              the row specified by 'iter' has any children.
 *                              We only have a list and thus no children.
 *
 *****************************************************************************/
 
static gboolean gtk_list_model_iter_has_child (GtkTreeModel *tree_model, GtkTreeIter* iter)
{
  return FALSE;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_iter_n_children: Returns the number of children the row
 *                               specified by 'iter' has. This is usually 0,
 *                               as we only have a list and thus do not have
 *                               any children to any rows. A special case is
 *                               when 'iter' is NULL, in which case we need
 *                               to return the number of top-level nodes,
 *                               ie. the number of rows in our list.
 *
 *****************************************************************************/
 
static gint gtk_list_model_iter_n_children (GtkTreeModel *tree_model, GtkTreeIter* iter)
{
  GtkListModel* gtk_list_model;
  
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), -1);
  g_return_val_if_fail (iter == NULL || VALID_ITER(iter), FALSE);
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  /* special case: if iter == NULL, return number of top-level rows */
  if (!iter)
    return gtk_list_model->n_rows(gtk_list_model->data);
 
  return 0; /* otherwise, this is easy again for a list */
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_iter_nth_child: If the row specified by 'parent' has any
 *                              children, set 'iter' to the n-th child and
 *                              return TRUE if it exists, otherwise FALSE.
 *                              A special case is when 'parent' is NULL, in
 *                              which case we need to set 'iter' to the n-th
 *                              row if it exists.
 *
 *****************************************************************************/
 
static gboolean gtk_list_model_iter_nth_child (GtkTreeModel *tree_model, GtkTreeIter* iter, GtkTreeIter* parent, gint n)
{
  GtkListModel* gtk_list_model;
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), FALSE);
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  /* a list has only top-level rows */
  if (parent)
    return FALSE;
 
  /* special case: if parent == NULL, set iter to n-th top-level row */
 
  if( n >= gtk_list_model->n_rows(gtk_list_model->data) )
    return FALSE;
 
  if (n < 0) 
    return FALSE;
  
  int row = n;
 
  iter->stamp = gtk_list_model->stamp;
  ITER_SET(iter, row);
 
  return TRUE;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_iter_parent: Point 'iter' to the parent node of 'child'. As
 *                           we have a list and thus no children and no
 *                           parents of children, we can just return FALSE.
 *
 *****************************************************************************/
 
static gboolean gtk_list_model_iter_parent (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* child)
{
  return FALSE;
}
 
 
/*****************************************************************************
 *
 *  gtk_list_model_new:  This is what you use in your own code to create a
 *                    new custom list tree model for you to use.
 *
 *****************************************************************************/
 
GtkListModel *gtk_list_model_new (void* data, 
                                  int (*n_columns)(void* data),
                                  GType (*column_type)(void* data, int col),
                                  int (*n_rows)(void* data),
                                  void (*cell_value)(void* data, int row, int col, GValue* val)
                                  )
{
  GtkListModel *newmodel;
  newmodel = (GtkListModel*) g_object_new (GTK_LIST_TYPE_MODEL, NULL);
  g_assert( newmodel != NULL );
  newmodel->data = data;
  newmodel->n_columns = n_columns;
  newmodel->column_type = column_type;
  newmodel->n_rows = n_rows;
  newmodel->cell_value = cell_value;
  return newmodel;
}
 
void gtk_list_model_destroy(GtkListModel* model)
{
  g_object_unref(model);
}

void gtk_list_model_row_changed(GtkListModel* model, int row)
{
  GtkTreeIter iter;
  gtk_list_model_iter_nth_child (GTK_TREE_MODEL(model), &iter, NULL, row);
  GtkTreePath* path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, row);
  gtk_tree_model_row_changed(GTK_TREE_MODEL(model), path, &iter);
  gtk_tree_path_free(path);
}

void gtk_list_model_row_deleted(GtkListModel* model, int row)
{
  GtkTreeIter iter;
  gtk_list_model_iter_nth_child (GTK_TREE_MODEL(model), &iter, NULL, row);
  GtkTreePath* path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, row);
  gtk_tree_model_row_deleted(GTK_TREE_MODEL(model),path);
  gtk_tree_path_free(path);
}

void gtk_list_model_row_inserted(GtkListModel* model, int row)
{
  GtkTreeIter iter;
  gtk_list_model_iter_nth_child (GTK_TREE_MODEL(model), &iter, NULL, row);
  GtkTreePath* path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, row);
  gtk_tree_model_row_inserted(GTK_TREE_MODEL(model), path, &iter);
  gtk_tree_path_free(path);
}

void gtk_list_model_rows_reordered(GtkListModel* model, gint new_order[])
{
  GtkTreePath* path = gtk_tree_path_new();
  gtk_tree_model_rows_reordered(GTK_TREE_MODEL(model), path, NULL, new_order);
  gtk_tree_path_free(path);
}

int gtk_list_model_iter_to_row(GtkListModel* model, GtkTreeIter iter) {
  int row;
  GtkTreeIter *i = &iter;
  ITER_GET(i, row);
  return row;
}
