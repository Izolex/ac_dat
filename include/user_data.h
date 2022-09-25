#ifndef __AC_DAT__USER_DATA__H__
#define __AC_DAT__USER_DATA__H__


typedef void UserDataValue;
typedef int32_t UserDataSize;

struct userData {
    UserDataValue *value;
    UserDataSize size;
};
struct userDataList;


struct userData createUserData(UserDataSize size, UserDataValue *value);
struct userDataList *createUserDataList(size_t initialSize);
void userDataList_free(struct userDataList *userDataList);

#endif
