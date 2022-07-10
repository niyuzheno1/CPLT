
#ifndef TREE_SITTER_DLL_EXPORT_H
#define TREE_SITTER_DLL_EXPORT_H

#ifdef TREE_SITTER_DLL_STATIC_DEFINE
#  define TREE_SITTER_DLL_EXPORT
#  define TREE_SITTER_DLL_NO_EXPORT
#else
#  ifndef TREE_SITTER_DLL_EXPORT
#    ifdef tree_sitter_dll_EXPORTS
        /* We are building this library */
#      define TREE_SITTER_DLL_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define TREE_SITTER_DLL_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef TREE_SITTER_DLL_NO_EXPORT
#    define TREE_SITTER_DLL_NO_EXPORT 
#  endif
#endif

#ifndef TREE_SITTER_DLL_DEPRECATED
#  define TREE_SITTER_DLL_DEPRECATED __declspec(deprecated)
#endif

#ifndef TREE_SITTER_DLL_DEPRECATED_EXPORT
#  define TREE_SITTER_DLL_DEPRECATED_EXPORT TREE_SITTER_DLL_EXPORT TREE_SITTER_DLL_DEPRECATED
#endif

#ifndef TREE_SITTER_DLL_DEPRECATED_NO_EXPORT
#  define TREE_SITTER_DLL_DEPRECATED_NO_EXPORT TREE_SITTER_DLL_NO_EXPORT TREE_SITTER_DLL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TREE_SITTER_DLL_NO_DEPRECATED
#    define TREE_SITTER_DLL_NO_DEPRECATED
#  endif
#endif

#endif /* TREE_SITTER_DLL_EXPORT_H */
