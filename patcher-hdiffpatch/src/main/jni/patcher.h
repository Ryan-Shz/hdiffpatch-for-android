//
// Created by yanghui.yangh on 2016-07-11.
//

#ifndef ANDROID_DIFFPATCH_NATIVE_PATCH_H
#define ANDROID_DIFFPATCH_NATIVE_PATCH_H

#define APPLY_ERROR_UNKOWN      (-1000)
#define SUCCESS                (0)

#define INVALID_PARAMETERS      (-2000)
#define MALLOC_MEMERY_ERROR     (-3000)
#define THREAD_CREAT_ERROR      (-4000)
#define THREAD_JOIN_ERROR (-5000)

#define PATCH_OPEN_ERROR        (-10000)
#define PATCH_SEEK_ERROR        (-11000)
#define PATCH_READ_ERROR        (-12000)
#define PATCH_CLOSE_ERROR       (-13000)
#define PATCH_DATA_ERROR        (-14000)

#define OLD_FILE_OPEN_ERROR     (-20000)
#define OLD_FILE_READ_ERROR     (-21000)
#define OLD_FILE_SEEK_ERROR     (-22000)
#define OLD_FILE_CLOSE_ERROR    (-23000)

#define NEW_FILE_OPEN_ERROR     (-30000)
#define NEW_FILE_SEEK_ERROR     (-31000)
#define NEW_FILE_WRITE_ERROR    (-32000)
#define NEW_FILE_CLOSE_ERROR    (-33000)
#define NEW_FILE_DELETE_ERROR   (-34000)
#define NEW_FILE_RENAME_ERROR   (-35000)


#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL applyPatch(JNIEnv *env, jobject obj, jstring oldFilePath,
                                 jstring diffFilePath, jstring newFilePath);


#ifdef __cplusplus
}
#endif
#endif //ANDROID_DIFFPATCH_NATIVE_PATCH_H
