#define _USE_MATH_DEFINES
#include <gtk/gtk.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// =============================================================================
//                             CONSTANTES GLOBALES
// =============================================================================
#define MAX_SIZE_FOR_SLOW_ALGOS 10000
#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT 300
#define VISUAL_NODES_PER_ROW 15
#define NODE_DISPLAY_BATCH 50

// =============================================================================
//                             DÉFINITIONS TYPES ET ÉNUMÉRATIONS
// =============================================================================
// CODES COULEURS (Thème Premium & Eye-Friendly)
#define BG_PRIMARY "#0A031A"   // Indigo très profond
#define BG_SECONDARY "#150D2B" // Violet nuit doux
#define BG_TERTIARY "#231840"  // Nuance tertiaire
#define BG_CARD "rgba(21, 13, 43, 0.88)"
#define ACCENT_PRIMARY "#7A5CFF" // Violet Saphir (Plus doux que le Magenta)
#define ACCENT_SECONDARY                                                       \
  "#00D1B2" // Teal/Turquoise (Plus reposant que le Cyan pur)
#define ACCENT_SUCCESS "#2ECC71" // Vert Émeraude
#define ACCENT_WARNING "#F1C40F" // Jaune Tournesol
#define ACCENT_DANGER "#E74C3C"  // Rouge Alizarine
#define TEXT_PRIMARY "#FFFFFF"   // Blanc pur
#define TEXT_SECONDARY "#A098B8" // Lavande grisé (Plus discret)
#define BORDER_COLOR "#3A2E59"   // Bordure plus douce
#define GLOW_PRIMARY "rgba(122, 92, 255, 0.15)"
#define GLOW_SECONDARY "rgba(0, 209, 178, 0.15)"

// CODES COULEURS (Thème Light Neo matched)
#define LIGHT_BG_PRIMARY "#F5F5FA"
#define LIGHT_BG_SECONDARY "#FFFFFF"
#define LIGHT_BG_TERTIARY "#E0E0E8"
#define LIGHT_ACCENT_PRIMARY "#5D3FD3"   // Deep Saphir/Iris
#define LIGHT_ACCENT_SECONDARY "#008B8B" // Dark Teal
#define LIGHT_TEXT_PRIMARY "#120B24"     // Very Dark Indigo
#define LIGHT_TEXT_SECONDARY "#554466"   // Muted Purple
#define LIGHT_BORDER_COLOR "#CCCCDD"

typedef enum { DATA_INT, DATA_FLOAT, DATA_CHAR, DATA_STRING } DataType;

typedef enum {
  STRUCTURE_ARRAY,
  STRUCTURE_SIMPLE_LIST,
  STRUCTURE_DOUBLE_LIST
} StructureType;

// =============================================================================
//                             STRUCTURES DE DONNÉES
// =============================================================================
typedef struct {
  double time_ns;
  size_t size;
} PerformancePoint;

typedef struct {
  char *algo_name;
  int num_points;
  PerformancePoint *points;
  int algo_index;
} ComparisonCurve;

typedef struct {
  DataType type;
  size_t size;
  void *data;
  size_t element_size;
  int (*compare_func)(const void *, const void *);
} Array;

typedef struct Node {
  void *data;
  struct Node *next;
  struct Node *prev;
} Node;

typedef struct {
  DataType type;
  size_t size;
  Node *head;
  Node *tail;
  gboolean is_doubly_linked;
  int (*compare_func)(const void *, const void *);
} LinkedList;

typedef struct TreeNode TreeNode;
struct TreeNode {
  void *data;
  TreeNode *left;
  TreeNode *right;
  GSList *children;
  TreeNode *parent;
};

// Structures pour l'affichage de l'arbre (Reingold-Tilford)
typedef struct RTNode RTNode;
struct RTNode {
  TreeNode *tree;
  GSList *children;
  RTNode *parent;
  RTNode *leftmost;
  RTNode *rightmost;
  RTNode *thread;
  RTNode *ancestor;
  double prelim;
  double mod;
  double change;
  double shift;
  int number;
  int depth;
};

typedef struct {
  double x_index;
  int depth;
  double x_px;
  double y_px;
} NodePos;

// --- Structures des Graphes ---
typedef struct GraphNode GraphNode;
typedef struct GraphEdge GraphEdge;

struct GraphEdge {
  GraphNode *target;
  double weight;
};

struct GraphNode {
  int id;
  void *data;    // Value
  double x, y;   // Visual pos
  GSList *edges; // Adjacency

  // Algo temp data
  double dist;
  GraphNode *prev;
  gboolean visited;
};

typedef struct {
  DataType type;
  GSList *nodes; // List of GraphNode*
  int node_count;
  int next_id_counter;
  gboolean is_directed;
} Graph;

// Main App Data
typedef struct {
  GtkWidget *window;

  // Dashboard Elements
  GtkWidget *view_stack;
  GtkWidget *status_label;
  GtkWidget *title_label;
  GtkWidget *nav_array_btn;
  GtkWidget *nav_list_btn;
  GtkWidget *nav_tree_btn;
  GtkWidget *nav_graph_btn;

  // Stats Cards
  GtkWidget *lbl_stat_array;
  GtkWidget *lbl_stat_list;
  GtkWidget *lbl_stat_tree;
  GtkWidget *lbl_stat_graph;

  // Controls
  GtkWidget *data_type_combo;
  GtkWidget *algo_selector_combo;
  GtkWidget *structure_type_label; // To show current mode

  // Array Input
  GtkWidget *array_mode_combo; // Random / Manuel
  GtkWidget *array_manual_entry;
  GtkWidget *array_size_entry;
  GtkWidget *array_canvas_before; // TextView
  GtkWidget *array_canvas_after;  // TextView
  GtkTextBuffer *buffer_array_before;
  GtkTextBuffer *buffer_array_after;

  // List Input
  GtkWidget *list_mode_combo;
  GtkWidget *list_manual_entry;
  GtkWidget *list_size_entry;
  GtkWidget *list_type_combo; // Simple/Double
  GtkWidget *list_canvas;
  GtkWidget *list_val_entry;
  GtkWidget *list_loc_combo; // Debut/Fin/Pos
  GtkWidget *list_pos_entry;
  GtkWidget *list_op_combo; // Insert/Del/Mod
  GtkWidget *list_data_type_combo;

  // Tree Input
  GtkWidget *tree_mode_combo;
  GtkWidget *tree_manual_entry;
  GtkWidget *tree_size_entry;
  GtkWidget *tree_canvas;
  GtkWidget *tree_type_combo;
  GtkWidget *tree_degree_spin;
  GtkWidget *tree_traversal_combo; // BFS, Pre, In, Post
  GtkWidget *tree_data_type_combo;

  // Graph Input
  GtkWidget *graph_type_combo; // Orienté / Non-Orienté
  GtkWidget *graph_val_entry;
  GtkWidget *graph_src_entry;
  GtkWidget *graph_dst_entry;
  GtkWidget *graph_weight_entry;
  GtkWidget *graph_algo_combo; // Dijkstra, Bellman, Floyd
  GtkWidget *graph_start_entry;
  GtkWidget *graph_end_entry;
  GtkWidget *graph_canvas;
  GtkWidget *graph_data_type_combo;
  GtkTextBuffer *graph_log_buffer;

  // Data
  Array *original_array;
  Array *sorted_array;
  LinkedList *current_list;

  // Tree Data
  gpointer tree_root;
  DataType tree_data_type;
  gboolean tree_is_nary;
  int tree_nary_degree;
  double tree_scale;
  double tree_offset_x;
  double tree_offset_y;
  gboolean tree_dragging;
  double tree_last_x;
  double tree_last_y;
  GtkTextBuffer *tree_log_buffer;

  // Graph Data
  Graph *current_graph;
  gboolean graph_dragging;
  GraphNode *graph_drag_node;
  GraphNode *graph_sel_source;
  GraphNode *graph_sel_dest;

  // Linking State
  gboolean graph_linking;
  GraphNode *graph_link_start;
  double graph_mouse_x;
  double graph_mouse_y;

  // Comparison
  GtkWidget *compare_window;
  GtkWidget *comparison_canvas;
  ComparisonCurve *comparison_data[8];
  int num_curves;
  int num_sizes;
  size_t *test_sizes;
  GtkWidget *comparison_progress;
  gboolean is_dark_theme;
  gboolean is_benchmarking;

  // Global Settings
  double animation_speed; // 0.0 to 1.0 (0=Instant, 1=Very Slow)
  DataType default_data_type;

  // Pseudo-code Panel
  GtkWidget *pseudo_panel;
  GtkWidget *pseudo_view;
  GtkTextBuffer *pseudo_buffer;
  int current_pseudo_line;

  // Animation Window
  GtkWidget *anim_win;
  GtkWidget *anim_canvas;
  Array *demo_array;

  // Session Stats
  int total_ops;
  double total_time_ms;
  int structures_created;

  // Home Screen Widgets
  GtkWidget *lbl_home_total_ops;
  GtkWidget *lbl_home_total_time;
  GtkWidget *lbl_home_struct_count;
  GtkWidget *speed_scale;
  GtkWidget *box_history;
  GtkWidget *btn_mode;    // Reference to the MODE button in header
  GSList *recent_history; // List of char*
  char cmd_buffer[64];    // Buffer for typed commands
} AppData;

// =============================================================================
//                             PROTOTYPES DES FONCTIONS
// =============================================================================
static void open_animation_window(AppData *app);
static void on_anim_window_destroy(GtkWidget *widget, gpointer user_data);
uint64_t get_nanoseconds();
size_t get_element_size(DataType type);
int (*get_compare_func(DataType type))(const void *, const void *);
void set_status(AppData *app_data, const char *format, ...);
void update_dashboard_stats(AppData *app, int structure_idx, size_t count,
                            double time_ms);
void show_mode_popup(GtkWidget *widget, gpointer user_data);
void set_buffer_markup(GtkTextBuffer *buffer, const char *markup);

// Tree Utils
void reorder_tree(AppData *app);
void on_tree_op_action(GtkWidget *widget, gpointer user_data);
void on_tree_transform(GtkWidget *widget, gpointer user_data);
// Arrays
Array *create_array(DataType type, size_t size);
void free_array(Array *array);
Array *copy_array(const Array *original);
void fill_array_random(Array *array);
char *array_to_string(const Array *array, gboolean styled, AppData *app);
uint64_t sort_array_wrapper(AppData *app, Array *array, int algo_index);

// Lists
LinkedList *create_linked_list(DataType type, gboolean is_doubly_linked);
void free_linked_list(LinkedList *list);
void fill_linked_list_random(LinkedList *list, size_t size);
void insert_to_linkedlist(LinkedList *list, Node *new_node, int position);
gboolean delete_from_linkedlist(LinkedList *list, int position);
gboolean modify_linkedlist(LinkedList *list, int position,
                           const char *value_str);
char *list_to_string(const LinkedList *list, gboolean styled);
Node *create_node(DataType type, const void *value);
uint64_t sort_list_wrapper(AppData *app, LinkedList *list, int algo_index);
LinkedList *copy_linked_list(const LinkedList *src);

// Trees
TreeNode *create_tree_node(DataType type, const void *value);
void free_tree(TreeNode *root, DataType type);
void insert_tree_random(AppData *app_data, size_t count);
gboolean insert_tree_manual(AppData *app_data, const char *value_str);
gboolean modify_tree_node(AppData *app_data, const char *old_str,
                          const char *new_str);
gboolean delete_tree_node(AppData *app_data, const char *value_str);
size_t tree_count(TreeNode *root);
int tree_depth(TreeNode *root);
void traverse_tree_bfs(TreeNode *root, GString *out, DataType type);
void traverse_tree_dfs_pre(TreeNode *root, GString *out, DataType type);
void traverse_tree_dfs_in(TreeNode *root, GString *out, DataType type);
void traverse_tree_dfs_post(TreeNode *root, GString *out, DataType type);
TreeNode *convert_nary_to_binary(TreeNode *root);

// Helper for parsing
void parse_and_fill_struct(AppData *app, const char *input,
                           int struct_type); // 0=Arr, 1=List, 2=Tree

// UI Callbacks
void on_nav_switch(GtkWidget *widget, gpointer user_data);
void on_generate_array(GtkWidget *widget, gpointer user_data);
void on_sort_array(GtkWidget *widget, gpointer user_data);
void on_generate_list(GtkWidget *widget, gpointer user_data);
void on_sort_list(GtkWidget *widget, gpointer user_data);
void on_list_action(GtkWidget *widget, gpointer user_data); // Insert/Mod/Del
void on_generate_tree(GtkWidget *widget, gpointer user_data);
void on_tree_action(GtkWidget *widget, gpointer user_data); // Traversal changes
void on_graph_action(GtkWidget *widget, gpointer user_data); // Graph buttons
void on_compare_launch(GtkWidget *widget, gpointer user_data);

// Drawing
gboolean on_draw_list(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_draw_tree(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_draw_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_draw_compare(GtkWidget *widget, cairo_t *cr, gpointer user_data);

// Utils
void apply_css(GtkWidget *widget, AppData *app);
void on_toggle_theme(GtkWidget *widget, gpointer user_data);
void on_speed_changed(GtkRange *range, gpointer user_data);
void on_reset_session(GtkWidget *widget, gpointer user_data);
void draw_arrow(cairo_t *cr, double x1, double y1, double x2, double y2);

// =============================================================================
//                             UTILITAIRES ET SYSTÈME
// =============================================================================

uint64_t get_nanoseconds() {
#ifdef _WIN32
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (uint64_t)((double)counter.QuadPart * 1000000000.0 / freq.QuadPart);

#else
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    return 0;
  }
  return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif
}

size_t get_element_size(DataType type) {
  switch (type) {
  case DATA_INT:
    return sizeof(int);
  case DATA_FLOAT:
    return sizeof(float);
  case DATA_CHAR:
    return sizeof(char);
  case DATA_STRING:
    return sizeof(char *);
  }
  return 0;
}

static int compare_ints(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}
static int compare_floats(const void *a, const void *b) {
  float fa = *(const float *)a;
  float fb = *(const float *)b;
  return (fa < fb) ? -1 : (fa > fb) ? 1 : 0;
}
static int compare_chars(const void *a, const void *b) {
  return (*(char *)a - *(char *)b);
}
static int compare_strings(const void *a, const void *b) {
  const char *sa = *(const char **)a;
  const char *sb = *(const char **)b;
  if (!sa && !sb)
    return 0;
  if (!sa)
    return -1;
  if (!sb)
    return 1;
  return strcmp(sa, sb);
}

int (*get_compare_func(DataType type))(const void *, const void *) {
  switch (type) {
  case DATA_INT:
    return compare_ints;
  case DATA_FLOAT:
    return compare_floats;
  case DATA_CHAR:
    return compare_chars;
  case DATA_STRING:
    return compare_strings;
  }
  return NULL;
}

void set_status(AppData *app_data, const char *format, ...) {
  if (!app_data || !app_data->status_label)
    return;
  va_list args;
  va_start(args, format);
  gchar *text = g_strdup_vprintf(format, args);
  va_end(args);
  gtk_label_set_markup(
      GTK_LABEL(app_data->status_label),
      g_strdup_printf("<span foreground=\"%s\" weight=\"bold\">%s</span>",
                      ACCENT_SUCCESS, text));
  g_free(text);
  while (gtk_events_pending())
    gtk_main_iteration();
}

void update_dashboard_stats(AppData *app, int structure_idx, size_t count,
                            double time_ms) {
  if (!app)
    return;

  // Accumulate Global Stats if time_ms > 0 (meaning an operation occurred)
  if (time_ms >= 0) {
    app->total_ops++;
    app->total_time_ms += time_ms;

    // Update Home Labels if we are on Home or just generally
    if (app->lbl_home_total_ops) {
      char s[64];
      snprintf(s, 64, "%d", app->total_ops);
      gtk_label_set_text(GTK_LABEL(app->lbl_home_total_ops), s);
    }
    if (app->lbl_home_total_time) {
      char s[64];
      if (app->total_time_ms < 1.0)
        snprintf(s, 64, "%.0f ns", app->total_time_ms * 1000000.0);
      else
        snprintf(s, 64, "%.2f ms", app->total_time_ms);
      gtk_label_set_text(GTK_LABEL(app->lbl_home_total_time), s);
    }
  }

  // structure_idx: 0=Array, 1=List, 2=Tree
  char buf[128];
  char time_str[64];
  if (time_ms < 0)
    snprintf(time_str, sizeof(time_str), "--");
  else if (time_ms < 1.0)
    snprintf(time_str, sizeof(time_str), "%.0f ns", time_ms * 1000000.0);
  else
    snprintf(time_str, sizeof(time_str), "%.2f ms", time_ms);

  if (structure_idx == 0 && app->lbl_stat_array) {
    snprintf(buf, sizeof(buf), "%zu Éléments\n%s", count, time_str);
    gtk_label_set_text(GTK_LABEL(app->lbl_stat_array), buf);
  } else if (structure_idx == 1 && app->lbl_stat_list) {
    snprintf(buf, sizeof(buf), "%zu Nœuds\n%s", count, time_str);
    gtk_label_set_text(GTK_LABEL(app->lbl_stat_list), buf);
  } else if (structure_idx == 2 && app->lbl_stat_tree) {
    snprintf(buf, sizeof(buf), "%zu Nodes\nDepth: %.0f", count, time_ms);
    gtk_label_set_text(GTK_LABEL(app->lbl_stat_tree), buf);
  } else if (structure_idx == 3 && app->lbl_stat_graph) {
    snprintf(buf, sizeof(buf), "%zu Sommets\n%s", count, time_str);
    gtk_label_set_text(GTK_LABEL(app->lbl_stat_graph), buf);
  }
}

// Helpers for Pseudo-code (Moved logic to open_animation_window for the UI
// part)

void set_pseudo_code(AppData *app, const char *code) {
  if (!app || !app->pseudo_buffer || app->is_benchmarking)
    return;
  gtk_text_buffer_set_text(app->pseudo_buffer, code, -1);
  app->current_pseudo_line = -1;
}

void highlight_pseudo_line(AppData *app, int line) {
  if (!app || !app->pseudo_buffer || app->is_benchmarking ||
      app->animation_speed <= 0)
    return;
  GtkTextIter start, end;
  GtkTextBuffer *buffer = app->pseudo_buffer;

  // Clear previous highlight
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  gtk_text_buffer_remove_tag_by_name(buffer, "highlight", &start, &end);

  if (line >= 0) {
    gtk_text_buffer_get_iter_at_line(buffer, &start, line);
    end = start;
    gtk_text_iter_forward_to_line_end(&end);
    gtk_text_buffer_apply_tag_by_name(buffer, "highlight", &start, &end);

    // Auto-scroll to highlight
    GtkTextMark *mark = gtk_text_buffer_get_mark(buffer, "scroll_mark");
    if (!mark) {
      mark = gtk_text_buffer_create_mark(buffer, "scroll_mark", &start, FALSE);
    } else {
      gtk_text_buffer_move_mark(buffer, mark, &start);
    }
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(app->pseudo_view), mark);
  }

  while (gtk_events_pending())
    gtk_main_iteration();
}

void add_history_entry(AppData *app, const char *text) {
  if (!app || !text)
    return;

  // Get current time [HH:MM]
  GDateTime *now = g_date_time_new_now_local();
  char *timestamp = g_date_time_format(now, "[%H:%M]");
  g_date_time_unref(now);

  char *entry_text = g_strdup_printf("%s %s", timestamp, text);
  g_free(timestamp);

  // Increase limit to 20
  if (g_slist_length(app->recent_history) >= 20) {
    g_free(app->recent_history->data);
    app->recent_history =
        g_slist_remove(app->recent_history, app->recent_history->data);
  }

  app->recent_history = g_slist_append(app->recent_history, entry_text);

  // Update UI if box exists
  if (app->box_history) {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(app->box_history));
    for (iter = children; iter != NULL; iter = g_list_next(iter))
      gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    GSList *rev = g_slist_copy(app->recent_history);
    rev = g_slist_reverse(rev);
    for (GSList *s = rev; s != NULL; s = s->next) {
      GtkWidget *lbl = gtk_label_new(NULL);
      char *escaped = g_markup_escape_text((char *)s->data, -1);
      gtk_label_set_markup(GTK_LABEL(lbl),
                           g_strdup_printf("<span size='small' alpha='70%%' "
                                           "weight='medium'>&gt;&gt; %s</span>",
                                           escaped));
      g_free(escaped);
      gtk_widget_set_halign(lbl, GTK_ALIGN_START);
      gtk_box_pack_start(GTK_BOX(app->box_history), lbl, FALSE, FALSE, 4);
    }
    g_slist_free(rev);
    gtk_widget_show_all(app->box_history);
  }
}

// =============================================================================
//                             GESTION DES TABLEAUX (ARRAYS)
// =============================================================================

static void swap(void *a, void *b, size_t size) {
  char temp[size];
  memcpy(temp, a, size);
  memcpy(a, b, size);
  memcpy(b, temp, size);
}

Array *create_array(DataType type, size_t size) {
  Array *array = g_new0(Array, 1);
  array->type = type;
  array->size = size;
  array->element_size = get_element_size(type);
  array->compare_func = get_compare_func(type);
  array->data = calloc(size, array->element_size);
  return array;
}

void free_array(Array *array) {
  if (!array)
    return;
  if (array->type == DATA_STRING && array->data) {
    for (size_t i = 0; i < array->size; i++)
      g_free(*((char **)array->data + i));
  }
  if (array->data)
    free(array->data);
  g_free(array);
}

Array *copy_array(const Array *original) {
  if (!original)
    return NULL;
  Array *copy = create_array(original->type, original->size);
  if (original->type == DATA_STRING) {
    for (size_t i = 0; i < original->size; i++) {
      const char *src = *((const char **)original->data + i);
      *((char **)copy->data + i) = src ? g_strdup(src) : NULL;
    }
  } else {
    memcpy(copy->data, original->data, original->size * original->element_size);
  }
  return copy;
}

// Helper for random string
static char *generate_random_string() {
  int len = 5 + rand() % 6; // 5 to 10 chars
  char *s = g_malloc(len + 1);
  for (int i = 0; i < len; i++) {
    s[i] = 'a' + rand() % 26;
  }
  s[len] = '\0';
  return s;
}

void fill_array_random(Array *array) {
  if (!array)
    return;
  srand(time(NULL));
  for (size_t i = 0; i < array->size; i++) {
    void *ptr = (char *)array->data + i * array->element_size;
    switch (array->type) {
    case DATA_INT:
      *(int *)ptr = rand() % 10000;
      break;
    case DATA_FLOAT:
      *(float *)ptr = (float)rand() / (float)(RAND_MAX / 1000.0);
      break;
    case DATA_CHAR:
      *(char *)ptr = (char)(33 + rand() % 94);
      break;
    case DATA_STRING: {
      if (*((char **)ptr))
        g_free(*((char **)ptr));
      *((char **)ptr) = generate_random_string();
      break;
    }
    }
  }
}

char *array_to_string(const Array *array, gboolean styled, AppData *app) {
  if (!array)
    return g_strdup("");
  GString *str = g_string_sized_new(array->size * 20);

  // Performance safeguard
  if (array->size > 3000) {
    styled = FALSE;
  }

  for (size_t i = 0; i < array->size; i++) {
    // No limit check requested
    void *ptr = (char *)array->data + i * array->element_size;
    char val_str[128];
    switch (array->type) {
    case DATA_INT:
      snprintf(val_str, 128, "%d", *(int *)ptr);
      break;
    case DATA_FLOAT:
      snprintf(val_str, 128, "%.2f", *(float *)ptr);
      break;
    case DATA_CHAR:
      snprintf(val_str, 128, "'%c'", *(char *)ptr);
      break;
    case DATA_STRING:
      snprintf(val_str, 128, "\"%s\"", *(char **)ptr ? *(char **)ptr : "NULL");
      break;
    }

    if (styled) {
      // Logic for theme
      const char *color1;
      const char *color2;

      if (app && app->is_dark_theme) {
        // Dark Theme Colors: White text with subtle Teal alternating
        color1 = TEXT_PRIMARY;
        color2 = ACCENT_SECONDARY;
      } else {
        // Light Theme Colors
        color1 = LIGHT_TEXT_PRIMARY;
        color2 = LIGHT_ACCENT_SECONDARY;
      }

      const char *color = (i % 4 == 0) ? color2 : color1;

      char *escaped = g_markup_escape_text(val_str, -1);
      g_string_append_printf(
          str,
          "<span foreground=\"%s\" font_family=\"Outfit, sans-serif\" "
          "weight=\"500\" size=\"11000\">%s</span>",
          color, escaped);
      g_free(escaped);
    } else {
      g_string_append(str, val_str);
    }

    if (i < array->size - 1)
      g_string_append(str, "   "); // Wider spacing
  }
  return g_string_free(str, FALSE);
}

// =============================================================================
//                             ALGORITHMES DE TRI
// =============================================================================

// Helper to refresh visualization during animation
static void refresh_sort_visual(AppData *app) {
  if (app->is_benchmarking || app->animation_speed <= 0)
    return;

  const char *vis_child =
      gtk_stack_get_visible_child_name(GTK_STACK(app->view_stack));

  if (g_strcmp0(vis_child, "view_array") == 0 && app->sorted_array) {
    char *s = array_to_string(app->sorted_array, TRUE, app);
    set_buffer_markup(app->buffer_array_after, s);
    g_free(s);
  } else if (g_strcmp0(vis_child, "view_list") == 0) {
    gtk_widget_queue_draw(app->list_canvas);
  }

  // Also refresh the separate animation window if open
  if (app->anim_canvas) {
    gtk_widget_queue_draw(app->anim_canvas);
  }

  // Allow UI to refresh
  while (gtk_events_pending())
    gtk_main_iteration();

  g_usleep((gulong)(app->animation_speed * 500000));
}

static void bubble_sort(AppData *app, void *data, size_t size,
                        size_t element_size,
                        int (*cmp)(const void *, const void *)) {
  set_pseudo_code(app, "BUBBLE_SORT(Tableau A):\n"
                       "  N = Taille(A)\n"
                       "  Pour i de 0 \xC3\xA0 N-1 faire:\n"
                       "    Echange = Faux\n"
                       "    Pour j de 0 \xC3\xA0 N-i-1 faire:\n"
                       "      Si A[j] > A[j+1] alors:\n"
                       "        Echanger(A[j], A[j+1])\n"
                       "        Echange = Vrai\n"
                       "      Fin Si\n"
                       "    Fin Pour\n"
                       "    Si Echange == Faux alors: STOP\n"
                       "  Fin Pour");

  for (size_t i = 0; i < size - 1; i++) {
    gboolean swapped = FALSE;
    highlight_pseudo_line(app, 2);
    for (size_t j = 0; j < size - i - 1; j++) {
      highlight_pseudo_line(app, 4);
      highlight_pseudo_line(app, 5);
      if (cmp((char *)data + j * element_size,
              (char *)data + (j + 1) * element_size) > 0) {
        highlight_pseudo_line(app, 6);
        swap((char *)data + j * element_size,
             (char *)data + (j + 1) * element_size, element_size);
        swapped = TRUE;
        refresh_sort_visual(app);
      }
    }
    if (!swapped) {
      highlight_pseudo_line(app, 10);
      break;
    }
  }
  highlight_pseudo_line(app, -1);
}

