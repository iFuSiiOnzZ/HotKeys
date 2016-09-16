#include <sys/stat.h> /* struct stat, stat */
#include <stdio.h>	  /* FILE, fopen_s, fread, fclose */
#include <utils.h>

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