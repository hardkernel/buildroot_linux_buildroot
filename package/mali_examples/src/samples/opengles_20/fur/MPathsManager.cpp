#include "MPathsManager.h"

MPathsManager& MPathsManager::getInstance()
  {
  static MPathsManager mgr;
  return mgr;
  }

MPathsManager::MPathsManager()
    :
    theJNIEnv(NULL),
    theAppId("")
  {
  }

MPathsManager::~MPathsManager()
  {
  theJNIEnv = NULL;
  theAppId = "";
  }

MPath MPathsManager::getFullPathStatic(
    const MPath& aFile)
  {
  return getInstance().getFullPath(aFile);
  }

bool MPathsManager::initializeStatic(
    JNIEnv* aEnv/* = NULL*/,
    const MString& aAppIdentifier/* = ""*/)
  {
  return getInstance().initialize(aEnv, aAppIdentifier);
  }

bool MPathsManager::initialize(
    JNIEnv* aEnv/* = NULL*/,
    const MString& aAppIdentifier/* = ""*/)
  {
  theJNIEnv = aEnv;
  theAppId =  aAppIdentifier;
  return true;
  }

MPath MPathsManager::getFullPath(
    const MPath& aFile)
  {
#if defined(_WIN32) || defined(__gnu_linux__)
    MPath r = "assets/";
    r += aFile;
    return r;
#elif defined(__android__)
  MPath resFileDir("/data/data/");
  resFileDir += theAppId;
  resFileDir += "/";
  //
  if (!aFile.getLength())
    return resFileDir;
  //
  MPath resFilePath(resFileDir);
  resFilePath += aFile;
  //
  if (theJNIEnv == NULL)
    {
    LOGE("[MPathsManager::getFullPath(\"%s\")] The managers is not initialized!\n", aFile.getData());
    }
  getAndroidAsset(theJNIEnv, (char*)resFileDir.getData(), (char*)resFilePath.getData(), (char*)aFile.getData());
  //
  return resFilePath;
#else
    return "NO_PLATFORM";
#endif
  }