static void insertion_sort(AppData *app, void *data, size_t size,
                           size_t element_size,
                           int (*cmp)(const void *, const void *)) {
  for (size_t i = 1; i < size; i++) {
    highlight_pseudo_line(app, 3);
    void *key = malloc(element_size);
    memcpy(key, (char *)data + i * element_size, element_size);
    int j = i - 1;
    while (j >= 0 && cmp((char *)data + j * element_size, key) > 0) {
      highlight_pseudo_line(app, 6);
      memcpy((char *)data + (j + 1) * element_size,
             (char *)data + j * element_size, element_size);
      j--;
      refresh_sort_visual(app);
    }
    memcpy((char *)data + (j + 1) * element_size, key, element_size);
    free(key);
    refresh_sort_visual(app);
  }
}

static void shell_sort(AppData *app, void *data, size_t size,
                       size_t element_size,
                       int (*cmp)(const void *, const void *)) {
  for (size_t gap = size / 2; gap > 0; gap /= 2) {
    highlight_pseudo_line(app, 2);
    for (size_t i = gap; i < size; i++) {
      highlight_pseudo_line(app, 3);
      void *temp = malloc(element_size);
      memcpy(temp, (char *)data + i * element_size, element_size);
      size_t j;
      for (j = i;
           j >= gap && cmp((char *)data + (j - gap) * element_size, temp) > 0;
           j -= gap) {
        highlight_pseudo_line(app, 5);
        memcpy((char *)data + j * element_size,
               (char *)data + (j - gap) * element_size, element_size);
        refresh_sort_visual(app);
      }
      memcpy((char *)data + j * element_size, temp, element_size);
      free(temp);
      refresh_sort_visual(app);
    }
  }
}

static size_t partition(AppData *app, void *data, size_t low, size_t high,
                        size_t es, int (*cmp)(const void *, const void *)) {
  highlight_pseudo_line(app, 8);
  void *pivot = (char *)data + high * es;
  size_t i = low;
  for (size_t j = low; j < high; j++) {
    highlight_pseudo_line(app, 11);
    if (cmp((char *)data + j * es, pivot) <= 0) {
      highlight_pseudo_line(app, 12);
      swap((char *)data + i * es, (char *)data + j * es, es);
      i++;
      refresh_sort_visual(app);
    }
  }
  highlight_pseudo_line(app, 16);
  swap((char *)data + i * es, (char *)data + high * es, es);
  refresh_sort_visual(app);
  return i;
}
static void qs_rec(AppData *app, void *data, size_t low, size_t high, size_t es,
                   int (*cmp)(const void *, const void *)) {
  if (low < high) {
    highlight_pseudo_line(app, 2);
    size_t pi = partition(app, data, low, high, es, cmp);
    if (pi > 0) {
      highlight_pseudo_line(app, 3);
      qs_rec(app, data, low, pi - 1, es, cmp);
    }
    highlight_pseudo_line(app, 4);
    qs_rec(app, data, pi + 1, high, es, cmp);
  }
}

uint64_t sort_array_wrapper(AppData *app, Array *array, int algo) {
  if (!array || array->size == 0)
    return 0;
  uint64_t start = get_nanoseconds();
  if (algo == 0) {
    if (!app->is_benchmarking)
      set_pseudo_code(app, "BUBBLE_SORT(Tableau A):\n"
                           "  N = Taille(A)\n"
                           "  Pour i de 0 \xC3\xA0 N-1 faire:\n"
                           "    Echange = Faux\n"
                           "    Pour j de 0 \xC3\xA0 N-i-1 faire:\n"
                           "      Si A[j] > A[j+1] alors:\n"
                           "        Echanger(A[j], A[j+1])\n"
                           "        Echange = Vrai\n"
                           "      Fin Si\n"
                           "    Fin Pour\n"
                           "    Si Echange == Faux alors: STOP\n"
                           "  Fin Pour");
    bubble_sort(app, array->data, array->size, array->element_size,
                array->compare_func);
  } else if (algo == 1) {
    if (!app->is_benchmarking)
      set_pseudo_code(app, "INSERTION_SORT(Tableau A):\n"
                           "  N = Taille(A)\n"
                           "  Pour i de 1 \xC3\xA0 N-1 faire:\n"
                           "    cle = A[i]\n"
                           "    j = i - 1\n"
                           "    Tant que j >= 0 et A[j] > cle faire:\n"
                           "      A[j+1] = A[j]\n"
                           "      j = j - 1\n"
                           "    Fin Tant que\n"
                           "    A[j+1] = cle\n"
                           "  Fin Pour");
    insertion_sort(app, array->data, array->size, array->element_size,
                   array->compare_func);
  } else if (algo == 2) {
    if (!app->is_benchmarking)
      set_pseudo_code(
          app, "SHELL_SORT(Tableau A):\n"
               "  N = Taille(A)\n"
               "  Pour ecart de N/2 \xC3\xA0 0 faire:\n"
               "    Pour i de ecart \xC3\xA0 N-1 faire:\n"
               "      temp = A[i]\n"
               "      Pour j de i \xC3\xA0 ecart et A[j-ecart] > temp faire:\n"
               "        A[j] = A[j-ecart]\n"
               "      Fin Pour\n"
               "      A[j] = temp\n"
               "    Fin Pour\n"
               "  Fin Pour");
    shell_sort(app, array->data, array->size, array->element_size,
               array->compare_func);
  } else if (algo == 3 && array->size > 0) {
    if (!app->is_benchmarking)
      set_pseudo_code(app, "QUICK_SORT(A, bas, haut):\n"
                           "  Si bas < haut alors:\n"
                           "    p = Partition(A, bas, haut)\n"
                           "    QUICK_SORT(A, bas, p - 1)\n"
                           "    QUICK_SORT(A, p + 1, haut)\n"
                           "  Fin Si\n\n"
                           "PARTITION(A, bas, haut):\n"
                           "  pivot = A[haut]\n"
                           "  i = bas\n"
                           "  Pour j de bas \xC3\xA0 haut-1 faire:\n"
                           "    Si A[j] <= pivot alors:\n"
                           "      Echanger(A[i], A[j])\n"
                           "      i = i + 1\n"
                           "    Fin Si\n"
                           "  Fin Pour\n"
                           "  Echanger(A[i], A[haut])\n"
                           "  Retourner i");
    qs_rec(app, array->data, 0, array->size - 1, array->element_size,
           array->compare_func);
  }
  return get_nanoseconds() - start;
}

// =============================================================================
//                             LISTES CHAÎNÉES
// =============================================================================

void free_node(Node *node, DataType type) {
  if (!node)
    return;
  if (type == DATA_STRING && node->data)
    g_free(*(char **)node->data);
  if (node->data)
    free(node->data);
  g_free(node);
}

Node *create_node(DataType type, const void *value) {
  Node *node = g_new0(Node, 1);
  size_t size = get_element_size(type);
  node->data = malloc(size);
  if (value) {
    if (type == DATA_STRING)
      *((char **)node->data) = g_strdup(*(const char **)value);
    else
      memcpy(node->data, value, size);
  }
  return node;
}

LinkedList *create_linked_list(DataType type, gboolean is_doubly_linked) {
  LinkedList *list = g_new0(LinkedList, 1);
  list->type = type;
  list->is_doubly_linked = is_doubly_linked;
  return list;
}

void free_linked_list(LinkedList *list) {
  if (!list)
    return;
  Node *cur = list->head;
  while (cur) {
    Node *next = cur->next;
    free_node(cur, list->type);
    cur = next;
  }
  g_free(list);
}

LinkedList *copy_linked_list(const LinkedList *src) {
  if (!src)
    return NULL;
  LinkedList *copy = create_linked_list(src->type, src->is_doubly_linked);
  Node *cur = src->head;
  while (cur) {
    Node *n = create_node(src->type, cur->data);
    insert_to_linkedlist(copy, n, -1);
    cur = cur->next;
  }
  return copy;
}

void insert_to_linkedlist(LinkedList *list, Node *new_node, int position) {
  if (!list || !new_node)
    return;
  if (list->size == 0) {
    list->head = list->tail = new_node;
  } else if (position == 0) {
    new_node->next = list->head;
    if (list->is_doubly_linked)
      list->head->prev = new_node;
    list->head = new_node;
  } else if (position < 0 || (size_t)position >= list->size) {
    list->tail->next = new_node;
    if (list->is_doubly_linked)
      new_node->prev = list->tail;
    list->tail = new_node;
  } else {
    Node *current = list->head;
    for (int i = 0; i < position - 1; i++)
      current = current->next;
    new_node->next = current->next;
    new_node->prev = current;
    if (current->next && list->is_doubly_linked)
      current->next->prev = new_node;
    current->next = new_node;
  }
  list->size++;
}

gboolean delete_from_linkedlist(LinkedList *list, int position) {
  if (!list || list->size == 0 || position < 0 ||
      (size_t)position >= list->size)
    return FALSE;
  Node *del = NULL;
  if (position == 0) {
    del = list->head;
    list->head = list->head->next;
    if (list->head && list->is_doubly_linked)
      list->head->prev = NULL;
    else if (!list->head)
      list->tail = NULL;
  } else {
    Node *cur = list->head;
    for (int i = 0; i < position; i++)
      cur = cur->next;
    del = cur;
    if (del->prev)
      del->prev->next = del->next;
    if (del->next && list->is_doubly_linked)
      del->next->prev = del->prev;
    if (del == list->tail)
      list->tail = del->prev;
  }
  free_node(del, list->type);
  list->size--;
  return TRUE;
}

gboolean modify_linkedlist(LinkedList *list, int position,
                           const char *val_str) {
  if (!list || list->size == 0 || position < 0 ||
      (size_t)position >= list->size)
    return FALSE;
  Node *cur = list->head;
  for (int i = 0; i < position; i++)
    cur = cur->next;

  void *nd = malloc(get_element_size(list->type));
  if (list->type == DATA_INT)
    *(int *)nd = atoi(val_str);
  else if (list->type == DATA_FLOAT)
    *(float *)nd = atof(val_str);
  else if (list->type == DATA_CHAR)
    *(char *)nd = val_str[0];
  else if (list->type == DATA_STRING) {
    if (cur->data)
      g_free(*(char **)cur->data);
    *(char **)nd = g_strdup(val_str);
  }
  if (list->type != DATA_STRING && cur->data)
    free(cur->data);
  cur->data = nd;
  return TRUE;
}

void fill_linked_list_random(LinkedList *list, size_t size) {
  while (list->head)
    delete_from_linkedlist(list, 0);
  srand(time(NULL));
  for (size_t i = 0; i < size; i++) {
    Node *n = create_node(list->type, NULL);
    // Manually fill cause create_node copies value
    void *ptr = n->data;
    switch (list->type) {
    case DATA_INT:
      *(int *)ptr = rand() % 1000;
      break;
    case DATA_FLOAT:
      *(float *)ptr = (float)rand() / (RAND_MAX / 100.0);
      break;
    case DATA_CHAR:
      *(char *)ptr = (char)(33 + rand() % 94);
      break;
    case DATA_STRING: {
      *((char **)ptr) = generate_random_string();
    } break;
    }
    insert_to_linkedlist(list, n, -1);
  }
}

char *list_to_string(const LinkedList *list, gboolean styled) {
  if (!list || list->size == 0)
    return g_strdup("Liste vide");
  GString *str = g_string_sized_new(list->size * 20);

  // Performance safeguard for lists too
  if (list->size > 1000) {
    styled = FALSE;
  }

  Node *cur = list->head;
  while (cur) {
    char b[128];
    switch (list->type) {
    case DATA_INT:
      snprintf(b, 128, "%d", *(int *)cur->data);
      break;
    case DATA_FLOAT:
      snprintf(b, 128, "%.2f", *(float *)cur->data);
      break;
    case DATA_CHAR:
      snprintf(b, 128, "%c", *(char *)cur->data);
      break;
    case DATA_STRING:
      snprintf(b, 128, "%s", *(char **)cur->data);
      break;
    }
    if (styled)
      g_string_append_printf(str, "<span foreground=\"%s\">%s</span> ",
                             ACCENT_PRIMARY, b);
    else
      g_string_append_printf(str, "%s ", b);
    if (cur->next)
      g_string_append(str, "-> ");
    cur = cur->next;
  }
  return g_string_free(str, FALSE);
}

uint64_t sort_list_wrapper(AppData *app, LinkedList *list, int algo) {
  if (!list || list->size == 0)
    return 0;
  // Copy to array, sort, copy back
  Array *arr = create_array(list->type, list->size);
  Node *cur = list->head;
  for (size_t i = 0; i < list->size; i++) {
    void *dest = (char *)arr->data + i * arr->element_size;
    if (list->type == DATA_STRING)
      *((char **)dest) = g_strdup(*(char **)cur->data);
    else
      memcpy(dest, cur->data, arr->element_size);
    cur = cur->next;
  }
  uint64_t t = sort_array_wrapper(app, arr, algo);
  cur = list->head;
  for (size_t i = 0; i < list->size; i++) {
    void *src = (char *)arr->data + i * arr->element_size;
    if (list->type == DATA_STRING) {
      if (cur->data)
        g_free(*(char **)cur->data);
      *(char **)cur->data = g_strdup(*(char **)src);
    } else
      memcpy(cur->data, src, arr->element_size);
    cur = cur->next;
  }
  free_array(arr);
  return t;
}

// =============================================================================
//                             GESTION DES ARBRES
// =============================================================================

TreeNode *create_tree_node(DataType type, const void *value) {
  TreeNode *n = g_new0(TreeNode, 1);
  size_t sz = get_element_size(type);
  n->data = malloc(sz);
  if (value) {
    if (type == DATA_STRING)
      *((char **)n->data) = g_strdup(*(char **)value);
    else
      memcpy(n->data, value, sz);
  }
  return n;
}

void free_tree(TreeNode *root, DataType type) {
  if (!root)
    return;
  if (root->children) {
    for (GSList *it = root->children; it; it = it->next)
      free_tree((TreeNode *)it->data, type);
    g_slist_free(root->children);
  }
  if (root->left)
    free_tree(root->left, type);
  if (root->right)
    free_tree(root->right, type);
  if (type == DATA_STRING && root->data)
    g_free(*(char **)root->data);
  if (root->data)
    free(root->data);
  g_free(root);
}

static void _insert_nary_level(TreeNode *root, TreeNode *node, int degree) {
  GQueue *q = g_queue_new();
  g_queue_push_tail(q, root);
  while (!g_queue_is_empty(q)) {
    TreeNode *cur = g_queue_pop_head(q);
    int c = g_slist_length(cur->children);
    if (c < degree) {
      cur->children = g_slist_append(cur->children, node);
      node->parent = cur;
      g_queue_free(q);
      return;
    }
    for (GSList *it = cur->children; it; it = it->next)
      g_queue_push_tail(q, it->data);
  }
  g_queue_free(q);
}

void insert_tree_random(AppData *app, size_t count) {
  if (!app)
    return;
  DataType t = app->tree_data_type;
  srand(time(NULL));

  for (size_t i = 0; i < count; i++) {
    // Generate Random Value safely
    void *val_ptr = NULL;
    int r_int;
    float r_float;
    char r_char;
    char *r_str = NULL;

    if (t == DATA_INT) {
      r_int = rand() % 1000;
      val_ptr = &r_int;
    } else if (t == DATA_FLOAT) {
      r_float = (float)rand() / (RAND_MAX / 1000.0);
      val_ptr = &r_float;
    } else if (t == DATA_CHAR) {
      r_char = (char)('a' + rand() % 26);
      val_ptr = &r_char;
    } else if (t == DATA_STRING) {
      r_str = generate_random_string();
      val_ptr = &r_str; // Point to char*
    }

    TreeNode *n = NULL;
    // Special handling if root doesn't exist
    if (!app->tree_root) {
      app->tree_root = create_tree_node(t, val_ptr);
    } else {
      n = create_tree_node(t, val_ptr);
      if (app->tree_is_nary) {
        _insert_nary_level((TreeNode *)app->tree_root, n,
                           app->tree_nary_degree);
      } else {
        // BST Insert
        TreeNode **ptr = (TreeNode **)&app->tree_root;
        int (*cmp)(const void *, const void *) = get_compare_func(t);
        while (*ptr) {
          TreeNode *cur = *ptr;
          if (cmp(n->data, cur->data) < 0)
            ptr = &cur->left;
          else
            ptr = &cur->right;
        }
        *ptr = n;
      }
    }

    if (r_str)
      g_free(r_str);
  }
}

gboolean insert_tree_manual(AppData *app, const char *val_str) {
  if (!app)
    return FALSE;
  DataType t = app->tree_data_type;
  void *v = malloc(get_element_size(t));
  if (t == DATA_INT)
    *(int *)v = atoi(val_str);
  else if (t == DATA_FLOAT)
    *(float *)v = atof(val_str);
  else if (t == DATA_CHAR)
    *(char *)v = val_str[0];
  else if (t == DATA_STRING)
    *(char **)v = g_strdup(val_str);

  TreeNode *n = create_tree_node(t, v);
  if (t != DATA_STRING)
    free(v);
  else
    g_free(*(char **)v); // cleanup temp

  if (!app->tree_root) {
    app->tree_root = n;
    return TRUE;
  }

  if (app->tree_is_nary) {
    _insert_nary_level((TreeNode *)app->tree_root, n, app->tree_nary_degree);
  } else {
    TreeNode **ptr = (TreeNode **)&app->tree_root;
    int (*cmp)(const void *, const void *) = get_compare_func(t);
    while (*ptr) {
      if (cmp(n->data, (*ptr)->data) < 0)
        ptr = &(*ptr)->left;
      else
        ptr = &(*ptr)->right;
    }
    *ptr = n;
  }
  return TRUE;
}

// Helper to find node by string value (BFS)
static TreeNode *find_node_bfs(TreeNode *root, const char *val_str,
                               DataType t) {
  if (!root)
    return NULL;
  GQueue *q = g_queue_new();
  g_queue_push_tail(q, root);
  while (!g_queue_is_empty(q)) {
    TreeNode *n = g_queue_pop_head(q);
    char b[128];
    if (t == DATA_INT)
      snprintf(b, 128, "%d", *(int *)n->data);
    else if (t == DATA_STRING)
      snprintf(b, 128, "%s", *(char **)n->data);
    // ... simplify
    if (g_strcmp0(b, val_str) == 0) {
      g_queue_free(q);
      return n;
    }

    if (n->children)
      for (GSList *it = n->children; it; it = it->next)
        g_queue_push_tail(q, it->data);
    if (n->left)
      g_queue_push_tail(q, n->left);
    if (n->right)
      g_queue_push_tail(q, n->right);
  }
  g_queue_free(q);
  return NULL;
}

gboolean modify_tree_node(AppData *app, const char *old_str,
                          const char *new_str) {
  TreeNode *n = find_node_bfs(app->tree_root, old_str, app->tree_data_type);
  if (!n)
    return FALSE;
  // Update data
  DataType t = app->tree_data_type;
  if (t == DATA_INT)
    *(int *)n->data = atoi(new_str);
  // ... others
  return TRUE;
}

// BFS Search to find parent of a node with specific value
static TreeNode *find_parent_bfs(TreeNode *root, TreeNode *target) {
  if (!root || root == target)
    return NULL;
  GQueue *q = g_queue_new();
  g_queue_push_tail(q, root);
  while (!g_queue_is_empty(q)) {
    TreeNode *curr = g_queue_pop_head(q);

    // Check children (N-ary)
    if (curr->children) {
      for (GSList *it = curr->children; it; it = it->next) {
        if (it->data == target) {
          g_queue_free(q);
          return curr;
        }
        g_queue_push_tail(q, it->data); // Continue search
      }
    }
    // Check Left/Right (Binary)
    if (curr->left == target || curr->right == target) {
      g_queue_free(q);
      return curr;
    }
    if (curr->left)
      g_queue_push_tail(q, curr->left);
    if (curr->right)
      g_queue_push_tail(q, curr->right);
  }
  g_queue_free(q);
  return NULL;
}

gboolean delete_tree_node(AppData *app, const char *val_str) {
  if (!app->tree_root)
    return FALSE;
  TreeNode *target =
      find_node_bfs((TreeNode *)app->tree_root, val_str, app->tree_data_type);
  if (!target) {
    set_status(app, "Noeud introuvable: %s", val_str);
    return FALSE;
  }

  if (target == app->tree_root) {
    // Delete root -> delete all
    free_tree((TreeNode *)app->tree_root, app->tree_data_type);
    app->tree_root = NULL;
    return TRUE;
  }

  TreeNode *parent = find_parent_bfs((TreeNode *)app->tree_root, target);
  if (parent) {
    if (app->tree_is_nary) {
      parent->children = g_slist_remove(parent->children, target);
    } else {
      if (parent->left == target)
        parent->left = NULL;
      else if (parent->right == target)
        parent->right = NULL;
    }
    free_tree(target, app->tree_data_type); // Free subtree
    return TRUE;
  }
  return FALSE;
}

// Reorder Tree (Balance BST) logic
static void _collect_nodes(TreeNode *root, GPtrArray *arr) {
  if (!root)
    return;
  g_ptr_array_add(arr, root);
  if (root->children) {
    for (GSList *it = root->children; it; it = it->next)
      _collect_nodes((TreeNode *)it->data, arr);
  }
  _collect_nodes(root->left, arr);
  _collect_nodes(root->right, arr);
}

// Compare for qsort of raw data pointers
static int _cmp_wrapper_int(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}
static int _cmp_wrapper_float(const void *a, const void *b) {
  float fa = *(float *)a;
  float fb = *(float *)b;
  return (fa < fb) ? -1 : (fa > fb) ? 1 : 0;
}
static int _cmp_wrapper_char(const void *a, const void *b) {
  return *(char *)a - *(char *)b;
}
static int _cmp_wrapper_str(const void *a, const void *b) {
  return strcmp(*(char **)a, *(char **)b);
}

// Standard BST insert - Removed unused function _bst_insert

// Balanced BST construction from sorted array
static TreeNode *_build_balanced(void **data_array, int start, int end,
                                 DataType type) {
  if (start > end)
    return NULL;
  int mid = (start + end) / 2;
  TreeNode *n = create_tree_node(type, data_array[mid]);
  n->left = _build_balanced(data_array, start, mid - 1, type);
  n->right = _build_balanced(data_array, mid + 1, end, type);
  return n;
}

void reorder_tree(AppData *app) {
  if (!app->tree_root)
    return;

  // 1. Collect all data
  GPtrArray *nodes = g_ptr_array_new();
  _collect_nodes((TreeNode *)app->tree_root, nodes);

  if (nodes->len == 0) {
    g_ptr_array_free(nodes, TRUE);
    return;
  }

  // Extract raw data copies
  size_t el_size = get_element_size(app->tree_data_type);
  int count = nodes->len;
  void *raw_data = malloc(count * el_size); // Array of values

  for (int i = 0; i < count; i++) {
    TreeNode *n = (TreeNode *)g_ptr_array_index(nodes, i);
    void *dest = (char *)raw_data + i * el_size;
    if (app->tree_data_type == DATA_STRING) {
      *((char **)dest) = g_strdup(*(char **)n->data);
    } else {
      memcpy(dest, n->data, el_size);
    }
  }

  // 2. Clear old tree
  free_tree((TreeNode *)app->tree_root, app->tree_data_type);
  app->tree_root = NULL;
  g_ptr_array_free(nodes, TRUE);

  // 3. Sort
  int (*cmp)(const void *, const void *);
  switch (app->tree_data_type) {
  case DATA_INT:
    cmp = _cmp_wrapper_int;
    break;
  case DATA_FLOAT:
    cmp = _cmp_wrapper_float;
    break;
  case DATA_CHAR:
    cmp = _cmp_wrapper_char;
    break;
  case DATA_STRING:
    cmp = _cmp_wrapper_str;
    break;
  default:
    cmp = _cmp_wrapper_int;
    break;
  }
  qsort(raw_data, count, el_size, cmp);

  // 4. Rebuild Balanced BST
  // To handle strings correctly (pointer to pointer), we need careful passing
  // The _build_balanced function expects array of pointers to data? No,
  // raw_data is flat array. Exception: String is array of char* pointers.

  // Actually, let's just make an array of void* pointers to the raw elements
  // for easier indexing
  void **ptr_array = malloc(count * sizeof(void *));
  for (int i = 0; i < count; i++) {
    ptr_array[i] = (char *)raw_data + i * el_size;
  }

  app->tree_root =
      _build_balanced(ptr_array, 0, count - 1, app->tree_data_type);
  app->tree_is_nary = FALSE; // Enforced

  free(ptr_array);

  // Free raw string copies if string, but wait, create_tree_node copies them.
  // So we must free own copies.
  if (app->tree_data_type == DATA_STRING) {
    for (int i = 0; i < count; i++) {
      g_free(*((char **)raw_data + i));
    }
  }
  free(raw_data);

  set_status(app, "Arbre ordonné et équilibré (BST).");
}

size_t tree_count(TreeNode *root) {
  if (!root)
    return 0;
  size_t c = 1;
  if (root->children)
    for (GSList *it = root->children; it; it = it->next)
      c += tree_count((TreeNode *)it->data);
  c += tree_count(root->left);
  c += tree_count(root->right);
  return c;
}

int tree_depth(TreeNode *root) {
  if (!root)
    return 0;
  int max = 0;
  if (root->children) {
    for (GSList *it = root->children; it; it = it->next) {
      int d = tree_depth((TreeNode *)it->data);
      if (d > max)
        max = d;
    }
    return max + 1;
  }
  int l = tree_depth(root->left);
  int r = tree_depth(root->right);
  return (l > r ? l : r) + 1;
}

void traverse_tree_bfs(TreeNode *root, GString *out, DataType type) {
  if (!root)
    return;
  GQueue *q = g_queue_new();
  g_queue_push_tail(q, root);
  while (!g_queue_is_empty(q)) {
    TreeNode *n = g_queue_pop_head(q);
    char b[128];
    if (type == DATA_INT)
      snprintf(b, 128, "%d ", *(int *)n->data);
    else if (type == DATA_FLOAT)
      snprintf(b, 128, "%.1f ", *(float *)n->data);
    else if (type == DATA_CHAR)
      snprintf(b, 128, "%c ", *(char *)n->data);
    else if (type == DATA_STRING)
      snprintf(b, 128, "%s ", *(char **)n->data);
    g_string_append(out, b);
    if (n->children)
      for (GSList *it = n->children; it; it = it->next)
        g_queue_push_tail(q, it->data);
    if (n->left)
      g_queue_push_tail(q, n->left);
    if (n->right)
      g_queue_push_tail(q, n->right);
  }
  g_queue_free(q);
}

TreeNode *convert_nary_to_binary(TreeNode *root) {
  if (!root)
    return NULL;
  if (root->children) {
    TreeNode *first = (TreeNode *)root->children->data;
    root->left = convert_nary_to_binary(first);
    GSList *it = root->children->next;
    TreeNode *prev = root->left;
    while (it) {
      prev->right = convert_nary_to_binary((TreeNode *)it->data);
      prev = prev->right;
      it = it->next;
    }
    g_slist_free(root->children);
    root->children = NULL;
  }
  return root;
}

