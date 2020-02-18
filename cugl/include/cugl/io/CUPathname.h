//
//  CUPathname.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for an abstract pathname.  This pathname
//  provides a cross-platform way to create directories and access files.
//  It is used by all of the other I/O classes.
//
//  Because pathnames are intended to be on the stack, we do not provide any
//  shared pointer support in this class.
//
//  This module is heavily inspired by the Java File class.  Much of the API
//  is taken from that class.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 11/22/16
//
#ifndef __CU_PATHNAME_H__
#define __CU_PATHNAME_H__
#include <string>
#include <vector>
#include <functional>

namespace cugl {

#pragma mark -
#pragma mark Pathname

/**
 * This class is abstract representation of file and directory pathnames.
 *
 * Pathname strings are system-dependent. Path representation on Unix systems
 * is very different from Window systems. This class provides a system
 * independent way of referring to pathnames. It detects the current platform
 * and converts the pathname to the correct format.
 *
 * An abstract pathname has two components.
 *
 * 1. An optional system-dependent prefix string, such as a disk-drive specifier, 
 *    "/" for the UNIX root directory, or "\\\\" for a Microsoft Windows UNC 
 *    pathname.
 *
 * 2. A sequence of zero or more string names separated by a system-dependent
 *    path separator, such as "/" for UNIX systems or "\\" for Windows.
 *
 * A pathname may either be absolute or relative.  Absolute pathnames start
 * with the optional prefix, and there is no more information necessary to
 * locate the file that it denotes. A relative pathname, on the other hand, 
 * must be interpreted as the suffix of some other pathname. In CUGL, all
 * relative names are resolve against the save directory 
 * ({@see Application#getSaveDirectory()}), as this is the only directory with
 * both read and write access on all platforms.
 *
 * When converted back to a string, Pathname objects never end in a path
 * separator, even when they represent a directory.
 *
 * Instances of Pathname are immutable. Once created, the pathname represented 
 * by a Pathname object will never change.
 *
 * IMPORTANT: Never use a Pathname object to refer to an asset in the asset
 * directory ({@see Application#getAssetDirectory()}).  This is not guaranteed
 * to be a proper directory, as the assets may be packaged together in a 
 * special read-only bundle (e.g. Android).  Attempts to navigate this bundle
 * like a directory will fail.
 */
class Pathname {
private:
    /** The short name of the path, ignoring any parent folders */
    std::string _shortname;
    /** The (potentially relative) name of the path */
    std::string _pathname;
    /** The absolute, normalized name of the path */
    std::string _fullpath;
    
#pragma mark Normalization
    /**
     * Returns the given path, normalized to the current platform
     *
     * Normalization replaces all path separators with the correct system-
     * dependent versions.  If the path is absolute, it also normalizes the
     * path prefix (e.g. capitalizing drive letters on Windows and so on).
     * However, it does not convert a relative path into an absolute one.
     *
     * @param path  The path to normalize.
     *
     * @return the given path, normalized to the current platform
     */
    static std::string normalize(const std::string& path);

    /**
     * Returns the given path, canonicalized to the current platform
     *
     * Canonicalization does everything that normalization does, plus it 
     * converts a relative path to its absolute equivalent. It replaces all
     * path separators with the correct system-dependent versions.  It also 
     * normalizes the path prefix (e.g. capitalizing drive letters on Windows 
     * and so on).
     *
     * In addition, canonicalization removes all redundant directories (e.g.
     * the directories . and ..).  However, it does not expand links or
     * shortcuts as is often the case with path canonicalization.
     *
     * @param path  The path to canonicalize.
     *
     * @return the given path, canonicalized to the current platform
     */
    static std::string canonicalize(const std::string& path);

