#include <gtk/gtk.h>
#include <stdlib.h>

#include "lib/socket.h"
#include "lib/logger.h"
#include "lib/threads.h"

int thread_num = 10;
int job_num = 30;
int timeout = 3;

GtkWidget* buffer;

void logger(const char* msg) {
  gtk_text_buffer_insert_at_cursor(buffer, msg, strlen(msg));
}

void init_server_wrapper(void* args) {

  int err = init_server(thread_num, job_num, timeout);
  if (err < 0) {
    Error("Error code %d\n", err);
  }

}

static void start_server(GtkWidget *widget, gpointer data) {

  GtkWidget** entrys = data;

  struct thread_pool* pool = init_thread_pool(thread_num, job_num);

  set_listen_addr(gtk_entry_get_text(entrys[0]));
  set_listen_port(atoi(gtk_entry_get_text(entrys[1])));
  set_fs_path(gtk_entry_get_text(entrys[2]));
  thread_pool_add_task(pool, init_server_wrapper, NULL);

}

static void kill_server(GtkWidget *widget, gpointer data) {
  stop_server();
}

int main(int argc, char *argv[]) {
    
  gtk_init(&argc, &argv);

  // window
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_set_size_request(window, 720, 480);
  gtk_window_set_title(GTK_WINDOW(window), "HTTP light");
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);

  // table_window
  GtkWidget* table_window = gtk_table_new(6, 4, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table_window), 3);
  gtk_table_set_row_spacing(GTK_TABLE(table_window), 0, 3);
  gtk_container_add(GTK_CONTAINER(window), table_window);

  // frame_actions
  GtkWidget* frame_actions = gtk_frame_new("Actions");
  gtk_frame_set_shadow_type(GTK_FRAME(frame_actions), GTK_SHADOW_ETCHED_IN);
  gtk_container_set_border_width(GTK_CONTAINER(frame_actions), 5);
  gtk_table_attach_defaults(GTK_TABLE(table_window), frame_actions, 11, 12, 0, 1);

  // table_actions
  GtkWidget* table_actions = gtk_table_new(6, 4, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table_actions), 3);
  gtk_table_set_row_spacing(GTK_TABLE(table_actions), 0, 3);
  gtk_container_add(GTK_CONTAINER(frame_actions), table_actions);

  // frame_settings
  GtkWidget* frame_settings = gtk_frame_new("Settings");
  gtk_frame_set_shadow_type(GTK_FRAME(frame_settings), GTK_SHADOW_ETCHED_IN);
  gtk_container_set_border_width(GTK_CONTAINER(frame_settings), 5);
  gtk_table_attach_defaults(GTK_TABLE(table_window), frame_settings, 0, 11, 0, 1);

  // table_settings
  GtkWidget* table_settings = gtk_table_new(6, 4, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table_settings), 3);
  gtk_table_set_row_spacing(GTK_TABLE(table_settings), 0, 3);
  gtk_container_add(GTK_CONTAINER(frame_settings), table_settings);

  // setting_labels
  GtkWidget* label1 = gtk_label_new("IP");
  gtk_table_attach(GTK_TABLE(table_settings), label1, 0, 1, 0, 1, 
    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

  GtkWidget* label2 = gtk_label_new("Port");
  gtk_table_attach(GTK_TABLE(table_settings), label2, 2, 3, 0, 1, 
    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

  GtkWidget* label3 = gtk_label_new("Path");
  gtk_table_attach(GTK_TABLE(table_settings), label3, 0, 1, 1, 2, 
    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

  // setting_entrys
  GtkWidget* entrys[3];

  entrys[0] = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(table_settings), entrys[0], 1, 2, 0, 1, 
    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_entry_set_text(entrys[0], "127.0.0.1");

  entrys[1] = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(table_settings), entrys[1], 3, 4, 0, 1, 
    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_entry_set_text(entrys[1], "14514");

  entrys[2] = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(table_settings), entrys[2], 1, 4, 1, 2, 
    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_entry_set_text(entrys[2], "/");

  // frame_log
  GtkWidget* frame_log = gtk_frame_new("Log");
  gtk_frame_set_shadow_type(GTK_FRAME(frame_log), GTK_SHADOW_ETCHED_IN);
  gtk_container_set_border_width(GTK_CONTAINER(frame_log), 5);
  gtk_table_attach_defaults(GTK_TABLE(table_window), frame_log, 0, 12, 2, 27);

  // Scrollable window
  GtkWidget* scroll_log = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(frame_log), scroll_log);

  // text_view
  GtkWidget* log = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(log), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(log), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(log), 15);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_log), log);

  // buffer
  buffer = gtk_text_buffer_new(NULL);
  gtk_text_view_set_buffer(log, buffer);
  set_logger(logger);

  // button_start
  GtkWidget* btn_start = gtk_button_new_with_label("Start Server");
  gtk_widget_set_size_request(btn_start, 150, 30);
  gtk_container_set_border_width(GTK_CONTAINER(btn_start), 5);
  gtk_table_attach(GTK_TABLE(table_actions), btn_start, 3, 4, 1, 2,
      GTK_FILL, GTK_SHRINK, 1, 1);

  // buttons
  GtkWidget* valign = gtk_alignment_new(0, 0, 0, 0);
  gtk_table_attach(GTK_TABLE(table_actions), valign, 3, 4, 2, 3,
      GTK_FILL, GTK_SHRINK, 1, 1);

  // button_stop
  GtkWidget* btn_stop = gtk_button_new_with_label("Stop Server");
  gtk_widget_set_size_request(btn_stop, 150, 30);
  gtk_container_set_border_width(GTK_CONTAINER(btn_stop), 5);
  gtk_container_add(GTK_CONTAINER(valign), btn_stop);

  g_signal_connect(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), G_OBJECT(window));

  g_signal_connect(btn_start, "clicked", G_CALLBACK(start_server), (gpointer)entrys);
  g_signal_connect(btn_stop, "clicked", G_CALLBACK(kill_server), NULL);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}