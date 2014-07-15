/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include <android/log.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "JavaClass.h"
#include "AndroidPlatform.h"

namespace MaliSDK
{
    JavaClass::JavaClass(JNIEnv* env, const char* requiredClassPath):
        classPath(AndroidPlatform::copyString(requiredClassPath)),
        JNIEnvironment(env),
        intialized(false)
    {
        if (JNIEnvironment == NULL) 
        { 
            LOGE("ERROR - JavaClass: Environment cannot be NULL.\n"); 
            return; 
        }
        jClass = JNIEnvironment->FindClass(classPath);
        if (jClass == NULL) 
        { 
            LOGE("ERROR - JavaClass: Java class not found."); 
            return; 
        }
        intialized = true;
    }

    JavaClass::~JavaClass()
    {
        if (classPath != NULL) 
        {
            free(classPath);
        }
    }

    bool JavaClass::staticField(const char* fieldName, char** result)
    {
        if (!intialized || fieldName == NULL)
        {
            return false;
        }
        jfieldID fieldID = JNIEnvironment->GetStaticFieldID(jClass, fieldName, TJString);
        if (fieldID == NULL) 
        { 
            LOGE("ERROR - JavaClass: Field %s not found in %s\n", fieldName, classPath); 
            return false; 
        }
        jstring jValue = (jstring)JNIEnvironment->GetStaticObjectField(jClass, fieldID);
        const char *value = JNIEnvironment->GetStringUTFChars(jValue, 0);
        char * returnValue = AndroidPlatform::copyString(value);
        JNIEnvironment->ReleaseStringUTFChars(jValue, value);
        *result = returnValue;
        return true;
    }

    bool JavaClass::staticField(const char* fieldName, int* result)
    {
        if (!intialized || fieldName == NULL) return false;
        jfieldID fieldID = JNIEnvironment->GetStaticFieldID(jClass, fieldName, TJInt);
        if (fieldID == NULL) 
        { 
            LOGE("ERROR - JavaClass: Field %s not found in %s\n", fieldName, classPath); 
            return false; 
        }
        *result = JNIEnvironment->GetStaticIntField(jClass, fieldID);
        return true;
    }

    bool JavaClass::staticMethod(const char* methodName, int** returnValue, const char* param01)
    {
        if (!intialized || methodName == NULL || returnValue == NULL) 
        {
            return false;
        }
        jmethodID methodID = JNIEnvironment->GetStaticMethodID(jClass, methodName, JM(TJIntArr, TJString));
        if (methodID == NULL) 
        { 
            LOGE("ERROR - JavaClass: Method %s not found in %s.\n", methodName, classPath); 
            return false; 
        }
        jintArray javaReturn = (jintArray)JNIEnvironment->CallStaticObjectMethod(jClass, methodID, JNIEnvironment->NewStringUTF(param01));
        if (javaReturn == NULL) 
        { 
            LOGE("ERROR - JavaClass: A call to static method %s in %s failed.\n", methodName, classPath); 
            return false; 
        }
        jboolean copy; /* copy flag */
        *returnValue = (int*)JNIEnvironment->GetIntArrayElements(javaReturn, &copy);
        if (*returnValue == NULL) 
        { 
            LOGE("ERROR - JavaClass: An attempt to retrieve array data in method %s in %s failed.\n", methodName, classPath); 
            return false; 
        }
        return true;
    }

    bool JavaClass::staticMethod(const char* methodName, const char* param01, const char* param02)
    {
        if (!intialized || methodName == NULL)
        {
            return false;
        }
        jmethodID methodID = JNIEnvironment->GetStaticMethodID(jClass, methodName, JM(TJVoid, TJString TJString));
        if (methodID == NULL) 
        { 
            LOGE("ERROR - JavaClass: Method %s not found in %s.\n", methodName, classPath); 
            return false; 
        }
        JNIEnvironment->CallStaticVoidMethod(jClass, methodID, JNIEnvironment->NewStringUTF(param01), JNIEnvironment->NewStringUTF(param02));
        return true;
    }
}