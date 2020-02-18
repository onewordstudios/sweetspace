//
//  CUPathname.cpp
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
#include <cugl/base/CUBase.h>
#include <stdio.h>
#include <sys/stat.h>

#if defined (__ANDROID__) 
	#include <sys/vfs.h> 
 	#define statvfs statfs 
 	#define fstatvfs fstatfs 
	#include <unistd.h>
	#include <dirent.h>
#elif defined (__WINDOWS__)
	#include <io.h>  
	#define WINDOWS_TICK 10000000
	#define SEC_TO_UNIX_EPOCH 11644473600LL
#else
	#include <sys/statvfs.h> 
	#include <unistd.h>
	#include <dirent.h>
#endif 
#include <sstream>
#include <SDL/SDL.h>
#include <cugl/io/CUPathname.h>
#include <cugl/base/CUApplication.h>

using namespace cugl;

/** System-dependent path separators */
#if defined (__WINDOWS__)
    #define PATH_SEP '\\'
#else
    #define PATH_SEP '/'
#endif

#pragma mark -
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
std::string Pathname::normalize(const std::string& path) {
    std::string result;
#if defined (__WINDOWS__)
    // Take care of the prefix
    if (path.size() > 0 && path[0] == '/') {
        result = Application::get()->getSaveDirectory().substr(0,2);
        if (result[1] == ':') {
            result[0] = toupper(result[0]);
            result += '\\';
        }
        result += path.substr(1);
    } else {
        result = path;
    }
    
    // Replace the path separators.
    for(auto it = result.begin(); it != result.end(); ++it) {
        if (*it == '/') {
            *it = '\\';
        }
    }
#else
    // Take care of the prefix
    if (path.size() > 1 && (path[1] == ':' || path[0] == '\\')) {
        result = "/";
        int pos = path.size() > 2 && path[2] == '\\' ? 3 : 2;
        result += path.substr(pos);
    } else {
        result = path;
    }

    // Replace the path separators.
    for(auto it = result.begin(); it != result.end(); ++it) {
        if (*it == '\\') {
            *it = '/';
        }
    }
#endif
    // Remove any trailing separators
    if (result[result.size()-1] == PATH_SEP) {
        result.erase(result.end()-1);
    }
    return result;
}

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
std::string Pathname::canonicalize(const std::string& path) {
    std::string result = normalize(path);
    if (!isAbsolute(path)) {
        result = Application::get()->getSaveDirectory()+result;
    }
    
    // Now break it up
#if defined (__WINDOWS__)
    std::string::size_type curr = result[1] == ':' ? 3 : 2;
#else
    std::string::size_type curr = 1;
#endif
    
    // Compute the components.
    std::string volume = result.substr(0,curr);
    std::vector<std::string> components;
    while (curr != std::string::npos) {
        std::string::size_type next = result.find(PATH_SEP,curr);
        if (next == std::string::npos) {
            components.push_back(result.substr(curr));
        } else {
            components.push_back(result.substr(curr,next-curr));
        }
        curr = next == std::string::npos ? next : next+1;
    }
    
    // Handle the redundancies
    std::vector<std::string> canonical;
    for(auto it = components.begin(); it != components.end(); ++it) {
        if (*it == "..") {
            CUAssertLog(!canonical.empty(),"Error while canonicalizing pathname");
            canonical.erase(canonical.end()-1);
        } else if (*it != ".") {
            canonical.push_back(*it);
        }
    }
    
    // Build the final path
    result = volume;
    for(auto it = canonical.begin(); it != canonical.end(); ++it) {
        result.append(*it+PATH_SEP);
    }
    result.erase(result.end()-1);
    return result;
}

/**
 * Returns true if the given pathname is absolute.
 *
 * An absolute pathname starts with the optional volume prefix.
 *
 * @param path  The path to check
 *
 * @return true the given pathname is absolute.
 */
