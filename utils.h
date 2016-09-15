#ifndef UTILS_H
#define UTILS_H

unsigned int GetFileSize(char *pFileName);
void LogToFile(char *pMessage, char *pFile);
char *LoadStringFileIntoMemory(char *pFileName);

#endif
