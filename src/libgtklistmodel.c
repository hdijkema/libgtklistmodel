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
#include <elementals.h>

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

static inline int* indcopy(int* a)
{
  int* b = (int*) mc_malloc(sizeof(int));
  *b = *a;
  return b;
}

static inline void inddestroy(int* a)
{
  mc_free(a);
}

IMPLEMENT_EL_ARRAY(gtklistmodel_index, int, indcopy, inddestroy);


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

gboolean filter_out_func(void* data, int row)
{
  return TRUE;
}
 
static void gtk_list_model_init (GtkListModel* model)
{
  model->n_columns = n_columns;
  model->column_type = column_type;
  model->n_rows = n_rows;
  model->cell_value = cell_value;
  model->filter_out_func = filter_out_func;
  model->data = NULL;
  model->change_transaction = 0;
  model->index = gtklistmodel_index_new();
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
  log_debug("FINALIZING GTK_LIST_MODEL");
  GtkListModel* model = GTK_LIST_MODEL(object);
  gtklistmodel_index_destroy(model->index);
  
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
 
  //return (GTK_TREE_MODEL_LIST_ONLY) ; // My iters don't persist! | GTK_TREE_MODEL_ITERS_PERSIST);
  return 0;
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
  
  //log_debug("CALLED");
 
  g_assert(GTK_LIST_IS_MODEL(tree_model));
  g_assert(path!=NULL);
 
  model = GTK_LIST_MODEL(tree_model);
 
  indices = gtk_tree_path_get_indices(path);
  depth   = gtk_tree_path_get_depth(path);
 
  /* we do not allow children */
  g_assert(depth == 1); /* depth 1 = top level; a list only has top level nodes and no children */
 
  n = indices[0]; /* the n-th top level row */
  
  //log_debug3("COUNT = %d, ROW = %d", gtklistmodel_index_count(model->index), n);
  
  if ( n >= gtklistmodel_index_count(model->index) || n < 0) 
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
  
  //log_debug("CALLED");
 
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
  
  //log_debug("CALLED");
 
  g_return_if_fail (GTK_LIST_IS_MODEL (tree_model));

  gtk_list_model = GTK_LIST_MODEL(tree_model);
  
  g_return_if_fail (iter != NULL);
  g_return_if_fail (column < gtk_list_model->n_columns(gtk_list_model->data));
  g_return_if_fail (VALID_ITER(iter));
 
  g_value_init (value, gtk_list_model->column_type(gtk_list_model->data, column));
 
  int row;
  ITER_GET(iter, row);
  
  g_return_if_fail ( row >= 0 && row < gtklistmodel_index_count(gtk_list_model->index) );
  
  int index = *gtklistmodel_index_get(gtk_list_model->index, row);
  g_return_if_fail ( index >= 0 && index < gtk_list_model->n_rows(gtk_list_model->data) );
  
  gtk_list_model->cell_value(gtk_list_model->data, index, column, value);
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
  
  //log_debug("CALLED");
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), FALSE);
 
  if (iter == NULL || !VALID_ITER(iter))
    return FALSE;
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  int row;
  ITER_GET(iter, row);
  
  //log_debug3("COUNT = %d, ROW = %d",gtklistmodel_index_count(gtk_list_model->index), row);
 
  /* Is this the last record in the list? */
  if ((row + 1) >= gtklistmodel_index_count(gtk_list_model->index) )
    return FALSE;
 
  //log_debug("yes");
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
  
  //log_debug("CALLED");
 
  g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);
 
  /* this is a list, nodes have no children */
  if (parent)
    return FALSE;
 
  /* parent == NULL is a special case; we need to return the first top-level row */
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), FALSE);
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  /* No rows => no first row */
  if (gtklistmodel_index_count(gtk_list_model->index) == 0)
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
  
  //log_debug("CALLED");
  
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), -1);
  g_return_val_if_fail (iter == NULL || VALID_ITER(iter), FALSE);
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
  
  //log_debug2("ITER COUNT = %d", gtklistmodel_index_count(gtk_list_model->index));
 
  /* special case: if iter == NULL, return number of top-level rows */
  if (!iter)
    return gtklistmodel_index_count(gtk_list_model->index);
 
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
  
  //log_debug("CALLED");
 
  g_return_val_if_fail (GTK_LIST_IS_MODEL (tree_model), FALSE);
 
  gtk_list_model = GTK_LIST_MODEL(tree_model);
 
  /* a list has only top-level rows */
  if (parent)
    return FALSE;
 
  /* special case: if parent == NULL, set iter to n-th top-level row */
  
  //log_debug2("n = %d", n);
 
  if( n >= gtklistmodel_index_count(gtk_list_model->index) )
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