bool Pathname::isAbsolute(const std::string& path) {
#if defined (__WINDOWS__)
    return path.size() > 1 && (path[1] == ':' || path[0] == '\\');
#else
    return path.size() > 0 && path[0] == '/';
#endif
}

#pragma mark -
#pragma mark Constructors
/**
 * Creates a copy of the given pathname
 *
 * This is the classic copy constructor.  The previous pathname object
 * is unaffected.
 *
 * @param copy  The pathname to copy
 */
Pathname::Pathname(const Pathname& copy) :
_pathname(copy._pathname),
_fullpath(copy._fullpath),
_shortname(copy._shortname)
{
}

/**
 * Creates a copy of the given pathname
 *
 * This is the classic move constructor.  The data from the previous
 * pathname object is merged with this object.
 *
 * @param copy  The pathname to copy
 */
Pathname::Pathname(Pathname&& copy) noexcept :
_pathname(std::move(copy._pathname)),
_fullpath(std::move(copy._pathname)),
_shortname(std::move(copy._shortname))
{
}

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
Pathname::Pathname(const std::string& path) {
    _pathname = normalize(path);
    _fullpath = canonicalize(path);
    // This should never fail because of canonicalize
    std::string::size_type n = _fullpath.rfind(PATH_SEP);
    _shortname = _fullpath.substr(n+1);
}

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
Pathname::Pathname(const std::string& parent, const std::string& child) {
    CUAssertLog(child.find(PATH_SEP) == std::string::npos,
                "Child %s contains a path separator",child.c_str());
    
    _pathname = normalize(parent)+PATH_SEP+child;
    _fullpath = canonicalize(parent)+PATH_SEP+child;
    _shortname = child;
}

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
Pathname::Pathname(const Pathname& parent, const std::string& child) {
    CUAssertLog(child.find(PATH_SEP) == std::string::npos,
                "Child %s contains a path separator",child.c_str());
    
    _pathname = parent.getPathname()+PATH_SEP+child;
    _fullpath = parent.getAbsoluteName()+PATH_SEP+child;
    _shortname = child;
}


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
bool Pathname::exists() const {
    struct stat status;
    return stat(_fullpath.c_str(), &status) == 0;
}

/**
 * Returns true if the file denoted by this pathname is a directory.
 *
 * This value will return false is the file does not exist. This value is
 * recomputed each time the method is called.  If the file is created later
 * (such as with {@link createDirectory}, it will return true.
 *
 * @return true if the file denoted by this pathname is a directory.
 */
