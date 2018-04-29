/** \file */
#ifndef COOL_LIMITS_H
#define COOL_LIMITS_H

/**
 Magic number for casm file format and Major/Minor version
*/

#define COOL_OBJ_MAGIC_STRING "0xdaccaaa"
#define COOL_OBJ_MAGIC 0xdaccaaa

#define COOL_OBJ_MAJOR_STRING "0"
#define COOL_OBJ_MAJOR 0x0000000

#define COOL_OBJ_MINOR_STRING "1"
#define COOL_OBJ_MINOR 0x0000001

/**
 Newlines are different based on OS.
*/
#define COOL_NEWLINE "\n"

/**
 This is the max size for any identifier ie variable names, function names
 actor names etc. I expect to hit this at some point and hopefully trigger an
 assert.
 */
#define COOL_PARSER_ID_LENGTH 31

/**
 Casm file extension. I'm using .asm at this time becasue the Vim syntax file 
 almost works for what I need. I know I can set this in Vim and I will 
 eventually.
 */
#define COOL_ASM_FILE_EXT ".asm"
#define COOL_ASM_FILE_EXT_LENGTH 4

/**
 Fake language extension. Note. Ruby's vim syntax file almost works.
 :set syntax=ruby
 */
#define COOL_LANG_FILE_EXT ".cool"
#define COOL_LANG_FILE_EXT_LENGTH 5

/**
 Maximum symbol length. This includes things like addresses.
 */
#define COOL_SYM_LENGTH_LIMIT 128

/**
 \todo Class path. This is really only here for testing. This needs to be part 
 of configuration. For now it's where all the test fixtures live. By convention 

 Log.cool == Cool Language file

 Log.S    == generated casm file
 
 log.asm == hand written casm test fixture.
*/
#define COOL_CLASS_PATH "/git/ghub/cool/src/cool/tests/asm"


/**
 \todo Everything like file paths and length etc need to be in an arch header
 where we can change them based on OS etc
 */
#define COOL_MAX_FILE_LENGTH 256
#define COOL_MAX_PATH_LENGTH 256
#define COOL_MAX_FILE_PATH_LENGTH 512




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
#define COOL_MAX_OBJECT_METHOD_SIGNATURE 32

/** 
 Max code in method ie instruction count.
 */
#define COOL_MAX_OBJECT_METHOD_SIZE 1024


/* Max namespace name size. */

#define COOL_MAX_NAMESPACE_SIZE 16

/* Max method name size. */

#define COOL_MAX_OBJECT_METHOD_NAME_SIZE 16

/*
 VM Limits...
 */

/**
 Max OS threads that can be created.
 */
#define COOL_MAX_OS_THREADS 4

/**
 How many registers does out default VM have?
 */
#define COOL_VM_REG_COUNT 64

/**
 More importantly how many registers in the stack frame. Please see
 COOL_MAX_VM_SAVED_STACK_FRAME_ARGS
 */
#define COOL_VM_STACK_REG_COUNT 16

/**
 Inital address book size. I intend to keep the address book in contigous 
 memory. Moving address books about for address that get called a lot might
 impact cache hit performance. Using realloc might prove to be a PITA, 
 we'll see.
 */
#define COOL_VM_ADDRESS_BOOK_INIT_SIZE 1024
/**
 Max address and max signature are related. What people would normally associate
 with a message signature is going to be an address in the VM
 */
#define COOL_VM_MAX_ADDRESS_LENGTH 64

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


/**
 Arbitrary maximum local variables in a function.
 */
#define COOL_MAX_VM_FUNC_LOCALS 32


#endif /* COOL_CONSTANTS_H */
