/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef JAVACLASS_H
#define JAVACLASS_H

#include <jni.h>

#include <cstdio>
#include <cstdlib>

// Java method signatures
#define JM(ret,params) "(" params ")" ret
// Java types
#define TJString "Ljava/lang/String;"
#define TJInt "I"
#define TJVoid "V"
#define TJIntArr "[I"

namespace MaliSDK
{
    /**
     * \brief Wraps a Java class to allow access to it's static fields and methods using JNI.
     */
    class JavaClass
    {
    private:
        char* classPath;
        jclass jClass;
        JNIEnv* JNIEnvironment;
        bool intialized;

    public:
        /**
         * \brief Constructor taking the Java environment and the required class path.
         *
         * Checks for the existance of the class and prints an error if it can't be found.
         * \param[in] JNIEnvironment  A pointer to the JNI environment which allows interfacing with the Java Virtual Machine (JVM).
         *                            Allows extensive interaction with the JVM including accessing Java classes, fields and methods.
         *                            This pointer is provided as part of a JNI call from Java to C++.
         * \param[in] classPath The class path to the Java class to wrap.
         */
        JavaClass(JNIEnv* JNIEnvironment, const char* classPath);

        /**
         * Default destructor.
         */
        virtual ~JavaClass();

        /**
         * \brief Access a static String field of the Java class.
         * \param[in] fieldName The name of the static field to access.
         * \param[out] result Pointer to where the String value of the static field will be placed.
         * \return True if the operation was successful.
         */
        bool staticField(const char* fieldName, char** result);
        
        /**
         * \brief Access a static integer field of the Java class.
         * \param[in] fieldName The name of the static field to access.
         * \param[out] result Pointer to where the integer value of the static field will be placed.
         * \return True if the operation was successful.
         */
        bool staticField(const char* fieldName, int* result);

        /**
         * \brief Call a static method with one parameter which returns an integer array within the Java class.
         * \param[in] methodName The name of the static method to call.
         * \param[out] returnValue Pointer to an integer array where the return value of the static method will be placed.
         * \param[in] param01 A parameter that will be passed to the static method of the Java class.
         * \return True if the operation was successful.
         */
        bool staticMethod(const char* methodName, int** returnValue, const char* param01);
        
        /**
         * \brief Call a static method with two parameters which doesn't return a value within the Java class.
         * \param[in] methodName The name of the static method to call.
         * \param[in] param01 The first parameter that will be passed to the static method of the Java class.
         * \param[in] param02 The second parameter that will be passed to the static method of the Java class.       
         * \return True if the operation was successful.
         */
        bool staticMethod(const char* methodName, const char* param01, const char* param02);
    };
}
#endif /* JAVACLASS_H */