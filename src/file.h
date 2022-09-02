#ifndef FILE_H
#define FILE_H

#include "typedefs.h"

void file_store(const char *targetPath, const Automaton *automaton, const Tail *tail);

FileData file_load(const char *targetPath);

#endif