void gtk_list_model_refilter(GtkListModel* model)
{
  int i, N;
  gtklistmodel_index a = gtklistmodel_index_new();
  for(i=0, N = model->n_rows(model->data); i < N; ++i) {
    if (model->filter_out_func(model->data, i)) {
      gtklistmodel_index_append(a, &i);
    }
  }
  
  //log_debug2("INDEX COUNT = %d", gtklistmodel_index_count(a));
  
  gtklistmodel_index old = model->index;
  model->index = a;
  
  int currentN = gtklistmodel_index_count(old);
  int newN = gtklistmodel_index_count(a);
  int restN = -1;
  
  if (currentN > newN) { // newN smaller, signal remove of new records
    int i;
    for(i = currentN - 1; i >= newN; --i) {
      GtkTreePath* path = gtk_tree_path_new();
      gtk_tree_path_append_index(path, i);
      gtk_tree_model_row_deleted(GTK_TREE_MODEL(model), path);
      gtk_tree_path_free(path);
    }
    restN = newN;
  } else if (currentN < newN) { // newN bigger, signal insert of records
    int i;
    for(i = currentN; i < newN; ++i) {
      GtkTreePath* path = gtk_tree_path_new();
      GtkTreeIter iter;
      gtk_tree_path_append_index(path, i);
      ITER_SET((&iter), i);
      gtk_tree_model_row_inserted(GTK_TREE_MODEL(model), path, &iter);
      gtk_tree_path_free(path);
    }
    restN = currentN;
  }
  
  // signal change of all other records
  
  {
    int i;
    for(i = 0; i < restN; ++i) {
      GtkTreePath* path = gtk_tree_path_new();
      GtkTreeIter iter;
      gtk_tree_path_append_index(path, i);
      ITER_SET((&iter), i);
      gtk_tree_model_row_changed(GTK_TREE_MODEL(model), path, &iter);
      gtk_tree_path_free(path);
    }
  }
  
  // destroy old index
  
  gtklistmodel_index_destroy(old);
}

 
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
  newmodel->filter_out_func = filter_out_func;
  newmodel->index = gtklistmodel_index_new(); 
  newmodel->change_transaction = 0;
  gtk_list_model_refilter(newmodel);
  return newmodel;
}
 
void gtk_list_model_destroy(GtkListModel* model)
{
  g_object_unref(G_OBJECT(model));
}

void gtk_list_model_set_filter(GtkListModel* model, gboolean (*f)(void* data, int row))
{
  model->filter_out_func = f;
  gtk_list_model_refilter(model);
}

/*
  This method will start a change job. 
  If you're inserting or changing or removing 
  entries in this model, make sure you call
  this method at the start of the job.
  Calling this function can be nested. 
  When the last commit_change() is called, 
  the model will be refiltered.
*/
void gtk_list_model_begin_change(GtkListModel* model)
{ 
  model->change_transaction += 1; 
}

void gtk_list_model_commit_change(GtkListModel* model)
{
  g_return_if_fail(model->change_transaction > 0);
  
  model->change_transaction -= 1;
  if (model->change_transaction == 0) {
    gtk_list_model_refilter(model);  
  }
}

int gtk_list_model_iter_to_row(GtkListModel* model, GtkTreeIter iter) {
  int row = -1;
  int index_row;
  ITER_GET((&iter), index_row);
  if (index_row < gtklistmodel_index_count(model->index)) {
    row = *gtklistmodel_index_get(model->index, index_row);
  }
  return row;
}

void gtk_list_model_row_to_iter(GtkListModel* model, int row, GtkTreeIter *iter)
{
  iter->stamp = model->stamp;
  int index, N;
  for(index = 0, N = gtklistmodel_index_count(model->index); 
            index < N && *gtklistmodel_index_get(model->index, index) != row; ++index);
  if (index == N) index = 0;
  ITER_SET(iter, index);
}

void gtk_list_model_iterate_with_func(GtkListModel* model, void (*f)(void *model, int row, void *data), void* data)
{
  int i, N;
  for(i = 0, N = gtklistmodel_index_count(model->index); i < N; ++i) {
    f(model->data, *gtklistmodel_index_get(model->index, i), data);
  }
}

static int mycmp(GtkListModel* model, int* row1, int* row2) 
{
  return model->sort_cmp(model->data, *row1, *row2);
}

void gtk_list_model_sort(GtkListModel* model, int (*cmp)(void* data, int row1, int row2)) {
  model->sort_cmp = cmp;
  gtklistmodel_index_sort1(model->index, model, (int (*)(void*, int*, int*)) mycmp);
  {
    int i, N;
    for(i = 0, N = gtklistmodel_index_count(model->index); i < N; ++i) {
      GtkTreePath* path = gtk_tree_path_new();
      GtkTreeIter iter;
      gtk_tree_path_append_index(path, i);
      ITER_SET((&iter), i);
      gtk_tree_model_row_changed(GTK_TREE_MODEL(model), path, &iter);
      gtk_tree_path_free(path);
    }
  }
}