void traverse_tree_dfs_pre(TreeNode *root, GString *out, DataType type) {
  if (!root)
    return;
  char b[128] = {0};
  if (type == DATA_INT)
    snprintf(b, 128, "%d ", *(int *)root->data);
  else if (type == DATA_FLOAT)
    snprintf(b, 128, "%.2f ", *(float *)root->data);
  else if (type == DATA_CHAR)
    snprintf(b, 128, "%c ", *(char *)root->data);
  else if (type == DATA_STRING)
    snprintf(b, 128, "%s ", *(char **)root->data);
  g_string_append(out, b);

  if (root->children) {
    for (GSList *it = root->children; it; it = it->next)
      traverse_tree_dfs_pre((TreeNode *)it->data, out, type);
  }
  traverse_tree_dfs_pre(root->left, out, type);
  traverse_tree_dfs_pre(root->right, out, type);
}

void traverse_tree_dfs_in(TreeNode *root, GString *out, DataType type) {
  if (!root)
    return;
  traverse_tree_dfs_in(root->left, out, type);

  char b[128] = {0};
  if (type == DATA_INT)
    snprintf(b, 128, "%d ", *(int *)root->data);
  else if (type == DATA_FLOAT)
    snprintf(b, 128, "%.2f ", *(float *)root->data);
  else if (type == DATA_CHAR)
    snprintf(b, 128, "%c ", *(char *)root->data);
  else if (type == DATA_STRING)
    snprintf(b, 128, "%s ", *(char **)root->data);
  g_string_append(out, b);

  if (root->children) {
    for (GSList *it = root->children; it; it = it->next)
      traverse_tree_dfs_in((TreeNode *)it->data, out, type);
  }

  traverse_tree_dfs_in(root->right, out, type);
}

void traverse_tree_dfs_post(TreeNode *root, GString *out, DataType type) {
  if (!root)
    return;
  if (root->children) {
    for (GSList *it = root->children; it; it = it->next)
      traverse_tree_dfs_post((TreeNode *)it->data, out, type);
  }
  traverse_tree_dfs_post(root->left, out, type);
  traverse_tree_dfs_post(root->right, out, type);

  char b[128] = {0};
  if (type == DATA_INT)
    snprintf(b, 128, "%d ", *(int *)root->data);
  else if (type == DATA_FLOAT)
    snprintf(b, 128, "%.2f ", *(float *)root->data);
  else if (type == DATA_CHAR)
    snprintf(b, 128, "%c ", *(char *)root->data);
  else if (type == DATA_STRING)
    snprintf(b, 128, "%s ", *(char **)root->data);
  g_string_append(out, b);
}

void parse_and_fill_struct(AppData *app, const char *input, int struct_type) {
  if (!input || strlen(input) == 0)
    return;
  char *dup = g_strdup(input);
  char **tokens = g_strsplit(dup, " ", -1);

  int type_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(app->data_type_combo));

  int count = 0;
  while (tokens[count])
    count++;

  if (struct_type == 0) { // Array
    if (app->original_array)
      free_array(app->original_array);
    app->original_array = create_array(type_idx, count);
    for (int i = 0; i < count; i++) {
      void *ptr = (char *)app->original_array->data +
                  i * app->original_array->element_size;
      if (type_idx == DATA_INT)
        *(int *)ptr = atoi(tokens[i]);
      else if (type_idx == DATA_FLOAT)
        *(float *)ptr = atof(tokens[i]);
      else if (type_idx == DATA_CHAR)
        *(char *)ptr = tokens[i][0];
      else if (type_idx == DATA_STRING)
        *((char **)ptr) = g_strdup(tokens[i]);
    }
  } else if (struct_type == 1) { // List
    if (app->current_list)
      free_linked_list(app->current_list);
    app->current_list = create_linked_list(type_idx, FALSE);
    for (int i = 0; i < count; i++) {
      Node *n = create_node(type_idx, NULL);
      void *ptr = n->data;
      if (type_idx == DATA_INT)
        *(int *)ptr = atoi(tokens[i]);
      else if (type_idx == DATA_FLOAT)
        *(float *)ptr = atof(tokens[i]);
      else if (type_idx == DATA_CHAR)
        *(char *)ptr = tokens[i][0];
      else if (type_idx == DATA_STRING)
        *((char **)ptr) = g_strdup(tokens[i]);
      insert_to_linkedlist(app->current_list, n, -1);
    }
  } else if (struct_type == 2) { // Tree
    if (app->tree_root) {
      free_tree((TreeNode *)app->tree_root, app->tree_data_type);
      app->tree_root = NULL;
    }
    for (int i = 0; i < count; i++) {
      insert_tree_manual(app, tokens[i]);
    }
  }

  g_strfreev(tokens);
  g_free(dup);
}

// =============================================================================
//                             GESTION DES GRAPHES
// =============================================================================

Graph *create_graph(DataType type, gboolean directed) {
  Graph *g = g_new0(Graph, 1);
  g->type = type;
  g->is_directed = directed;
  g->next_id_counter = 1;
  return g;
}

// Helper to layout circular
void layout_graph_circular(Graph *g, double cx, double cy, double radius) {
  if (!g || !g->nodes)
    return;
  int count = g->node_count;
  if (count == 0)
    return;

  double angle_step = 2 * M_PI / count;
  int i = 0;
  // Sort visually by ID to keep order stable? No, list order is fine.
  // Actually, random order might be annoying. Let's iterate list.
  for (GSList *l = g->nodes; l; l = l->next) {
    GraphNode *n = (GraphNode *)l->data;
    double angle = i * angle_step - M_PI / 2; // Start top
    n->x = cx + radius * cos(angle);
    n->y = cy + radius * sin(angle);
    i++;
  }
}

GraphNode *create_graph_node(Graph *g, void *data) {
  GraphNode *n = g_new0(GraphNode, 1);
  n->id = g->next_id_counter++;
  n->data = data;

  // Pos will be updated by layout call
  n->x = 0;
  n->y = 0;

  g->nodes = g_slist_append(g->nodes, n);
  g->node_count++;
  return n;
}

void add_graph_edge(Graph *g, GraphNode *src, GraphNode *dest, double weight) {
  if (!g || !src || !dest)
    return;

  GraphEdge *e = g_new0(GraphEdge, 1);
  e->target = dest;
  e->weight = weight;
  src->edges = g_slist_append(src->edges, e);

  if (!g->is_directed) {
    GraphEdge *e2 = g_new0(GraphEdge, 1);
    e2->target = src;
    e2->weight = weight;
    dest->edges = g_slist_append(dest->edges, e2);
  }
}

GraphNode *find_graph_node_by_val(Graph *g, const char *val_str) {
  if (!g)
    return NULL;
  // Naive search
  for (GSList *l = g->nodes; l; l = l->next) {
    GraphNode *n = (GraphNode *)l->data;
    char buf[128];
    if (g->type == DATA_INT)
      snprintf(buf, 128, "%d", *(int *)n->data);
    else if (g->type == DATA_FLOAT)
      snprintf(buf, 128, "%.2f", *(float *)n->data);
    else if (g->type == DATA_CHAR)
      snprintf(buf, 128, "%c", *(char *)n->data);
    else if (g->type == DATA_STRING)
      snprintf(buf, 128, "%s", *(char **)n->data);

    if (strcmp(buf, val_str) == 0)
      return n;
  }
  return NULL;
}

// === ALL PATHS ALGORITHM ===
static void find_paths_dfs(GraphNode *u, GraphNode *d, int *visited_ids,
                           GraphNode **path, int path_index, Graph *g,
                           GString *out_log, int *path_count) {
  if (*path_count >= 50)
    return; // Limit results

  visited_ids[u->id] = 1;
  path[path_index] = u;
  path_index++;

  if (u == d) {
    (*path_count)++;
    g_string_append_printf(out_log, "Chemin %d: ", *path_count);
    for (int i = 0; i < path_index; i++) {
      char buf[64];
      if (g->type == DATA_INT)
        snprintf(buf, 64, "%d", *(int *)path[i]->data);
      else if (g->type == DATA_FLOAT)
        snprintf(buf, 64, "%.1f", *(float *)path[i]->data);
      else if (g->type == DATA_CHAR)
        snprintf(buf, 64, "%c", *(char *)path[i]->data);
      else
        snprintf(buf, 64, "%s", *(char **)path[i]->data);

      g_string_append_printf(out_log, "%s%s", buf,
                             (i < path_index - 1) ? " -> " : "");
    }

    // Calculate Cost
    double cost = 0;
    for (int i = 0; i < path_index - 1; i++) {
      for (GSList *e = path[i]->edges; e; e = e->next) {
        if (((GraphEdge *)e->data)->target == path[i + 1]) {
          cost += ((GraphEdge *)e->data)->weight;
          break;
        }
      }
    }
    g_string_append_printf(out_log, " (Coût: %.0f)\n", cost);
  } else {
    for (GSList *e = u->edges; e; e = e->next) {
      GraphNode *v = ((GraphEdge *)e->data)->target;
      if (!visited_ids[v->id]) {
        find_paths_dfs(v, d, visited_ids, path, path_index, g, out_log,
                       path_count);
      }
    }
  }

  path_index--;
  visited_ids[u->id] = 0;
}

void find_all_paths(AppData *app, GraphNode *start, GraphNode *end,
                    GString *log) {
  if (!app->current_graph)
    return;

  int max_id = 0;
  for (GSList *l = app->current_graph->nodes; l; l = l->next) {
    if (((GraphNode *)l->data)->id > max_id)
      max_id = ((GraphNode *)l->data)->id;
  }

  int *visited = calloc(max_id + 1, sizeof(int));
  GraphNode **path = calloc(max_id + 1, sizeof(GraphNode *));
  int path_count = 0;

  g_string_append_printf(log, "\n=== TOUS LES CHEMINS POSSIBLES (DFS) ===\n");
  find_paths_dfs(start, end, visited, path, 0, app->current_graph, log,
                 &path_count);

  if (path_count == 0)
    g_string_append(log, "Aucun chemin trouvé.\n");

  free(visited);
  free(path);
}

// Algorithms
void reset_graph_algo_state(Graph *g) {
  for (GSList *l = g->nodes; l; l = l->next) {
    GraphNode *n = (GraphNode *)l->data;
    n->dist = 1e18; // Infinity
    n->prev = NULL;
    n->visited = FALSE;
  }
}

// 0=Dijkstra, 1=Bellman, 2=Floyd
void run_graph_path(AppData *app, int algo_idx) {
  if (!app->current_graph)
    return;

  GraphNode *start = NULL;
  GraphNode *end = NULL;

  if (app->graph_sel_source && app->graph_sel_dest) {
    start = app->graph_sel_source;
    end = app->graph_sel_dest;
  } else {
    // Fallback to text (optional, but good to keep if user still uses inputs)
    // BUT, the requirement is "Selection MUST be done by mouse".
    // We can just rely on selection.
    const char *start_s = gtk_entry_get_text(GTK_ENTRY(app->graph_start_entry));
    const char *end_s = gtk_entry_get_text(GTK_ENTRY(app->graph_end_entry));
    if (start_s && *start_s && end_s && *end_s) {
      start = find_graph_node_by_val(app->current_graph, start_s);
      end = find_graph_node_by_val(app->current_graph, end_s);
    }
  }

  if (!start || !end) {
    set_status(app, "Sélectionner source et destination (clic)");
    return;
  }

  // Update text entries for clarity
  // We can convert node data to string
  // ... (helper to get string needed, assuming simple types)
  // Just show ID or status is enough
  set_status(app, "Calcul du chemin (Algo %d)...", algo_idx);

  GString *log = g_string_new("");

  // 1. Give ALL Paths first
  find_all_paths(app, start, end, log);
  g_string_append(log, "\n=== CHEMIN LE PLUS COURT ===\n");

  // Pre-check for negative weights (Dijkstra)
  if (algo_idx == 0) {
    for (GSList *l = app->current_graph->nodes; l; l = l->next) {
      GraphNode *u = (GraphNode *)l->data;
      for (GSList *e = u->edges; e; e = e->next) {
        if (((GraphEdge *)e->data)->weight < 0) {
          set_status(app, "Erreur: Poids négatifs interdits pour Dijkstra !");
          return;
        }
      }
    }
  }

  reset_graph_algo_state(app->current_graph);

  if (algo_idx == 2) {
    set_pseudo_code(app,
                    "FLOYD_WARSHALL(G):\n"
                    "  dist = Matrice de poids (u, v)\n"
                    "  Pour k de 1 \xC3\xA0 N faire:\n"
                    "    Pour i de 1 \xC3\xA0 N faire:\n"
                    "      Pour j de 1 \xC3\xA0 N faire:\n"
                    "        Si dist[i][j] > dist[i][k] + dist[k][j] alors:\n"
                    "          dist[i][j] = dist[i][k] + dist[k][j]\n"
                    "        Fin Si\n"
                    "      Fin Pour\n"
                    "    Fin Pour\n"
                    "  Fin Pour");
    // === FLOYD-WARSHALL ===
    // Map Nodes to 0..N-1
    int N = app->current_graph->node_count;
    GraphNode **map = g_new0(GraphNode *, N);
    int idx = 0;
    for (GSList *l = app->current_graph->nodes; l; l = l->next)
      map[idx++] = (GraphNode *)l->data;

    // Indices of start/end
    int s_idx = -1, e_idx = -1;
    for (int i = 0; i < N; i++) {
      if (map[i] == start)
        s_idx = i;
      if (map[i] == end)
        e_idx = i;
    }

    // Init matrices
    double *dist = g_new0(double, N *N);
    int *next_hop = g_new0(int, N *N); // Store next node index

    for (int i = 0; i < N * N; i++) {
      dist[i] = 1e18;
      next_hop[i] = -1;
    }
    for (int i = 0; i < N; i++)
      dist[i * N + i] = 0;

    // Fill edges
    for (int i = 0; i < N; i++) {
      for (GSList *e = map[i]->edges; e; e = e->next) {
        GraphEdge *edge = (GraphEdge *)e->data;
        int j = -1;
        // Find j (inefficient but OK for N<100)
        for (int k = 0; k < N; k++)
          if (map[k] == edge->target) {
            j = k;
            break;
          }

        if (j != -1) {
          if (edge->weight < dist[i * N + j]) {
            dist[i * N + j] = edge->weight;
            next_hop[i * N + j] = j;
          }
        }
      }
    }

    // Core Algorithm
    for (int k = 0; k < N; k++) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          if (dist[i * N + k] < 1e18 && dist[k * N + j] < 1e18) {
            if (dist[i * N + k] + dist[k * N + j] < dist[i * N + j]) {
              dist[i * N + j] = dist[i * N + k] + dist[k * N + j];
              next_hop[i * N + j] = next_hop[i * N + k];
            }
          }
        }
      }
    }

    // Extract Result
    start->dist = 0;
    if (dist[s_idx * N + e_idx] < 1e18) {
      end->dist = dist[s_idx * N + e_idx];

      // Reconstruct path for visual highlight (set prev pointers)
      int curr = s_idx;
      while (curr != e_idx) {
        int nxt = next_hop[curr * N + e_idx];
        if (nxt == -1)
          break;

        map[nxt]->prev = map[curr];
        map[curr]->visited = TRUE;
        map[nxt]->visited = TRUE;

        curr = nxt;
      }
    } else {
      end->dist = 1e18;
    }

    g_free(dist);
    g_free(next_hop);
    g_free(map);

  } else {
    // === DIJKSTRA / BELLMAN-FORD ===
    start->dist = 0;
    int n_count = app->current_graph->node_count;

    if (algo_idx == 0) { // Dijkstra
      set_pseudo_code(app, "DIJKSTRA(G, source):\n"
                           "  Pour chaque sommet v dans G faire:\n"
                           "    dist[v] = INFINI, prec[v] = NUL\n"
                           "  Fin Pour\n"
                           "  dist[source] = 0\n"
                           "  Q = Tous les sommets de G\n"
                           "  Tant que Q n'est pas vide faire:\n"
                           "    u = Sommet dans Q avec dist minimale\n"
                           "    Retirer u de Q\n"
                           "    Pour chaque voisin v de u faire:\n"
                           "      alt = dist[u] + poids(u, v)\n"
                           "      Si alt < dist[v] alors:\n"
                           "        dist[v] = alt, prec[v] = u\n"
                           "      Fin Si\n"
                           "    Fin Pour\n"
                           "  Fin Tant que");
      for (int i = 0; i < n_count; i++) {
        GraphNode *u = NULL;
        double min_d = 1e18;
        for (GSList *l = app->current_graph->nodes; l; l = l->next) {
          GraphNode *x = (GraphNode *)l->data;
          if (!x->visited && x->dist < min_d) {
            min_d = x->dist;
            u = x;
          }
        }

        if (!u || min_d >= 1e18)
          break;
        u->visited = TRUE;
        if (u == end)
          break;

        for (GSList *e_it = u->edges; e_it; e_it = e_it->next) {
          GraphEdge *edge = (GraphEdge *)e_it->data;
          GraphNode *v = edge->target;
          if (!v->visited && u->dist + edge->weight < v->dist) {
            v->dist = u->dist + edge->weight;
            v->prev = u;
          }
        }
      }
    } else { // Bellman-Ford
      set_pseudo_code(
          app, "BELLMAN_FORD(G, source):\n"
               "  Pour chaque sommet v dans G faire:\n"
               "    dist[v] = INFINI, prec[v] = NUL\n"
               "  Fin Pour\n"
               "  dist[source] = 0\n"
               "  R\xC3\xA9p\xC3\xA9ter N-1 fois:\n"
               "    Pour chaque ar\xC3\xAAte (u, v) avec poids w faire:\n"
               "      Si dist[u] + w < dist[v] alors:\n"
               "        dist[v] = dist[u] + w, prec[v] = u\n"
               "      Fin Si\n"
               "    Fin Pour\n"
               "  Fin R\xC3\xA9p\xC3\xA9ter");
      for (int i = 0; i < n_count - 1; i++) {
        gboolean changed = FALSE;
        for (GSList *l = app->current_graph->nodes; l; l = l->next) {
          GraphNode *u = (GraphNode *)l->data;
          if (u->dist >= 1e18)
            continue;
          for (GSList *e_it = u->edges; e_it; e_it = e_it->next) {
            GraphEdge *edge = (GraphEdge *)e_it->data;
            GraphNode *v = edge->target;
            if (u->dist + edge->weight < v->dist) {
              v->dist = u->dist + edge->weight;
              v->prev = u;
              changed = TRUE;
            }
          }
        }
        if (!changed)
          break;
      }
    }
  }

  // Log initialized above
  g_string_append_printf(
      log, "Chemin de N%d vers N%d (%s):\n", start->id, end->id,
      algo_idx == 0 ? "Dijkstra"
                    : (algo_idx == 1 ? "Bellman-Ford" : "Floyd-Warshall"));

  if (end->dist < 1e18) {
    g_string_append_printf(log, "Distance totale: %.2f\nPath: ", end->dist);
    GList *path = NULL;
    GraphNode *curr = end;

    int safeguard = 0;
    while (curr && safeguard++ < app->current_graph->node_count + 1) {
      path = g_list_prepend(path, curr);
      if (curr == start)
        break;
      curr = curr->prev;
    }

    for (GList *l = path; l; l = l->next) {
      GraphNode *zn = (GraphNode *)l->data;
      char buf[64];
      if (app->current_graph->type == DATA_INT)
        snprintf(buf, 64, "%d", *(int *)zn->data);
      else if (app->current_graph->type == DATA_FLOAT)
        snprintf(buf, 64, "%.1f", *(float *)zn->data);
      else if (app->current_graph->type == DATA_CHAR)
        snprintf(buf, 64, "%c", *(char *)zn->data);
      else
        snprintf(buf, 64, "%s", *(char **)zn->data);
      g_string_append_printf(log, "%s%s", buf, l->next ? " -> " : "");
    }
    g_list_free(path);
  } else {
    g_string_append(log, "Aucun chemin trouvé.");
  }

  gtk_text_buffer_set_text(app->graph_log_buffer, log->str, -1);
  g_string_free(log, TRUE);
  gtk_widget_queue_draw(app->graph_canvas);
}

// Helper to get string rep
char *get_graph_representation(Graph *g, int type) { // 0=Matrix, 1=List
  if (!g || !g->nodes)
    return g_strdup("Graphe vide.");
  GString *s = g_string_new("");

  // Map ID to Index
  int count = g->node_count;
  GraphNode **nodes_arr = g_new0(GraphNode *, count);
  int i = 0;
  for (GSList *l = g->nodes; l; l = l->next)
    nodes_arr[i++] = (GraphNode *)l->data;

  // Header
  if (type == 0) { // Matrix
    g_string_append(s, "      ");
    for (int i = 0; i < count; i++) {
      char buf[32];
      if (g->type == DATA_INT)
        snprintf(buf, 32, "%d", *(int *)nodes_arr[i]->data);
      else if (g->type == DATA_CHAR)
        snprintf(buf, 32, "%c", *(char *)nodes_arr[i]->data);
      else
        snprintf(buf, 32, "N%d", i); // Fallback usually
      g_string_append_printf(s, "%-6s", buf);
    }
    g_string_append(s, "\n");

    for (int i = 0; i < count; i++) {
      char buf[32];
      if (g->type == DATA_INT)
        snprintf(buf, 32, "%d", *(int *)nodes_arr[i]->data);
      else if (g->type == DATA_CHAR)
        snprintf(buf, 32, "%c", *(char *)nodes_arr[i]->data);
      else
        snprintf(buf, 32, "N%d", i);
      g_string_append_printf(s, "%-6s", buf);

      for (int j = 0; j < count; j++) {
        // Find edge i->j
        double w = 0;
        gboolean found = FALSE;
        for (GSList *e = nodes_arr[i]->edges; e; e = e->next) {
          GraphEdge *edge = (GraphEdge *)e->data;
          if (edge->target == nodes_arr[j]) {
            w = edge->weight;
            found = TRUE;
            break;
          }
        }
        if (found)
          g_string_append_printf(s, "%-6.0f", w);
        else
          g_string_append(s, "-     ");
      }
      g_string_append(s, "\n");
    }
  } else { // List
    for (int i = 0; i < count; i++) {
      char buf[64];
      if (g->type == DATA_INT)
        snprintf(buf, 64, "%d", *(int *)nodes_arr[i]->data);
      else
        snprintf(buf, 64, "N%d", i);

      g_string_append_printf(s, "%s -> ", buf);
      for (GSList *e = nodes_arr[i]->edges; e; e = e->next) {
        GraphEdge *edge = (GraphEdge *)e->data;
        char buf2[64];
        if (g->type == DATA_INT)
          snprintf(buf2, 64, "%d", *(int *)edge->target->data);
        else
          snprintf(buf2, 64, "N%d", edge->target->id); // Simplify
        g_string_append_printf(s, "[%s:%.0f] ", buf2, edge->weight);
      }
      g_string_append(s, "\n");
    }
  }
  g_free(nodes_arr);
  return g_string_free(s, FALSE);
}

void show_graph_representation(AppData *app) {
  GtkWidget *dialog = gtk_dialog_new_with_buttons(
      "Repr\xC3\xA9sentation du Graphe", GTK_WINDOW(app->window),
      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "Fermer",
      GTK_RESPONSE_CLOSE, NULL);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
  apply_css(dialog, app);

  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_container_set_border_width(GTK_CONTAINER(content), 10);

  GtkWidget *notebook = gtk_notebook_new();
  gtk_widget_set_name(notebook, "rep_notebook");
  gtk_box_pack_start(GTK_BOX(content), notebook, TRUE, TRUE, 0);

  // Matrix Tab
  GtkWidget *scroll1 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scroll1, -1, 350);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  GtkWidget *tv1 = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(tv1), FALSE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(tv1), TRUE);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(tv1), 15);
  gtk_text_view_set_top_margin(GTK_TEXT_VIEW(tv1), 15);
  gtk_container_add(GTK_CONTAINER(scroll1), tv1);
  char *mat = get_graph_representation(app->current_graph, 0);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv1)), mat,
                           -1);
  g_free(mat);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll1,
                           gtk_label_new("Matrice d'Adjacence"));

  // List Tab
  GtkWidget *scroll2 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scroll2, -1, 350);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll2),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  GtkWidget *tv2 = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(tv2), FALSE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(tv2), TRUE);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(tv2), 15);
  gtk_text_view_set_top_margin(GTK_TEXT_VIEW(tv2), 15);
  gtk_container_add(GTK_CONTAINER(scroll2), tv2);
  char *lst = get_graph_representation(app->current_graph, 1);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv2)), lst,
                           -1);
  g_free(lst);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll2,
                           gtk_label_new("Liste d'Adjacence"));

  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

void on_generate_graph(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  DataType t =
      gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_data_type_combo));
  int gtype_idx =
      gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_type_combo));
  gboolean directed = (gtype_idx == 0);

  // Seed for true randomness
  srand(time(NULL));

  // Clear existing
  app->current_graph = create_graph(t, directed);

  // Randomize node count (5 to 8)
  int count = 5 + (rand() % 4);
  GraphNode **nodes = g_new0(GraphNode *, count);

  for (int i = 0; i < count; i++) {
    void *val = malloc(get_element_size(t));
    if (t == DATA_INT)
      *(int *)val = 10 + (rand() % 90);
    else if (t == DATA_CHAR)
      *(char *)val = 'A' + (rand() % 26);
    else if (t == DATA_STRING)
      *((char **)val) = g_strdup_printf("N%d", 1 + (rand() % 20));
    else
      *(float *)val = (float)(10 + (rand() % 90)) / 1.5;

    nodes[i] = create_graph_node(app->current_graph, val);
  }

  // Create a structured but varied graph
  for (int i = 0; i < count; i++) {
    // Hamiltonian Cycle (ensures connectivity)
    add_graph_edge(app->current_graph, nodes[i], nodes[(i + 1) % count],
                   5 + (rand() % 15));

    // Random additional edges for complexity (30% chance)
    if (rand() % 100 < 35) {
      int target = (i + 2 + (rand() % (count - 3))) % count;
      if (target != i) {
        add_graph_edge(app->current_graph, nodes[i], nodes[target],
                       10 + (rand() % 10));
      }
    }
  }

  // Layout
  GtkAllocation alloc;
  gtk_widget_get_allocation(app->graph_canvas, &alloc);
  double cx = (alloc.width > 0) ? alloc.width / 2.0 : 400.0;
  double cy = (alloc.height > 0) ? alloc.height / 2.0 : 250.0;
  double r =
      (alloc.width > 0) ? MIN(alloc.width, alloc.height) / 2.0 - 80.0 : 150.0;
  if (r < 50)
    r = 130;

  layout_graph_circular(app->current_graph, cx, cy, r);

  set_status(app, "Nouveau graphe aléatoire généré (%d sommets)", count);
  add_history_entry(app, "Génération Graphe Varié");
  update_dashboard_stats(app, 3, app->current_graph->node_count, 0);
  gtk_widget_queue_draw(app->graph_canvas);
  g_free(nodes);
}

