#include <node.h>

#if NODE_MODULE_VERSION >= 14
#include "node-v0.11/lzma-stream-asynccode.cpp"
#else
#include "node-v0.10/lzma-stream-asynccode.cpp"
#endif
