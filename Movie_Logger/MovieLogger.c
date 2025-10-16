#include <gtk/gtk.h>
#include <mysql/mysql.h>

typedef struct
{
    GtkWidget *entry1;
    GtkWidget *entry2;
    GtkWidget *entry3;
    GtkWidget *output_label;

} Entries;
// set size of new widgets
void with_margin(GtkWidget *widget, int margin, int width, int height)
{
    gtk_widget_set_margin_start(widget, margin);
    gtk_widget_set_margin_end(widget, margin);
    gtk_widget_set_size_request(widget, width, height);
}
void on_button_clicked(GtkWidget *widget, gpointer data)
{
    Entries *e = (Entries *)data;
    const gchar *input1 = gtk_entry_get_text(GTK_ENTRY(e->entry1));
    const gchar *input2 = gtk_entry_get_text(GTK_ENTRY(e->entry2));
    const gchar *input3 = gtk_entry_get_text(GTK_ENTRY(e->entry3));

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "Root@1234", "iotdb", 0, NULL, 0))
    {
        g_printerr("MySQL connect error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    gchar *query = g_strdup_printf("insert into Movies(Name, Year, Rating) values('%s',%s,%s)", input1, input2, input3);

    if (mysql_query(conn, query))
    {
        g_printerr("Insert error: %s\n", mysql_error(conn));
    }

    g_free(query);
    mysql_close(conn);

    /* echo back to user */
    gchar *msg = g_strdup_printf("Last Saved:\n%s (%s)  Rating %s/5", input1, input2, input3);
    gtk_label_set_text(GTK_LABEL(e->output_label), msg);
    gtk_label_set_xalign(GTK_LABEL(e->output_label), 0.0);
    g_free(msg);
}

void on_view_clicked(GtkWidget *widget, gpointer data)
{
    MYSQL *conn = mysql_init(NULL);
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (!mysql_real_connect(conn, "localhost", "root", "Root@1234", "iotdb", 0, NULL, 0))
    {
        g_printerr("MySQL connect error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    if (mysql_query(conn, "SELECT * FROM Movies Order by timestamp desc"))
    {
        g_printerr("Query error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    res = mysql_store_result(conn);
    if (!res)
    {
        mysql_close(conn);
        return;
    }

    
    // Create window
    GtkWidget *view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(view_window), "Movie Log");
    gtk_window_set_default_size(GTK_WINDOW(view_window), 500, 400);

    // Create list store with 5 columns: ID, Name, Year, Rating, Date
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, 
                                              G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    // Populate the list store
    GtkTreeIter iter;
    while ((row = mysql_fetch_row(res)))
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, row[0],  // ID
                          1, row[1],  // Name
                          2, row[2],  // Year
                          3, row[3],  // Rating
                          4, row[4],  // Date
                          -1);
    }

    mysql_free_result(res);
    mysql_close(conn);

    // Create tree view
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store); // Tree view now holds reference

    // Create columns
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    
    GtkTreeViewColumn *col_id = gtk_tree_view_column_new_with_attributes(
        "ID", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_fixed_width(col_id, 50);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_id);

    GtkTreeViewColumn *col_name = gtk_tree_view_column_new_with_attributes(
        "Movie Name", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_min_width(col_name, 200);
    gtk_tree_view_column_set_resizable(col_name, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_name);

    GtkTreeViewColumn *col_year = gtk_tree_view_column_new_with_attributes(
        "Year", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_fixed_width(col_year, 80);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_year);

    GtkTreeViewColumn *col_rating = gtk_tree_view_column_new_with_attributes(
        "Rating", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_fixed_width(col_rating, 80);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_rating);

    GtkTreeViewColumn *col_date = gtk_tree_view_column_new_with_attributes(
        "Date", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_min_width(col_date, 120);
    gtk_tree_view_column_set_resizable(col_date, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_date);

    // Add to scrolled window
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tree_view);
    gtk_container_add(GTK_CONTAINER(view_window), scroll);

    gtk_widget_show_all(view_window);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    // css
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "/home/rohit/Desktop/GUI_LIB/MovieLog/style.css", NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // window

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Movie Logger");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 400); // width: 400px, height: 300px
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *label1 = gtk_label_new("Enter Movie Name:");
    GtkWidget *entry1 = gtk_entry_new();
    with_margin(entry1, 50, 200, 10);

    GtkWidget *label2 = gtk_label_new("Enter Release Year:");
    GtkWidget *entry2 = gtk_entry_new();
    with_margin(entry2, 50, 200, 10);

    GtkWidget *label3 = gtk_label_new("Enter Rating /5:");
    GtkWidget *entry3 = gtk_entry_new();
    with_margin(entry3, 50, 200, 10);

    GtkWidget *button = gtk_button_new_with_label("Submit Entry");
    with_margin(button, 50, 200, 10);

    GtkWidget *view_button = gtk_button_new_with_label("View Movie Log");
    with_margin(view_button, 50, 200, 10);

    GtkWidget *output_label = gtk_label_new("");
    with_margin(output_label, 50, 50, 10);

    // fields for each entry in DB
    Entries *entries = g_new(Entries, 1);
    entries->entry1 = entry1;
    entries->entry2 = entry2;
    entries->entry3 = entry3;
    entries->output_label = output_label;

    // connect functions to buttons
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), entries);
    g_signal_connect(view_button, "clicked", G_CALLBACK(on_view_clicked), NULL);

    // sets sequence of object on window
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), entry1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), entry2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), entry3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), view_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), output_label, FALSE, FALSE, 0);

    // gives name to widgets for css
    gtk_widget_set_name(entry1, "entry1");
    gtk_widget_set_name(entry2, "entry2");
    gtk_widget_set_name(entry3, "entry3");
    gtk_widget_set_name(button, "submit_button");
    gtk_widget_set_name(view_button, "view_button");
    gtk_widget_set_name(label1, "label1");
    gtk_widget_set_name(label2, "label2");
    gtk_widget_set_name(label3, "label3");

    gtk_widget_set_name(window, "main-window");

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