    /**
     * Returns true if the given pathname is absolute.
     *
     * An absolute pathname starts with the optional volume prefix.
     *
     * @param path  The path to check
     *
     * @return true the given pathname is absolute.
     */
    static bool isAbsolute(const std::string& path);

    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a degenerate path name
     *
     * Attempts to use the pathname will fail, as it does not have a proper
     * absolute path.  This constructor is for internal use only.
     */
    Pathname() {}

public:
    /**
     * Creates a copy of the given pathname
     *
     * This is the classic copy constructor.  The previous pathname object
     * is unaffected.
     *
     * @param copy  The pathname to copy
     */
    Pathname(const Pathname& copy);

    /**
     * Creates a copy of the given pathname
     *
     * This is the classic move constructor.  The data from the previous 
     * pathname object is merged with this object.
     *
     * @param copy  The pathname to copy
     */
    Pathname(Pathname&& copy) noexcept;

    /** 
     * Creates a pathname for the given path
     *
     * The specified path may be absolute or relative.  Relative paths always
     * refer to the save directory ({@see Application#getSaveDirectory()}),
     * as this is the only cross-platform directory for reading and writing.
     * If you wish to refer to any other directory, you must use an absolute
     * path instead.
     *
     * @param path  The path to represent
     */
    Pathname(const std::string& path);

    /**
     * Creates a pathname for the given path
     *
     * The specified path may be absolute or relative.  Relative paths always
     * refer to the save directory ({@see Application#getSaveDirectory()}),
     * as this is the only cross-platform directory for reading and writing.
     * If you wish to refer to any other directory, you must use an absolute
     * path instead.
     *
     * @param path  The path to represent
     */
    Pathname(const char* path) : Pathname(std::string(path)) {}
    
    /**
     * Creates a pathname for the given parent and child
     *
     * The parent should refer to a directory, while the child may either be
     * a directory or a file.  The directory specified by the parent may be
     * absolute or relative.  Relative paths always refer to the save directory 
     * ({@see Application#getSaveDirectory()}), as this is the only cross-
     * platform directory for reading and writing. If you wish to refer to any 
     * other directory, you must use an absolute path instead.
     *
     * The child should have no path separators in it.  If it does, this will
     * produce an error.
     *
     * @param parent    The parent directory
     * @param child     The child file or directory
     */
    Pathname(const std::string& parent, const std::string& child);
    
    /**
     * Creates a pathname for the given parent and child
     *
     * The parent should refer to a directory, while the child may either be
     * a directory or a file.  The directory specified by the parent may be
     * absolute or relative.  Relative paths always refer to the save directory
     * ({@see Application#getSaveDirectory()}), as this is the only cross-
     * platform directory for reading and writing. If you wish to refer to any
     * other directory, you must use an absolute path instead.
     *
     * The child should have no path separators in it.  If it does, this will
     * produce an error.
     *
     * @param parent    The parent directory
     * @param child     The child file or directory
     */
    Pathname(const char* parent, const std::string& child) : Pathname(std::string(parent),child) {}
    
    /**
     * Creates a pathname for the given parent and child
     *
     * The parent should refer to a directory, while the child may either be
     * a directory or a file.  The directory specified by the parent may be
     * absolute or relative.  Relative paths always refer to the save directory
     * ({@see Application#getSaveDirectory()}), as this is the only cross-
     * platform directory for reading and writing. If you wish to refer to any
     * other directory, you must use an absolute path instead.
     *
     * The child should have no path separators in it.  If it does, this will
     * produce an error.
     *
     * @param parent    The parent directory
     * @param child     The child file or directory
     */
    Pathname(const std::string& parent, const char* child) : Pathname(parent,std::string(child)) {}
    
    /**
     * Creates a pathname for the given parent and child
     *
     * The parent should refer to a directory, while the child may either be
     * a directory or a file.  The directory specified by the parent may be
     * absolute or relative.  Relative paths always refer to the save directory
     * ({@see Application#getSaveDirectory()}), as this is the only cross-
     * platform directory for reading and writing. If you wish to refer to any
     * other directory, you must use an absolute path instead.
     *
     * The child should have no path separators in it.  If it does, this will
     * produce an error.
     *
     * @param parent    The parent directory
     * @param child     The child file or directory
     */
    Pathname(const char* parent, const char* child) : Pathname(std::string(parent),std::string(child)) {}

