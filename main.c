#include <gtk/gtk.h>
#include <stdbool.h>
#include "sequence.h"
#include "list.h"
#include "sequencesdatabase.h"
#include "gsp.h"

static gboolean finish_and_fill_gui(gpointer data);
static gpointer run_gsp_logic(gpointer data);
static void fill_output_view(SequencesList *frequent_sequences);
gboolean update_process_log(gpointer data);

SequencesDatabase *db = NULL;
SequencesList *frequent_sequences = NULL;

GtkBuilder *builder;
GtkWidget *window;

typedef struct {
    SequencesDatabase *db;
    size_t minsupp;
} GSPWorkerData;


void gui_status_listener(const char* message) {
    g_idle_add((GSourceFunc)update_process_log, g_strdup(message));
}

static gpointer run_gsp_logic(gpointer data) {
    GSPWorkerData *worker_data = (GSPWorkerData*)data;
    SequencesList *result = gsp(worker_data->db, worker_data->minsupp, gui_status_listener);
    g_idle_add((GSourceFunc)finish_and_fill_gui, result);
    g_free(worker_data);
    frequent_sequences = result;
    return NULL;
}

static gboolean finish_and_fill_gui(gpointer data) {
    SequencesList *result = (SequencesList*)data;
    
    fill_output_view(result);

    GtkWidget *start_button = GTK_WIDGET(gtk_builder_get_object(builder, "start_button"));
    GtkWidget *load_button = GTK_WIDGET(gtk_builder_get_object(builder, "load_button"));

    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(load_button, TRUE);
    
    return FALSE; 
}

gboolean update_process_log(gpointer data) {
    char *message = (char*)data;

    GtkTreeView *tv = GTK_TREE_VIEW(gtk_builder_get_object(builder, "process_tv"));
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(tv));
    
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, message, -1);

    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    gtk_tree_view_scroll_to_cell(tv, path, NULL, FALSE, 0, 0);
    gtk_tree_path_free(path);

    g_free(message);

    return FALSE;
}

void fill_input_view(SequencesDatabase *db) {
    if(db == NULL) {
        return;
    }

    GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "input_tv"));
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_view));
    gtk_list_store_clear(store);
    for(size_t i = 0; i < db->size; i++) {
        Sequence *sequence = db->sequences[i];
        char *sequence_str = sequence_toString(sequence);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, sequence_str,-1);
        free(sequence_str);
    }
    
}

static void fill_output_view(SequencesList *frequent_sequences) {
    if(frequent_sequences == NULL) {
        return;
    }

    GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "result_tv"));
    if(tree_view == NULL) {
        printf("jedigovna\n");
    }
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_view));
    if(store == NULL) {
        printf("pusikurac\n");
    }
    gtk_list_store_clear(store);
    SequencesListNode *current_node = frequent_sequences->head;
    while(current_node) {
        Sequence *sequence = current_node->sequence;
        char *sequence_str = sequence_toString(sequence);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, sequence_str, 1, (guint64)sequence->support, -1);
        free(sequence_str);
        current_node = current_node->next;
    }
}

void on_load_button_clicked(GtkButton *load_button, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    dialog = gtk_file_chooser_dialog_new("Choose sequence data base", GTK_WINDOW(window), action, "_Close", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if(res == GTK_RESPONSE_ACCEPT) {
        char *filepath;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filepath = gtk_file_chooser_get_filename(chooser);
        //database_free(db);
        db = database_load(filepath);
        fill_input_view(db);
        g_free(filepath);
    }
    gtk_widget_destroy(dialog);
}

void on_export_button_clicked(GtkButton *export_button, gpointer data) {
    printf("export button clicked\n");
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;
    dialog = gtk_file_chooser_dialog_new("Choose sequence data base", GTK_WINDOW(window), action, "_Close", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if(res == GTK_RESPONSE_ACCEPT) {
        char *filepath;
        filepath = gtk_file_chooser_get_filename(chooser);
        if(frequent_sequences && frequent_sequences->head) {
            FILE *file = fopen(filepath, "w");
            if(file) {
                SequencesListNode *current = frequent_sequences->head;
                while(current) {
                    char *spmf_line = sequence_toSPMF(current->sequence);
                    if(spmf_line) {
                        fprintf(file, "%s\n", spmf_line);
                        free(spmf_line);
                    }
                    current = current->next;
                }
                fclose(file);
            }
        }
        g_free(filepath);
    }
    gtk_widget_destroy(dialog);

}

void on_start_button_clicked(GtkButton *start_button, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(gtk_builder_get_object(builder, "support_entry"));
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    int num = atoi(text);
    if(num <= 0) {
        return;
    }
    if(db == NULL) {
        return;
    }
    size_t minsupp = (size_t)num;

    gtk_widget_set_sensitive(GTK_WIDGET(start_button), FALSE);
    GtkWidget *load_button = GTK_WIDGET(gtk_builder_get_object(builder, "load_button"));
    gtk_widget_set_sensitive(load_button, FALSE);

    GSPWorkerData *worker_data = malloc(sizeof(GSPWorkerData));
    worker_data->db = db;
    worker_data->minsupp = minsupp;

    g_thread_new("GSP_WORKER", run_gsp_logic, worker_data);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("V1.glade");

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    GtkWidget *load_button = GTK_WIDGET(gtk_builder_get_object(builder, "load_button"));
    GtkWidget *start_button = GTK_WIDGET(gtk_builder_get_object(builder, "start_button"));
    GtkWidget *export_button = GTK_WIDGET(gtk_builder_get_object(builder, "export_button"));

    g_signal_connect(load_button, "clicked", G_CALLBACK(on_load_button_clicked), window);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), window);
    g_signal_connect(export_button, "clicked", G_CALLBACK(on_export_button_clicked), window);
    
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}