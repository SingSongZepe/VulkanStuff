#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <array>

#define DEBUG 1

#if DEBUG
#define ENABLE_validation_layers 1
#else
#define ENABLE_validation_layers 0
#endif

#if DEBUG
#define DEBUG_MESSAGE_NORMAL(msg) \
    std::cout << "[SingSongLog NOR]: "<< msg << std::endl;
#else
#define DEBUG_MESSAGE_NORMAL(msg)
#endif

#if DEBUG
#define DEBUG_MESSAGE_WARNING(msg) \
    std::cout << "[SingSongLog WAR]: "<< msg << std::endl;
#else
#define DEBUG_MESSAGE_WARNING(msg)
#endif

#if DEBUG
#define DEBUG_MESSAGE_ERROR(msg) \
    std::cout << "[SingSongLog ERR]: "<< msg << std::endl;
#else
#define DEBUG_MESSAGE_ERROR(msg)
#endif

#if DEBUG
#define DEBUG_MESSAGE(msg, type) \
    std::cout << "[SingSongLog " << #type << "]: " << msg << std::endl;
#else
#define DEBUG_MESSAGE(msg)
#endif

extern const std::array<const char*, 1> validation_layers;


#endif // __GLOBAL_H__