    /**
     * Creates a pathname for the given parent and child
     *
     * The parent pathname should refer to a directory, while the child may 
     * either be a directory or a file. The child should have no path separators 
     * in it.  If it does, this will produce an error.
     *
     * @param parent    The parent directory
     * @param child     The child file or directory
     */
    Pathname(const Pathname& parent, const std::string& child);
    
    /**
     * Creates a pathname for the given parent and child
     *
     * The parent pathname should refer to a directory, while the child may
     * either be a directory or a file. The child should have no path separators
     * in it.  If it does, this will produce an error.
     *
     * @param parent    The parent directory
     * @param child     The child file or directory
     */
    Pathname(const Pathname& parent, const char* child) : Pathname(parent,std::string(child)) {}
    
    /**
     * Deletes this pathname, releasing all resources
     */
    ~Pathname() {}

    
#pragma mark -
#pragma mark Path Types
    /**
     * Returns true if the file or directory denoted by this pathname exists.
     *
     * This value is recomputed each time the method is called.  If the file
     * is created later (such as with {@link createFile}, it will return true.
     *
     * @return true if the file or directory denoted by this pathname exists.
     */
    bool exists() const;
    
    /**
     * Returns true if the file denoted by this pathname is a directory.
     * 
     * This value will return false is the file does not exist. This value is 
     * recomputed each time the method is called.  If the file is created later 
     * (such as with {@link createDirectory}, it will return true.
     *
     * @return true if the file denoted by this pathname is a directory.
     */
    bool isDirectory() const;
    
    /**
     * Returns true if the file denoted by this pathname is a normal file.
     *
     * This value will return false is the file does not exist. This value is
     * recomputed each time the method is called.  If the file is created later
     * (such as with {@link createFile}, it will return true.
     *
     * @return true if the file denoted by this pathname is a normal file.
     */
    bool isFile() const;

    /**
     * Returns true if the file named by this pathname is a hidden file.
     *
     * This value will return false is the file does not exist. This value is
     * recomputed each time the method is called.  If the file is created later
     * (such as with {@link createFile}, it will return true.
     *
     * @return true if the file named by this pathname is a hidden file.
     */
    bool isHidden() const { return exists() && _shortname[0] == '.'; }
    
    
#pragma mark -
#pragma mark Path Names
    /**
     * Returns true if this pathname is absolute.
     *
     * @return true if this pathname is absolute.
     */
    bool isAbsolute() const { return _pathname == _fullpath; }
    
    /**
     * Returns the name of the file or directory denoted by this pathname.
     *
     * The value returned is the short name, without any parent directories.
     *
     * @return the name of the file or directory denoted by this pathname.
     */
    std::string getName() const { return _shortname; }

    /**
     * Returns the pathname string for this pathname.
     *
     * This value is the normalized version of the pathname provided to the
     * constructor.  Path separators have been replaced with the correct
     * version for the platform.  However, the path is relative is the original
     * value was relative.
     *
     * @return the pathname string for this pathname.
     */
    std::string getPathname() const { return _pathname; }

    /**
     * Returns the pathname string of this pathname's parent
     *
     * The parent is extracted from the absolute path, which allows us to 
     * specify a specific parent even in the case of a relative path.  This
     * method returns the empty string if this pathname does not have a
     * parent directory.
     *
     * @return the pathname string of this pathname's parent
     */
    std::string getParentName() const;
    
    /**
     * Returns the pathname of this pathname's parent
     *
     * The parent pathname is computed from the absolute path, which allows us
     * to specify a specific parent even in the case of a relative path.  This
     * method returns the volume root if this pathname does not have a parent 
     * directory. 
     *
     * @return the pathname of this pathname's parent
     */
    Pathname getParentPath() const;
    