void on_graph_action(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  const char *id = gtk_widget_get_name(widget);

  if (g_strcmp0(id, "add_node") == 0) {
    if (!app->current_graph) {
      DataType t =
          gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_data_type_combo));
      int gtype_idx = 0;
      if (app->graph_type_combo)
        gtype_idx =
            gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_type_combo));
      app->current_graph = create_graph(t, gtype_idx == 0);
    }

    // Generate Random Value Automatically
    static char rnd_buf[32];
    srand(time(NULL) + rand()); // Mix logic
    if (app->current_graph->type == DATA_INT)
      snprintf(rnd_buf, 32, "%d", rand() % 100);
    else if (app->current_graph->type == DATA_FLOAT)
      snprintf(rnd_buf, 32, "%.1f", (float)(rand() % 1000) / 10.0);
    else if (app->current_graph->type == DATA_CHAR)
      snprintf(rnd_buf, 32, "%c", 'A' + rand() % 26);
    else
      snprintf(rnd_buf, 32, "N%d", rand() % 100);

    const char *val = rnd_buf;

    // Parse value
    void *data = malloc(get_element_size(app->current_graph->type));
    if (app->current_graph->type == DATA_INT)
      *(int *)data = atoi(val);
    else if (app->current_graph->type == DATA_FLOAT)
      *(float *)data = atof(val);
    else if (app->current_graph->type == DATA_CHAR)
      *(char *)data = val[0];
    else
      *((char **)data) = g_strdup(val);

    create_graph_node(app->current_graph, data);

    // Auto-Layout Circular
    GtkAllocation alloc;
    gtk_widget_get_allocation(app->graph_canvas, &alloc);
    double cx = alloc.width / 2.0;
    double cy = alloc.height / 2.0;
    double r = MIN(alloc.width, alloc.height) / 2.0 - 50.0;
    if (r < 50)
      r = 50;
    layout_graph_circular(app->current_graph, cx, cy, r);

    set_status(app, "Noeud ajouté");
    gtk_entry_set_text(GTK_ENTRY(app->graph_val_entry), ""); // Clear entry
    gtk_widget_queue_draw(app->graph_canvas);
    update_dashboard_stats(app, 3, app->current_graph->node_count, 0);

  } else if (g_strcmp0(id, "add_edge") == 0) {
    if (!app->current_graph)
      return;
    const char *s = gtk_entry_get_text(GTK_ENTRY(app->graph_src_entry));
    const char *d = gtk_entry_get_text(GTK_ENTRY(app->graph_dst_entry));
    const char *w = gtk_entry_get_text(GTK_ENTRY(app->graph_weight_entry));

    GraphNode *ns = app->graph_sel_source
                        ? app->graph_sel_source
                        : find_graph_node_by_val(app->current_graph, s);
    GraphNode *nd = app->graph_sel_dest
                        ? app->graph_sel_dest
                        : find_graph_node_by_val(app->current_graph, d);
    if (ns && nd) {
      double weight_val = 0;
      gboolean calc_dist = TRUE;

      if (w && *w) {
        weight_val = atof(w);
        if (weight_val != 0)
          calc_dist = FALSE;
      }

      if (calc_dist) {
        double dx = ns->x - nd->x;
        double dy = ns->y - nd->y;
        weight_val = sqrt(dx * dx + dy * dy);
        // Scale down for realism (e.g. 1 unit = 10 pixels)
        weight_val = weight_val / 10.0;
        if (weight_val < 1.0)
          weight_val = 1.0;

        weight_val = round(weight_val); // Round to integer for cleaner look

        // Update UI to show the calculated weight
        char w_str[64];
        snprintf(w_str, sizeof(w_str), "%.0f", weight_val);
        gtk_entry_set_text(GTK_ENTRY(app->graph_weight_entry), w_str);
      }

      add_graph_edge(app->current_graph, ns, nd, weight_val);
      // Auto deselect after link? The prompt doesn't say. Let's keep
      // selection for chain linking? "L'utilisateur ne doit pas saisir..."
      // usually implies smooth flow. Let's clear destination so user can
      // click next node to link from same source? Or clear both? Let's clear
      // both to be safe.
      app->graph_sel_source = NULL;
      app->graph_sel_dest = NULL;
      gtk_widget_queue_draw(app->graph_canvas);

      set_status(app, "Arc ajouté (Poids: %.0f)", weight_val);
      gtk_entry_set_text(GTK_ENTRY(app->graph_weight_entry),
                         ""); // Clear weight
    } else {
      set_status(app, "Sélectionnez 2 nœuds à la souris d'abord");
    }
  } else if (g_strcmp0(id, "reset_graph") == 0) {
    if (app->current_graph) {
      // Free graph logic should be robust
      // For now, simpler: just nullify if we don't have deep free func ready
      // (we do have free_graph logic? No, we have free_nodes logic?) We have
      // create_graph but no destroy_graph fully? Let's iterate and free.
      // ... Simple hack: just make a new empty graph

      DataType t =
          gtk_combo_box_get_active(GTK_COMBO_BOX(app->data_type_combo));
      int gtype_idx =
          gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_type_combo));
      gboolean directed = (gtype_idx == 0);
      // Free nodes
      // TODO: Real free
      // ...

      app->current_graph = create_graph(t, directed);
      app->graph_sel_source = NULL;
      app->graph_sel_dest = NULL;
      gtk_widget_queue_draw(app->graph_canvas);
      set_status(app, "Graphe réinitialisé");
      set_pseudo_code(
          app,
          "S\xC3\xA9lectionnez un algorithme\npour voir son pseudo-code ici.");
    }
  } else if (g_strcmp0(id, "run_algo") == 0) {
    int algo = gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_algo_combo));
    run_graph_path(app, algo);
    const char *aname = gtk_combo_box_text_get_active_text(
        GTK_COMBO_BOX_TEXT(app->graph_algo_combo));
    add_history_entry(
        app, g_strdup_printf("Algo Graphe (%s)", aname ? aname : "Chemin"));
  } else if (g_strcmp0(id, "rep_graph") == 0) {
    if (app->current_graph) {
      show_graph_representation(app);
      add_history_entry(app, "Affichage Repr\xC3\xA9sentation Graphe");
    } else
      set_status(app, "Aucun graphe.");
  }

  if (g_strcmp0(id, "add_node") == 0)
    add_history_entry(app, "Ajout Noeud Graphe");
  if (g_strcmp0(id, "add_edge") == 0)
    add_history_entry(app, "Ajout Ar\xC3\xAAte Graphe");

  update_dashboard_stats(
      app, 3, app->current_graph ? app->current_graph->node_count : 0, -1);
}

void on_graph_type_changed(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->current_graph)
    return;
  int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
  app->current_graph->is_directed = (idx == 0);
  gtk_widget_queue_draw(app->graph_canvas);
}

// --- Interactions Souris (Graphes) ---

gboolean on_graph_button_press(GtkWidget *widget, GdkEventButton *event,
                               gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->current_graph || !app->current_graph->nodes)
    return FALSE;

  // Check collision with nodes
  for (GSList *l = app->current_graph->nodes; l; l = l->next) {
    GraphNode *n = (GraphNode *)l->data;
    double dx = event->x - n->x;
    double dy = event->y - n->y;
    if (dx * dx + dy * dy <= 25 * 25) { // Hit radius slightly larger
      if (event->button == 1) {         // Left click

        // Drag logic
        app->graph_dragging = TRUE;
        app->graph_drag_node = n;

        // Selection Logic
        if (!app->graph_sel_source) {
          app->graph_sel_source = n;
          set_status(app, "Source sélectionnée (ID: %d)", n->id);
          // Auto fill fields? No, user wants mouse only, but filling fields
          // helps debug
        } else if (!app->graph_sel_dest && n != app->graph_sel_source) {
          app->graph_sel_dest = n;
          set_status(app, "Destination sélectionnée (ID: %d)", n->id);

          // Auto-calculate shortest path on selection complete?
          // Requirement says: "Le programme doit automatiquement calculer..."
          // Let's trigger it? Or wait for button?
          // "Les nœuds sélectionnés doivent être visuellement mis en
          // évidence"
          // - done in Draw "Le programme doit automatiquement calculer et
          // afficher le chemin le plus court" We can trigger path finding
          // here if Algo is not Floyd (Floyd is all-pairs, might be heavy? No
          // typical N is small)
          int algo =
              gtk_combo_box_get_active(GTK_COMBO_BOX(app->graph_algo_combo));
          run_graph_path(app, algo);

        } else {
          // If clicking again, maybe reset?
          // If clicking source again -> Deselect?
          if (n == app->graph_sel_source) {
            app->graph_sel_source = NULL;
            app->graph_sel_dest = NULL; // Reset sequence
            set_status(app, "Sélection annulée");
          } else if (n == app->graph_sel_dest) {
            app->graph_sel_dest = NULL;
            set_status(app, "Destination désélectionnée");
          } else {
            // New Source
            app->graph_sel_source = n;
            app->graph_sel_dest = NULL;
            set_status(app, "Nouvelle source sélectionnée (ID: %d)", n->id);
          }
        }

        gtk_widget_queue_draw(app->graph_canvas);
        return TRUE;
      }
    }
  }
  return FALSE;
}

gboolean on_graph_motion(GtkWidget *widget, GdkEventMotion *event,
                         gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (app->graph_dragging && app->graph_drag_node) {
    app->graph_drag_node->x = event->x;
    app->graph_drag_node->y = event->y;
    gtk_widget_queue_draw(app->graph_canvas);
    return TRUE;
  }
  if (app->graph_linking) {
    app->graph_mouse_x = event->x;
    app->graph_mouse_y = event->y;
    gtk_widget_queue_draw(app->graph_canvas);
    return TRUE;
  }
  return FALSE;
}

gboolean on_graph_button_release(GtkWidget *widget, GdkEventButton *event,
                                 gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (app->graph_dragging) {
    app->graph_dragging = FALSE;
    app->graph_drag_node = NULL;
    return TRUE;
  }
  if (app->graph_linking) {
    app->graph_linking = FALSE;
    // Check drop target
    for (GSList *l = app->current_graph->nodes; l; l = l->next) {
      GraphNode *n = (GraphNode *)l->data;
      double dx = event->x - n->x;
      double dy = event->y - n->y;
      if (dx * dx + dy * dy <= 25 * 25) { // Hit
        if (n != app->graph_link_start) {
          // Create Link
          double dist = sqrt(pow(n->x - app->graph_link_start->x, 2) +
                             pow(n->y - app->graph_link_start->y, 2));
          double weight = round(dist / 10.0);
          if (weight < 1)
            weight = 1;

          // Check for duplicates? add_graph_edge allows multi-multigraphs?
          // Simple implem: just add it.
          add_graph_edge(app->current_graph, app->graph_link_start, n, weight);
          set_status(app, "Lien créé: N%d -> N%d (Poids: %.0f)",
                     app->graph_link_start->id, n->id, weight);
          gtk_widget_queue_draw(app->graph_canvas);
        }
        break;
      }
    }
    app->graph_link_start = NULL;
    gtk_widget_queue_draw(app->graph_canvas);
    return TRUE;
  }
  return FALSE;
}

gboolean on_draw_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->current_graph)
    return FALSE;

  // Theme Colors
  double bg_r, bg_g, bg_b;
  double edge_r, edge_g, edge_b;
  double body_r, body_g, body_b;
  double border_r, border_g, border_b;
  double text_r, text_g, text_b;

  if (app->is_dark_theme) {
    bg_r = 0.04;
    bg_g = 0.01;
    bg_b = 0.10; // #0A031A
    edge_r = 0.3;
    edge_g = 0.4;
    edge_b = 0.6;
    body_r = 0.08;
    body_g = 0.05;
    body_b = 0.17; // #150D2B
    border_r = 0.48;
    border_g = 0.36;
    border_b = 1.0; // Saphir
    text_r = 1.0;
    text_g = 1.0;
    text_b = 1.0;
  } else {
    bg_r = 0.96;
    bg_g = 0.96;
    bg_b = 0.98;
    edge_r = 0.7;
    edge_g = 0.7;
    edge_b = 0.8;
    body_r = 1.0;
    body_g = 1.0;
    body_b = 1.0;
    border_r = 0.36;
    border_g = 0.25;
    border_b = 0.83; // #5D3FD3 (LIGHT_ACCENT_PRIMARY)
    text_r = 0.07;
    text_g = 0.04;
    text_b = 0.14; // #120B24 (LIGHT_TEXT_PRIMARY)
  }

  // Background
  cairo_set_source_rgb(cr, bg_r, bg_g, bg_b);
  cairo_paint(cr);

  // Edges
  for (GSList *l = app->current_graph->nodes; l; l = l->next) {
    GraphNode *u = (GraphNode *)l->data;
    for (GSList *e = u->edges; e; e = e->next) {
      GraphEdge *edge = (GraphEdge *)e->data;
      GraphNode *v = edge->target;

      // Visual dedup for undirected
      if (!app->current_graph->is_directed && u->id > v->id)
        continue;

      gboolean is_path = FALSE;
      if (v->prev == u && v->visited && u->visited)
        is_path = TRUE;
      else if (!app->current_graph->is_directed && u->prev == v && u->visited &&
               v->visited)
        is_path = TRUE;

      // --- NEON GLOW FOR EDGES ---
      if (is_path) {
        // Thick Glow Layer
        cairo_set_source_rgba(cr, 0.0, 1.0, 0.6, 0.15);
        cairo_set_line_width(cr, 8.0);
        cairo_move_to(cr, u->x, u->y);
        cairo_line_to(cr, v->x, v->y);
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 0.0, 1.0, 0.6); // Neon Green
        cairo_set_line_width(cr, 3.0);
      } else {
        // Normal Glow Layer
        cairo_set_source_rgba(cr, edge_r, edge_g, edge_b, 0.2);
        cairo_set_line_width(cr, 4.0);
        cairo_move_to(cr, u->x, u->y);
        cairo_line_to(cr, v->x, v->y);
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, edge_r, edge_g, edge_b);
        cairo_set_line_width(cr, 1.5);
      }

      if (app->current_graph->is_directed) {
        // Offset arrow to stop at node boundary (Radius ~20)
        double angle = atan2(v->y - u->y, v->x - u->x);
        double dist = sqrt(pow(v->x - u->x, 2) + pow(v->y - u->y, 2));
        double r = 24.0; // Node radius + border
        if (dist > r * 2) {
          double x2 = v->x - r * cos(angle);
          double y2 = v->y - r * sin(angle);
          draw_arrow(cr, u->x, u->y, x2, y2);
        }
      } else {
        cairo_move_to(cr, u->x, u->y);
        cairo_line_to(cr, v->x, v->y);
        cairo_stroke(cr);
      }

      // Weight Label
      double mx = (u->x + v->x) / 2;
      double my = (u->y + v->y) / 2;
      char wbuf[32];
      snprintf(wbuf, 32, "%.0f", edge->weight);

      cairo_set_font_size(cr, 11);
      cairo_text_extents_t ext;
      cairo_text_extents(cr, wbuf, &ext);

      // Label Background (Dark box)
      cairo_set_source_rgba(cr, bg_r, bg_g, bg_b, 0.85);
      cairo_rectangle(cr, mx - ext.width / 2 - 4, my - ext.height / 2 - 4,
                      ext.width + 8, ext.height + 8);
      cairo_fill(cr);

      cairo_set_source_rgb(cr, 0.0, 1.0, 1.0); // Bright Cyan
      cairo_move_to(cr, mx - ext.width / 2, my + ext.height / 2);
      cairo_show_text(cr, wbuf);
    }
  }

  // Nodes
  for (GSList *l = app->current_graph->nodes; l; l = l->next) {
    GraphNode *u = (GraphNode *)l->data;

    // --- NEON GLOW FOR NODES ---
    double node_r = 20.0;
    cairo_set_source_rgba(cr, border_r, border_g, border_b, 0.1);
    for (int i = 1; i <= 3; i++) {
      cairo_arc(cr, u->x, u->y, node_r + i * 2, 0, 2 * M_PI);
      cairo_fill(cr);
    }

    // Shadow
    cairo_set_source_rgba(cr, 0, 0, 0, 0.3);
    cairo_arc(cr, u->x + 3, u->y + 3, node_r, 0, 2 * M_PI);
    cairo_fill(cr);

    // Body
    cairo_set_source_rgb(cr, body_r, body_g, body_b);
    cairo_arc(cr, u->x, u->y, node_r, 0, 2 * M_PI);
    cairo_fill_preserve(cr);

    // Border
    cairo_set_line_width(cr, 2.5);
    cairo_set_source_rgb(cr, border_r, border_g, border_b);

    if (u->visited && (u->prev || u->dist == 0)) {
      cairo_set_source_rgb(cr, 0.0, 1.0, 0.6); // Neon Green
    }

    // Selection Highlights
    if (u == app->graph_sel_source) {
      cairo_set_source_rgb(cr, 1.0, 0.8, 0.0); // Bright Gold
      cairo_set_line_width(cr, 5.0);
      cairo_stroke_preserve(cr);
      cairo_set_source_rgba(cr, 1.0, 0.8, 0.0, 0.2);
      cairo_arc(cr, u->x, u->y, node_r + 5, 0, 2 * M_PI);
      cairo_fill(cr);
      cairo_set_source_rgb(cr, 1.0, 0.8, 0.0);
    } else if (u == app->graph_sel_dest) {
      cairo_set_source_rgb(cr, 1.0, 0.0, 0.5); // Neon Pink
      cairo_set_line_width(cr, 5.0);
      cairo_stroke_preserve(cr);
      cairo_set_source_rgba(cr, 1.0, 0.0, 0.5, 0.2);
      cairo_arc(cr, u->x, u->y, node_r + 5, 0, 2 * M_PI);
      cairo_fill(cr);
      cairo_set_source_rgb(cr, 1.0, 0.0, 0.5);
    }

    cairo_stroke(cr);

    // Label
    char b[32] = {0};
    if (app->current_graph->type == DATA_INT)
      snprintf(b, 32, "%d", *(int *)u->data);
    else if (app->current_graph->type == DATA_FLOAT)
      snprintf(b, 32, "%.1f", *(float *)u->data);
    else if (app->current_graph->type == DATA_CHAR)
      snprintf(b, 32, "%c", *(char *)u->data);
    else
      snprintf(b, 32, "%.5s", *(char **)u->data);

    // Text Color
    cairo_set_source_rgb(cr, text_r, text_g, text_b);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, b, &ext);
    cairo_move_to(cr, u->x - ext.width / 2, u->y + ext.height / 2);
    cairo_show_text(cr, b);
  }

  // Draw Linking Line (Rubber Band)
  if (app->graph_linking && app->graph_link_start) {
    cairo_set_source_rgba(cr, 0.0, 0.8, 0.4, 0.6); // Green semi-transparent
    cairo_set_line_width(cr, 2.0);
    cairo_set_dash(cr, (double[]){5.0, 5.0}, 2, 0); // Dashed

    cairo_move_to(cr, app->graph_link_start->x, app->graph_link_start->y);
    cairo_line_to(cr, app->graph_mouse_x, app->graph_mouse_y);
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0); // Reset dash
  }

  return FALSE;
}

// Helper to draw an arrow
void draw_arrow(cairo_t *cr, double x1, double y1, double x2, double y2) {
  double angle = atan2(y2 - y1, x2 - x1);
  double arrow_len = 10.0;
  double arrow_angle = M_PI / 6.0; // 30 degrees

  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, x2, y2);
  cairo_stroke(cr);

  // Arrowhead
  arrow_len = 15.0; // Larger
  cairo_move_to(cr, x2, y2);
  cairo_line_to(cr, x2 - arrow_len * cos(angle - arrow_angle),
                y2 - arrow_len * sin(angle - arrow_angle));
  cairo_move_to(cr, x2, y2);
  cairo_line_to(cr, x2 - arrow_len * cos(angle + arrow_angle),
                y2 - arrow_len * sin(angle + arrow_angle));
  cairo_stroke(cr);
}

// =============================================================================
//                             MOTEURS DE RENDU (CAIRO)
// =============================================================================

gboolean on_draw_list(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  LinkedList *list = app->current_list;
  GtkAllocation alloc;
  gtk_widget_get_allocation(widget, &alloc);

  // Theme Colors
  double bg_r, bg_g, bg_b;
  double node_r, node_g, node_b;
  double text_r, text_g, text_b;
  double accent_r, accent_g, accent_b;

  if (app->is_dark_theme) {
    bg_r = 0.04;
    bg_g = 0.01;
    bg_b = 0.10; // #0A031A
    node_r = 0.08;
    node_g = 0.05;
    node_b = 0.17; // #150D2B
    text_r = 1.0;
    text_g = 1.0;
    text_b = 1.0;
    accent_r = 0.0;
    accent_g = 0.82;
    accent_b = 0.70; // #00D1B2 Teal
  } else {
    bg_r = 0.96;
    bg_g = 0.96;
    bg_b = 0.98;
    node_r = 1.0;
    node_g = 1.0;
    node_b = 1.0;
    text_r = 0.07;
    text_g = 0.04;
    text_b = 0.14; // #120B24 (LIGHT_TEXT_PRIMARY)
    accent_r = 0.0;
    accent_g = 0.54;
    accent_b = 0.54; // #008B8B (LIGHT_ACCENT_SECONDARY)
  }

  // Background
  cairo_set_source_rgb(cr, bg_r, bg_g, bg_b);
  cairo_paint(cr);

  if (!list || list->size == 0) {
    cairo_set_source_rgb(cr, text_r, text_g, text_b);
    cairo_select_font_face(cr, "Orbitron", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, "Liste vide", &ext);
    cairo_move_to(cr, (alloc.width - ext.width) / 2, alloc.height / 2);
    cairo_show_text(cr, "Liste vide");
    return FALSE;
  }

  int NODE_W = 70, NODE_H = 45, PAD_X = 35, PAD_Y = 60;
  int nodes_per_row = (alloc.width - 40) / (NODE_W + PAD_X);
  if (nodes_per_row < 1)
    nodes_per_row = 1;

  Node *cur = list->head;
  size_t idx = 0;
  while (cur) {
    int row = idx / nodes_per_row;
    int col = idx % nodes_per_row;
    int x = 20 + col * (NODE_W + PAD_X);
    int y = 40 + row * (NODE_H + PAD_Y);

    // Arrow
    if (idx > 0) {
      int prev_row = (idx - 1) / nodes_per_row;
      int prev_col = (idx - 1) % nodes_per_row;
      int px = 20 + prev_col * (NODE_W + PAD_X);
      int py = 40 + prev_row * (NODE_H + PAD_Y);
      cairo_set_source_rgb(cr, accent_r, accent_g, accent_b);
      cairo_set_line_width(cr, 2.0);

      if (row == prev_row) {
        // Horizontal
        double y_line = y + NODE_H / 2;
        if (list->is_doubly_linked) {
          // Double: Upper arrow ->, Lower arrow <-
          draw_arrow(cr, px + NODE_W, y_line - 5, x, y_line - 5);
          draw_arrow(cr, x, y_line + 5, px + NODE_W, y_line + 5);
        } else {
          draw_arrow(cr, px + NODE_W, y_line, x, y_line);
        }
      } else {
        // Curve
        cairo_move_to(cr, px + NODE_W / 2, py + NODE_H);
        cairo_curve_to(cr, px + NODE_W / 2, py + NODE_H + 20, x + NODE_W / 2,
                       y - 20, x + NODE_W / 2, y);
        cairo_stroke(cr);

        // Simple indicator for direction
        if (list->is_doubly_linked) {
          cairo_move_to(cr, px + NODE_W / 2 + 5, py + NODE_H);
          cairo_curve_to(cr, px + NODE_W / 2 + 5, py + NODE_H + 20,
                         x + NODE_W / 2 + 5, y - 20, x + NODE_W / 2 + 5, y);
          cairo_stroke(cr);
        }
      }
    }

    // Node Box
    cairo_set_source_rgb(cr, node_r, node_g, node_b);
    cairo_rectangle(cr, x, y, NODE_W, NODE_H);
    cairo_fill_preserve(cr);
    // Border color (Magents vs Darker Magenta)
    if (app->is_dark_theme)
      cairo_set_source_rgb(cr, 0.48, 0.36, 1.0); // #7A5CFF
    else
      cairo_set_source_rgb(cr, 0.3, 0.2, 0.7); // Darker Saphir
    cairo_stroke(cr);

    // Text
    char b[32];
    switch (list->type) {
    case DATA_INT:
      snprintf(b, 32, "%d", *(int *)cur->data);
      break;
    case DATA_FLOAT:
      snprintf(b, 32, "%.2f", *(float *)cur->data);
      break;
    case DATA_CHAR:
      snprintf(b, 32, "%c", *(char *)cur->data);
      break;
    case DATA_STRING:
      snprintf(b, 32, "%.6s", *(char **)cur->data);
      break;
    default:
      snprintf(b, 32, "...");
      break;
    }
    cairo_set_source_rgb(cr, text_r, text_g, text_b);
    cairo_set_font_size(cr, 12);
    // Center text
    cairo_text_extents_t ext;
    cairo_text_extents(cr, b, &ext);
    cairo_move_to(cr, x + (NODE_W - ext.width) / 2,
                  y + (NODE_H + ext.height) / 2);
    cairo_show_text(cr, b);

    cur = cur->next;
    idx++;
  }
  return FALSE;
}

// Tree drawing helper
static void assign_positions(TreeNode *root, GHashTable *pos_map,
                             double *next_x, int depth, gboolean is_nary) {
  if (!root)
    return;
  NodePos *np = g_new0(NodePos, 1);
  np->depth = depth;

  if (is_nary && root->children) {
    GSList *it = root->children;
    double first = -1, last = -1;
    while (it) {
      assign_positions((TreeNode *)it->data, pos_map, next_x, depth + 1,
                       is_nary);
      NodePos *cp = (NodePos *)g_hash_table_lookup(pos_map, it->data);
      if (cp) {
        if (first < 0)
          first = cp->x_index;
        last = cp->x_index;
      }
      it = it->next;
    }
    if (first < 0)
      np->x_index = (*next_x)++;
    else
      np->x_index = (first + last) / 2.0;

  } else if (!is_nary && (root->left || root->right)) {
    if (root->left)
      assign_positions(root->left, pos_map, next_x, depth + 1, is_nary);
    if (root->right)
      assign_positions(root->right, pos_map, next_x, depth + 1, is_nary);
    NodePos *lp = root->left ? g_hash_table_lookup(pos_map, root->left) : NULL;
    NodePos *rp =
        root->right ? g_hash_table_lookup(pos_map, root->right) : NULL;
    if (lp && rp)
      np->x_index = (lp->x_index + rp->x_index) / 2.0;
    else if (lp)
      np->x_index = lp->x_index + 0.5;
    else if (rp)
      np->x_index = rp->x_index - 0.5;
    else
      np->x_index = (*next_x)++;
  } else {
    np->x_index = (*next_x)++;
  }
  g_hash_table_insert(pos_map, root, np);
}

