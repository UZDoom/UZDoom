#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

namespace ZipFile {

class FZipFile;

FZipFile *Open(FILE* file);
size_t Read(FZipFile* zip, uint32_t entry, void* data, size_t length);
int FindEntry(FZipFile* zip, const char* name);
size_t Length(FZipFile* zip, uint32_t entry);
void Close(FZipFile* zip);

}
