#include <node.h>

#if NODE_MODULE_VERSION >= 14
#include "node-v0.11/module.cpp"
#else
#include "node-v0.10/module.cpp"
#endif