gboolean on_draw_tree(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->tree_root)
    return FALSE;

  // Theme Colors
  double bg_r, bg_g, bg_b;
  double line_r, line_g, line_b;
  double body_inner_r, body_inner_g, body_inner_b;
  double body_outer_r, body_outer_g, body_outer_b;
  double text_r, text_g, text_b;

  if (app->is_dark_theme) {
    bg_r = 0.04;
    bg_g = 0.01;
    bg_b = 0.10; // #0A031A
    line_r = 0.0;
    line_g = 0.82;
    line_b = 0.70; // Teal
    body_inner_r = 0.48;
    body_inner_g = 0.36;
    body_inner_b = 1.0; // Saphir
    body_outer_r = 0.08;
    body_outer_g = 0.05;
    body_outer_b = 0.17; // Dark Violet
    text_r = 1.0;
    text_g = 1.0;
    text_b = 1.0;
  } else {
    bg_r = 0.96;
    bg_g = 0.96;
    bg_b = 0.98;
    line_r = 0.0;
    line_g = 0.54;
    line_b = 0.54; // #008B8B (LIGHT_ACCENT_SECONDARY)
    body_inner_r = 0.36;
    body_inner_g = 0.25;
    body_inner_b = 0.83; // #5D3FD3 (LIGHT_ACCENT_PRIMARY)
    body_outer_r = 1.0;
    body_outer_g = 1.0;
    body_outer_b = 1.0; // White
    text_r = 0.07;
    text_g = 0.04;
    text_b = 0.14; // #120B24 (LIGHT_TEXT_PRIMARY)
  }

  GHashTable *pos_map = g_hash_table_new(g_direct_hash, g_direct_equal);
  double next_x = 0;
  assign_positions((TreeNode *)app->tree_root, pos_map, &next_x, 0,
                   app->tree_is_nary);

  // Calc spacing
  int depth = tree_depth((TreeNode *)app->tree_root);

  // Dynamic sizing
  // Dynamic sizing
  // Calculate required width based on node count (next_x)
  double min_node_w = 50.0;
  double total_w = (next_x + 1) * min_node_w;
  double total_h = (depth + 2) * 70.0;

  GtkAllocation alloc;
  gtk_widget_get_allocation(widget, &alloc);

  // Resize widget if needed (for scroll)
  if (total_w > alloc.width || total_h > alloc.height) {
    gtk_widget_set_size_request(widget, (int)total_w, (int)total_h);
  }

  // Use the LARGER of the actual allocation or our calculated total_w
  // This ensures that if the window hasn't resized yet, we still scale based on
  // the logical size so we don't bunch everything up or clip.
  double effective_w = (alloc.width > total_w) ? (double)alloc.width : total_w;
  double effective_h =
      (alloc.height > total_h) ? (double)alloc.height : total_h;

  // Padding
  double pad_x = 20;
  double pad_y = 20;

  double x_scale = (effective_w - 2 * pad_x) / (next_x + 1);
  if (x_scale < 1)
    x_scale = 1; // Safety

  double y_scale = (effective_h - 2 * pad_y) / (depth + 2);
  if (y_scale < 1)
    y_scale = 1;

  // We need to re-center if small
  double offset_x = pad_x;
  if ((next_x + 1) * x_scale < effective_w) {
    offset_x = (effective_w - (next_x + 1) * x_scale) / 2.0 + pad_x;
  }
  double offset_y = pad_y;

  // Background
  cairo_set_source_rgb(cr, bg_r, bg_g, bg_b);
  cairo_paint(cr);

  GHashTableIter iter;
  gpointer key, value;

  // Draw edges
  cairo_set_line_width(cr, 1.2);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
  g_hash_table_iter_init(&iter, pos_map);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    TreeNode *n = (TreeNode *)key;
    NodePos *np = (NodePos *)value;
    cairo_set_source_rgba(cr, line_r, line_g, line_b, 0.4);

    // Only draw paths relevant to current mode
    if (app->tree_is_nary && n->children) {
      for (GSList *it = n->children; it; it = it->next) {
        NodePos *cp = g_hash_table_lookup(pos_map, it->data);
        if (cp) {
          double x1 = (np->x_index + 1) * x_scale + offset_x; // Added offset
          double y1 = (np->depth + 1) * y_scale + offset_y;   // Added offset
          double x2 = (cp->x_index + 1) * x_scale + offset_x; // Added offset
          double y2 = (cp->depth + 1) * y_scale + offset_y;   // Added offset
          cairo_move_to(cr, x1, y1);
          cairo_line_to(cr, x2, y2);
          cairo_stroke(cr);
        }
      }
    }

    // Draw Binary edges if NOT N-ary
    if (!app->tree_is_nary) {
      if (n->left) {
        NodePos *cp = g_hash_table_lookup(pos_map, n->left);
        if (cp) {
          double x1 = (np->x_index + 1) * x_scale + offset_x;
          double y1 = (np->depth + 1) * y_scale + offset_y;
          double x2 = (cp->x_index + 1) * x_scale + offset_x;
          double y2 = (cp->depth + 1) * y_scale + offset_y;
          cairo_move_to(cr, x1, y1);
          cairo_line_to(cr, x2, y2);
          cairo_stroke(cr);
        }
      }
      if (n->right) {
        NodePos *cp = g_hash_table_lookup(pos_map, n->right);
        if (cp) {
          double x1 = (np->x_index + 1) * x_scale + offset_x;
          double y1 = (np->depth + 1) * y_scale + offset_y;
          double x2 = (cp->x_index + 1) * x_scale + offset_x;
          double y2 = (cp->depth + 1) * y_scale + offset_y;
          cairo_move_to(cr, x1, y1);
          cairo_line_to(cr, x2, y2);
          cairo_stroke(cr);
        }
      }
    }
  }

  // Draw Nodes (Premium)
  g_hash_table_iter_init(&iter, pos_map);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    NodePos *np = (NodePos *)value;
    double x = (np->x_index + 1) * x_scale + offset_x; // Added offset
    double y = (np->depth + 1) * y_scale + offset_y;   // Added offset
    double r = 20.0;

    // Shadow
    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
    cairo_arc(cr, x + 3, y + 3, r, 0, 2 * M_PI);
    cairo_fill(cr);

    // Gradient Body
    cairo_pattern_t *pat =
        cairo_pattern_create_radial(x - 5, y - 5, 2, x, y, r);
    cairo_pattern_add_color_stop_rgb(pat, 0.0, body_inner_r, body_inner_g,
                                     body_inner_b);
    cairo_pattern_add_color_stop_rgb(pat, 1.0, body_outer_r, body_outer_g,
                                     body_outer_b);
    cairo_set_source(cr, pat);
    cairo_arc(cr, x, y, r, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);

    // Border
    cairo_set_source_rgb(cr, line_r, line_g, line_b);
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, x, y, r, 0, 2 * M_PI);
    cairo_stroke(cr);

    // Draw text
    char b[32] = {0};
    DataType t = app->tree_data_type;
    TreeNode *tn = (TreeNode *)key; // key is the node
    if (t == DATA_INT)
      snprintf(b, 32, "%d", *(int *)tn->data);
    else if (t == DATA_FLOAT)
      snprintf(b, 32, "%.1f", *(float *)tn->data);
    else if (t == DATA_CHAR)
      snprintf(b, 32, "%c", *(char *)tn->data);
    else if (t == DATA_STRING)
      snprintf(b, 32, "%.5s", *(char **)tn->data);

    cairo_set_source_rgb(cr, text_r, text_g, text_b);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, b, &extents);
    cairo_move_to(cr, x - extents.width / 2, y + extents.height / 2);
    cairo_show_text(cr, b);
  }

  // Cleanup
  g_hash_table_iter_init(&iter, pos_map);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    g_free(value);
  }
  g_hash_table_destroy(pos_map);

  return FALSE;
}

// =============================================================================
//                             MODULE DE COMPARAISON (BENCHMARK)
// =============================================================================

void run_comparison_benchmark(AppData *app) {
  if (!app)
    return;
  app->is_benchmarking = TRUE;

  // Determine Max Size from User Input
  size_t user_max = 1000;

  // Check which view is active
  const char *vis_child =
      gtk_stack_get_visible_child_name(GTK_STACK(app->view_stack));
  const char *entry_text = NULL;

  if (g_strcmp0(vis_child, "view_array") == 0) {
    entry_text = gtk_entry_get_text(GTK_ENTRY(app->array_size_entry));
  } else if (g_strcmp0(vis_child, "view_list") == 0) {
    entry_text = gtk_entry_get_text(GTK_ENTRY(app->list_size_entry));
  }

  if (entry_text && strlen(entry_text) > 0) {
    user_max = atoi(entry_text);
  }

  if (user_max < 100)
    user_max = 100;
  // Cap at 15000 to prevent freezing with O(N^2) algorithms
  if (user_max > 15000)
    user_max = 15000;

  // Validate: Don't run if size is 0 or invalid
  if (user_max == 0) {
    set_status(app, "Erreur: Taille invalide pour la comparaison");
    return;
  }

  // Generate 5 dynamic steps
  int num_sizes = 5;
  size_t *sizes = malloc(sizeof(size_t) * num_sizes);
  for (int i = 0; i < num_sizes; i++) {
    sizes[i] = (user_max * (i + 1)) / num_sizes;
  }

  const char *algo_names[] = {"Bubble Sort", "Insertion Sort", "Shell Sort",
                              "Quick Sort"};
  int algos[] = {0, 1, 2, 3};
  int num_algos = 4;

  app->num_curves = num_algos;
  app->num_sizes = num_sizes;

  // Clean up old
  for (int i = 0; i < 8; i++) {
    if (app->comparison_data[i]) {
      if (app->comparison_data[i]->points)
        free(app->comparison_data[i]->points);
      g_free(app->comparison_data[i]);
      app->comparison_data[i] = NULL;
    }
  }

  int type_idx = app->original_array ? app->original_array->type : DATA_INT;

  // Determine active tab
  const char *vis =
      gtk_stack_get_visible_child_name(GTK_STACK(app->view_stack));
  int is_list_mode = (g_strcmp0(vis, "view_list") == 0);

  for (int a = 0; a < num_algos; a++) {
    ComparisonCurve *curve = g_new0(ComparisonCurve, 1);
    curve->algo_name = (char *)algo_names[a]; // static string
    curve->algo_index = algos[a];
    curve->num_points = num_sizes;
    curve->points = calloc(num_sizes, sizeof(PerformancePoint));

    for (int s = 0; s < num_sizes; s++) {
      size_t sz = sizes[s];
      uint64_t dur = 0;

      // SKIP SLOW ALGORITHMS FOR LARGE SIZES
      // Algo 0=Bubble, 1=Insertion. They are O(N^2).
      // 20000^2 = 400M ops. 400ms-1s.
      // 50000^2 = 2.5B ops. 2.5s-5s.
      // 100000^2 = 100s. Too long.
      // Skip if > 20000 for Bubble/Insertion
      if ((algos[a] == 0 || algos[a] == 1) && sz > 20000) {
        dur = 0; // 0 means skipped
      } else {
        if (is_list_mode) {
          LinkedList *l = create_linked_list(type_idx, FALSE);
          fill_linked_list_random(l, sz);
          dur = sort_list_wrapper(app, l, algos[a]);
          free_linked_list(l);
        } else {
          Array *arr = create_array(type_idx, sz);
          fill_array_random(arr);
          dur = sort_array_wrapper(app, arr, algos[a]);
          free_array(arr);
        }
      }

      curve->points[s].size = sz;
      curve->points[s].time_ns = dur;
    }
    app->comparison_data[a] = curve;
  }
  g_free(sizes); // Don't forget to free sizes array generated earlier

  if (app->compare_window)
    gtk_widget_queue_draw(
        gtk_bin_get_child(GTK_BIN(app->compare_window))); // Redraw drawing area
  app->is_benchmarking = FALSE;
}

static void render_performance_graph(cairo_t *cr, double W, double H,
                                     AppData *app) {
  // Global scale factor relative to a base height of 600px
  double f = H / 600.0;
  if (f < 0.5)
    f = 0.5;

  double PAD = 85.0 * f;

  // --- Background (Opaque) ---
  if (app->is_dark_theme) {
    cairo_set_source_rgb(cr, 0.02, 0.01, 0.05);
  } else {
    cairo_set_source_rgb(cr, 0.96, 0.96, 0.98);
  }
  cairo_paint(cr);

  if (!app || !app->comparison_data[0]) {
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_font_size(cr, 20 * f);
    cairo_move_to(cr, W / 2 - 100 * f, H / 2);
    cairo_show_text(cr, "Benchmark en cours...");
    return;
  }

  // --- Title ---
  if (app->is_dark_theme)
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.8);
  else
    cairo_set_source_rgb(cr, 0.0, 0.4, 0.6);

  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 22 * f);
  const char *title = "ANALYSE DES PERFORMANCES";
  cairo_text_extents_t te;
  cairo_text_extents(cr, title, &te);
  cairo_move_to(cr, (W - te.width) / 2, 45 * f);
  cairo_show_text(cr, title);

  // Find maxes
  double max_time = 0, max_size = 0;
  for (int i = 0; i < app->num_curves; i++) {
    if (!app->comparison_data[i])
      continue;
    for (int j = 0; j < app->comparison_data[i]->num_points; j++) {
      if ((double)app->comparison_data[i]->points[j].time_ns > max_time)
        max_time = (double)app->comparison_data[i]->points[j].time_ns;
      if ((double)app->comparison_data[i]->points[j].size > max_size)
        max_size = (double)app->comparison_data[i]->points[j].size;
    }
  }
  if (max_time == 0)
    max_time = 1;
  if (max_size == 0)
    max_size = 1;

  // --- Grid ---
  cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.2);
  cairo_set_line_width(cr, 1.0 * f);
  for (int i = 0; i <= 5; i++) {
    double x = PAD + i * (W - 2 * PAD) / 5.0;
    double y = (H - PAD) - i * (H - 2 * PAD) / 5.0;
    cairo_move_to(cr, x, PAD);
    cairo_line_to(cr, x, H - PAD);
    cairo_move_to(cr, PAD, y);
    cairo_line_to(cr, W - PAD, y);
    cairo_stroke(cr);
  }

  const char *compl[] = {"O(n\xC2\xB2)", "O(n\xC2\xB2)", "O(n log\xC2\xB2 n)",
                         "O(n log n)"};
  double colors[4][3] = {
      {0.91, 0.30, 0.24}, // Soft Red (Bubble)
      {0.95, 0.77, 0.06}, // Soft Amber (Insertion)
      {0.48, 0.36, 1.00}, // Sapphire (Shell)
      {0.00, 0.82, 0.70}  // Teal (Quick)
  };

  // --- Curves ---
  for (int i = 0; i < app->num_curves; i++) {
    if (!app->comparison_data[i])
      continue;

    // Area
    cairo_set_source_rgba(cr, colors[i % 4][0], colors[i % 4][1],
                          colors[i % 4][2], 0.15);
    int first = 1;
    double lx = PAD;
    for (int j = 0; j < app->comparison_data[i]->num_points; j++) {
      double t = (double)app->comparison_data[i]->points[j].time_ns;
      double sz = (double)app->comparison_data[i]->points[j].size;
      double x = PAD + (sz / max_size) * (W - 2 * PAD);
      double y = (H - PAD) - (t / max_time) * (H - 2 * PAD);

      if (first) {
        cairo_move_to(cr, x, H - PAD);
        cairo_line_to(cr, x, y);
        first = 0;
      } else
        cairo_line_to(cr, x, y);
      lx = x;
    }
    cairo_line_to(cr, lx, H - PAD);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Line
    cairo_set_source_rgb(cr, colors[i % 4][0], colors[i % 4][1],
                         colors[i % 4][2]);
    cairo_set_line_width(cr, 3.5 * f);
    first = 1;
    for (int j = 0; j < app->comparison_data[i]->num_points; j++) {
      double t = (double)app->comparison_data[i]->points[j].time_ns;
      if (t == 0 && j > 0)
        continue;
      double sz = (double)app->comparison_data[i]->points[j].size;
      double x = PAD + (sz / max_size) * (W - 2 * PAD);
      double y = (H - PAD) - (t / max_time) * (H - 2 * PAD);

      if (first) {
        cairo_move_to(cr, x, y);
        first = 0;
      } else
        cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);
  }

  // --- Legend ---
  cairo_set_font_size(cr, 14 * f);
  for (int i = 0; i < app->num_curves; i++) {
    if (!app->comparison_data[i])
      continue;
    double leg_x = W - 220 * f, leg_y = PAD + 20 * f + i * 28 * f;
    cairo_set_source_rgb(cr, colors[i % 4][0], colors[i % 4][1],
                         colors[i % 4][2]);
    cairo_rectangle(cr, leg_x, leg_y, 14 * f, 14 * f);
    cairo_fill(cr);

    if (app->is_dark_theme)
      cairo_set_source_rgb(cr, 1, 1, 1);
    else
      cairo_set_source_rgb(cr, 0.07, 0.04, 0.14); // #120B24

    cairo_move_to(cr, leg_x + 25 * f, leg_y + 12 * f);
    char buf[128];
    snprintf(buf, sizeof(buf), "%s : %s", app->comparison_data[i]->algo_name,
             compl[i % 4]);
    cairo_show_text(cr, buf);
  }

  // --- Axis labels ---
  cairo_set_font_size(cr, 11 * f);
  if (app->is_dark_theme)
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
  else
    cairo_set_source_rgb(cr, 0.33, 0.27, 0.40); // #554466

  for (int i = 0; i <= 5; i++) {
    // X-axis
    double val_x = max_size * i / 5.0;
    double x = PAD + i * (W - 2 * PAD) / 5.0;
    char bx[16];
    snprintf(bx, 16, "%.0f", val_x);
    cairo_move_to(cr, x - 15 * f, H - PAD + 25 * f);
    cairo_show_text(cr, bx);

    // Y-axis
    double val_y = max_time * i / 5.0;
    double y = (H - PAD) - i * (H - 2 * PAD) / 5.0;
    char by[32];
    if (val_y >= 1000000)
      snprintf(by, 32, "%.1f ms", val_y / 1000000.0);
    else
      snprintf(by, 32, "%.0f ns", val_y);
    cairo_move_to(cr, PAD - 75 * f, y + 5 * f);
    cairo_show_text(cr, by);
  }

  // Axis Titles
  if (app->is_dark_theme)
    cairo_set_source_rgb(cr, 1, 1, 1);
  else
    cairo_set_source_rgb(cr, 0, 0, 0);

  cairo_set_font_size(cr, 15 * f);
  cairo_move_to(cr, W / 2 - 40 * f, H - 15 * f);
  cairo_show_text(cr, "Taille (n)");
}

gboolean on_draw_compare(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  double W = gtk_widget_get_allocated_width(widget);
  double H = gtk_widget_get_allocated_height(widget);
  render_performance_graph(cr, W, H, app);
  return FALSE;
}

// =============================================================================
//                             CALLBACKS ET LOGIQUE UI
// =============================================================================

// Helper to set markup safely
void set_buffer_markup(GtkTextBuffer *buffer, const char *markup) {
  if (!buffer)
    return;
  gtk_text_buffer_set_text(buffer, "", -1);
  if (!markup)
    return;
  GtkTextIter iter;
  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_insert_markup(buffer, &iter, markup, -1);
}

// Helper to refresh title with correct theme colors
void refresh_title(AppData *app) {
  if (!app->title_label)
    return;

  const char *visible =
      gtk_stack_get_visible_child_name(GTK_STACK(app->view_stack));
  const char *suffix = NULL;

  if (g_strcmp0(visible, "view_array") == 0)
    suffix = "TABLEAUX";
  else if (g_strcmp0(visible, "view_list") == 0)
    suffix = "LISTES";
  else if (g_strcmp0(visible, "view_tree") == 0)
    suffix = "ARBRES";
  else if (g_strcmp0(visible, "view_graph") == 0)
    suffix = "GRAPHES";

  // Colors
  const char *c1 = app->is_dark_theme ? ACCENT_PRIMARY : LIGHT_ACCENT_PRIMARY;
  const char *c2 =
      app->is_dark_theme ? ACCENT_SECONDARY : LIGHT_ACCENT_SECONDARY;
  const char *c3 = app->is_dark_theme ? TEXT_SECONDARY : LIGHT_TEXT_SECONDARY;
  const char *c_title =
      app->is_dark_theme ? "#FFFFFF" : LIGHT_TEXT_PRIMARY; // Title Suffix Color

  char buf[1024];
  if (suffix) {
    snprintf(buf, 1024,
             "<span size='xx-large' weight='900' foreground='%s'>ALGO</span>"
             "<span size='xx-large' weight='900' foreground='%s'>VISUAL</span>"
             " <span size='xx-large' foreground='#555'> :: </span>"
             "<span size='xx-large' weight='800' foreground='%s'>%s</span>",
             c1, c2, c_title, suffix);
  } else {
    snprintf(buf, 1024,
             "<span size='xx-large' weight='900' foreground='%s'>ALGO</span>"
             "<span size='xx-large' weight='900' foreground='%s'>VISUAL</span>",
             c1, c2);
  }
  gtk_label_set_markup(GTK_LABEL(app->title_label), buf);
}

void on_nav_switch(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  const char *id = gtk_widget_get_name(widget);

  // Get parent boxes that we want to hide on home page
  GtkWidget *stats_box = g_object_get_data(G_OBJECT(app->window), "stats_box");
  GtkWidget *settings_box =
      g_object_get_data(G_OBJECT(app->window), "settings_box");
  GtkWidget *nav_box = g_object_get_data(G_OBJECT(app->window), "nav_box");

  if (g_strcmp0(id, "nav_home") == 0) {
    gtk_stack_set_visible_child_name(GTK_STACK(app->view_stack), "view_home");
    gtk_widget_hide(stats_box);
    gtk_widget_hide(settings_box);
    gtk_widget_hide(nav_box);

    // Reset Title
    refresh_title(app);
    gtk_widget_hide(app->pseudo_panel);
    return;
  }

  // Show common controls
  gtk_widget_show(app->pseudo_panel);
  // Stats: Show only relevant stat
  gtk_widget_show(stats_box);
  gtk_widget_hide(app->lbl_stat_array);
  gtk_widget_hide(app->lbl_stat_list);
  gtk_widget_hide(app->lbl_stat_tree);
  gtk_widget_hide(app->lbl_stat_graph);
  // Also hide parents of labels (the cards)
  GtkWidget *card_array =
      gtk_widget_get_parent(gtk_widget_get_parent(app->lbl_stat_array));
  GtkWidget *card_list =
      gtk_widget_get_parent(gtk_widget_get_parent(app->lbl_stat_list));
  GtkWidget *card_tree =
      gtk_widget_get_parent(gtk_widget_get_parent(app->lbl_stat_tree));
  GtkWidget *card_graph =
      gtk_widget_get_parent(gtk_widget_get_parent(app->lbl_stat_graph));
  gtk_widget_hide(card_array);
  gtk_widget_hide(card_list);
  gtk_widget_hide(card_tree);
  gtk_widget_hide(card_graph);

  // Nav Box visible everywhere except Home (handled above)
  // CHANGE: Hide nav box inside specific views too for distinct separation
  gtk_widget_hide(nav_box);

  if (g_strcmp0(id, "nav_array") == 0) {
    gtk_stack_set_visible_child_name(GTK_STACK(app->view_stack), "view_array");
    update_dashboard_stats(
        app, 0, app->original_array ? app->original_array->size : 0, -1);

    // Contextual: Show Array Settings & Stat
    gtk_widget_show_all(settings_box);
    gtk_widget_show(card_array);

    refresh_title(app);

  } else if (g_strcmp0(id, "nav_list") == 0) {
    gtk_stack_set_visible_child_name(GTK_STACK(app->view_stack), "view_list");
    update_dashboard_stats(app, 1,
                           app->current_list ? app->current_list->size : 0, -1);

    // Contextual: Hide Settings (not relevant?), Show List Stat
    // Contextual: Hide Settings (not relevant?), Show List Stat
    gtk_widget_hide(settings_box);
    gtk_widget_show(card_list);

    refresh_title(app);

  } else if (g_strcmp0(id, "nav_tree") == 0) {
    gtk_stack_set_visible_child_name(GTK_STACK(app->view_stack), "view_tree");
    update_dashboard_stats(app, 2, tree_count((TreeNode *)app->tree_root), -1);

    // Contextual: Hide Settings, Show Tree Stat
    // Contextual: Hide Settings, Show Tree Stat
    gtk_widget_hide(settings_box);
    gtk_widget_show(card_tree);

    refresh_title(app);

  } else if (g_strcmp0(id, "nav_graph") == 0) {
    gtk_stack_set_visible_child_name(GTK_STACK(app->view_stack), "view_graph");
    update_dashboard_stats(
        app, 3, app->current_graph ? app->current_graph->node_count : 0, -1);

    // Contextual: Hide Settings, Show Graph Stat
    gtk_widget_hide(settings_box);
    gtk_widget_show(card_graph);

    refresh_title(app);
  }
}

// CALLBACKS MODIFIED FOR MANUAL INPUT
void on_generate_array(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  int mode = gtk_combo_box_get_active(
      GTK_COMBO_BOX(app->array_mode_combo)); // 0=Random, 1=Manual

  if (mode == 1) { // Manual
    const char *txt = gtk_entry_get_text(GTK_ENTRY(app->array_manual_entry));
    parse_and_fill_struct(app, txt, 0);
    set_status(app, "Tableau g\xC3\xA9n\xC3\xA9r\xC3\xA9 manuellement");
  } else {
    const char *sz_str = gtk_entry_get_text(GTK_ENTRY(app->array_size_entry));
    size_t sz = atoi(sz_str);
    if (sz <= 0)
      sz = 100;
    int type_idx =
        gtk_combo_box_get_active(GTK_COMBO_BOX(app->data_type_combo));
    if (app->original_array)
      free_array(app->original_array);
    app->original_array = create_array(type_idx, sz);
    fill_array_random(app->original_array);
    set_status(
        app, "Tableau g\xC3\xA9n\xC3\xA9r\xC3\xA9 (%zu \xC3\xA9l\xC3\xA9ments)",
        sz);
  }

  if (app->sorted_array) {
    free_array(app->sorted_array);
    app->sorted_array = NULL;
  }

  char *s = array_to_string(app->original_array, TRUE, app);
  set_buffer_markup(app->buffer_array_before, s);
  g_free(s);
  gtk_text_buffer_set_text(app->buffer_array_after, "", -1);

  update_dashboard_stats(
      app, 0, app->original_array ? app->original_array->size : 0, -1);
  add_history_entry(
      app,
      g_strdup_printf("G\xC3\xA9n\xC3\xA9ration Tableau (%zu)",
                      app->original_array ? app->original_array->size : 0));
}

// ... Sort Array unchanged ...

