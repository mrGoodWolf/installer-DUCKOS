#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <dirent.h>

#define ISO_URL "https://aaron.sonin.me/duckOS/duckOS-v8.img"
#define ISO_FILE "duckOS-v8.img"

// Function to download file using libcurl
int download_file(const char *url, const char *filename)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    fp = fopen(filename, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return -1;
    }

    return 0;
}



// Function to list drives
char **list_drives(int *count)
{
    struct dirent *de;
    DIR *dr = opendir("/dev");
    char **drives = NULL;
    int i = 0;

    if (dr == NULL) {
        return NULL;
    }

    while ((de = readdir(dr)) != NULL) {
        if (strncmp(de->d_name, "", 2) == 0) {
            drives = realloc(drives, sizeof(char *) * (i + 1));
            drives[i] = strdup(de->d_name);
            i++;
        }
    }

    closedir(dr);
    *count = i;
    return drives;
}

// Function to free drives
void free_drives(char **drives, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        free(drives[i]);
    }
    free(drives);
}

int main(void)
{
    char **drives;
    int count, i;
    char* choice;

    // List drives
    drives = list_drives(&count);
    if (!drives) {
        printf("Failed to list drives\n");
        return 1;
    }

    // Print drives and let user choose
    printf("Please choose a drive:\n");
    for (i = 0; i < count; i++) {
        printf("%d: %s\n", i + 1, drives[i]);
    }
    printf("Choice: ");
    scanf("%d", choice);

    // Check choice
    if (choice < 1 || choice > count) {
        printf("Invalid choice\n");
        free_drives(drives, count);
        return 1;
    }

    // Download the ISO file
    if (download_file(ISO_URL, ISO_FILE) != 0) {
        printf("Failed to download ISO file\n");
        free_drives(drives, count);
        return 1;
    }

    printf("Successfully downloaded ISO file to %s\n", ISO_FILE);

    FILE* file = fopen(ISO_FILE, "rb"); // Open the ISO file
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Assuming you've already connected the USB drive
    FILE* usbDrive = fopen(choice, "wb"); // Open the USB drive
    if (!usbDrive) {
        perror("Error opening USB drive");
        fclose(file);
        return 1;
    }

    char buffer[4096];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        fwrite(buffer, 1, bytesRead, usbDrive); // Write data to USB drive
    }

    fclose(file);
    fclose(usbDrive);

    char *grub;
    printf("select a drive for grub:");
    scanf(grub);

    get_popen("grub-install", grub);

    return 0;
}

void get_popen(char *cmd, char * DATA) {
    FILE *pf;

    // Execute a process listing
    sprintf(cmd, "ps aux wwwf"); 

    // Setup our pipe for reading and execute our command.
    pf = popen(cmd,"r"); 

    // Error handling

    // Get the data from the process execution
    fgets(DATA, strlen(DATA) , pf);

    // the data is now in 'data'

    if (pclose(pf) != 0) {
        fprintf(stderr," Error: Failed to close command stream \n");
    }
}