    /**
     * Returns the absolute pathname string of this pathname.
     *
     * The absolute path is fixed, as the only relative pathnames that we 
     * allow are in the save directory ({@see Application#getSaveDirectory()}).
     *
     * @return the absolute pathname string of this pathname.
     */
    std::string getAbsoluteName() const { return _fullpath; }

    /**
     * Returns the absolute form of this pathname.
     *
     * The absolute path is fixed, as the only relative pathnames that we
     * allow are in the save directory ({@see Application#getSaveDirectory()}).
     *
     * @return the absolute form of this pathname.
     */
    Pathname getAbsolutePath() const;
    
    /** 
     * Returns the suffix for the leaf file of this path.
     *
     * A suffix is any part of the file name after a final period. If there
     * is no suffix, this method returns the empty string.
     *
     * @return the suffix for the leaf file of this path.
     */
    std::string getSuffix() const;
    
    
#pragma mark -
#pragma mark Path Hierarchy
    /**
     * Returns the volume prefix for this path.
     *
     * This method will return a value even if the path is a relative one.
     *
     * @return the volume prefix for this path.
     */
    std::string getVolume() const;

    /**
     * Returns the system-dependent path separator for this pathname.
     *
     * Path separators are the same across all files.  Hence this method can
     * be called staticly.
     *
     * @return the system-dependent path separator for this pathname.
     */
    static std::string getSeparator();

    /**
     * Returns the hierarchical components of this pathname.
     *
     * The result vector will not contain the volume.  However it will contain
     * every intermediate directory and the leaf child (file or directory).
     * Together with the volume and path separator, this can be use to 
     * reconstruct the absolute pathname.
     *
     * @return the hierarchical components of this pathname.
     */
    std::vector<std::string> getComponents() const;
    
    /**
     * Returns a list of strings naming the files and directories in the pathname
     *
     * This method assumes that this pathname denotes a directory. If it does
     * not, the list will be empty.
     *
     * @return a list of strings naming the files and directories in the pathname
     */
    std::vector<std::string> list() const;
    
    /**
     * Returns a filtered list of strings naming the files and directories in the pathname
     *
     * This method assumes that this pathname denotes a directory. If it does
     * not, the list will be empty.
     *
     * The filter will only be given the shortname of each file.  It will not
     * have access to the full path.
     *
     * @return a filtered list of strings naming the files and directories in the pathname
     */
    std::vector<std::string> list(const std::function<bool(const std::string& file)>& filter) const;

    /**
     * Returns a list of pathnames for the files and directories in the pathname
     *
     * This method assumes that this pathname denotes a directory. If it does
     * not, the list will be empty.
     *
     * @return a list of pathnames for the files and directories in the pathname
     */
    std::vector<Pathname> listPaths() const;
    
    /**
     * Returns a filtered list of pathnames for the files and directories in the pathname
     *
     * This method assumes that this pathname denotes a directory. If it does
     * not, the list will be empty.
     *
     * @return a filtered list of pathnames for the files and directories in the pathname
     */
    std::vector<Pathname> listPaths(const std::function<bool(const Pathname& path)>& filter) const;
    

#pragma mark -
#pragma mark Path Creation
    /**
     * Creates a new, empty file named by this pathname
     *
     * This method succeeds if and only if a file with this name does not yet 
     * exist.  The file will be an empty, regular file.
     *
     * @return true if the file was successfully created.
     */
    bool createFile();

    /**
     * Deletes the file denoted by this pathname.
     *
     * This method will fail if the file is not a regular file or if it does
     * not yet exist.
     *
     * @return true if the file was successfully deleted.
     */
    bool deleteFile();

    /**
     * Creates the directory named by this pathname.
     *
     * This method succeeds if and only if a file or directory with this name 
     * does not yet exist.
     *
     * @return true if the directory was successfully created.
     */
    bool createDirectory();
    
    /**
     * Deletes the directory denoted by this pathname.
     *
     * This method will fail if the file is not a directory or if it does not
     * yet exist.
     *
     * @return true if the directory was successfully deleted.
     */
    bool deleteDirectory();
    
