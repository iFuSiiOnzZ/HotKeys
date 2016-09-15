#include <sys/stat.h> /* struct stat, stat */
#include <Windows.h>  /* HANDLE, CreateFileA, WriteFile, CloseHandle */
#include <stdio.h>	  /* FILE, fopen_s, fread, fclose */
#include <time.h>     /* struct tm, time, localtime_s, strftime */
#include "utils.h"

#define NEWLINE "\r\n"

void LogToFile(char *pMessage, char *pFile)
{
    HANDLE hFile = CreateFileA(pFile, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);

    if(hFile)
    {
        DWORD NumberOfBytesWritten = 0;

        time_t RawTime = 0 ;
        struct tm s_TimeInfo = { 0 };

        time (&RawTime);
        localtime_s(&s_TimeInfo, &RawTime);

        char pBuffer[80] = { 0 };
        strftime(pBuffer, 80 , "%d/%m/%y %H:%M:%S", &s_TimeInfo);

        WriteFile(hFile, pBuffer, (DWORD) strlen(pBuffer), &NumberOfBytesWritten, NULL);
        WriteFile(hFile,  "    ",                       4, &NumberOfBytesWritten, NULL);

        WriteFile(hFile, pMessage, (DWORD) strlen(pMessage), &NumberOfBytesWritten, NULL);
        WriteFile(hFile,  NEWLINE,  (DWORD) strlen(NEWLINE), &NumberOfBytesWritten, NULL);
        
        CloseHandle(hFile);
    }
}

unsigned int GetFileSize(char *pFileName)
{
    struct stat stat_buf = { 0 };
    int rc = stat(pFileName, &stat_buf);

    return rc == 0 ? stat_buf.st_size : 0;
}

char *LoadStringFileIntoMemory(char *pFileName)
{
    FILE *pFileHandle = NULL;
    char *pFileContent = NULL;

    fopen_s(&pFileHandle, pFileName, "r");
    if (!pFileHandle) return pFileContent;

    unsigned int FileSize = GetFileSize(pFileName);
    pFileContent = (char *) malloc(sizeof(char) * (FileSize + 1));

    size_t r = fread(pFileContent, 1, FileSize, pFileHandle);
    pFileContent[r] = '\0';

    fclose(pFileHandle);
    return pFileContent;
}