void on_generate_list(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  int mode = gtk_combo_box_get_active(GTK_COMBO_BOX(app->list_mode_combo));
  int type_idx =
      gtk_combo_box_get_active(GTK_COMBO_BOX(app->list_data_type_combo));

  if (mode == 1) { // Manual
    const char *txt = gtk_entry_get_text(GTK_ENTRY(app->list_manual_entry));
    parse_and_fill_struct(app, txt, 1);
    set_status(app, "Liste g\xC3\xA9n\xC3\xA9r\xC3\xA9\x65 manuellement");
    set_status(app, "Liste g\xC3\xA9n\xC3\xA9r\xC3\xA9\x65 manuellement");
  } else {
    const char *sz_str = gtk_entry_get_text(GTK_ENTRY(app->list_size_entry));
    size_t sz = atoi(sz_str);
    if (sz <= 0)
      sz = 10;

    int is_double =
        gtk_combo_box_get_active(GTK_COMBO_BOX(app->list_type_combo)) ==
        1; // 0=Simple, 1=Double

    if (app->current_list)
      free_linked_list(app->current_list);
    app->current_list = create_linked_list(type_idx, is_double);
    fill_linked_list_random(app->current_list, sz);
    set_status(app, "Liste g\xC3\xA9n\xC3\xA9r\xC3\xA9\x65");
  }

  // Resize canvas for scrolling
  if (app->current_list) {
    int NODE_W = 70, PAD_X = 35;
    int width = gtk_widget_get_allocated_width(app->list_canvas);
    if (width < 100)
      width = 800; // default assumption
    int nodes_per_row = (width - 40) / (NODE_W + PAD_X);
    if (nodes_per_row < 1)
      nodes_per_row = 1;

    int rows = (app->current_list->size + nodes_per_row - 1) / nodes_per_row;
    int NODE_H = 45, PAD_Y = 60;
    int height = 40 + rows * (NODE_H + PAD_Y) + 100; // Extra padding

    if (height < 500)
      height = 500; // Min height
    gtk_widget_set_size_request(app->list_canvas, -1, height);
  }

  gtk_widget_queue_draw(app->list_canvas);
  update_dashboard_stats(app, 1,
                         app->current_list ? app->current_list->size : 0, -1);
  add_history_entry(
      app, g_strdup_printf("G\xC3\xA9n\xC3\xA9ration Liste (%zu)",
                           app->current_list ? app->current_list->size : 0));
}

// ... Sort List unchanged ...

void on_generate_tree(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  int mode = gtk_combo_box_get_active(GTK_COMBO_BOX(app->tree_mode_combo));

  if (app->tree_root) {
    free_tree((TreeNode *)app->tree_root, app->tree_data_type);
    app->tree_root = NULL;
  }

  app->tree_data_type =
      gtk_combo_box_get_active(GTK_COMBO_BOX(app->tree_data_type_combo));
  int t_type = gtk_combo_box_get_active(GTK_COMBO_BOX(app->tree_type_combo));
  app->tree_is_nary = (t_type == 1);
  app->tree_nary_degree =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->tree_degree_spin));

  if (mode == 1) {
    const char *txt = gtk_entry_get_text(GTK_ENTRY(app->tree_manual_entry));
    parse_and_fill_struct(app, txt, 2);
    set_status(app, "Arbre g\xC3\xA9n\xC3\xA9r\xC3\xA9 manuellement");
  } else {
    const char *sz_str = gtk_entry_get_text(GTK_ENTRY(app->tree_size_entry));
    size_t sz = atoi(sz_str);
    if (sz <= 0)
      sz = 10;
    insert_tree_random(app, sz);
    set_status(app,
               "Arbre g\xC3\xA9n\xC3\xA9r\xC3\xA9 al\xC3\xA9\x61toirement");
  }

  gtk_widget_queue_draw(app->tree_canvas);
  update_dashboard_stats(app, 2, tree_count((TreeNode *)app->tree_root),
                         (double)tree_depth((TreeNode *)app->tree_root));
  add_history_entry(app,
                    g_strdup_printf("G\xC3\xA9n\xC3\xA9ration Arbre (%d)",
                                    tree_count((TreeNode *)app->tree_root)));
}

void on_tree_action(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->tree_root)
    return;
  int method =
      gtk_combo_box_get_active(GTK_COMBO_BOX(app->tree_traversal_combo));
  GString *out = g_string_new(NULL);

  if (method == 0) {
    set_pseudo_code(app, "BFS(racine):\n"
                         "  File Q = Vide\n"
                         "  Enfiler racine dans Q\n"
                         "  Tant que Q n'est pas vide faire:\n"
                         "    n = D\xC3\xA9filer de Q\n"
                         "    Visiter(n)\n"
                         "    Pour chaque enfant c de n faire:\n"
                         "      Enfiler c dans Q\n"
                         "    Fin Pour\n"
                         "  Fin Tant que");
    traverse_tree_bfs((TreeNode *)app->tree_root, out, app->tree_data_type);
  } else if (method == 1) {
    set_pseudo_code(app, "PRE_ORDRE(n):\n"
                         "  Si n est nul retourner\n"
                         "  Visiter(n)\n"
                         "  PRE_ORDRE(n.gauche)\n"
                         "  PRE_ORDRE(n.droite)");
    traverse_tree_dfs_pre((TreeNode *)app->tree_root, out, app->tree_data_type);
  } else if (method == 2) {
    set_pseudo_code(app, "IN_ORDRE(n):\n"
                         "  Si n est nul retourner\n"
                         "  IN_ORDRE(n.gauche)\n"
                         "  Visiter(n)\n"
                         "  IN_ORDRE(n.droite)");
    traverse_tree_dfs_in((TreeNode *)app->tree_root, out, app->tree_data_type);
  } else if (method == 3) {
    set_pseudo_code(app, "POST_ORDRE(n):\n"
                         "  Si n est nul retourner\n"
                         "  POST_ORDRE(n.gauche)\n"
                         "  POST_ORDRE(n.droite)\n"
                         "  Visiter(n)");
    traverse_tree_dfs_post((TreeNode *)app->tree_root, out,
                           app->tree_data_type);
  }

  gtk_text_buffer_set_text(app->tree_log_buffer, out->str, -1);
  g_string_free(out, TRUE);
}

void on_sort_array(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->original_array)
    return;

  if (app->sorted_array)
    free_array(app->sorted_array);
  app->sorted_array = copy_array(app->original_array);

  int algo = gtk_combo_box_get_active(GTK_COMBO_BOX(app->algo_selector_combo));
  const char *btn_id = gtk_widget_get_name(widget);
  double old_speed = app->animation_speed;

  // If clicked on "Trier" (instant), we force speed to 0
  if (g_strcmp0(btn_id, "btn_sort_animate") != 0) {
    app->animation_speed = 0;
  } else {
    // Open animation window
    open_animation_window(app);
  }

  uint64_t t = sort_array_wrapper(app, app->sorted_array, algo);
  app->animation_speed = old_speed;

  char *s = array_to_string(app->sorted_array, TRUE, app);
  set_buffer_markup(app->buffer_array_after, s);
  g_free(s);

  double ms = t / 1000000.0;
  set_status(app,
             "Tri termin\xC3\xA9"
             " en %.3f ms",
             ms);
  update_dashboard_stats(app, 0, app->sorted_array->size, ms);

  const char *algo_name = gtk_combo_box_text_get_active_text(
      GTK_COMBO_BOX_TEXT(app->algo_selector_combo));
  add_history_entry(app, g_strdup_printf("Tri Tableau (%s)",
                                         algo_name ? algo_name : "Inconnu"));
}

void on_sort_list(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->current_list)
    return;
  int algo = gtk_combo_box_get_active(GTK_COMBO_BOX(app->algo_selector_combo));
  const char *btn_id = gtk_widget_get_name(widget);
  double old_speed = app->animation_speed;

  if (g_strcmp0(btn_id, "btn_list_animate") != 0) {
    app->animation_speed = 0;
  } else {
    open_animation_window(app);
  }

  uint64_t t = sort_list_wrapper(app, app->current_list, algo);
  app->animation_speed = old_speed;

  // Resize canvas for scrolling
  if (app->current_list) {
    int NODE_W = 70, PAD_X = 35;
    int width = gtk_widget_get_allocated_width(app->list_canvas);
    if (width < 100)
      width = 800;
    int nodes_per_row = (width - 40) / (NODE_W + PAD_X);
    if (nodes_per_row < 1)
      nodes_per_row = 1;

    int rows = (app->current_list->size + nodes_per_row - 1) / nodes_per_row;
    int NODE_H = 45, PAD_Y = 60;
    int height = 40 + rows * (NODE_H + PAD_Y) + 100;

    if (height < 500)
      height = 500;
    gtk_widget_set_size_request(app->list_canvas, -1, height);
  }

  gtk_widget_queue_draw(app->list_canvas);
  double ms = t / 1000000.0;
  set_status(app, "Liste tri\xC3\xA9\x65 en %.3f ms", ms);
  update_dashboard_stats(app, 1, app->current_list->size, ms);
  const char *algo_name = gtk_combo_box_text_get_active_text(
      GTK_COMBO_BOX_TEXT(app->algo_selector_combo));
  add_history_entry(app, g_strdup_printf("Tri Liste (%s)",
                                         algo_name ? algo_name : "Inconnu"));
}

void on_reset_structure(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  const char *id = gtk_widget_get_name(widget);

  if (g_strcmp0(id, "reset_array") == 0) {
    if (app->original_array)
      free_array(app->original_array);
    if (app->sorted_array)
      free_array(app->sorted_array);
    app->original_array = NULL;
    app->sorted_array = NULL;
    gtk_text_buffer_set_text(app->buffer_array_before, "", -1);
    gtk_text_buffer_set_text(app->buffer_array_after, "", -1);
    set_status(app, "Tableau r\xC3\xA9initialis\xC3\xA9");
    set_pseudo_code(
        app,
        "S\xC3\xA9lectionnez un algorithme\npour voir son pseudo-code ici.");
    update_dashboard_stats(app, 0, 0, -1);
    add_history_entry(app, "R\xC3\xA9initialisation Tableau");
  } else if (g_strcmp0(id, "reset_list") == 0) {
    if (app->current_list)
      free_linked_list(app->current_list);
    app->current_list = NULL;
    gtk_widget_queue_draw(app->list_canvas);
    // Resize back to default?
    gtk_widget_set_size_request(app->list_canvas, -1, 400);
    set_status(app, "Liste r\xC3\xA9initialis\xC3\xA9\x65");
    set_pseudo_code(
        app,
        "S\xC3\xA9lectionnez un algorithme\npour voir son pseudo-code ici.");
    update_dashboard_stats(app, 1, 0, -1);
  } else if (g_strcmp0(id, "reset_tree") == 0) {
    if (app->tree_root)
      free_tree((TreeNode *)app->tree_root, app->tree_data_type);
    app->tree_root = NULL;
    gtk_widget_queue_draw(app->tree_canvas);
    set_status(app, "Arbre r\xC3\xA9initialis\xC3\xA9");
    set_pseudo_code(
        app,
        "S\xC3\xA9lectionnez un algorithme\npour voir son pseudo-code ici.");
    update_dashboard_stats(app, 2, 0, -1);
  }
}

void on_list_action(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->current_list)
    return;
  int op = gtk_combo_box_get_active(GTK_COMBO_BOX(app->list_op_combo));
  const char *val = gtk_entry_get_text(GTK_ENTRY(app->list_val_entry));

  int loc = gtk_combo_box_get_active(
      GTK_COMBO_BOX(app->list_loc_combo)); // 0=Deb, 1=Fin, 2=Pos
  int pos = 0;
  if (loc == 0)
    pos = 0;
  else if (loc == 1)
    pos = -1; // End
  else {
    pos = atoi(gtk_entry_get_text(GTK_ENTRY(app->list_pos_entry)));
  }

  // For modify/delete, we need actual index if pos is -1 (end)
  if (op != 0 && pos == -1) {
    if (app->current_list->size > 0)
      pos = app->current_list->size - 1;
    else
      return; // empty
  }

  if (op == 0) { // Insert
    // Parse value based on type
    void *val_ptr = malloc(get_element_size(app->current_list->type));
    DataType t = app->current_list->type;

    if (t == DATA_INT)
      *(int *)val_ptr = atoi(val);
    else if (t == DATA_FLOAT)
      *(float *)val_ptr = atof(val);
    else if (t == DATA_CHAR)
      *(char *)val_ptr = val[0];
    else if (t == DATA_STRING)
      *((char **)val_ptr) =
          g_strdup(val); // will be duplicated again in create_node but that's
                         // fine or we pass dereferenced

    // create_node expects pointer to value
    // For string, it expects char**

    Node *n = create_node(t, val_ptr);

    // cleanup temp
    if (t == DATA_STRING)
      g_free(*(char **)val_ptr);
    free(val_ptr);

    insert_to_linkedlist(app->current_list, n, pos);
  } else if (op == 1) { // Modify
    modify_linkedlist(app->current_list, pos, val);
  } else if (op == 2) { // Delete
    delete_from_linkedlist(app->current_list, pos);
  }

  // Resize canvas for scrolling
  if (app->current_list) {
    int NODE_W = 70, PAD_X = 35;
    int width = gtk_widget_get_allocated_width(app->list_canvas);
    if (width < 100)
      width = 800;
    int nodes_per_row = (width - 40) / (NODE_W + PAD_X);
    if (nodes_per_row < 1)
      nodes_per_row = 1;

    int rows = (app->current_list->size + nodes_per_row - 1) / nodes_per_row;
    int NODE_H = 45, PAD_Y = 60;
    int height = 40 + rows * (NODE_H + PAD_Y) + 100;

    if (height < 500)
      height = 500;
    gtk_widget_set_size_request(app->list_canvas, -1, height);
  }
  gtk_widget_queue_draw(app->list_canvas);
  update_dashboard_stats(app, 1, app->current_list->size, -1);
}

void export_comparison_png(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;

  if (!app->comparison_data[0]) {
    set_status(app, "Lancez d'abord l'analyse pour avoir un graphique !");
    return;
  }

  GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "ENREGISTRER L'ANALYSE GRAPHIQUE",
      app->compare_window ? GTK_WINDOW(app->compare_window)
                          : GTK_WINDOW(app->window),
      GTK_FILE_CHOOSER_ACTION_SAVE, "Annuler", GTK_RESPONSE_CANCEL, "Exporter",
      GTK_RESPONSE_ACCEPT, NULL);

  gtk_window_resize(GTK_WINDOW(dialog), 480, 360);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  apply_css(dialog, app); // Apply our custom theme to the dialog

  // Add PNG Filter
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(filter, "*.png");
  gtk_file_filter_set_name(filter, "Images PNG (*.png)");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  // Set default directory to Desktop
  const char *desktop_path = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
  if (desktop_path) {
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), desktop_path);
  }

  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
                                                 TRUE);
  gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                    "analyse_performance.png");

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    int width = 1200, height = 800; // High resolution
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    // Render directly with explicit dimensions
    render_performance_graph(cr, (double)width, (double)height, app);
    cairo_surface_flush(surface); // Crucial for image stability before writing

    cairo_status_t status = cairo_surface_write_to_png(surface, filename);
    if (status != CAIRO_STATUS_SUCCESS) {
      set_status(app, "Erreur d'enregistrement (Code %d)", status);
    } else {
      set_status(app, "Graphique export\xC3\xA9 avec succ\xC3\xA8s !");
    }

    g_free(filename);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
  }

  gtk_widget_destroy(dialog);
}

// Helper to open Floating Animation Window
static gboolean on_draw_anim_canvas(GtkWidget *widget, cairo_t *cr,
                                    gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->demo_array)
    return FALSE;

  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);

  // Background
  if (app->is_dark_theme) {
    cairo_set_source_rgb(cr, 0.05, 0.03, 0.1);
  } else {
    cairo_set_source_rgb(cr, 0.95, 0.95, 0.98);
  }
  cairo_paint(cr);

  int n = app->demo_array->size;
  double pad = 40;
  double bar_space = (width - 2 * pad) / n;
  double bar_w = bar_space * 0.7;

  for (int i = 0; i < n; i++) {
    int val = ((int *)app->demo_array->data)[i];
    double bar_h = (double)val / 20.0 * (height - 100);
    double x = pad + i * bar_space + (bar_space - bar_w) / 2.0;
    double y = height - bar_h - 60;

    // Bar with gradient (Saphir Premium)
    cairo_pattern_t *pat = cairo_pattern_create_linear(x, y, x, y + bar_h);
    cairo_pattern_add_color_stop_rgb(pat, 0, 0.48, 0.36, 1.0); // Saphir
    cairo_pattern_add_color_stop_rgb(pat, 1, 0.15, 0.1, 0.4);  // Deep Blue
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, x, y, bar_w, bar_h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);

    // Border
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 1.5);
    cairo_rectangle(cr, x, y, bar_w, bar_h);
    cairo_stroke(cr);

    // Text
    char buf[12];
    snprintf(buf, 12, "%d", val);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 22);
    cairo_move_to(cr, x + (bar_w / 2.0) - 10, height - 20);
    cairo_show_text(cr, buf);
  }
  return FALSE;
}

static void open_animation_window(AppData *app) {
  // 1. Create Window
  app->anim_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(app->anim_win),
                       "D\xC3\xA9monstration P\xC3\xA9\x64\x61gogique");
  gtk_window_set_default_size(GTK_WINDOW(app->anim_win), 950, 650);
  gtk_window_set_transient_for(GTK_WINDOW(app->anim_win),
                               GTK_WINDOW(app->window));
  gtk_window_set_modal(GTK_WINDOW(app->anim_win), TRUE);

  GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_container_add(GTK_CONTAINER(app->anim_win), main_hbox);
  gtk_container_set_border_width(GTK_CONTAINER(main_hbox), 20);

  // --- Left Side: Algorithm Info & Canvas ---
  GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_box_pack_start(GTK_BOX(main_hbox), left_vbox, TRUE, TRUE, 0);

  int algo_idx =
      gtk_combo_box_get_active(GTK_COMBO_BOX(app->algo_selector_combo));
  const char *algo_names[] = {"Tri \xC3\xA0 Bulles", "Tri par Insertion",
                              "Tri Shell", "Tri Rapide (QuickSort)"};
  const char *algo_desc[] = {
      "Principe : On compare les \xC3\xA9l\xC3\xA9ments adjacents 2 \xC3\xA0 "
      "2. S'ils sont mal ordonn\xC3\xA9s, on les \xC3\xA9"
      "change. Le plus grand "
      "'remonte' progressivement comme une bulle.",
      "Principe : On ins\xC3\xA8re chaque \xC3\xA9l\xC3\xA9ment \xC3\xA0 sa "
      "place "
      "dans la partie tri\xC3\xA9\x65, comme on range des cartes \xC3\xA0 "
      "jouer.",
      "Principe : Am\xC3\xA9lioration du tri par insertion utilisant un "
      "ecart qui diminue, permettant des d\xC3\xA9placements rapides.",
      "Principe : Utilise un pivot pour diviser le tableau en deux parties "
      "(plus petits / plus grands) et r\xC3\xA9"
      "curse."};

  GtkWidget *lbl_title = gtk_label_new(NULL);
  char title_markup[256];
  snprintf(
      title_markup, 256,
      "<span size='xx-large' weight='heavy' foreground='#D600B2'>%s</span>",
      algo_names[algo_idx]);
  gtk_label_set_markup(GTK_LABEL(lbl_title), title_markup);
  gtk_box_pack_start(GTK_BOX(left_vbox), lbl_title, FALSE, FALSE, 0);

  GtkWidget *lbl_princ = gtk_label_new(algo_desc[algo_idx]);
  gtk_label_set_line_wrap(GTK_LABEL(lbl_princ), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lbl_princ), 50);
  gtk_box_pack_start(GTK_BOX(left_vbox), lbl_princ, FALSE, FALSE, 0);

  // Visual Canvas
  app->anim_canvas = gtk_drawing_area_new();
  gtk_widget_set_size_request(app->anim_canvas, -1, 300);
  g_signal_connect(app->anim_canvas, "draw", G_CALLBACK(on_draw_anim_canvas),
                   app);
  gtk_box_pack_start(GTK_BOX(left_vbox), app->anim_canvas, TRUE, TRUE, 10);

  // --- Right Side: Pseudo-code ---
  GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_size_request(right_vbox, 400, -1);
  gtk_box_pack_start(GTK_BOX(main_hbox), right_vbox, FALSE, FALSE, 0);

  GtkWidget *lbl_code_head = gtk_label_new("Pseudo-code Algorithmique");
  gtk_style_context_add_class(gtk_widget_get_style_context(lbl_code_head),
                              "stat-label");
  gtk_box_pack_start(GTK_BOX(right_vbox), lbl_code_head, FALSE, FALSE, 0);

  GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(right_vbox), scrolled, TRUE, TRUE, 0);

  app->pseudo_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(app->pseudo_view), FALSE);
  app->pseudo_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->pseudo_view));

  PangoFontDescription *font_desc =
      pango_font_description_from_string("Monospace Bold 11");
  gtk_widget_override_font(app->pseudo_view, font_desc);
  pango_font_description_free(font_desc);

  gtk_text_buffer_create_tag(app->pseudo_buffer, "highlight", "background",
                             ACCENT_PRIMARY, "foreground", "white", "weight",
                             PANGO_WEIGHT_BOLD, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled), app->pseudo_view);

  // --- Start Demo Logic ---
  if (app->demo_array)
    free_array(app->demo_array);

  // Increase to 12 elements for better visualization
  int n_demo = 12;
  app->demo_array = create_array(DATA_INT, n_demo);
  int demo_init[] = {15, 3, 11, 7, 19, 2, 14, 8, 1, 12, 6, 17};
  for (int i = 0; i < n_demo; i++)
    ((int *)app->demo_array->data)[i] = demo_init[i];

  app->animation_speed = 0.8;
  app->is_benchmarking = FALSE;

  gtk_widget_show_all(app->anim_win);
  g_signal_connect(app->anim_win, "destroy", G_CALLBACK(on_anim_window_destroy),
                   app);

  while (gtk_events_pending())
    gtk_main_iteration();

  sort_array_wrapper(app, app->demo_array, algo_idx);
}

static void on_anim_window_destroy(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (app->demo_array) {
    free_array(app->demo_array);
    app->demo_array = NULL;
  }
  app->pseudo_buffer = NULL;
  app->anim_canvas = NULL;
  app->anim_win = NULL;
}

void on_animate_launch(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  open_animation_window(app);
}

void on_compare_launch(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;

  if (!app->compare_window) {
    app->compare_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->compare_window),
                         "ANALYSE DES PERFORMANCES");
    gtk_window_set_default_size(GTK_WINDOW(app->compare_window), 800, 600);
    g_signal_connect(app->compare_window, "destroy",
                     G_CALLBACK(gtk_widget_destroyed), &app->compare_window);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app->compare_window), vbox);

    // Toolbar for export
    GtkWidget *hbox_tools = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox_tools), 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_tools, FALSE, FALSE, 0);

    GtkWidget *btn_export = gtk_button_new_with_label("EXPORTER PNG");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_export),
                                "suggested-action");
    g_signal_connect(btn_export, "clicked", G_CALLBACK(export_comparison_png),
                     app);
    gtk_box_pack_start(GTK_BOX(hbox_tools), btn_export, FALSE, FALSE, 0);

    app->comparison_canvas = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), app->comparison_canvas, TRUE, TRUE, 0);
    g_signal_connect(app->comparison_canvas, "draw",
                     G_CALLBACK(on_draw_compare), app);

    app->comparison_progress = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), app->comparison_progress, FALSE, FALSE,
                       0);
  } else {
    gtk_window_present(GTK_WINDOW(app->compare_window));
    gtk_widget_queue_draw(app->comparison_canvas);
  }

  gtk_widget_show_all(app->compare_window);
  run_comparison_benchmark(app);
}

void on_tree_op_action(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  const char *id = gtk_widget_get_name(widget);

  GtkWidget *entry_val =
      (GtkWidget *)g_object_get_data(G_OBJECT(app->window), "tree_op_val");
  GtkWidget *entry_old =
      (GtkWidget *)g_object_get_data(G_OBJECT(app->window), "tree_op_old");
  const char *val = gtk_entry_get_text(GTK_ENTRY(entry_val));

  if (g_strcmp0(id, "insert") == 0) {
    insert_tree_manual(app, val);
    set_status(app, "Noeud %s inséré", val);
  } else if (g_strcmp0(id, "delete") == 0) {
    delete_tree_node(app, val);
    set_status(app, "Noeud %s supprimé", val);
  } else if (g_strcmp0(id, "modify") == 0) {
    const char *old = gtk_entry_get_text(GTK_ENTRY(entry_old));
    modify_tree_node(app, old, val);
    set_status(app, "Noeud modifié");
  } else if (g_strcmp0(id, "order") == 0) {
    reorder_tree(app);
  }

  gtk_widget_queue_draw(app->tree_canvas);
  on_tree_action(NULL, app); // update log
  update_dashboard_stats(app, 2, tree_count((TreeNode *)app->tree_root),
                         (double)tree_depth((TreeNode *)app->tree_root));
}

void on_tree_transform(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (!app->tree_root)
    return;
  app->tree_root = convert_nary_to_binary((TreeNode *)app->tree_root);
  app->tree_is_nary = FALSE;
  gtk_widget_queue_draw(app->tree_canvas);
  set_status(app, "Transformé en Arbre Binaire");
  update_dashboard_stats(app, 2, tree_count((TreeNode *)app->tree_root),
                         (double)tree_depth((TreeNode *)app->tree_root));
}

// =============================================================================
//                             INTERFACE PRINCIPALE (DASHBOARD)
// =============================================================================

