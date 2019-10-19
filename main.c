#include <gtk/gtk.h>

#include "lib/socket.h"
#include "lib/threads.h"

char* listen_addr = "127.0.0.1";
int listen_port = 14514;
char* fs_path = "/";

int thread_num = 10;
int job_num = 30;
int timeout = 3;

void init_server_wrapper(void* args) {
  init_server(thread_num, job_num, timeout);
}

static void start_server(GtkWidget *widget, gpointer data) {

  struct thread_pool* pool = init_thread_pool(thread_num, job_num);

  set_listen_addr(listen_addr);
  set_listen_port(listen_port);
  set_fs_path("/home/drac_zhang/Workplace/Coding/Calcaccel/calcaccel");
  thread_pool_add_task(pool, init_server_wrapper, NULL);

}

static void kill_server(GtkWidget *widget, gpointer data) {
  stop_server();
}

/*
 *  - window
 *    + table
 *      * frame(actions)
 *        - table
 *          + button(Start)
 *          + button(Stop)
 *      * frame(log)
 *        - table
 *          + text_view
 */ 

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
  gtk_table_attach_defaults(GTK_TABLE(table_window), frame_actions, 0, 6, 0, 1);

  // table_actions
  GtkWidget* table_actions = gtk_table_new(6, 4, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table_actions), 3);
  gtk_table_set_row_spacing(GTK_TABLE(table_actions), 0, 3);
  gtk_container_add(GTK_CONTAINER(frame_actions), table_actions);

  // frame_log
  GtkWidget* frame_log = gtk_frame_new("Log");
  gtk_frame_set_shadow_type(GTK_FRAME(frame_log), GTK_SHADOW_ETCHED_IN);
  gtk_table_attach_defaults(GTK_TABLE(table_window), frame_log, 0, 6, 2, 6);

  // table_log
  GtkWidget* table_log = gtk_table_new(6, 4, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table_log), 3);
  gtk_table_set_row_spacing(GTK_TABLE(table_log), 0, 3);
  gtk_container_add(GTK_CONTAINER(frame_log), table_log);

  // text_view
  GtkWidget* log = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(log), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(log), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(log), 15);
  gtk_table_attach(GTK_TABLE(table_log), log, 1, 2, 0, 1, 
      GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 1, 1);

  // button_start
  GtkWidget* btn_start = gtk_button_new_with_label("Start Server");
  gtk_widget_set_size_request(btn_start, 50, 30);
  gtk_table_attach(GTK_TABLE(table_actions), btn_start, 2, 3, 1, 2,
      GTK_FILL, GTK_SHRINK, 1, 1);

  // button_stop
  GtkWidget* btn_stop = gtk_button_new_with_label("Stop Server");
  gtk_widget_set_size_request(btn_stop, 50, 30);
  gtk_table_attach(GTK_TABLE(table_actions), btn_stop, 5, 6, 1, 2,
      GTK_FILL, GTK_SHRINK, 1, 1);

  g_signal_connect(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), G_OBJECT(window));

  g_signal_connect(btn_start, "clicked", G_CALLBACK(start_server), (gpointer)table_actions);
  g_signal_connect(btn_stop, "clicked", G_CALLBACK(kill_server), NULL);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}