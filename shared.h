// shared static routines between the daemon and the wrapper
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#if !defined(ASEPSIS_INLINE)
    #if (defined (__GNUC__) && (__GNUC__ == 4)) || defined (__clang__)
        #define ASEPSIS_INLINE static __inline__ __attribute__((always_inline))
    #else
        #define ASEPSIS_INLINE static __inline__
    #endif
#endif

extern void asepsis_setup_safe(void);
extern void asepsis_setup_logging(void);

extern int g_asepsis_disabled;

////////////////////////////////////////////////////////////////////////////////////////////////////

ASEPSIS_INLINE int isDisabled() {
    asepsis_setup_safe();
    return g_asepsis_disabled;
}

ASEPSIS_INLINE void underscorePatch(char* path) {
    size_t len = strlen(path);
    if (len<DSSTORE_LEN) {
        return; // safety-net
    }
    *(path + len - DSSTORE_LEN) = '_';
}

// /some/dir/.DS_Store -> /usr/local/.dscage/some/dir/_DS_Store
// res must have at least size of PATH_MAX+PATH_MAX+1
// the command reports a failure for long paths
ASEPSIS_INLINE int makePrefixPath(const char* path, char* res) {
    size_t len1 = strlcpy(res, DSCAGE_PREFIX, PATH_MAX);
    size_t len2 = strlcpy(res + len1, path, SAFE_PREFIX_PATH_BUFFER_SIZE - len1);
    size_t total = len1 + len2;
    if (total>PATH_MAX) {
        return 0; // the prefixed path would be too long
    }
    return 1;
}

ASEPSIS_INLINE int isPrefixPath(const char* path) {
    if (!path) return 0;
    return strncmp(DSCAGE_PREFIX, path, DSSTORE_LEN) == 0;
}

ASEPSIS_INLINE int isDSStorePath(const char* path) {
    if (!path) return 0;
    size_t l = strlen(path);
    if (l<DSSTORE_LEN+1) return 0;
    const char* name = path+l-DSSTORE_LEN-1;
    if (strcmp(name, "/.DS_Store")!=0) {
        return 0;
    }
    
    // ignore .DS_Store files in */.Trashes/*
    if (strstr(path, "/.Trashes/")!=NULL) {
        DLOG("ignoring DS_Store in .Trashes: %s", path);
        return 0;
    }

    // ignore .DS_Store files in */.Trash/*
    if (strstr(path, "/.Trash/")!=NULL) {
        DLOG("ignoring DS_Store in .Trash: %s", path);
        return 0;
    }

    return 1;
}

ASEPSIS_INLINE int isDSStorePresentOnPath(const char* path) {
    if (!path) return 0;
    return access(path, F_OK)==0;
}

ASEPSIS_INLINE int isDirectory(const char* path) {
    struct stat sb;
    if (stat(path, &sb) == 0) {
        if (S_ISDIR (sb.st_mode)) {
            return 1;
        }
    }
    return 0;
}

// http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
// adapted bash source of recursive mkdir
ASEPSIS_INLINE int ensureDirectoryForPath(char* path) {
    int nmode = S_IRWXU|S_IRWXG|S_IRWXO;
    int parent_mode = S_IRWXU|S_IRWXG|S_IRWXO;
    int oumask;
    struct stat sb;
    char *p;
    
    // check full path for presence
    if (stat(path, &sb) == 0) {
        if (chmod(path, nmode)) {
            ERROR("%s: %s", path, strerror (errno));
            return 1;
        }
        // ok, dir or file exists and is ready
        return 0;
    }
    
    oumask = umask(0);
    p = path;
    
    // skip leading slashes
    while (*p == '/') {
        p++;
    }
    
    while ((p = strchr (p, '/'))) {
        *p = '\0'; // it is ok to write into path
        if (stat (path, &sb) != 0) {
            if (mkdir (path, parent_mode)) {
                ERROR("cannot create directory `%s': %s", path, strerror (errno));
                umask (oumask);
                return 1;
            }
        }
        else if (S_ISDIR (sb.st_mode) == 0) {
            ERROR("`%s': file exists but is not a directory", path);
            umask (oumask);
            return 1;
        }
        
        // restore slash and skip group of multiple slashes if present
        *p++ = '/'; 
        while (*p == '/') {
            p++;
        }
    }
    
    // the final component is assumed to be a file
    umask (oumask);
    return 0;
}