void apply_css(GtkWidget *widget, AppData *app) {
  const char *bg_primary = app->is_dark_theme ? BG_PRIMARY : LIGHT_BG_PRIMARY;
  const char *bg_secondary =
      app->is_dark_theme ? BG_SECONDARY : LIGHT_BG_SECONDARY;
  const char *bg_tertiary =
      app->is_dark_theme ? BG_TERTIARY : LIGHT_BG_TERTIARY;
  const char *text_primary =
      app->is_dark_theme ? TEXT_PRIMARY : LIGHT_TEXT_PRIMARY;
  const char *text_secondary =
      app->is_dark_theme ? TEXT_SECONDARY : LIGHT_TEXT_SECONDARY;
  const char *accent_primary =
      app->is_dark_theme ? ACCENT_PRIMARY : LIGHT_ACCENT_PRIMARY;
  const char *accent_secondary =
      app->is_dark_theme ? ACCENT_SECONDARY : LIGHT_ACCENT_SECONDARY;
  const char *border_color =
      app->is_dark_theme ? BORDER_COLOR : LIGHT_BORDER_COLOR;

  // Specific tweaks
  const char *entry_bg = app->is_dark_theme ? "#1a1f2b" : "#FFFFFF";
  const char *glow_primary =
      app->is_dark_theme ? GLOW_PRIMARY : "rgba(214, 0, 178, 0.2)";
  const char *glow_secondary =
      app->is_dark_theme ? GLOW_SECONDARY : "rgba(0, 163, 204, 0.2)";
  const char *placeholder_color =
      app->is_dark_theme ? "rgba(255, 255, 255, 0.5)" : "rgba(0, 0, 0, 0.4)";
  const char *textview_bg = app->is_dark_theme ? "#080412" : "#F8F8FF";

  GString *css = g_string_new("");
  g_string_append_printf(
      css,
      "window { background-color: %s; font-family: 'Segoe UI', 'Orbitron', "
      "'Roboto', sans-serif; }"
      "label { color: %s; }"
      "button { "
      "  background-image: none; "
      "  background-color: %s; "
      "  color: %s; "
      "  border: 1px solid %s; "
      "  border-radius: 4px; "
      "  padding: 10px 20px; "
      "  font-weight: 800; "
      "  text-transform: uppercase; "
      "  letter-spacing: 1.5px; "
      "  transition: all 150ms ease; "
      "}"
      "button:hover { "
      "  background-color: %s; "
      "  border-color: %s; "
      "  color: %s; "
      "  box-shadow: 0 0 15px %s; "
      "}"
      "button:active { background-color: %s; color: %s; }"
      "entry { background-color: %s; color: %s; border: 1px solid %s; "
      "border-radius: 4px; padding: 8px; font-family: monospace; }"
      "entry:focus { border-color: %s; box-shadow: 0 0 10px %s; }"
      "entry selection { background-color: %s; color: white; }"
      "entry *:placeholder { color: %s; }"
      "stack { background-color: transparent; }"
      ".card { background-color: %s; border-radius: 0px; border-left: 5px "
      "solid %s; margin: 5px; padding: 15px; box-shadow: 6px 6px 0px rgba(0, "
      "0, 0, 0.4); }"
      ".category-card { background-color: %s; border: 1px solid %s; "
      "border-bottom: 4px solid %s; padding: 25px; transition: all 250ms ease; "
      "margin: 5px; }"
      ".category-card:hover { background-color: %s; border-color: %s; "
      "box-shadow: 0 0 30px %s; margin-top: -5px; }"
      ".category-title { font-size: 22px; font-weight: 900; color: %s; "
      "margin-top: 10px; }"
      ".category-desc { font-size: 13px; color: %s; }"
      "treeview, list { background-color: %s; color: %s; }"
      "filechooser label, dialog label, messagedialog label { color: %s; }"
      "dialog, messagedialog, .dialog-vbox, .message-area { background-color: "
      "%s; color: %s; }"
      ".hero-title { font-size: 42px; font-weight: 900; color: %s; "
      "text-shadow: 0 0 20px %s; }"
      ".hero-sub { font-size: 14px; color: %s; letter-spacing: 4px; "
      "text-transform: uppercase; }"
      ".header { font-size: 24px; font-weight: 900; color: %s; letter-spacing: "
      "2px; text-shadow: 0 0 10px %s; }"
      ".stat-val { font-size: 20px; font-weight: 800; color: %s; }"
      ".stat-label { font-size: 11px; font-weight: 900; color: %s; "
      "text-transform: uppercase; letter-spacing: 2px; }"
      "textview, textview text { background-color: %s; color: %s; "
      "border-radius: 0; border: 1px solid %s; font-family: monospace; }"
      "textview text { background-color: transparent; padding: 10px; }"
      "combobox button { background-color: %s; border-radius: 4px; }"
      "scrollbar slider { background-color: %s; border-radius: 10px; }"
      "scrollbar slider:hover { background-color: %s; }"
      "notebook { background-color: %s; border: 1px solid %s; }"
      "notebook header { background-color: %s; }"
      "notebook tab { background-color: %s; padding: 10px; }"
      "notebook tab:checked { background-color: %s; border-bottom: 2px solid "
      "%s; }"
      "notebook tab label { color: %s; font-weight: 800; }",
      bg_primary, text_primary, bg_secondary, text_primary, border_color,
      bg_tertiary, accent_primary, accent_primary, glow_primary, accent_primary,
      bg_primary, entry_bg, text_primary, border_color, accent_secondary,
      glow_secondary, accent_primary, placeholder_color, bg_secondary,
      accent_primary, bg_secondary, border_color, accent_primary, bg_tertiary,
      accent_primary, glow_primary, accent_primary, text_secondary,
      bg_secondary, text_primary, text_primary, bg_primary, text_primary,
      accent_primary, glow_primary, accent_secondary, accent_primary,
      glow_primary, accent_primary, text_secondary, textview_bg, text_primary,
      border_color, bg_secondary, border_color, accent_primary, bg_primary,
      border_color, bg_secondary, bg_primary, bg_secondary, accent_primary,
      text_primary);

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider, css->str, -1, NULL);
  gtk_style_context_add_provider_for_screen(
      gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
  g_string_free(css, TRUE);
}

void refresh_data_display(AppData *app) {
  if (!app)
    return;

  // Refresh Arrays (Tableaux)
  if (app->original_array) {
    char *s = array_to_string(app->original_array, TRUE, app);
    set_buffer_markup(app->buffer_array_before, s);
    g_free(s);
  }
  if (app->sorted_array) {
    char *s = array_to_string(app->sorted_array, TRUE, app);
    set_buffer_markup(app->buffer_array_after, s);
    g_free(s);
  }

  // Forces redraw of all DrawingAreas (Lists, Trees, Graphs, Comparison)
  gtk_widget_queue_draw(app->window);
}

void on_speed_changed(GtkRange *range, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  double val = gtk_range_get_value(range);
  app->animation_speed = val / 100.0;
}

void on_reset_session(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  app->total_ops = 0;
  app->total_time_ms = 0;
  app->structures_created = 0;

  // Clear History
  if (app->recent_history) {
    g_slist_free_full(app->recent_history, g_free);
    app->recent_history = NULL;
  }

  if (app->box_history) {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(app->box_history));
    for (iter = children; iter != NULL; iter = g_list_next(iter))
      gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    GtkWidget *lbl_hint = gtk_label_new("Aucune action r\xC3\xA9"
                                        "cente");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_hint),
                                "dim-label");
    gtk_box_pack_start(GTK_BOX(app->box_history), lbl_hint, FALSE, FALSE, 0);
    gtk_widget_show(lbl_hint);
  }

  if (app->lbl_home_total_ops)
    gtk_label_set_text(GTK_LABEL(app->lbl_home_total_ops), "0");
  if (app->lbl_home_total_time)
    gtk_label_set_text(GTK_LABEL(app->lbl_home_total_time), "0 ms");

  set_status(app, "Session r\xC3\xA9initialis\xC3\xA9"
                  "e");
}

void on_toggle_theme(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;

  // Check if the widget has a specific theme mode set
  gpointer theme_mode_ptr = g_object_get_data(G_OBJECT(widget), "theme_mode");

  if (theme_mode_ptr) {
    // Set specific theme: 1 = dark, 0 = light
    int theme_mode = GPOINTER_TO_INT(theme_mode_ptr);
    app->is_dark_theme = (theme_mode == 1);
  } else {
    // Toggle theme (old behavior)
    app->is_dark_theme = !app->is_dark_theme;
  }

  apply_css(app->window, app);
  refresh_title(app);
  refresh_data_display(app);
}

void show_guide_popup(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  GtkWidget *dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(dialog), "Guide de D\xC3\xA9marrage");
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 480, -1);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
  gtk_container_add(GTK_CONTAINER(dialog), vbox);

  GtkWidget *lbl_title = gtk_label_new(NULL);
  gtk_label_set_markup(
      GTK_LABEL(lbl_title),
      "<span size='x-large' weight='bold' foreground='" ACCENT_SECONDARY
      "'>GUIDE RAPIDE</span>");
  gtk_box_pack_start(GTK_BOX(vbox), lbl_title, FALSE, FALSE, 5);

  const char *txt =
      "<b>1. Choisir une Structure</b>\n"
      "Allez dans l'un des onglets (<b>Tableaux, Listes, Arbres ou "
      "Graphes</b>) "
      "pour commencer une nouvelle visualisation.\n\n"
      "<b>2. Configurer les Donn\xC3\xA9\x65s</b>\n"
      "D\xC3\xA9\x66\x69nissez le <b>Type de Donn\xC3\xA9\x65</b> (Entier, "
      "Flottant, etc.). Utilisez le mode <i>Al\xC3\xA9\x61toire</i> pour une "
      "g\xC3\xA9n\xC3\xA9ration automatique ou <i>Manuel</i> pour saisir vos "
      "propres valeurs.\n\n"
      "<b>3. Ex\xC3\xA9\x63uter et Animer</b>\n"
      "S\xC3\xA9lectionnez un algorithme et cliquez sur "
      "<b>G\xC3\xA9n\xC3\xA9rer/Go</b>. Vous pouvez ajuster la <b>Vitesse</b> "
      "sur l'accueil pour mieux observer chaque \xC3\xA9tape de "
      "l'ex\xC3\xA9\x63ution.\n\n"
      "<b>4. Analyser les Performances</b>\n"
      "Cliquez sur le bouton <span "
      "foreground='#FF0055'><b>COMPARAISON</b></span> pour ouvrir une planche "
      "d'analyse graphique montrant la complexit\xC3\xA9 r\xC3\xA9\x65lle face "
      "\xC3\xA0 la th\xC3\xA9orie.";

  GtkWidget *lbl = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), txt);
  gtk_label_set_line_wrap(GTK_LABEL(lbl), TRUE);
  gtk_label_set_justify(GTK_LABEL(lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
  gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 5);

  GtkWidget *btn = gtk_button_new_with_label("FERMER");
  g_signal_connect_swapped(btn, "clicked", G_CALLBACK(gtk_widget_destroy),
                           dialog);
  gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 5);

  gtk_widget_show_all(dialog);
}

void on_history_clear(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  if (app->recent_history) {
    g_slist_free_full(app->recent_history, g_free);
    app->recent_history = NULL;
  }

  // Clear preview sidebar
  if (app->box_history) {
    GList *children =
        gtk_container_get_children(GTK_CONTAINER(app->box_history));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter))
      gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    GtkWidget *lbl = gtk_label_new("Historique vid\xC3\xA9");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "dim-label");
    gtk_box_pack_start(GTK_BOX(app->box_history), lbl, FALSE, FALSE, 0);
    gtk_widget_show(lbl);
  }

  // If this was called from the popup, we need to refresh the popup content
  // But simpler is to just close the popup or have the caller handle it.
  // Here we just notify the user.
  set_status(app, "Historique supprim\xC3\xA9");
}

void show_history_popup(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  GtkWidget *dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(dialog), "Historique Session");
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 550);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
  gtk_container_add(GTK_CONTAINER(dialog), vbox);

  GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *lbl_title = gtk_label_new(NULL);
  gtk_label_set_markup(
      GTK_LABEL(lbl_title),
      "<span size='x-large' weight='bold' foreground='" ACCENT_PRIMARY
      "'>HISTORIQUE DES ACTIONS</span>");
  gtk_box_pack_start(GTK_BOX(header), lbl_title, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 0);

  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER,
                                 GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

  GtkWidget *history_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_container_add(GTK_CONTAINER(scroll), history_box);

  if (!app->recent_history) {
    GtkWidget *empty_lbl = gtk_label_new("Aucune action r\xC3\xA9\x63\x65nte.");
    gtk_style_context_add_class(gtk_widget_get_style_context(empty_lbl),
                                "dim-label");
    gtk_box_pack_start(GTK_BOX(history_box), empty_lbl, TRUE, TRUE, 20);
  } else {
    GSList *rev = g_slist_copy(app->recent_history);
    rev = g_slist_reverse(rev);
    for (GSList *l = rev; l; l = l->next) {
      GtkWidget *item = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
      gtk_style_context_add_class(gtk_widget_get_style_context(item), "card");
      gtk_container_set_border_width(GTK_CONTAINER(item), 10);

      GtkWidget *lbl = gtk_label_new(NULL);
      char *escaped = g_markup_escape_text((char *)l->data, -1);
      gtk_label_set_markup(
          GTK_LABEL(lbl),
          g_strdup_printf("<span weight='medium'>%s</span>", escaped));
      g_free(escaped);
      gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
      gtk_box_pack_start(GTK_BOX(item), lbl, TRUE, TRUE, 0);

      gtk_box_pack_start(GTK_BOX(history_box), item, FALSE, FALSE, 0);
    }
    g_slist_free(rev);
  }

  // Footer Buttons
  GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(vbox), footer, FALSE, FALSE, 0);

  GtkWidget *btn_clear = gtk_button_new_with_label("TOUT EFFACER");
  gtk_style_context_add_class(gtk_widget_get_style_context(btn_clear),
                              "btn-danger");
  g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_history_clear), app);
  g_signal_connect_swapped(btn_clear, "clicked", G_CALLBACK(gtk_widget_destroy),
                           dialog);
  gtk_box_pack_start(GTK_BOX(footer), btn_clear, TRUE, TRUE, 0);

  GtkWidget *btn_close = gtk_button_new_with_label("FERMER");
  g_signal_connect_swapped(btn_close, "clicked", G_CALLBACK(gtk_widget_destroy),
                           dialog);
  gtk_box_pack_start(GTK_BOX(footer), btn_close, TRUE, TRUE, 0);

  apply_css(dialog, app);
  gtk_widget_show_all(dialog);
}

gboolean on_window_key_press(GtkWidget *widget, GdkEventKey *event,
                             gpointer user_data) {
  AppData *app = (AppData *)user_data;
  char c = (char)gdk_keyval_to_unicode(event->keyval);

  if (c >= 32 && c <= 126) {
    size_t len = strlen(app->cmd_buffer);
    if (len < sizeof(app->cmd_buffer) - 1) {
      app->cmd_buffer[len] = g_ascii_tolower(c);
      app->cmd_buffer[len + 1] = '\0';
    }

    if (strstr(app->cmd_buffer, "guide")) {
      show_guide_popup(NULL, app);
      app->cmd_buffer[0] = '\0';
    } else if (strstr(app->cmd_buffer, "history")) {
      show_history_popup(NULL, app);
      app->cmd_buffer[0] = '\0';
    } else if (strstr(app->cmd_buffer, "mode")) {
      show_mode_popup(NULL, app);
      app->cmd_buffer[0] = '\0';
    }
  } else if (event->keyval == GDK_KEY_Escape ||
             event->keyval == GDK_KEY_BackSpace) {
    app->cmd_buffer[0] = '\0';
  }

  return FALSE;
}
GtkWidget *create_category_card(const char *icon, const char *title,
                                const char *desc, const char *nav_id,
                                AppData *app) {
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_style_context_add_class(gtk_widget_get_style_context(card),
                              "category-card");

  GtkWidget *lbl_icon = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(lbl_icon),
                       g_strdup_printf("<span size='40000'>%s</span>", icon));
  gtk_box_pack_start(GTK_BOX(card), lbl_icon, FALSE, FALSE, 0);

  GtkWidget *lbl_title = gtk_label_new(title);
  gtk_style_context_add_class(gtk_widget_get_style_context(lbl_title),
                              "category-title");
  gtk_box_pack_start(GTK_BOX(card), lbl_title, FALSE, FALSE, 0);

  GtkWidget *lbl_desc = gtk_label_new(desc);
  gtk_label_set_line_wrap(GTK_LABEL(lbl_desc), TRUE);
  gtk_label_set_justify(GTK_LABEL(lbl_desc), GTK_JUSTIFY_CENTER);
  gtk_style_context_add_class(gtk_widget_get_style_context(lbl_desc),
                              "category-desc");
  gtk_box_pack_start(GTK_BOX(card), lbl_desc, FALSE, FALSE, 0);

  GtkWidget *btn = gtk_button_new_with_label("Lancer");
  gtk_widget_set_name(btn, nav_id);
  g_signal_connect(btn, "clicked", G_CALLBACK(on_nav_switch), app);
  gtk_box_pack_end(GTK_BOX(card), btn, FALSE, FALSE, 0);

  return card;
}

void show_mode_popup(GtkWidget *widget, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  static GtkWidget *popover = NULL;

  if (!popover) {
    popover = gtk_popover_new(app->btn_mode);
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(popover), vbox);

    // Bouton pour le thème Sombre
    GtkWidget *btn_dark = gtk_button_new();
    GtkWidget *box_dark = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(
        GTK_BOX(box_dark),
        gtk_image_new_from_icon_name("weather-clear-night-symbolic",
                                     GTK_ICON_SIZE_BUTTON),
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_dark), gtk_label_new("SOMBRE"), FALSE, FALSE,
                       0);
    gtk_container_add(GTK_CONTAINER(btn_dark), box_dark);
    g_object_set_data(G_OBJECT(btn_dark), "theme_mode",
                      GINT_TO_POINTER(1)); // 1 = dark
    g_signal_connect_swapped(btn_dark, "clicked", G_CALLBACK(gtk_widget_hide),
                             popover);
    g_signal_connect(btn_dark, "clicked", G_CALLBACK(on_toggle_theme), app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_dark, FALSE, FALSE, 0);

    // Bouton pour le thème Clair
    GtkWidget *btn_light = gtk_button_new();
    GtkWidget *box_light = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(box_light),
                       gtk_image_new_from_icon_name("weather-clear-symbolic",
                                                    GTK_ICON_SIZE_BUTTON),
                       FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_light), gtk_label_new("CLAIR"), FALSE, FALSE,
                       0);
    gtk_container_add(GTK_CONTAINER(btn_light), box_light);
    g_object_set_data(G_OBJECT(btn_light), "theme_mode",
                      GINT_TO_POINTER(0)); // 0 = light
    g_signal_connect_swapped(btn_light, "clicked", G_CALLBACK(gtk_widget_hide),
                             popover);
    g_signal_connect(btn_light, "clicked", G_CALLBACK(on_toggle_theme), app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_light, FALSE, FALSE, 0);

    gtk_widget_show_all(vbox);
  }

  gtk_widget_show(popover);
}

GtkWidget *create_stat_card(const char *title, GtkWidget **val_label_ptr) {
  GtkWidget *frame = gtk_frame_new(NULL);
  GtkStyleContext *context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "card");

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(frame), box);

  GtkWidget *lbl_title = gtk_label_new(title);
  context = gtk_widget_get_style_context(lbl_title);
  gtk_style_context_add_class(context, "stat-label");
  gtk_box_pack_start(GTK_BOX(box), lbl_title, FALSE, FALSE, 0);

  *val_label_ptr = gtk_label_new("--");
  context = gtk_widget_get_style_context(*val_label_ptr);
  gtk_style_context_add_class(context, "stat-val");
  gtk_box_pack_start(GTK_BOX(box), *val_label_ptr, TRUE, FALSE, 0);

  return frame;
}

