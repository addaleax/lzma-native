#include <node.h>

#if NODE_MODULE_VERSION >= 14
#include "node-v0.11/liblzma-functions.cpp"
#else
#include "node-v0.10/liblzma-functions.cpp"
#endif
