#include <string.h>
#include "memory.h"
#include "user_data.h"


UserData createUserData(const UserDataSize size, UserDataValue *value) {
    return (UserData){value, size};
}

UserDataList *createUserDataList(const size_t initialSize) {
    UserDataList *userDataList = safeAlloc(sizeof(UserDataList), "user data");
    userDataList->cells = safeAlloc(sizeof(UserData) * initialSize, "user data cells");
    memset(userDataList->cells, 0, sizeof(UserData) * initialSize);

    return userDataList;
}

void userDataList_reallocate(UserDataList *userDataList, const UserDataIndex oldSize, const UserDataIndex newSize) {
    userDataList->cells = safeRealloc(userDataList->cells, oldSize, newSize, sizeof(UserData), "user data cells");
    for (UserDataIndex i = oldSize; i < newSize; i++) {
        userDataList->cells[i].size = 0;
    }
}

void userDataList_set(UserDataList *userDataList, const UserDataIndex index, const UserData data) {
    userDataList->cells[index] = data;
}

UserData userDataList_get(const UserDataList *userDataList, const UserDataIndex index) {
    return userDataList->cells[index];
}

void userDataList_free(UserDataList *userDataList) {
    free(userDataList);
}
