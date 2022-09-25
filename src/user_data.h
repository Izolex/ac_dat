#ifndef USER_DATA_H
#define USER_DATA_H

#include "../include/user_data.h"

typedef struct userData UserData;
typedef int32_t UserDataIndex;

typedef struct userDataList {
    UserData *cells;
} UserDataList;


UserData userDataList_get(const UserDataList *userDataList, UserDataIndex index);
void userDataList_reallocate(UserDataList *userDataList, UserDataIndex oldSize, UserDataIndex newSize);
void userDataList_set(UserDataList *userDataList, UserDataIndex index, UserData userData);

#endif
