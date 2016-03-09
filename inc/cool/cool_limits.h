/**
 \file
*/
#ifndef COOL_LIMITS_H
#define COOL_LIMITS_H

/* 
 The main method signature does not vary and is in keeping with
 main(argc, argv) { return 0;}
 */
#define COOL_MAIN_METHOD_SIGNATURE "main:(IS)(I)"

/*
 \todo A lot of the limits here are very small... I just need a number
 for a buffer size. As the compiler gets built I'll change these 
 to suit requirements etc.
 
 I've set a lot of them deliberately low to flush out bugs.
 */

/* Maxiumum error string allowed to be set in any COOL ADT*/
#define COOL_MAX_ERR_STR 512

/* ByteCode File limits */
/*
 Maximum compiled Object file size. 
 \todo This is a small limit and will need to be increased 
 or removed. Java has a "method" size limit of 64k compiled 
 and can have 2^16 -1 methods in one class
 */
#define COOL_MAX_OBJECT_FILE_SIZE   1048576

/* 
 How many fields an object can have 
 */

#define COOL_MAX_OBJECT_FIELDS_COUNT 256

/* Max function/method count in one object.
 \todo max functions in one object, this is obviously much to
 small to be serious
 */

#define COOL_MAX_OBJECT_METHOD_COUNT 16

/*
 Max signature size not including namespace, this inclide the args ie
 concat:(DS)(S)
 The concat funtion takes a (Double, String) and returns a (Double), size
 is 15 strlen() + \0 
 */

#define COOL_MAX_OBJECT_METHOD_SIGNATURE 16

/* Max code in method ie instruction count. */

#define COOL_MAX_OBJECT_METHOD_SIZE 1024


/* Max namespace name size. */

#define COOL_MAX_NAMESPACE_SIZE 16

/* Max method name size. */

#define COOL_MAX_OBJECT_METHOD_NAME_SIZE 16

/*
 VM Limits...
 */
/** 
 When we load classes if we need more memory how many do 
 we allocate? Loding can happen while running so we need
 some sort of way to dynammically increase memory ie we 
 cannot load everything and allocate it once.
 */
#define COOL_MAX_VM_CLASSES_CALLOC              16


#define COOL_MAX_VM_SAVED_STACK_FRAME_REGISTERS 9

/** 
 How many registers does a stack frame have, this is fixed to 
 this number ie changing it will break something at this time.
 */
#define COOL_MAX_VM_SAVED_STACK_FRAME_ARGS      9

/**
 How many registers do I have in the called function that are
 local. This is also hard code ie don't cahnge this unless you've
 edited the frame code to deal with a changing number
 */

#define COOL_MAX_VM_METHOD_CALLEE_REGISTERS     32


#endif /* COOL_CONSTANTS_H */