    /**
     * Creates the directory for this pathname, including any necessary parent directories.
     *
     * This method is similar to {@link createDirectory} except that it also
     * creates any missing parent directories.  It may fail if it does not have
     * the appropriate permissions to create a parent.  Even if it fails, it 
     * may still successfully create some of the parents.
     *
     * @return true if the path was successfully created.
     */
    bool createPath();
    
    /**
     * Renames the file denoted by this pathname.
     *
     * This contents of this pathname will not be changed.  Instead, the file 
     * for this pathname will no longer exist. The file will now be referred
     * to via the provided pathname.
     *
     * This method will fail if there is no file for this pathname.
     *
     * @param path  The new pathname for this file.
     *
     * @return true if the file was successfully renamed.
     */
    bool renameTo(const std::string& path)  {
        return renameTo(Pathname(path));
    }

    /**
     * Renames the file denoted by this pathname.
     *
     * This contents of this pathname will not be changed.  Instead, the file
     * for this pathname will no longer exist. The file will now be referred
     * to via the provided pathname.
     *
     * This method will fail if there is no file for this pathname.
     *
     * @param path  The new pathname for this file.
     *
     * @return true if the file was successfully renamed.
     */
    bool renameTo(const char* path) {
        return renameTo(Pathname(path));
    }

    /**
     * Renames the file denoted by this pathname.
     *
     * This contents of this pathname will not be changed.  Instead, the file
     * for this pathname will no longer exist. The file will now be referred
     * to via the provided pathname.
     *
     * This method will fail if there is no file for this pathname.
     *
     * @param dest  The new pathname for this file.
     *
     * @return true if the file was successfully renamed.
     */
    bool renameTo(const Pathname& dest);
    
    
#pragma mark -
#pragma mark Path Access
    /**
     * Returns true if the application can read the file for this pathname.
     *
     * This method uses the access() POSIX method.  Therefore, it does not assume
     * that this application is the file owner, and correctly determines the
     * file access.
     *
     * @return true if the application can read the file for this pathname.
     */
    bool canRead();

    /**
     * Returns true if the application can execute the file for this pathname.
     *
     * Note that the only form of file execution supported by CUGL is searching
     * a directory.
     *
     * This method uses the access() POSIX method.  Therefore, it does not assume
     * that this application is the file owner, and correctly determines the
     * file access.
     *
     * @return true if the application can execute the file for this pathname.
     */
    bool canSearch();

    /**
     * Returns true if the application can modify the file for this pathname.
     *
     * This method uses the access() POSIX method.  Therefore, it does not assume
     * that this application is the file owner, and correctly determines the
     * file access.
     *
     * @return true if the application can modify the file for this pathname.
     */
    bool canWrite();

    /**
     * Sets the owner's read permission for this pathname.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @param readable  Whether the owner may read this file
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setReadable(bool readable);
    
    /**
     * Sets the owner's or everybody's read permission for this pathname.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @param readable  Whether this file may be read
     * @param ownerOnly Whether to apply this change only to the owner
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setReadable(bool readable, bool ownerOnly);
    
    /**
     * Marks this file or directory so that only read operations are allowed.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setReadOnly();

    /**
     * Sets the owner's execution permission for this pathname.
     *
     * Note that the only form of file execution supported by CUGL is searching
     * a directory.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @param searchable    Whether the owner may execute (search) this file
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setSearchable(bool searchable);
    
    /**
     * Sets the owner's or everybody's execution permission for this pathname.
     *
     * Note that the only form of file execution supported by CUGL is searching
     * a directory.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @param searchable    Whether this file may be excuted
     * @param ownerOnly     Whether to apply this change only to the owner
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setSearchable(bool searchable, bool ownerOnly);

    /**
     * Sets the owner's write permission for this pathname.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @param writable  Whether the owner may write to this file
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setWritable(bool writable);
    
    /**
     * Sets the owner's or everybody's write permission for this pathname.
     *
     * The owner may or may not be this application.  The method will return
     * false if the application does not have permission for this change.
     *
     * @param writable  Whether this file may be written to
     * @param ownerOnly Whether to apply this change only to the owner
     *
     * @return true if the file permissions where successfully changed.
     */
    bool setWritable(bool writable, bool ownerOnly);
    

    
#pragma mark -
#pragma mark Path Size
    /**
     * Returns the length of the file denoted by this pathname.
     *
     * The value is measured in bytes.
     *
     * @return the length of the file denoted by this pathname.
     */
    size_t length() const;