bool Pathname::isDirectory() const {
#if defined (__WINDOWS__)
	DWORD ftyp = GetFileAttributesA(_fullpath.c_str());
	return !(ftyp == INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    return (err == 0 && S_ISDIR(status.st_mode));
#endif
}

/**
 * Returns true if the file denoted by this pathname is a normal file.
 *
 * This value will return false is the file does not exist. This value is
 * recomputed each time the method is called.  If the file is created later
 * (such as with {@link createFile}, it will return true.
 *
 * @return true if the file denoted by this pathname is a normal file.
 */
bool Pathname::isFile() const {
#if defined (__WINDOWS__)
	DWORD ftyp = GetFileAttributesA(_fullpath.c_str());
	return !(ftyp == INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_NORMAL);
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    return (err == 0 && S_ISREG(status.st_mode));
#endif
}


#pragma mark -
#pragma mark Path Names
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
std::string Pathname::getParentName() const {
    // Get prefix including separator
    std::string parent = _fullpath.substr(0,_fullpath.size()-_shortname.size());
    std::string::size_type n = parent.rfind(PATH_SEP,parent.size()-2);
#if defined (__WINDOWS__)
    if (n != std::string::npos && n > 0)
#else
    if (n != std::string::npos)
#endif
    {
        parent = parent.substr(n+1);
        parent.erase(parent.end()-1);
    }
    
    return parent;
}

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
Pathname Pathname::getParentPath() const {
    Pathname copy;
    copy._fullpath = getParentName();
    copy._pathname = copy._fullpath;

    // This should never fail because of canonicalize
    std::string::size_type n = copy._fullpath.rfind(PATH_SEP);
    copy._shortname = copy._fullpath.substr(n+1);

    return copy;
}

/**
 * Returns the absolute form of this pathname.
 *
 * The absolute path is fixed, as the only relative pathnames that we
 * allow are in the save directory ({@see Application#getSaveDirectory()}).
 *
 * @return the absolute form of this pathname.
 */
Pathname Pathname::getAbsolutePath() const {
    Pathname copy(*this);
    copy._pathname = _fullpath;
    return copy;
}

/**
 * Returns the suffix for the leaf file of this path.
 *
 * A suffix is any part of the file name after a final period. If there
 * is no suffix, this method returns the empty string.
 *
 * @return the suffix for the leaf file of this path.
 */
std::string Pathname::getSuffix() const {
    std::string::size_type a = _fullpath.rfind('.');
    std::string::size_type b = _fullpath.rfind(PATH_SEP);
    if (a != std::string::npos && b < a) {
        return _fullpath.substr(a+1);
    }
    return std::string();
}

#pragma mark -
#pragma mark Path Hierarchy
/**
 * Returns the volume prefix for this path.
 *
 * This method will return a value even if the path is a relative one.
 *
 * @return the volume prefix for this path.
 */
std::string Pathname::getVolume() const {
#if defined (__WINDOWS__)
    std::string::size_type curr = _fullpath[1] == ':' ? 3 : 2;
#else
    std::string::size_type curr = 1;
#endif
    return _fullpath.substr(0,curr);
}

/**
 * Returns the system-dependent path separator for this pathname.
 *
 * Path separators are the same across all files.  Hence this method can
 * be called staticly.
 *
 * @return the system-dependent path separator for this pathname.
 */
std::string Pathname::getSeparator() {
    std::string result;
    result += PATH_SEP;
    return result;
}

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
std::vector<std::string> Pathname::getComponents() const {
#if defined (__WINDOWS__)
    std::string::size_type curr = _fullpath[1] == ':' ? 3 : 2;
#else
    std::string::size_type curr = 1;
#endif
    
    std::vector<std::string> result;
    while (curr != std::string::npos) {
        std::string::size_type next = _fullpath.find(PATH_SEP,curr);
        if (next == std::string::npos) {
            result.push_back(_fullpath.substr(curr));
        } else {
            result.push_back(_fullpath.substr(curr,next-curr));
        }
        curr = next == std::string::npos ? next : next+1;
    }
    return result;
}

/**
 * Returns a list of strings naming the files and directories in the pathname
 *
 * This method assumes that this pathname denotes a directory. If it does
 * not, the list will be empty.
 *
 * @return a list of strings naming the files and directories in the pathname
 */
std::vector<std::string> Pathname::list() const {
#if defined (__WINDOWS__)
	std::vector<std::string> result;

	std::string search_path(_fullpath);
	WIN32_FIND_DATA search_data;
	if (search_path[search_path.size() - 1] != '\\') {
		search_path.append("\\");
	}

	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile((search_path+"*").c_str(), &search_data);

	bool avail = true;
	while (avail && handle != INVALID_HANDLE_VALUE) {
		std::string file(search_data.cFileName);
		if (file != "." && file != "..") {
			result.push_back(file);
		}
		avail = (FindNextFile(handle, &search_data) != FALSE);
	}

	//Close the handle after use or memory/resource leak
	FindClose(handle);
	return result;
#else
	std::vector<std::string> result;
	std::string sep = "";
	if (_fullpath[_fullpath.size() - 1] != '/') {
		sep = "/";
	}
    // This is POSIX.
    DIR *directory = opendir(_fullpath.c_str());
    
    struct dirent *contents;
    struct stat status;
    if (directory != nullptr) {
        contents = readdir(directory);
        while (contents != nullptr) {
            std::string item = contents->d_name;
            int err = stat((_fullpath+sep+item).c_str(), &status);
            if (err == 0 && item != "." && item != "..") {
                result.push_back(item);
            }
            contents = readdir(directory);
        }
        closedir (directory);
    }

    return result;
#endif
}

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
std::vector<std::string> Pathname::list(const std::function<bool(const std::string& file)>& filter) const {
#if defined (__WINDOWS__)
	std::vector<std::string> result;

	std::string search_path(_fullpath);
	WIN32_FIND_DATA search_data;
	if (search_path[search_path.size() - 1] != '\\') {
		search_path.append("\\");
	}

	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile((search_path+"*").c_str(), &search_data);

	bool avail = true;
	while (avail && handle != INVALID_HANDLE_VALUE) {
		std::string file(search_data.cFileName);
		if (filter(file) && file != "." && file != "..") {
			result.push_back(file);
		}
		avail = (FindNextFile(handle, &search_data) != FALSE);
	}

	//Close the handle after use or memory/resource leak
	FindClose(handle);
	return result;
#else
	std::vector<std::string> result;
	std::string sep = "";
	if (_fullpath[_fullpath.size() - 1] != '/') {
		sep = "/";
	}

    // This is POSIX.  We HOPE this works on Windows.
    DIR *directory = opendir(_fullpath.c_str());
    
    struct dirent *contents;
    struct stat status;
    if (directory != nullptr) {
        contents = readdir(directory);
        while (contents != nullptr) {
            std::string item = contents->d_name;
            if (filter(item)) {
                int err = stat((_fullpath+sep+item).c_str(), &status);
                if (err == 0 && item != "." && item != "..") {
                    result.push_back(item);
                }
            }
            contents = readdir(directory);
        }
        closedir (directory);
    }
    
    return result;
#endif
}

/**
 * Returns a list of pathnames for the files and directories in the pathname
 *
 * This method assumes that this pathname denotes a directory. If it does
 * not, the list will be empty.
 *
 * @return a list of pathnames for the files and directories in the pathname
 */
std::vector<Pathname> Pathname::listPaths() const {
#if defined (__WINDOWS__)
	std::vector<Pathname> result;

	std::string search_path(_fullpath);
	WIN32_FIND_DATA search_data;
	if (search_path[search_path.size() - 1] != '\\') {
		search_path.append("\\");
	}

	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile((search_path + "*").c_str(), &search_data);

	bool avail = true;
	while (avail && handle != INVALID_HANDLE_VALUE) {
		std::string file(search_data.cFileName);
		if (file != "." && file != "..") {
			result.push_back(Pathname(search_path + file));
		}
		avail = (FindNextFile(handle, &search_data) != FALSE);
	}

	//Close the handle after use or memory/resource leak
	FindClose(handle);
	return result;
#else
	std::vector<Pathname> result;
	std::string sep = "";
	if (_fullpath[_fullpath.size() - 1] != '/') {
		sep = "/";
	}

	// This is POSIX.  We HOPE this works on Windows.
	DIR *directory = opendir(_fullpath.c_str());

	struct dirent *contents;
	struct stat status;
	if (directory != nullptr) {
		contents = readdir(directory);
		while (contents != nullptr) {
			std::string item = contents->d_name;
			int err = stat((_fullpath + sep + item).c_str(), &status);
			if (err == 0 && item != "." && item != "..") {
				result.push_back(Pathname(_fullpath + sep + item));
			}
			contents = readdir(directory);
		}
		closedir(directory);
	}

	return result;
#endif
}

/**
 * Returns a filtered list of pathnames for the files and directories in the pathname
 *
 * This method assumes that this pathname denotes a directory. If it does
 * not, the list will be empty.
 *
 * @return a filtered list of pathnames for the files and directories in the pathname
 */
std::vector<Pathname> Pathname::listPaths(const std::function<bool(const Pathname& path)>& filter) const {
#if defined (__WINDOWS__)
	std::vector<Pathname> result;

	std::string search_path(_fullpath);
	WIN32_FIND_DATA search_data;
	if (search_path[search_path.size() - 1] != '\\') {
		search_path.append("\\");
	}

	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile((search_path + "*").c_str(), &search_data);

	bool avail = true;
	while (avail && handle != INVALID_HANDLE_VALUE) {
		std::string file(search_data.cFileName);
		if (file != "." && file != "..") {
			Pathname path(search_path + file);
			if (filter(path)) {
				result.push_back(path);
			}
		}
		avail = (FindNextFile(handle, &search_data) != FALSE);
	}

	//Close the handle after use or memory/resource leak
	FindClose(handle);
	return result;
#else
	std::vector<Pathname> result;
	std::string sep = "";
	if (_fullpath[_fullpath.size() - 1] != '/') {
		sep = "/";
	}

    // This is POSIX.  We HOPE this works on Windows.
    DIR *directory = opendir(_fullpath.c_str());
    
    struct dirent *contents;
    struct stat status;
    if (directory != nullptr) {
        contents = readdir(directory);
        while (contents != nullptr) {
            std::string item = contents->d_name;
            int err = stat((_fullpath+sep+item).c_str(), &status);
            if (err == 0 && item != "." && item != "..") {
                Pathname found(_fullpath+sep+item);
                if (filter(found)) {
                    result.push_back(found);
                }
            }
            contents = readdir(directory);
        }
        closedir (directory);
    }
    
    return result;
#endif
}


#pragma mark -
#pragma mark Path Creation
/**
 * Creates a new, empty file named by this abstract pathname
 *
 * This method succeeds if and only if a file with this name does not yet
 * exist.  The file will be an empty, regular file.
 *
 * @return true if the file was successfully created.
 */
bool Pathname::createFile()  {
    if (!exists()) {
        SDL_RWops* stream = SDL_RWFromFile(_fullpath.c_str(), "w");
        if (stream) {
            SDL_RWclose(stream);
            return true;
        }
    }
    return false;
}

/**
 * Deletes the file denoted by this abstract pathname.
 *
 * This method will fail if the file is not a regular file or if it does
 * not yet exist.
 *
 * @return true if the file was successfully deleted.
 */
bool Pathname::deleteFile()  {
    if (isFile()) {
        return std::remove(_fullpath.c_str()) == 0;
    }
    return false;
}

/**
 * Creates the directory named by this pathname.
 *
 * This method succeeds if and only if a file or directory with this name
 * does not yet exist.
 *
 * @return true if the directory was successfully created.
 */
bool Pathname::createDirectory() {
#if defined (__WINDOWS__)
	CreateDirectory(_fullpath.c_str(), NULL);
	return ERROR_ALREADY_EXISTS != GetLastError();
#else
    if (!exists()) {
        return mkdir(_fullpath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
    }
    return false;
#endif
}

/**
 * Deletes the directory denoted by this pathname.
 *
 * This method will fail if the file is not a directory or if it does not
 * yet exist.
 *
 * @return true if the directory was successfully deleted.
 */
bool Pathname::deleteDirectory() {
    if (isDirectory()) {
        return std::remove(_fullpath.c_str()) == 0;
    }
    return false;
}

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
bool Pathname::createPath() {
#if defined (__WINDOWS__)
	std::vector<std::string> components = getComponents();
	std::string testpath = getVolume();
	testpath.erase(testpath.end() - 1);

	// Travel down the hierarchy
	struct stat status;
	for (auto it = components.begin(); it != components.end(); ++it) {
		testpath += PATH_SEP + *it;
		int err = stat(testpath.c_str(), &status);
		if (err == 0) {
			return false;
		} else if (err != 0) {
			CreateDirectory(_fullpath.c_str(), NULL);
			if (ERROR_ALREADY_EXISTS == GetLastError()) {
				return false;
			}
		} else {
			DWORD ftyp = GetFileAttributesA(_fullpath.c_str());
			if ((ftyp == INVALID_FILE_ATTRIBUTES) || !(ftyp & FILE_ATTRIBUTE_DIRECTORY)) {
				return false;
			}
		}
	}

	return true;
#else
    std::vector<std::string> components = getComponents();
    std::string testpath = getVolume();
    testpath.erase(testpath.end()-1);
    
    // Travel down the hierarchy
    struct stat status;
    for(auto it = components.begin(); it != components.end(); ++it) {
        testpath += PATH_SEP+*it;
        int err = stat(testpath.c_str(), &status);
        if (err == 0 && !S_ISDIR(status.st_mode)) {
            return false;
        } else if (err != 0) {
            int make = mkdir(testpath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            if (make != 0) {
                return false;
            }
        }
    }
    
    return true;
#endif
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
bool Pathname::renameTo(const Pathname& dest)  {
    if (exists()) {
        return std::rename(_fullpath.c_str(),dest._fullpath.c_str()) == 0;
    }
    return false;
}


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
bool Pathname::canRead() {
#if defined (__WINDOWS__)
	bool ronly = _access(_fullpath.c_str(), 4) != -1;
	bool rdwrt = _access(_fullpath.c_str(), 6) != -1;
	return ronly || rdwrt;
#else
    return access(_fullpath.c_str(), R_OK);
#endif
}

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
bool Pathname::canSearch() {
#if defined (__WINDOWS__)
	bool ronly = _access(_fullpath.c_str(), 4) != -1;
	bool rdwrt = _access(_fullpath.c_str(), 6) != -1;
	return ronly || rdwrt;
#else
	return access(_fullpath.c_str(), X_OK);
#endif
}

/**
 * Returns true if the application can modify the file for this pathname.
 *
 * This method uses the access() POSIX method.  Therefore, it does not assume
 * that this application is the file owner, and correctly determines the
 * file access.
 *
 * @return true if the application can modify the file for this pathname.
 */
bool Pathname::canWrite() {
#if defined (__WINDOWS__)
	bool wonly = _access(_fullpath.c_str(), 2) != -1;
	bool rdwrt = _access(_fullpath.c_str(), 6) != -1;
	return wonly || rdwrt;
#else
	return access(_fullpath.c_str(), W_OK);
#endif
}

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
bool Pathname::setReadable(bool readable) {
#if defined (__WINDOWS__)
	CUAssertLog(false, "Windows does not support this functionality");
	return false;
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        if (readable) {
            status.st_mode = status.st_mode | S_IRUSR;
        } else {
            status.st_mode = status.st_mode & ~S_IRUSR;
        }
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}

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
bool Pathname::setReadable(bool readable, bool ownerOnly) {
    if (ownerOnly) {
        return setReadable(readable);
    }
    
#if defined (__WINDOWS__)
	CUAssertLog(false, "Windows does not support this functionality");
	return false;
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        if (readable) {
            status.st_mode = status.st_mode | S_IRUSR | S_IRGRP | S_IROTH;
        } else {
            status.st_mode = status.st_mode & ~S_IRUSR & ~S_IRGRP & ~S_IROTH;
        }
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}

/**
 * Marks this file or directory so that only read operations are allowed.
 *
 * The owner may or may not be this application.  The method will return
 * false if the application does not have permission for this change.
 *
 * @return true if the file permissions where successfully changed.
 */
bool Pathname::setReadOnly() {
#if defined (__WINDOWS__)
	CUAssertLog(false, "Windows does not support this functionality");
	return false;
#else
	struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        status.st_mode = status.st_mode & ~S_IWUSR & ~S_IWGRP & ~S_IWOTH;
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}

/**
 * Sets the owner's execution permission for this pathname.
 *
 * Note that the only form of file execution supported by CUGL is searching
 * a directory.
 *
 * The owner may or may not be this application.  The method will return
 * false if the application does not have permission for this change.
 *
 * @param readable  Whether the owner may execute (search) this file
 *
 * @return true if the file permissions where successfully changed.
 */
bool Pathname::setSearchable(bool searchable) {
#if defined (__WINDOWS__)
	CUAssertLog(false, "Windows does not support this functionality");
	return false;
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        if (searchable) {
            status.st_mode = status.st_mode | S_IXUSR;
        } else {
            status.st_mode = status.st_mode & ~S_IXUSR;
        }
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}

/**
 * Sets the owner's or everybody's execution permission for this pathname.
 *
 * Note that the only form of file execution supported by CUGL is searching
 * a directory.
 *
 * The owner may or may not be this application.  The method will return
 * false if the application does not have permission for this change.
 *
 * @param readable  Whether this file may be excuted
 * @param ownerOnly Whether to apply this change only to the owner
 *
 * @return true if the file permissions where successfully changed.
 */
bool Pathname::setSearchable(bool searchable, bool ownerOnly) {
    if (ownerOnly) {
        return setSearchable(searchable);
    }
    
#if defined (__WINDOWS__)
	CUAssertLog(false, "Windows does not support this functionality");
	return false;
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        if (searchable) {
            status.st_mode = status.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;
        } else {
            status.st_mode = status.st_mode & ~S_IXUSR & ~S_IXGRP & ~S_IXOTH;
        }
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}

/**
 * Sets the owner's write permission for this pathname.
 *
 * The owner may or may not be this application.  The method will return
 * false if the application does not have permission for this change.
 *
 * @param readable  Whether the owner may write to this file
 *
 * @return true if the file permissions where successfully changed.
 */
bool Pathname::setWritable(bool writable) {
#if defined (__WINDOWS__)
	CUAssertLog(false, "This functionality is not yet implemented in Windows");
	return false;
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        if (writable) {
            status.st_mode = status.st_mode | S_IWUSR;
        } else {
            status.st_mode = status.st_mode & ~S_IWUSR;
        }
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}

/**
 * Sets the owner's or everybody's write permission for this pathname.
 *
 * The owner may or may not be this application.  The method will return
 * false if the application does not have permission for this change.
 *
 * @param readable  Whether this file may be written to
 * @param ownerOnly Whether to apply this change only to the owner
 *
 * @return true if the file permissions where successfully changed.
 */
bool Pathname::setWritable(bool writable, bool ownerOnly) {
    if (ownerOnly) {
        return setWritable(writable);
    }
    
#if defined (__WINDOWS__)
	CUAssertLog(false, "Windows does not support this functionality");
	return false;
#else
	struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        if (writable) {
            status.st_mode = status.st_mode | S_IWUSR | S_IWGRP | S_IWOTH;
        } else {
            status.st_mode = status.st_mode & ~S_IWUSR & ~S_IWGRP & ~S_IWOTH;
        }
        return chmod(_fullpath.c_str(), status.st_mode) == 0;
    }
    return false;
#endif
}



#pragma mark -
#pragma mark Path Size
/**
 * Returns the length of the file denoted by this pathname.
 *
 * The value is measured in bytes.
 *
 * @return the length of the file denoted by this pathname.
 */
size_t Pathname::length() const {
    size_t result = 0;
    Uint8 buf[256];
    SDL_RWops* rw = SDL_RWFromFile(_fullpath.c_str(), "rb");
    
    // This is really the only reliable way to do this
    size_t amt = SDL_RWread(rw, buf, 1, sizeof (buf));
    while (amt > 0) {
        result += amt;
        amt = SDL_RWread(rw, buf, 1, sizeof (buf));
    }
    SDL_RWclose(rw);
    return result;
}

/**
 * Returns the time that the file for this pathname was last modified.
 *
 * The value is in seconds since the last epoch (e.g. January 1, 1970 on
 * Unix systems)
 *
 * @return the time that the file for this pathname was last modified.
 */
Uint64 Pathname::lastModified() const {
#if defined (__WINDOWS__)
	WIN32_FIND_DATA search_data;
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile(_fullpath.c_str(), &search_data);

	Uint64 result = 0;
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	bool ok = GetFileTime(handle, &creationTime, &lastAccessTime, &lastWriteTime);
	if (ok) {
		FILETIME localFileTime;
		FileTimeToLocalFileTime(&lastWriteTime, &localFileTime);
		SYSTEMTIME sysTime;
		FileTimeToSystemTime(&localFileTime, &sysTime);
		struct tm tmtime = { 0 };
		tmtime.tm_year = sysTime.wYear - 1900;
		tmtime.tm_mon = sysTime.wMonth - 1;
		tmtime.tm_mday = sysTime.wDay;
		tmtime.tm_hour = sysTime.wHour;
		tmtime.tm_min = sysTime.wMinute;
		tmtime.tm_sec = sysTime.wSecond;
		tmtime.tm_wday = 0;
		tmtime.tm_yday = 0;
		tmtime.tm_isdst = -1;
		time_t ret = mktime(&tmtime);
		result = static_cast<Uint64> (ret);
	}

	FindClose(handle);
	return result;
#else
    struct stat status;
    int err = stat(_fullpath.c_str(), &status);
    if (err == 0) {
        return (Uint64)status.st_mtime;
    }
    return 0;
#endif
}

/**
 * Returns the number of unallocated bytes in the partition for this pathname.
 *
 * The value is for the partition containing the given file or directory.
 * Just because space is free does not mean that it is available.  The
 * space may be restricted to priviledged users.
 *
 * @return the number of unallocated bytes in the partition for this pathname.
 */
size_t Pathname::getFreeSpace() const {
#if defined (__WINDOWS__)
	ULARGE_INTEGER freeBytesAvail;
	ULARGE_INTEGER totalNumBytes;
	ULARGE_INTEGER totalBytesAvail;

	bool okay = GetDiskFreeSpaceEx(_fullpath.c_str(), &freeBytesAvail, &totalNumBytes, &totalBytesAvail);
	if (okay) {
		return (size_t)totalBytesAvail.QuadPart;
	}
	return 0;
#else
    struct statvfs status;
    int err = statvfs(_fullpath.c_str(), &status);
    if (err  == 0) {
        return status.f_bfree*status.f_bsize;
    }
    return 0;
#endif
}

/**
 * Returns the number of available bytes in the partition for this pathname.
 *
 * The value is for the partition containing the given file or directory.
 * This method is similar to {@link getFreeSpace} except that it measures
 * the number of bytes available for unpriviledged users.
 *
 * @return the number of available bytes in the partition for this pathname.
 */
size_t Pathname::getAvailableSpace() const {
#if defined (__WINDOWS__)
	ULARGE_INTEGER freeBytesAvail;
	ULARGE_INTEGER totalNumBytes;
	ULARGE_INTEGER totalBytesAvail;

	bool okay = GetDiskFreeSpaceEx(_fullpath.c_str(), &freeBytesAvail, &totalNumBytes, &totalBytesAvail);
	if (okay) {
		return (size_t)freeBytesAvail.QuadPart;
	}
	return 0;
#else
	struct statvfs status;
    int err = statvfs(_fullpath.c_str(), &status);
    if (err  == 0) {
        return status.f_bavail*status.f_bsize;
    }
    return 0;
#endif
}

/**
 * Returns the size of the partition named by this pathname.
 *
 * The value is for the partition containing the given file or directory.
 *
 * @return the size of the partition named by this pathname.
 */
size_t Pathname::getTotalSpace() const {
#if defined (__WINDOWS__)
	ULARGE_INTEGER freeBytesAvail;
	ULARGE_INTEGER totalNumBytes;
	ULARGE_INTEGER totalBytesAvail;

	bool okay = GetDiskFreeSpaceEx(_fullpath.c_str(), &freeBytesAvail, &totalNumBytes, &totalBytesAvail);
	if (okay) {
		return (size_t)totalNumBytes.QuadPart;
	}
	return 0;
#else
	struct statvfs status;
    int err = statvfs(_fullpath.c_str(), &status);
    if (err  == 0) {
        return status.f_blocks*status.f_bsize;
    }
    return 0;
#endif
}
