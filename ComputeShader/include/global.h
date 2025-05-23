
#define DEBUG 1

#if DEBUG
#define DEBUG_MESSAGE_NORMAL(msg) \
    std::cout << "[SingSongLog NOR]: "<< #msg << std::endl;
#else
#define DEBUG_MESSAGE_NORMAL(msg)
#endif

#if DEBUG
#define DEBUG_MESSAGE_WARNING(msg) \
    std::cout << "[SingSongLog WAR]: "<< #msg << std::endl;
#else
#define DEBUG_MESSAGE_WARNING(msg)
#endif

#if DEBUG
#define DEBUG_MESSAGE_ERROR(msg) \
    std::cout << "[SingSongLog ERR]: "<< #msg << std::endl;
#else
#define DEBUG_MESSAGE_ERROR(msg)
#endif

#if DEBUG
#define DEBUG_MESSAGE(msg, type) \
    std::cout << "[SingSongLog " << #type << "]: " << #msg << std::endl;
#else
#define DEBUG_MESSAGE(msg)
#endif

#define ENABLE_VALIDATION_LAYER