int main(int argc, char *argv[]) {
  setlocale(LC_NUMERIC, "C");
  gtk_init(&argc, &argv);
  AppData *app = g_new0(AppData, 1);

  // Init App
  app->is_dark_theme = TRUE; // Default
  app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(app->window), "Analyseur de Tri Moderne");
  gtk_window_set_default_size(GTK_WINDOW(app->window), 1024, 768);
  g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(app->window, "key-press-event",
                   G_CALLBACK(on_window_key_press), app);
  apply_css(app->window, app);
  refresh_title(app);

  // Initialize Settings
  app->animation_speed = 0.5;
  app->default_data_type = DATA_INT;
  app->total_ops = 0;
  app->total_time_ms = 0;
  app->structures_created = 0;
  app->cmd_buffer[0] = '\0';

  GtkWidget *vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(vbox_main), 15);
  gtk_container_add(GTK_CONTAINER(app->window), vbox_main);

  // === HEADER ===
  GtkWidget *hbox_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(vbox_main), hbox_header, FALSE, FALSE, 0);

  GtkWidget *lbl_app = gtk_label_new(NULL);
  gtk_label_set_markup(
      GTK_LABEL(lbl_app),
      "<span size='xx-large' weight='900' foreground='" ACCENT_PRIMARY
      "'>ALGO</span><span size='xx-large' weight='900' "
      "foreground='" ACCENT_SECONDARY "'>VISUAL</span>");
  app->title_label = lbl_app; // Store reference
  gtk_box_pack_start(GTK_BOX(hbox_header), lbl_app, FALSE, FALSE, 0);

  // --- Navigation Group (Right) ---
  GtkWidget *hbox_nav_group = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(hbox_nav_group),
                              "linked");
  gtk_box_pack_end(GTK_BOX(hbox_header), hbox_nav_group, FALSE, FALSE, 0);

  // Home Button
  GtkWidget *btn_home = gtk_button_new();
  GtkWidget *box_home = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(
      GTK_BOX(box_home),
      gtk_image_new_from_icon_name("go-home-symbolic", GTK_ICON_SIZE_BUTTON),
      FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_home), gtk_label_new("ACCUEIL"), FALSE, FALSE,
                     0);
  gtk_container_add(GTK_CONTAINER(btn_home), box_home);
  gtk_widget_set_name(btn_home, "nav_home");
  g_signal_connect(btn_home, "clicked", G_CALLBACK(on_nav_switch), app);
  gtk_box_pack_start(GTK_BOX(hbox_nav_group), btn_home, FALSE, FALSE, 0);

  // Theme Button (with dropdown)
  GtkWidget *btn_theme = gtk_button_new();
  app->btn_mode = btn_theme; // Re-use the pointer for the popover
  GtkWidget *box_theme = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(GTK_BOX(box_theme),
                     gtk_image_new_from_icon_name("display-brightness-symbolic",
                                                  GTK_ICON_SIZE_BUTTON),
                     FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_theme), gtk_label_new("THEME"), FALSE, FALSE,
                     0);
  gtk_container_add(GTK_CONTAINER(btn_theme), box_theme);
  g_signal_connect(btn_theme, "clicked", G_CALLBACK(show_mode_popup), app);
  gtk_box_pack_start(GTK_BOX(hbox_nav_group), btn_theme, FALSE, FALSE, 0);

  // --- Info Group ---
  GtkWidget *hbox_info_group = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(hbox_info_group),
                              "linked");
  gtk_box_pack_end(GTK_BOX(hbox_header), hbox_info_group, FALSE, FALSE, 15);

  // History Button
  GtkWidget *btn_hist = gtk_button_new();
  GtkWidget *box_hist = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(GTK_BOX(box_hist),
                     gtk_image_new_from_icon_name(
                         "document-open-recent-symbolic", GTK_ICON_SIZE_BUTTON),
                     FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_hist), gtk_label_new("HISTORIQUE"), FALSE,
                     FALSE, 0);
  gtk_container_add(GTK_CONTAINER(btn_hist), box_hist);
  g_signal_connect(btn_hist, "clicked", G_CALLBACK(show_history_popup), app);
  gtk_box_pack_start(GTK_BOX(hbox_info_group), btn_hist, FALSE, FALSE, 0);

  // Guide Button
  GtkWidget *btn_guide = gtk_button_new();
  GtkWidget *box_guide = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(
      GTK_BOX(box_guide),
      gtk_image_new_from_icon_name("help-about-symbolic", GTK_ICON_SIZE_BUTTON),
      FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_guide), gtk_label_new("GUIDE"), FALSE, FALSE,
                     0);
  gtk_container_add(GTK_CONTAINER(btn_guide), box_guide);
  g_signal_connect(btn_guide, "clicked", G_CALLBACK(show_guide_popup), app);
  gtk_box_pack_start(GTK_BOX(hbox_info_group), btn_guide, FALSE, FALSE, 0);

  // --- Tools Group ---
  GtkWidget *hbox_tool_group = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(hbox_tool_group),
                              "linked");
  gtk_box_pack_end(GTK_BOX(hbox_header), hbox_tool_group, FALSE, FALSE, 15);

  // Comparison Button
  GtkWidget *btn_compare_hdr = gtk_button_new();
  GtkWidget *box_comp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(GTK_BOX(box_comp),
                     gtk_image_new_from_icon_name("office-chart-area-symbolic",
                                                  GTK_ICON_SIZE_BUTTON),
                     FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_comp), gtk_label_new("COMPARAISON"), FALSE,
                     FALSE, 0);
  gtk_container_add(GTK_CONTAINER(btn_compare_hdr), box_comp);
  gtk_style_context_add_class(gtk_widget_get_style_context(btn_compare_hdr),
                              "suggested-action");
  g_signal_connect(btn_compare_hdr, "clicked", G_CALLBACK(on_compare_launch),
                   app);
  gtk_box_pack_start(GTK_BOX(hbox_tool_group), btn_compare_hdr, FALSE, FALSE,
                     0);

  // Reset Button
  GtkWidget *btn_reset_hdr = gtk_button_new();
  GtkWidget *box_reset = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(GTK_BOX(box_reset),
                     gtk_image_new_from_icon_name("view-refresh-symbolic",
                                                  GTK_ICON_SIZE_BUTTON),
                     FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_reset), gtk_label_new("RESET"), FALSE, FALSE,
                     0);
  gtk_container_add(GTK_CONTAINER(btn_reset_hdr), box_reset);
  g_signal_connect(btn_reset_hdr, "clicked", G_CALLBACK(on_reset_session), app);
  gtk_box_pack_start(GTK_BOX(hbox_tool_group), btn_reset_hdr, FALSE, FALSE, 0);

  // === STATS CARDS ===
  GtkWidget *hbox_cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  g_object_set_data(G_OBJECT(app->window), "stats_box", hbox_cards);
  gtk_box_pack_start(GTK_BOX(vbox_main), hbox_cards, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(hbox_cards),
                     create_stat_card("TABLEAUX", &app->lbl_stat_array), TRUE,
                     TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_cards),
                     create_stat_card("LISTES", &app->lbl_stat_list), TRUE,
                     TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_cards),
                     create_stat_card("ARBRES", &app->lbl_stat_tree), TRUE,
                     TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_cards),
                     create_stat_card("GRAPHES", &app->lbl_stat_graph), TRUE,
                     TRUE, 0);

  // Global Settings Bar (Type & Algo)
  GtkWidget *hbox_settings = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  g_object_set_data(G_OBJECT(app->window), "settings_box", hbox_settings);
  gtk_box_pack_start(GTK_BOX(vbox_main), hbox_settings, FALSE, FALSE, 0);

  app->data_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->data_type_combo),
                                 "Entier (INT)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->data_type_combo),
                                 "Flottant (FLOAT)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->data_type_combo),
                                 "Caract\xC3\xA8re (CHAR)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->data_type_combo),
                                 "Cha\xC3\xAEne (STRING)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->data_type_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_settings),
                     gtk_label_new("Donn\xC3\xA9"
                                   "es:"),
                     FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(hbox_settings), app->data_type_combo, FALSE, FALSE,
                     0);

  app->algo_selector_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algo_selector_combo),
                                 "Bubble Sort");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algo_selector_combo),
                                 "Insertion Sort");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algo_selector_combo),
                                 "Shell Sort");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algo_selector_combo),
                                 "Quick Sort");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->algo_selector_combo), 3);
  gtk_box_pack_start(GTK_BOX(hbox_settings), gtk_label_new("Algo:"), FALSE,
                     FALSE, 5);
  gtk_box_pack_start(GTK_BOX(hbox_settings), app->algo_selector_combo, FALSE,
                     FALSE, 0);

  GtkWidget *btn_compare = gtk_button_new_with_label("Comparer");
  g_signal_connect(btn_compare, "clicked", G_CALLBACK(on_compare_launch), app);
  gtk_box_pack_end(GTK_BOX(hbox_settings), btn_compare, FALSE, FALSE, 0);

  GtkWidget *btn_anim = gtk_button_new_with_label("Animer");
  gtk_style_context_add_class(gtk_widget_get_style_context(btn_anim),
                              "suggested-action");
  g_signal_connect(btn_anim, "clicked", G_CALLBACK(on_animate_launch), app);
  gtk_box_pack_end(GTK_BOX(hbox_settings), btn_anim, FALSE, FALSE, 10);

  // === NAVIGATION BUTTONS ===
  GtkWidget *hbox_nav = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  g_object_set_data(G_OBJECT(app->window), "nav_box", hbox_nav);
  gtk_style_context_add_class(gtk_widget_get_style_context(hbox_nav), "linked");
  gtk_box_pack_start(GTK_BOX(vbox_main), hbox_nav, FALSE, FALSE, 0);

  app->nav_array_btn = gtk_button_new_with_label("Tableaux");
  gtk_widget_set_name(app->nav_array_btn, "nav_array");
  app->nav_list_btn = gtk_button_new_with_label("Listes");
  gtk_widget_set_name(app->nav_list_btn, "nav_list");
  app->nav_tree_btn = gtk_button_new_with_label("Arbres");
  gtk_widget_set_name(app->nav_tree_btn, "nav_tree");
  app->nav_graph_btn = gtk_button_new_with_label("Graphes");
  gtk_widget_set_name(app->nav_graph_btn, "nav_graph");

  g_signal_connect(app->nav_array_btn, "clicked", G_CALLBACK(on_nav_switch),
                   app);
  g_signal_connect(app->nav_list_btn, "clicked", G_CALLBACK(on_nav_switch),
                   app);
  g_signal_connect(app->nav_tree_btn, "clicked", G_CALLBACK(on_nav_switch),
                   app);
  g_signal_connect(app->nav_graph_btn, "clicked", G_CALLBACK(on_nav_switch),
                   app);

  gtk_box_pack_start(GTK_BOX(hbox_nav), app->nav_array_btn, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_nav), app->nav_list_btn, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_nav), app->nav_tree_btn, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_nav), app->nav_graph_btn, TRUE, TRUE, 0);

  // === CONTENT AREA (Stack + Pseudo) ===
  GtkWidget *hbox_content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
  gtk_box_pack_start(GTK_BOX(vbox_main), hbox_content, TRUE, TRUE, 0);

  app->view_stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(app->view_stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  gtk_box_pack_start(GTK_BOX(hbox_content), app->view_stack, TRUE, TRUE, 0);

  // Pseudo-code sidebar removed from main UI as requested.

  // -- View Home --
  GtkWidget *view_home = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_container_set_border_width(GTK_CONTAINER(view_home), 20);
  gtk_widget_set_valign(view_home, GTK_ALIGN_FILL); // Fill screen

  GtkWidget *lbl_hero = gtk_label_new("STRUCTURES DE DONN\xC3\x89"
                                      "ES");
  gtk_style_context_add_class(gtk_widget_get_style_context(lbl_hero),
                              "hero-title");
  gtk_box_pack_start(GTK_BOX(view_home), lbl_hero, FALSE, FALSE, 0);

  GtkWidget *lbl_sub = gtk_label_new("Exploration & Performance Algorithmique");
  gtk_style_context_add_class(gtk_widget_get_style_context(lbl_sub),
                              "hero-sub");
  gtk_box_pack_start(GTK_BOX(view_home), lbl_sub, FALSE, FALSE, 0);

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
  gtk_box_pack_start(GTK_BOX(view_home), grid, TRUE, TRUE, 0);

  // Create category grid
  gtk_grid_attach(
      GTK_GRID(grid),
      create_category_card(
          "", "TABLEAUX",
          "Stockage contigu, acc\xC3\xA8s instantan\xC3\xA9. Id\xC3\xA9"
          "al pour "
          "comprendre les tris de base.",
          "nav_array", app),
      0, 0, 1, 1);

  gtk_grid_attach(GTK_GRID(grid),
                  create_category_card("", "LISTES",
                                       "N\xC5\x93uds cha\xC3\xAEn\xC3\xA9s "
                                       "dynamiques. Manipulation flexible des "
                                       "donn\xC3\xA9"
                                       "es en m\xC3\xA9moire.",
                                       "nav_list", app),
                  1, 0, 1, 1);

  gtk_grid_attach(
      GTK_GRID(grid),
      create_category_card(
          "", "ARBRES",
          "Structures hi\xC3\xA9rarchiques. Recherche binaire, parcours "
          "et r\xC3\xA9"
          "cursivit\xC3\xA9.",
          "nav_tree", app),
      0, 1, 1, 1);

  gtk_grid_attach(
      GTK_GRID(grid),
      create_category_card(
          "", "GRAPHES",
          "R\xC3\xA9seaux complexes. Algorithmes de plus court chemin et "
          "parcours Dijkstra.",
          "nav_graph", app),
      1, 1, 1, 1);

  // === SESSION SUMMARY ===
  GtkWidget *hbox_summary = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_widget_set_halign(hbox_summary, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(view_home), hbox_summary, FALSE, FALSE, 5);

  GtkWidget *v1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  GtkWidget *l1 = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(l1),
                       "<span size='small' alpha='50%'>OPERATIONS</span>");
  app->lbl_home_total_ops = gtk_label_new("0");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(app->lbl_home_total_ops), "stat-val");
  gtk_box_pack_start(GTK_BOX(v1), l1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(v1), app->lbl_home_total_ops, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_summary), v1, FALSE, FALSE, 10);

  GtkWidget *v2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  GtkWidget *l2 = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(l2),
                       "<span size='small' alpha='50%'>TEMPS TOTAL</span>");
  app->lbl_home_total_time = gtk_label_new("0 ms");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(app->lbl_home_total_time), "stat-val");
  gtk_box_pack_start(GTK_BOX(v2), l2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(v2), app->lbl_home_total_time, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_summary), v2, FALSE, FALSE, 10);

  // === GLOBAL SETTINGS ===
  GtkWidget *hbox_glob_settings = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
  gtk_widget_set_halign(hbox_glob_settings, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(view_home), hbox_glob_settings, FALSE, FALSE, 5);

  GtkWidget *lbl_speed = gtk_label_new("Vitesse d'animation :");
  gtk_box_pack_start(GTK_BOX(hbox_glob_settings), lbl_speed, FALSE, FALSE, 0);

  app->speed_scale =
      gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_draw_value(GTK_SCALE(app->speed_scale), FALSE);
  gtk_range_set_value(GTK_RANGE(app->speed_scale), 50);
  gtk_widget_set_size_request(app->speed_scale, 200, -1);
  g_signal_connect(app->speed_scale, "value-changed",
                   G_CALLBACK(on_speed_changed), app);
  gtk_box_pack_start(GTK_BOX(hbox_glob_settings), app->speed_scale, FALSE,
                     FALSE, 0);

  // Actions Footer removed (moved to header)

  GtkWidget *scroll_home = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_home),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(scroll_home), view_home);

  gtk_stack_add_named(GTK_STACK(app->view_stack), scroll_home, "view_home");

  // -- View Array --
  GtkWidget *view_array = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(view_array, 10);

  GtkWidget *hbox_arr_config = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(view_array), hbox_arr_config, FALSE, FALSE, 0);

  // Mode selection
  app->array_mode_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->array_mode_combo),
                                 "Al\xC3\xA9"
                                 "atoire");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->array_mode_combo),
                                 "Manuel");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->array_mode_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_arr_config), app->array_mode_combo, FALSE,
                     FALSE, 0);

  // Inputs
  app->array_size_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(app->array_size_entry), "100");
  gtk_widget_set_size_request(app->array_size_entry, 60, -1);
  gtk_box_pack_start(GTK_BOX(hbox_arr_config), gtk_label_new("Taille:"), FALSE,
                     FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_arr_config), app->array_size_entry, FALSE,
                     FALSE, 0);

  app->array_manual_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->array_manual_entry),
                                 "Valeurs (10 20 ...)");
  gtk_box_pack_start(GTK_BOX(hbox_arr_config), app->array_manual_entry, TRUE,
                     TRUE, 0);

  // Actions Row
  GtkWidget *hbox_arr_actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(view_array), hbox_arr_actions, FALSE, FALSE, 0);

  GtkWidget *btn_gen_arr = gtk_button_new_with_label("G\xC3\xA9n\xC3\xA9rer");
  g_signal_connect(btn_gen_arr, "clicked", G_CALLBACK(on_generate_array), app);
  gtk_box_pack_start(GTK_BOX(hbox_arr_actions), btn_gen_arr, FALSE, FALSE, 0);

  GtkWidget *btn_sort_arr = gtk_button_new_with_label("Trier");
  gtk_widget_set_name(btn_sort_arr, "btn_sort_instant");
  g_signal_connect(btn_sort_arr, "clicked", G_CALLBACK(on_sort_array), app);
  gtk_box_pack_start(GTK_BOX(hbox_arr_actions), btn_sort_arr, FALSE, FALSE, 0);

  GtkWidget *btn_rst_arr = gtk_button_new_with_label("R\xC3\xA9initialiser");
  gtk_widget_set_name(btn_rst_arr, "reset_array");
  g_signal_connect(btn_rst_arr, "clicked", G_CALLBACK(on_reset_structure), app);
  gtk_box_pack_end(GTK_BOX(hbox_arr_actions), btn_rst_arr, FALSE, FALSE, 0);

  // Canvas with Wrap Mode
  GtkWidget *scroll1 = gtk_scrolled_window_new(NULL, NULL);
  app->array_canvas_before = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->array_canvas_before),
                              GTK_WRAP_WORD_CHAR); // wrap!
  app->buffer_array_before =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->array_canvas_before));
  gtk_container_add(GTK_CONTAINER(scroll1), app->array_canvas_before);

  GtkWidget *scroll2 = gtk_scrolled_window_new(NULL, NULL);
  app->array_canvas_after = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->array_canvas_after),
                              GTK_WRAP_WORD_CHAR); // wrap!
  app->buffer_array_after =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->array_canvas_after));
  gtk_container_add(GTK_CONTAINER(scroll2), app->array_canvas_after);

  GtkWidget *paned =
      gtk_paned_new(GTK_ORIENTATION_VERTICAL); // Vertical split often better
                                               // for "full space"
  gtk_paned_pack1(GTK_PANED(paned), scroll1, TRUE, FALSE);
  gtk_paned_pack2(GTK_PANED(paned), scroll2, TRUE, FALSE);
  gtk_box_pack_start(GTK_BOX(view_array), paned, TRUE, TRUE, 0);

  gtk_stack_add_named(GTK_STACK(app->view_stack), view_array, "view_array");

  // -- View List --
  GtkWidget *view_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  // Row 1: Generation Configuration
  GtkWidget *hbox_list_gen = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(view_list), hbox_list_gen, FALSE, FALSE, 0);

  app->list_mode_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_mode_combo),
                                 "Al\xC3\xA9"
                                 "atoire");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_mode_combo),
                                 "Manuel");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->list_mode_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), app->list_mode_combo, FALSE, FALSE,
                     0);

  app->list_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_type_combo),
                                 "Simple");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_type_combo),
                                 "Double");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->list_type_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), app->list_type_combo, FALSE, FALSE,
                     0);

  app->list_size_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(app->list_size_entry), "10");
  gtk_widget_set_size_request(app->list_size_entry, 50, -1);
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), gtk_label_new("Taille:"), FALSE,
                     FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), app->list_size_entry, FALSE, FALSE,
                     0);

  app->list_data_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_data_type_combo),
                                 "INT");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_data_type_combo),
                                 "FLOAT");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_data_type_combo),
                                 "CHAR");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_data_type_combo),
                                 "STRING");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->list_data_type_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), app->list_data_type_combo, FALSE,
                     FALSE, 0);

  app->list_manual_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->list_manual_entry),
                                 "Ex: 5 10 3");
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), app->list_manual_entry, TRUE, TRUE,
                     0);

  GtkWidget *btn_gen_list = gtk_button_new_with_label("G\xC3\xA9n\xC3\xA9rer");
  g_signal_connect(btn_gen_list, "clicked", G_CALLBACK(on_generate_list), app);
  gtk_box_pack_start(GTK_BOX(hbox_list_gen), btn_gen_list, FALSE, FALSE, 0);

  GtkWidget *btn_rst_list = gtk_button_new_with_label("R\xC3\xA9initialiser");
  gtk_widget_set_name(btn_rst_list, "reset_list");
  g_signal_connect(btn_rst_list, "clicked", G_CALLBACK(on_reset_structure),
                   app);
  gtk_box_pack_end(GTK_BOX(hbox_list_gen), btn_rst_list, FALSE, FALSE, 0);

  // Row 2: Operations
  GtkWidget *hbox_list_ops = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(view_list), hbox_list_ops, FALSE, FALSE, 0);

  app->list_op_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_op_combo),
                                 "Ins\xC3\xA9rer");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_op_combo),
                                 "Modifier");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_op_combo),
                                 "Supprimer");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_op_combo),
                                 "Rechercher");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->list_op_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_list_ops), app->list_op_combo, FALSE, FALSE,
                     0);

  app->list_loc_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_loc_combo),
                                 "D\xC3\xA9"
                                 "but");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_loc_combo),
                                 "Fin");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->list_loc_combo),
                                 "Position");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->list_loc_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_list_ops), app->list_loc_combo, FALSE, FALSE,
                     0);

  app->list_val_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->list_val_entry), "Val");
  gtk_widget_set_size_request(app->list_val_entry, 60, -1);
  gtk_box_pack_start(GTK_BOX(hbox_list_ops), app->list_val_entry, FALSE, FALSE,
                     0);

  app->list_pos_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->list_pos_entry), "Pos");
  gtk_widget_set_size_request(app->list_pos_entry, 40, -1);
  gtk_box_pack_start(GTK_BOX(hbox_list_ops), app->list_pos_entry, FALSE, FALSE,
                     0);

  GtkWidget *btn_act_list = gtk_button_new_with_label("Go");
  g_signal_connect(btn_act_list, "clicked", G_CALLBACK(on_list_action), app);
  gtk_box_pack_start(GTK_BOX(hbox_list_ops), btn_act_list, FALSE, FALSE, 0);

  // Separation for Sort
  gtk_box_pack_start(GTK_BOX(hbox_list_ops),
                     gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE,
                     10);

  GtkWidget *btn_sort_list = gtk_button_new_with_label("Trier");
  gtk_widget_set_name(btn_sort_list, "btn_list_instant");
  g_signal_connect(btn_sort_list, "clicked", G_CALLBACK(on_sort_list), app);
  gtk_box_pack_start(GTK_BOX(hbox_list_ops), btn_sort_list, FALSE, FALSE, 0);

  app->list_canvas = gtk_drawing_area_new();
  // Wrap in scrolled window
  GtkWidget *list_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(list_scroll), app->list_canvas);
  gtk_box_pack_start(GTK_BOX(view_list), list_scroll, TRUE, TRUE, 0);

  g_signal_connect(app->list_canvas, "draw", G_CALLBACK(on_draw_list), app);

  gtk_stack_add_named(GTK_STACK(app->view_stack), view_list, "view_list");

  // -- View Tree --
  GtkWidget *view_tree = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *hbox_tree_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

  app->tree_mode_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_mode_combo),
                                 "Al\xC3\xA9"
                                 "atoire");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_mode_combo),
                                 "Manuel");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->tree_mode_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_mode_combo, FALSE,
                     FALSE, 0);

  app->tree_size_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(app->tree_size_entry), "15");
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_size_entry, FALSE,
                     FALSE, 0);

  app->tree_data_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_data_type_combo),
                                 "INT");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_data_type_combo),
                                 "FLOAT");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_data_type_combo),
                                 "CHAR");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_data_type_combo),
                                 "STRING");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->tree_data_type_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_data_type_combo, FALSE,
                     FALSE, 0);

  app->tree_manual_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->tree_manual_entry),
                                 "Ex: 10 5 15...");
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_manual_entry, TRUE,
                     TRUE, 0);

  app->tree_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_type_combo),
                                 "Binaire");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_type_combo),
                                 "N-aire");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->tree_type_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_type_combo, FALSE,
                     FALSE, 0);

  app->tree_degree_spin = gtk_spin_button_new_with_range(2, 5, 1);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_degree_spin, FALSE,
                     FALSE, 0);

  GtkWidget *btn_gen_tree = gtk_button_new_with_label("G\xC3\xA9n\xC3\xA9rer");
  g_signal_connect(btn_gen_tree, "clicked", G_CALLBACK(on_generate_tree), app);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), btn_gen_tree, FALSE, FALSE, 0);

  GtkWidget *btn_rst_tree = gtk_button_new_with_label("R\xC3\xA9initialiser");
  gtk_widget_set_name(btn_rst_tree, "reset_tree");
  g_signal_connect(btn_rst_tree, "clicked", G_CALLBACK(on_reset_structure),
                   app);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), btn_rst_tree, FALSE, FALSE, 0);

  // Traversal Controls
  app->tree_traversal_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_traversal_combo),
                                 "Largeur (BFS)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_traversal_combo),
                                 "Profondeur (Pr\xC3\xA9\x66ixe)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_traversal_combo),
                                 "Profondeur (Infixe)");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tree_traversal_combo),
                                 "Profondeur (Postfixe)");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->tree_traversal_combo), 0);
  g_signal_connect(app->tree_traversal_combo, "changed",
                   G_CALLBACK(on_tree_action), app);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ctrl), app->tree_traversal_combo, FALSE,
                     FALSE, 0);

  gtk_box_pack_start(GTK_BOX(view_tree), hbox_tree_ctrl, FALSE, FALSE, 0);

  // Tree Action Controls
  GtkWidget *hbox_tree_ops = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(view_tree), hbox_tree_ops, FALSE, FALSE, 0);

  GtkWidget *entry_tree_val = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_tree_val), "Valeur");
  gtk_widget_set_size_request(entry_tree_val, 80, -1);
  g_object_set_data(G_OBJECT(app->window), "tree_op_val",
                    entry_tree_val); // Store for callback

  GtkWidget *entry_tree_old = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_tree_old), "Ancienne (Modif)");
  gtk_widget_set_size_request(entry_tree_old, 80, -1);
  g_object_set_data(G_OBJECT(app->window), "tree_op_old", entry_tree_old);

  gtk_box_pack_start(GTK_BOX(hbox_tree_ops), entry_tree_val, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ops), entry_tree_old, FALSE, FALSE, 0);

  GtkWidget *btn_ins = gtk_button_new_with_label("+");
  gtk_widget_set_name(btn_ins, "insert");
  g_signal_connect(btn_ins, "clicked", G_CALLBACK(on_tree_op_action), app);

  GtkWidget *btn_del = gtk_button_new_with_label("-");
  gtk_widget_set_name(btn_del, "delete");
  g_signal_connect(btn_del, "clicked", G_CALLBACK(on_tree_op_action), app);

  GtkWidget *btn_mod = gtk_button_new_with_label("Mod");
  gtk_widget_set_name(btn_mod, "modify");
  g_signal_connect(btn_mod, "clicked", G_CALLBACK(on_tree_op_action), app);

  gtk_box_pack_start(GTK_BOX(hbox_tree_ops), btn_ins, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ops), btn_del, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_tree_ops), btn_mod, FALSE, FALSE, 0);

  // Extra Features
  GtkWidget *btn_order = gtk_button_new_with_label("Ordonner");
  gtk_widget_set_name(btn_order, "order");
  g_signal_connect(btn_order, "clicked", G_CALLBACK(on_tree_op_action), app);
  gtk_box_pack_end(GTK_BOX(hbox_tree_ops), btn_order, FALSE, FALSE, 5);

  GtkWidget *btn_trans = gtk_button_new_with_label("N-aire > Binaire");
  g_signal_connect(btn_trans, "clicked", G_CALLBACK(on_tree_transform), app);
  gtk_box_pack_end(GTK_BOX(hbox_tree_ops), btn_trans, FALSE, FALSE, 5);

  app->tree_canvas = gtk_drawing_area_new();
  g_signal_connect(app->tree_canvas, "draw", G_CALLBACK(on_draw_tree), app);

  // Wrap canvas in scrolled window
  GtkWidget *tree_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tree_scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(tree_scroll), app->tree_canvas);

  gtk_box_pack_start(GTK_BOX(view_tree), tree_scroll, TRUE, TRUE, 0);

  // Label Traversal
  GtkWidget *lbl_traversal = gtk_label_new("Parcours de l'arbre :");
  gtk_widget_set_halign(lbl_traversal, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(view_tree), lbl_traversal, FALSE, FALSE, 5);

  GtkWidget *log_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(log_scroll, -1, 100);
  GtkWidget *tree_log = gtk_text_view_new();
  // Make it read only
  gtk_text_view_set_editable(GTK_TEXT_VIEW(tree_log), FALSE);
  app->tree_log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tree_log));
  gtk_container_add(GTK_CONTAINER(log_scroll), tree_log);
  gtk_box_pack_start(GTK_BOX(view_tree), log_scroll, FALSE, FALSE, 0);

  gtk_stack_add_named(GTK_STACK(app->view_stack), view_tree, "view_tree");

  // === VIEW GRAPH ===
  GtkWidget *view_graph = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(view_graph), 10);

  // Graph Controls
  GtkWidget *graph_ctrl_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_style_context_add_class(gtk_widget_get_style_context(graph_ctrl_box),
                              "card");
  gtk_box_pack_start(GTK_BOX(view_graph), graph_ctrl_box, FALSE, FALSE, 0);

  // Row 1: Config & Node Management
  GtkWidget *hbox_graph_config = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
  gtk_box_pack_start(GTK_BOX(graph_ctrl_box), hbox_graph_config, FALSE, FALSE,
                     0);

  app->graph_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_type_combo),
                                 "Orient\xC3\xA9");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_type_combo),
                                 "Non-Orient\xC3\xA9");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->graph_type_combo), 0);
  g_signal_connect(app->graph_type_combo, "changed",
                   G_CALLBACK(on_graph_type_changed), app);
  gtk_box_pack_start(GTK_BOX(hbox_graph_config), app->graph_type_combo, FALSE,
                     FALSE, 0);

  app->graph_data_type_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_data_type_combo),
                                 "INT");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_data_type_combo),
                                 "FLOAT");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_data_type_combo),
                                 "CHAR");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_data_type_combo),
                                 "STRING");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->graph_data_type_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_graph_config), app->graph_data_type_combo,
                     FALSE, FALSE, 0);

  GtkWidget *btn_gen_graph = gtk_button_new_with_label("G\xC3\xA9n\xC3\xA9rer");
  g_signal_connect(btn_gen_graph, "clicked", G_CALLBACK(on_generate_graph),
                   app);
  gtk_box_pack_start(GTK_BOX(hbox_graph_config), btn_gen_graph, FALSE, FALSE,
                     0);

  GtkWidget *btn_add_node = gtk_button_new_with_label("Ajouter Noeud");
  gtk_widget_set_name(btn_add_node, "add_node");
  g_signal_connect(btn_add_node, "clicked", G_CALLBACK(on_graph_action), app);
  gtk_box_pack_start(GTK_BOX(hbox_graph_config), btn_add_node, FALSE, FALSE, 0);

  GtkWidget *btn_reset_graph =
      gtk_button_new_with_label("R\xC3\xA9initialiser");
  gtk_widget_set_name(btn_reset_graph, "reset_graph");
  g_signal_connect(btn_reset_graph, "clicked", G_CALLBACK(on_graph_action),
                   app);
  gtk_box_pack_end(GTK_BOX(hbox_graph_config), btn_reset_graph, FALSE, FALSE,
                   0);

  // Row 2: Edges & Algorithms
  GtkWidget *hbox_graph_actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
  gtk_box_pack_start(GTK_BOX(graph_ctrl_box), hbox_graph_actions, FALSE, FALSE,
                     0);

  app->graph_val_entry = gtk_entry_new();
  app->graph_src_entry = gtk_entry_new();
  app->graph_dst_entry = gtk_entry_new();

  app->graph_weight_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->graph_weight_entry),
                                 "Poids (Vide=Auto)");
  gtk_widget_set_size_request(app->graph_weight_entry, 120, -1);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), app->graph_weight_entry,
                     FALSE, FALSE, 0);

  GtkWidget *btn_add_edge = gtk_button_new_with_label("Lier (Clic Souris)");
  gtk_widget_set_name(btn_add_edge, "add_edge");
  g_signal_connect(btn_add_edge, "clicked", G_CALLBACK(on_graph_action), app);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), btn_add_edge, FALSE, FALSE,
                     10);

  // Separation
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions),
                     gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE,
                     10);

  // Algorithms (Remaining part of Row 2)
  app->graph_algo_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_algo_combo),
                                 "Dijkstra");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_algo_combo),
                                 "Bellman-Ford");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->graph_algo_combo),
                                 "Floyd-Warshall");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->graph_algo_combo), 0);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), app->graph_algo_combo, FALSE,
                     FALSE, 0);

  app->graph_start_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->graph_start_entry),
                                 "D\xC3\xA9part");
  gtk_widget_set_size_request(app->graph_start_entry, 60, -1);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), app->graph_start_entry, FALSE,
                     FALSE, 0);

  app->graph_end_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app->graph_end_entry),
                                 "Arriv\xC3\xA9"
                                 "e");
  gtk_widget_set_size_request(app->graph_end_entry, 60, -1);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), app->graph_end_entry, FALSE,
                     FALSE, 0);

  GtkWidget *btn_run_algo = gtk_button_new_with_label("Chemin");
  gtk_widget_set_name(btn_run_algo, "run_algo");
  g_signal_connect(btn_run_algo, "clicked", G_CALLBACK(on_graph_action), app);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), btn_run_algo, FALSE, FALSE,
                     0);

  GtkWidget *btn_show_rep = gtk_button_new_with_label("Repr\xC3\xA9sentation");
  gtk_widget_set_name(btn_show_rep, "rep_graph");
  g_signal_connect(btn_show_rep, "clicked", G_CALLBACK(on_graph_action), app);
  gtk_box_pack_start(GTK_BOX(hbox_graph_actions), btn_show_rep, FALSE, FALSE,
                     0);

  // Canvas
  app->graph_canvas = gtk_drawing_area_new();
  gtk_widget_add_events(app->graph_canvas, GDK_BUTTON_PRESS_MASK |
                                               GDK_BUTTON1_MOTION_MASK |
                                               GDK_BUTTON_RELEASE_MASK);
  g_signal_connect(app->graph_canvas, "draw", G_CALLBACK(on_draw_graph), app);
  g_signal_connect(app->graph_canvas, "button-press-event",
                   G_CALLBACK(on_graph_button_press), app);
  g_signal_connect(app->graph_canvas, "motion-notify-event",
                   G_CALLBACK(on_graph_motion), app);
  g_signal_connect(app->graph_canvas, "button-release-event",
                   G_CALLBACK(on_graph_button_release), app);
  gtk_widget_set_size_request(app->graph_canvas, -1, 400);
  gtk_box_pack_start(GTK_BOX(view_graph), app->graph_canvas, TRUE, TRUE, 0);

  // Log
  GtkWidget *log_g_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(log_g_scroll, -1, 100);
  GtkWidget *graph_log = gtk_text_view_new();
  app->graph_log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(graph_log));
  gtk_container_add(GTK_CONTAINER(log_g_scroll), graph_log);
  gtk_box_pack_start(GTK_BOX(view_graph), log_g_scroll, FALSE, FALSE, 0);

  gtk_stack_add_named(GTK_STACK(app->view_stack), view_graph, "view_graph");

  // Show defaults
  // Initial state: Start at home
  on_nav_switch(btn_home, app);

  gtk_widget_show_all(app->window);

  // But we need to explicitly hide the boxes again because show_all shows them
  gtk_widget_hide(g_object_get_data(G_OBJECT(app->window), "stats_box"));
  gtk_widget_hide(g_object_get_data(G_OBJECT(app->window), "settings_box"));
  gtk_widget_hide(g_object_get_data(G_OBJECT(app->window), "nav_box"));

  gtk_main();
  return 0;
}
