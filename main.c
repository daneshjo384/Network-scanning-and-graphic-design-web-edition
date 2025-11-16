#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>

// ?????? ???? ????? ??????? ??????
typedef struct {
    char ip[16];
    char mac[18];
} Device;

#define MAX_DEVICES 256
static Device devices[MAX_DEVICES];
static int device_count = 0;

// ????? ????? ? ?????? ?????
void run_command(const char* cmd, char* output) {
    FILE* fp = popen(cmd, "r");
    if (fp) {
        fread(output, 1, 4095, fp);
        pclose(fp);
    }
}

// ???? ???? ? ?? ???? ?????
void scan_network() {
    char cmd[256];
    char output[4096] = {0};

    // ????? ???? ?? ???? ??? (????? 192.168.1.0/24)
    snprintf(cmd, sizeof(cmd), "arp-scan --local 2>/dev/null | grep -E '[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}' | head -20");

    run_command(cmd, output);

    char* line = strtok(output, "\n");
    while (line && device_count < MAX_DEVICES) {
        char ip[16], mac[18];
        if (sscanf(line, "%15s %17s", ip, mac) == 2) {
            strcpy(devices[device_count].ip, ip);
            strcpy(devices[device_count].mac, mac);
            device_count++;
        }
        line = strtok(NULL, "\n");
    }
}

// ????? ???????
static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *scrolled;
    GtkWidget *grid;
    int i;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Network Scanner");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scrolled);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrolled), grid);

    // ??????
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("IP Address"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("MAC Address"), 1, 0, 1, 1);

    for (i = 0; i < device_count; i++) {
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(devices[i].ip), 0, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(devices[i].mac), 1, i + 1, 1, 1);
    }

    gtk_widget_show_all(window);
}

// ????? ?? ???? JSON
void save_to_json(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Cannot open file for writing.\n");
        return;
    }

    fprintf(f, "{\n \"devices\": [\n");
    for (int i = 0; i < device_count; i++) {
        fprintf(f, "  {\"ip\":\"%s\", \"mac\":\"%s\"}", devices[i].ip, devices[i].mac);
        if (i < device_count - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, " ]\n}\n");

    fclose(f);
    printf("Data saved to %s\n", filename);
}

int main(int argc, char *argv[]) {
    scan_network();

    // ????? ?????
    save_to_json("network_scan.json");

    // ????? GUI
    GtkApplication *app = gtk_application_new("com.network.scanner", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}