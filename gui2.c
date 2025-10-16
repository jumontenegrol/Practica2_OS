#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <fcntl.h>      
#include <sys/types.h>  
#include <sys/stat.h>   

#define FIFO_IN  "/tmp/fifo_in"
#define FIFO_OUT "/tmp/fifo_out"

GtkWidget *entry_clave;
GtkWidget *entry_nombre;
GtkWidget *entry_id;
GtkWidget *textview_output;

// ---------- FUNCIONES EXISTENTES ----------
void on_search_clicked(GtkWidget *widget, gpointer data) {
    const char *clave = gtk_entry_get_text(GTK_ENTRY(entry_clave));
    const char *nombre = gtk_entry_get_text(GTK_ENTRY(entry_nombre));
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_output));
    gtk_text_buffer_set_text(buffer, "", -1);

    if (!clave || strlen(clave) == 0) {
        gtk_text_buffer_set_text(buffer, "Ingrese al menos la clave.\n", -1);
        return;
    }

    char nombre_val[128];
    char id_val[128];
    snprintf(nombre_val, sizeof(nombre_val), "%s", (nombre && strlen(nombre) > 0) ? nombre : "-");
    snprintf(id_val, sizeof(id_val), "%s", (id && strlen(id) > 0) ? id : "-");

    int f_out = open(FIFO_IN, O_WRONLY);
    if (f_out < 0) {
        gtk_text_buffer_set_text(buffer, "Error al abrir FIFO de salida.\n", -1);
        return;
    }

    char mensaje[512];
    int bytes = snprintf(mensaje, sizeof(mensaje), "%s,%s,%s", clave, nombre_val, id_val);
    mensaje[bytes] = '\0';
    write(f_out, mensaje, strlen(mensaje));
    close(f_out);

    int fd_in = open(FIFO_OUT, O_RDONLY);
    if (fd_in < 0) {
        perror("No se pudo abrir FIFO_OUT para leer");
        return;
    }

    FILE *fp = fdopen(fd_in, "r");
    if (!fp) {
        perror("Error en fdopen()");
        close(fd_in);
        return;
    }

    char buffer1[4096];
    while (fgets(buffer1, sizeof(buffer1), fp)) {
        gtk_text_buffer_insert_at_cursor(buffer, buffer1, -1);
    }
    fclose(fp);

    gtk_entry_set_text(GTK_ENTRY(entry_clave), "");
    gtk_entry_set_text(GTK_ENTRY(entry_nombre), "");
    gtk_entry_set_text(GTK_ENTRY(entry_id), "");
}

// ---------- NUEVO: AÑADIR DATOS ----------
void on_add_data_clicked(GtkWidget *widget, gpointer user_data[]) {
    GtkWidget *entry_add_clave = user_data[0];
    GtkWidget *entry_add_nombre = user_data[1];
    GtkWidget *entry_add_id = user_data[2];
    GtkWidget *textview_add = user_data[3];  

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_add));

    const char *clave = gtk_entry_get_text(GTK_ENTRY(entry_add_clave));
    const char *nombre = gtk_entry_get_text(GTK_ENTRY(entry_add_nombre));
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_add_id));

    if (!clave || strlen(clave) == 0) {
        gtk_text_buffer_set_text(buffer, "Debe ingresar una clave.\n", -1);
        return;
    }

    FILE *fp = fopen("datos.csv", "a");
    if (!fp) {
        gtk_text_buffer_set_text(buffer, "Error al abrir el archivo CSV.\n", -1);
        return;
    }

    fprintf(fp, "%s,%s,%s\n", clave, nombre, id);
    fclose(fp);

    gtk_text_buffer_set_text(buffer, "✅ Registro añadido correctamente.\n", -1);

    gtk_entry_set_text(GTK_ENTRY(entry_add_clave), "");
    gtk_entry_set_text(GTK_ENTRY(entry_add_nombre), "");
    gtk_entry_set_text(GTK_ENTRY(entry_add_id), "");
}

// ---------- VENTANA PARA AÑADIR DATOS ----------
void create_add_window(GtkWidget *widget, gpointer data) {
    GtkWidget *window, *grid, *label1, *label2, *label3;
    GtkWidget *entry_add_clave, *entry_add_nombre, *entry_add_id;
    GtkWidget *button_add;
    GtkWidget *textview_add;
    GtkTextBuffer *buffer;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Añadir nuevo registro");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 350);

    grid = gtk_grid_new();
    gtk_widget_set_name(grid, "add_box");
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(window), grid);

    label1 = gtk_label_new("Clave:");
    label2 = gtk_label_new("Nombre:");
    label3 = gtk_label_new("ID:");

    entry_add_clave = gtk_entry_new();
    entry_add_nombre = gtk_entry_new();
    entry_add_id = gtk_entry_new();
 
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_add_clave), "Ingrese clave...");
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_add_nombre), "Ingrese nombre...");
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_add_id), "Ingrese ID...");

    button_add = gtk_button_new_with_label("Guardar en CSV");

    textview_add = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_add), FALSE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_add));

    GtkWidget *user_data[4] = { entry_add_clave, entry_add_nombre, entry_add_id, GTK_WIDGET(buffer) };
    g_signal_connect(button_add, "clicked", G_CALLBACK(on_add_data_clicked), user_data);

    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_add_clave, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_add_nombre, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_add_id, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_add, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), textview_add, 0, 4, 2, 1);

    gtk_widget_show_all(window);
}

// ---------- MAIN ----------
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Buscador de Registros");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_name(vbox, "main_box");
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *label = gtk_label_new("Ingrese la clave a buscar:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    entry_clave = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_clave), "Clave...");
    gtk_box_pack_start(GTK_BOX(vbox), entry_clave, FALSE, FALSE, 0);

    entry_nombre = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nombre), "Nombre...");
    gtk_box_pack_start(GTK_BOX(vbox), entry_nombre, FALSE, FALSE, 0);

    entry_id = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_id), "ID...");
    gtk_box_pack_start(GTK_BOX(vbox), entry_id, FALSE, FALSE, 0);

    GtkWidget *button_search = gtk_button_new_with_label("Buscar registro");
    g_signal_connect(button_search, "clicked", G_CALLBACK(on_search_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_search, FALSE, FALSE, 0);

    GtkWidget *button_add_window = gtk_button_new_with_label("Añadir nuevo registro");
    g_signal_connect(button_add_window, "clicked", G_CALLBACK(create_add_window), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_add_window, FALSE, FALSE, 0);

    textview_output = gtk_text_view_new();
    gtk_widget_set_name(textview_output, "resultado");
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_output), GTK_WRAP_WORD_CHAR);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), textview_output);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
