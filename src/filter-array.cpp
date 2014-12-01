#include <node.h>

#if NODE_MODULE_VERSION >= 14
#include "node-v0.11/filter-array.cpp"
#else
#include "node-v0.10/filter-array.cpp"
#endif