    /**
     * Returns the time that the file for this pathname was last modified.
     *
     * The value is in seconds since the last epoch (e.g. January 1, 1970 on 
     * Unix systems)
     *
     * @return the time that the file for this pathname was last modified.
     */
    Uint64 lastModified() const;
    
    /**
     * Returns the number of unallocated bytes in the partition for this pathname.
     *
     * The value is for the partition containing the given file or directory.
     * Just because space is free does not mean that it is available.  The
     * space may be restricted to priviledged users.
     *
     * @return the number of unallocated bytes in the partition for this pathname.
     */
    size_t getFreeSpace() const;

    /**
     * Returns the number of available bytes in the partition for this pathname.
     *
     * The value is for the partition containing the given file or directory.
     * This method is similar to {@link getFreeSpace} except that it measures
     * the number of bytes available for unpriviledged users.
     *
     * @return the number of available bytes in the partition for this pathname.
     */
    size_t getAvailableSpace() const;

    /**
     * Returns the size of the partition named by this pathname.
     *
     * The value is for the partition containing the given file or directory.
     *
     * @return the size of the partition named by this pathname.
     */
    size_t getTotalSpace() const;

    
#pragma mark -
#pragma mark Path Operators
    /**
     * Returns true if this path is less than the given path.
     *
     * This comparison uses lexicographical of the absolute path names.
     *
     * @param other The pathname to compare against.
     *
     * @return True if this path is less than the given path.
     */
    bool operator<(const Pathname& other) const {
        return getAbsoluteName() < other.getAbsoluteName();
    }
    
    /**
     * Returns true if this path is less than or equal to the given path.
     *
     * This comparison uses lexicographical of the absolute path names.
     *
     * @param other The pathname to compare against.
     *
     * @return True if this path is less than or equal to the given path.
     */
    bool operator<=(const Pathname& other) const {
        return getAbsoluteName() <= other.getAbsoluteName();
    }
    
    /**
     * Returns true if this path is greater than the given path.
     *
     * This comparison uses lexicographical of the absolute path names.
     *
     * @param other The pathname to compare against.
     *
     * @return True if this path is greater than the given path.
     */
    bool operator>(const Pathname& other) const {
        return getAbsoluteName() > other.getAbsoluteName();
    }

    /**
     * Returns true if this path is greater than or equal to the given path.
     *
     * This comparison uses lexicographical of the absolute path names.
     *
     * @param other The pathname to compare against.
     *
     * @return True if this path is greater than or equal to the given path.
     */
    bool operator>=(const Pathname& other) const {
        return getAbsoluteName() >= other.getAbsoluteName();
    }

    /**
     * Returns true if this path is equal to the given path.
     *
     * This comparison uses lexicographical of the absolute path names.
     *
     * @param other The pathname to compare against.
     *
     * @return True if this path is equal to the given path.
     */
    bool operator==(const Pathname& other) const {
        return getAbsoluteName() == other.getAbsoluteName();
    }

    /**
     * Returns true if this path is not equal to the given path.
     *
     * This comparison uses lexicographical of the absolute path names.
     *
     * @param other The pathname to compare against.
     *
     * @return True if this path is not equal to the given path.
     */
    bool operator!=(const Pathname& other) const{
        return getAbsoluteName() != other.getAbsoluteName();
    }
    
};

}
#endif /* __CU_PATHNAME_H__ */
