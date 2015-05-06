/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _M_PATHS_MANAGER_HPP
#define _M_PATHS_MANAGER_HPP

#include "mCommon.h"

#include "MString.h"

#ifdef __android__
# include <jni.h>
#endif


/********************************************************************
This is a global object which we use to translate resource names from
PC format into Android format (if we're running on Android). It also
pulls things out of the bundle when requested, meaning we no longer
need a separate section of code which does all that work.

If you're on android, you'll need to configure things like the project
name and the JNIenv data pointer and so on.

PC users can laugh, stuff everything in "assets/" and it'll all be
peachy.

 ********************************************************************/

class MPathsManager
  {
public:

  // ----- Types -----

#ifndef __android__
  typedef void JNIEnv;
#endif

  // ----- Statics -----

  ///
  static MPathsManager& getInstance();

  // ----- Miscellaneous -----

  /**
   * @brief The method is a static version of the initialize method
   */
  static bool initializeStatic(JNIEnv* aEnv = NULL,
                               const MString& aAppIdentifier = "");

  /**
   * @brief The method is a static version of the getFullPath method
   */
  static MPath getFullPathStatic(const MPath& aFile);

private:

  // ----- Fields -----

  //
  JNIEnv* theJNIEnv;
  MString theAppId;


  // ----- Constructors and destructors -----

  //
  MPathsManager();

  //
  ~MPathsManager();


  // ----- Miscellaneous -----

  /**
   * @brief The method initialises resources and has to be called before any other methods within the class
   * @param aEnv JNIEnv pointer should be passed for Android platform otherwise NULL should be passed
   * @param aAppIdentifier Again aplicable only for Android platrofm and package Id should be passed
   *                       e.g. com.arm.mali.sdk.androidndk.fur, otherwise the parameter should be empty
   */
  bool initialize(JNIEnv* aEnv = NULL,
                  const MString& aAppIdentifier = "");

  /**
   * @brief The cross-platform method whic returns a full path to an asset file passed as a parameter
   * @param aFile An asset (file) name for which the full path is going to be returned
   * @return A full path to the asset.
   */
  MPath getFullPath(const MPath& aFile);


  // ----- Prevent to be used ------

  //
  MPathsManager(const MPathsManager &other);
  MPathsManager &operator=(const MPathsManager &other);

};

#endif
