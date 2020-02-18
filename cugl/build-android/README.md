# CUGL (Android)

This directory contains the Android NDK project for creating the CUGL library.  CUGL
is built as a static library, which is later linked to the application. 

Installing Android Tools
------------------------
To compile the CUGL library, you will need both the Android SDK and NDK installed. The 
easiest way to install these is through Android Studio.

https://developer.android.com/studio/index.html

While Android Studio is itself a pre-beta hunk of garbage, the included SDK Manager is the
best way to install the Android SDK.  You should download the tools for Android N 
(Android 24) as well as the NDK (under SDK Tools). On OS X we recommend that you install 
in **/Library/Android/sdk**. On Windows, we recommend that you install in **C:\Android\sdk**.

To Create the CUGL Library
--------------------------
You will need the Android NDK installed, and the installation directory should be in your 
path. Navigate the command line to this directory and type the command `ndk-build`.

This command will generate the static libraries in the **obj** directory. 
Currently we only generate libraries for the `armeabi`, `arm64-v8a`, `armeabi-v7a`, and 
`x86` platforms.  If you wish to change this you should change the `APP_ABI` setting in
**jni/Application.mk**

Cleaning Up
-----------
To delete the static libraries in **obj**, simply type `ndk-build clean`.