//
// Bested.h - Best Standard.h
//
// Author   - Fletcher M
//
// Created  - 04/08/25
// Modified - 02/11/25
//
// Make sure to...
//      #define BESTED_IMPLEMENTATION
// ...somewhere in your project


#ifdef __cplusplus
    extern "C" {
#endif


#ifndef BESTED_H
#define BESTED_H



// ===================================================
//                    Includes
// ===================================================

// for my ints.
#include <stdint.h>
// for 'true' and 'false'
#include <stdbool.h>
// any real project is gonna need these two, for 'malloc' and 'printf'.
#include <stdlib.h>
#include <stdio.h>

#include <stddef.h> // for 'offsetof'
#include <stdalign.h> // for 'alignof'

#include <assert.h> // mainly for static_assert, i define my own ASSERT




// ===================================================
//                    Classic Ints
// ===================================================

typedef uint64_t        u64;
typedef uint32_t        u32;
typedef uint16_t        u16;
typedef uint8_t         u8;

typedef int64_t         s64;
typedef int32_t         s32;
typedef int16_t         s16;
typedef int8_t          s8;

typedef u64             b64;
typedef u32             b32;
typedef u16             b16;
typedef u8              b8;

typedef float           f32;
typedef double          f64;

// Turn your unknown size enums into known size enums
#define enum8(type)     u8
#define enum16(type)    u16
#define enum32(type)    u32
#define enum64(type)    u64



// ===================================================
//                    Nice Macros
// ===================================================

// control flow helper, useful to define more of these for more specific cases.
#define defer_return(res) do { result = (res); goto defer; } while (0)


// I always forget how to call typeof()
#define Typeof(x)       __typeof__(x)
// stick the extra Typeof() in there to prevent [-Wgnu-alignof-expression]
#define Alignof(x)      alignof(Typeof(x))


// Not wrapping stuff in () because this is purely text based, nothing fancy can be done here.
#define Member(type, member)        ( ((type*)0)->member )
#define Member_Size(type, member)   sizeof( Member(type, member) )


// offsetof is also here. comes from stddef.h


// I really hate c++ sometimes
#ifdef __cplusplus
    #define ZEROED { /* Imagine there was a zero here */ }
#else
    #define ZEROED {0}
#endif


#define Min(a, b)   ({ Typeof(a) _a = (a); Typeof(b) _b = (b); _a < _b ? _a : _b; })
#define Max(a, b)   ({ Typeof(a) _a = (a); Typeof(b) _b = (b); _a > _b ? _a : _b; })

#define Sign(T, x)  ((T)((x) > 0) - (T)((x) < 0))
#define Abs(x)      (Sign(Typeof(x), (x)) * (x))

// TODO this has the same problem as Min & Max
#define Clamp(x, min, max)              ((x) < (min) ? (min) : (((x) > (max)) ? (max) : (x)))

#define Is_Between(x, lower, upper)     (((lower) <= (x)) && ((x) <= (upper)))


#define Array_Len(array)            (sizeof(array) / sizeof((array)[0]))

// integers only
#define Is_Pow_2(n)                 (((n) != 0) && (((n) & ((n)-1)) == 0))

#define Div_Ceil(x, y)      (((x) + (y) - 1) / (y))
#define Div_Floor(x, y)     ((x) / (y))


#define Bit(n) (1 << (n))
#define Has_Bit(n, pos) ((n) & (1 << (pos)))


#define Flag_Set(n, f)          ((n) |= (f))
#define Flag_Clear(n, f)        ((n) &= ~(f))
#define Flag_Toggle(n, f)       ((n) ^= (f))
#define Flag_Exists(n, f)       (((n) & (f)) == (f))    // Checks if all bits in 'f' are set in 'n'
#define Flag_Equals(n, f)       ((n) == (f))            // Checks exact equality
#define Flag_Intersects(n, f)   (((n) & (f)) != 0)      // Checks if any bits in 'f' are set in 'n'



#define PI              3.1415926535897932384626433
#define TAU             6.283185307179586476925286766559
#define E               2.718281828459045235360287471352
#define ROOT_2          1.414213562373095048801688724209

#define KILOBYTE        (1024UL)
#define MEGABYTE        (1024UL * 1024UL)
#define GIGABYTE        (1024UL * 1024UL * 1024UL)
#define TERABYTE        (1024UL * 1024UL * 1024UL * 1024UL)

#define THOUSAND        (1000UL)
#define MILLION         (1000UL * 1000UL)
#define BILLION         (1000UL * 1000UL * 1000UL)
#define TRILLION        (1000UL * 1000UL * 1000UL * 1000UL)


#define MILISECONDS_PER_SECOND          THOUSAND
#define MICROSECONDS_PER_SECOND         MILLION
#define NANOSECONDS_PER_SECOND          BILLION

// good for usleep
#define MILISECONDS_PER_MICROSECOND     (MICROSECONDS_PER_SECOND / MILISECONDS_PER_SECOND)




// ===================================================
//         What static really means
// ===================================================

// Mark a function that must be used in the current compilation block and cannot be seen outside of it.
#define internal            static
// mark a variable inside of a function that persists though function calls.
#define local_persist       static
// casey once said that all this dose is help the compiler deal with
// multithreaded code, by forceing the functions to not store the
// value of the variable into their own registers.
#define global_variable     static



// ===================================================
//         assert and aborting functions
// ===================================================

#define ASSERT(expr) do { if (!(expr)) {                                                    \
        fprintf(stderr, "===========================================\n");                   \
        fprintf(stderr, "%s:%d: ASSERTION ERROR: \"%s\"\n", __FILE__, __LINE__, #expr);     \
        fprintf(stderr, "===========================================\n");                   \
        abort();                                                                            \
    } } while (0)

#define PANIC(message, ...) do {                                                            \
        fprintf(stderr, "===========================================\n");                   \
        fprintf(stderr, "%s:%d: PANIC: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
        fprintf(stderr, "===========================================\n");                   \
        abort();                                                                            \
    } while (0)

#define UNREACHABLE() do {                                                                  \
        fprintf(stderr, "===========================================\n");                   \
        fprintf(stderr, "%s:%d: UNREACHABLE\n", __FILE__, __LINE__);                        \
        fprintf(stderr, "===========================================\n");                   \
        abort();                                                                            \
    } while (0)

#define TODO(message, ...) do {                                                             \
        fprintf(stderr, "===========================================\n");                   \
        fprintf(stderr, "%s:%d: TODO: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);     \
        fprintf(stderr, "===========================================\n");                   \
        abort();                                                                            \
    } while (0)



// ===================================================
//        Useful Memory Manipulation Functions
// ===================================================

static_assert(sizeof(u64) >= sizeof(void*), "A pointer is pretty much always a u64");

#define Ptr_To_U64(ptr)     ((u64)(ptr))
// it doesn't matter if this looses any bits, if your doing
// this it was probably a pointer in the first place.
#define U64_To_Ptr(U64)     ((void*)(U64))

b32 Mem_Is_Aligned          (void *ptr, u64 alignment);
u64 Mem_Align_Forward       (u64 size, u64 alignment); // returns aligned value
u64 Mem_Align_Back          (u64 size, u64 alignment); // returns aligned value
void *Mem_Byte_Offset       (void *ptr, s64 bytes);
s64 Mem_Ptr_Diff            (void *ptr1, void *ptr2);  // ptr1 - ptr2

void *Mem_Copy(void *dest, void *src, u64 size);
void *Mem_Move(void *dest, void *src, u64 size);
void *Mem_Set (void *ptr, u8 value, u64 size);
s32   Mem_Cmp (void *ptr1, void *ptr2, u64 count);
#define Mem_Eq(ptr1, ptr2, count)   (Mem_Cmp((ptr1), (ptr2), (count)) == 0)
#define Mem_Zero(ptr, size)         Mem_Set((ptr), 0, (size))
#define Mem_Zero_Struct(x)          Mem_Zero((x), sizeof(*(x)))


#ifndef BESTED_ALIGNED_ALLOC
    #define BESTED_ALIGNED_ALLOC(align, size)       aligned_alloc((align), (size))
    #define BESTED_FREE(ptr)                        free(ptr)
#else
    #ifndef BESTED_FREE
        #error "Must Define BESTED_FREE as well as BESTED_ALIGNED_MALLOC"
    #endif
#endif // BESTED_ALIGNED_ALLOC

// this is allways based on BESTED_ALLIGNED_ALLOC,
// but no code will assert if BESTED_ALIGNED_ALLOC dosent return the right allignment.
#define BESTED_MALLOC(size)                     BESTED_ALIGNED_ALLOC(Alignof(u64), size)



// ===================================================
//                      Atomics
// ===================================================

#ifdef __STDC_NO_ATOMICS__
    #error "This header uses atomics, could probably hide all the uses under a #define, if it was a problem"
#endif

#include <stdatomic.h>

#define Atomic(type) _Atomic(type)


// is it ever the case that a number may not be atomically set?
#define Atomic_Store(object, value)         atomic_store(object, value)
#define Atomic_Load(object)                 atomic_load(object)

// 'object' and 'expected' are pointers. 'desired' is just a value.
//
// on success, returns true,  and sets object   to desired
// on failure, returns false, and sets expected to object
//
// uses *_strong() because why not. (although *_weak() is faster in a loop apparently)
#define Atomic_Compare_And_Exchange(object, expected, desired)      atomic_compare_exchange_strong((object), (expected), (desired))

// all of these return the value they were before
#define Atomic_Exchange(object, operand)    atomic_exchange(object, operand)
#define Atomic_Add(object, operand)         atomic_fetch_add(object, operand)
#define Atomic_Sub(object, operand)         atomic_fetch_sub(object, operand)

#define Atomic_Or(object, operand)          atomic_fetch_or(object, operand)
#define Atomic_Xor(object, operand)         atomic_fetch_xor(object, operand)
#define Atomic_And(object, operand)         atomic_fetch_and(object, operand)

// A spin loop until the thread can capture the bool:
//      while (Atomic_Test_And_Set(object));
#define Atomic_Test_And_Set(object)         Atomic_Exchange(object, true)
#define Atomic_Clear(object)                Atomic_Store(object, false)

// capture a lock for the duration of some scope.
// this is made of 2 statements so dont put it right after an if statment or something...
//
// I shouldnt have to say this but NEVER try to escape the scope any other way then the bottom.
#define Atomic_Capture_Lock(lock) while (Atomic_Test_And_Set(lock)); for (int __lock_macro_holder = 0; __lock_macro_holder == 0; __lock_macro_holder = (Atomic_Clear(lock), 1))



// ===================================================
//                Misc Useful functions
// ===================================================

// only works on unix.
u64 nanoseconds_since_unspecified_epoch(void);



// ===================================================
//                      Arena
// ===================================================


#ifndef ARENA_ASSERT
    #define ARENA_ASSERT    ASSERT
#endif

#ifndef ARENA_REGION_DEFAULT_CAPACITY
    #define ARENA_REGION_DEFAULT_CAPACITY   (32 * KILOBYTE)
#endif // ARENA_REGION_DEFAULT_CAPACITY


//
// When you return a chunk of memory to the user, its important that the data is aligned to a
// register / pointer boundary. This means that when the user goes to read or write their ints
// or something, they dont have to do a stupid read across registers / memory locations, to get a single number.
// If you just did the most 'efficient' thing and alloc just the right amount of data and return the next section,
// you could not be aligned with a register boundary, and thus it would take a different kind of read by the CPU
// to get the memory to you, and to send it back down. This size should be something friendly to the CPU,
// (thats why we don't just use char or u8 here.)
//
// I use a 64-bit number here, (not just whatever the 'int' type is), so its easily known and mostly future proof.
// If you need 128-bit aligned pointers, just modify this file, or just only use the aligned parts,
// and waste a bit of memory, its fine to do so, this arena dose it all the time.
//
#define Default_Alignment   Alignof(u64)

typedef struct Region {
    struct Region *next;
    // just being explicit about bing in_bytes with these
    // two since my other arena is not in bytes.
    u64 count_in_bytes;
    u64 capacity_in_bytes;

    // used in Arena_Free()
    b32 do_not_free_this;
    // extra padding bytes.
    u8 padding[4];

    u8 data[];
} Region;

// TODO do a thing more like the String_Builder, aka have arrays in a linked list.
typedef struct Arena {
    Region *first, *last;

    // how much to allocate when a new Region is required,
    // == 0: ARENA_REGION_DEFAULT_CAPACITY
    //  > 0: is how much to allocate, in bytes,
    u64 minimum_allocation_size;

    // TODO the following flags should be put into an enum flag.

    // If this is set to True, (anything thats not zero),
    // when performing allocating functions, (such as Arena_alloc),
    // it may instead return null when trying to allocate memory for a new Region,
    // instead of calling its panic function, (by default panic is just assert(false))
    //
    // You may want this if you want your programs to be bullet proof,
    // to not panic at the first sign of trouble.
    b32 dont_panic_when_allocation_failure;

    // If this is set to True, if it runs out of memory in its current Region, it will panic.
    // Note. Arena_Initialize_First_Page() will still trigger this function.
    //
    // Uou may want this if you want more control over when your program allocates,
    // and want your program to have a fixed amount of memory usage.
    b32 panic_when_trying_to_allocate_new_page;
} Arena;

// used for marking and then rewinding an arena.
typedef struct Arena_Mark {
    Region *last;
    u64 count;
} Arena_Mark;

void *_Arena_Alloc(Arena *arena, u64 size_in_bytes, u64 alignment, b32 clear_to_zero);
// Clear to zero by default
#define Arena_Alloc(arena, size)                            _Arena_Alloc((arena), (size), Default_Alignment, true)
#define Arena_Alloc_Clear(arena, size, clear_to_zero)       _Arena_Alloc((arena), (size), Default_Alignment, (clear_to_zero))
#define Arena_Alloc_Struct(arena, type)                     (type *)_Arena_Alloc((arena),           sizeof(type), Alignof(type), true)
#define Arena_Alloc_Array(arena, count, type)               (type *)_Arena_Alloc((arena), (count) * sizeof(type), Alignof(type), true)

// get an arena's current position, to rewind it later.
Arena_Mark Arena_Get_Mark(Arena *arena);
// if the mark is not from this arena, your going to have a bad time.
void Arena_Set_To_Mark(Arena *arena, Arena_Mark mark);

// Will do nothing if the first page is already created.
//
// If 'first_page_size_in_bytes' is set to 0, the default is used, see 'minimum_allocation_size' comment above.
void Arena_Initialize_First_Page(Arena *arena, u64 first_page_size_in_bytes);

// Care has been taken, so that when Arena_free is called,
// the pointer to the buffer provided here will not be free'd.
void Arena_Add_Buffer_As_Storeage_Space(Arena *arena, void *buffer, u64 buffer_size_in_bytes);


// sprintf useing the arena as a buffer.
const char *Arena_sprintf(Arena *arena, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

void Arena_Clear(Arena *arena);
void Arena_Free (Arena *arena);



// ===================================================
//                      Pool
// ===================================================

typedef u32 Pool_Flag_Type;
#define NUM_POOL_ARENAS (sizeof(Pool_Flag_Type) * 8)

typedef struct Arena_Pool {
    Atomic(Pool_Flag_Type)  in_use_flags;
    Atomic(b32)             creating_new_pool_in_chain_lock;

    Arena arena_pool[NUM_POOL_ARENAS];

    struct Arena_Pool *next_pool;
} Arena_Pool;


// can be done concurrently
Arena *Pool_Get(Arena_Pool *pool);
// can be done concurrently, (but two threads cannot release the same pool)
void Pool_Release(Arena_Pool *pool, Arena *to_release);

// this could do something fuck-y if done when multiple threads are running,
// but your an idiot to do that.
void Pool_Free_Arenas(Arena_Pool *pool);



// ===================================================
//                       String
// ===================================================

// returns true if c is one of the ASCII whitespace characters.
b32 Is_Whitespace(char c);
#define To_Upper(c) (('a' <= (c) && (c) <= 'z') ? ((c) - 'a' + 'A') : (c))
#define To_Lower(c) (('A' <= (c) && (c) <= 'Z') ? ((c) - 'A' + 'a') : (c))


typedef struct {
    char *data;
    u64 length;
} String;

// Formatting for printf()
#define S_Fmt    "%.*s"
#define S_Arg(s) (int) (s).length, (s).data

String  String_From_C_Str(const char *str);
#define S(c_str)    String_From_C_Str(c_str)


const char *String_To_C_Str(Arena *arena, String s);
const char *temp_String_To_C_Str(String s);


typedef struct {
    b32 null_terminate;
} String_Duplicate_Opt;

// if arena == NULL, uses BESTED_MALLOC.
//
// by default dose not null_terminate
#define String_Duplicate(arena, string, ...) _String_Duplicate((arena), (string), (String_Duplicate_Opt){ __VA_ARGS__ })
String _String_Duplicate(Arena *arena, String s, String_Duplicate_Opt opt);

// in place.
void    String_To_Upper(String *s);

b32     String_Eq           (String s1, String s2);
b32     String_Starts_With  (String s, String prefix);
b32     String_Ends_With    (String s, String postfix);

b32     String_Contains_Char        (String s, char c);
// finds the first occurrence of c in s, -1 on failure
s64     String_Find_Index_Of_Char   (String s, char c);
// finds the first occurrence of needle in s, -1 on failure
// if needle.size == 0, returns -1.
s64     String_Find_Index_Of        (String s, String needle);

// finds the line at index, and returns a String of the line, strips '\n'
String  String_get_single_line(String s, u64 index);

// advance the data, subtract the size, in place.
// dose not check for null, or do any bounds checking. thats on you.
void    String_Advance  (String *s, s64 count);
// advance the data, subtract the size, returns a new copy.
// dose not check for null, or do any bounds checking. thats on you.
String  String_Advanced (String s, s64 count);

typedef b32 (*char_to_bool_func)(char);
// remove ASCII whitespace characters from the right of the String.
String  String_Trim_Right(String s);
// returns a String with the front chopped off, according to the test function.
// use Is_Whitespace to chop the whitespace off of the front.
String  String_Chop_While(String s, char_to_bool_func test_char_function);


String String_Path_to_Filename(String s);
String String_Remove_Extention(String s);

typedef enum {
    SGNL_Remove_Comments = 1 << 0,
    SGNL_Trim            = 1 << 1,
    SGNL_Skip_Empty      = 1 << 2,
    SGNL_All             = SGNL_Remove_Comments | SGNL_Trim | SGNL_Skip_Empty,
} String_Get_Next_Line_Flag;

// gets the next line in parseing, and advances parseing and line_num to match
// bool's toggle removeing comments that start with '#',
// trimming the output (from the right), and skiping over empty lines.
//
// 'result == parseing' signifies the end of the file.
// OR result.size == 0 if skip_empty is toggled.
String  String_Get_Next_Line(String *parseing, u64 *line_num, String_Get_Next_Line_Flag flags);


const char *temp_sprintf       (const char *format, ...) __attribute__ ((format (printf, 1, 2)));
// is null terminated
String      temp_String_sprintf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));



// ===================================================
//                    String Builder
// ===================================================

#define STRING_BUILDER_NUM_BUFFERS      32
static_assert(Is_Pow_2(STRING_BUILDER_NUM_BUFFERS), "For Speed");

// how much to alloc by default when a new segment of memory is needed.
#ifndef STRING_BUILDER_BUFFER_DEFAULT_SIZE
    // 4096 bytes is equal to the default page size in windows and linux. probably.
    #define STRING_BUILDER_BUFFER_DEFAULT_SIZE      (4 * KILOBYTE)
#endif

// This macro is called when something goes wrong. Recommended to just use assert.
//
// If the macro dose nothing, (or doesn't exist,) the results of some operations
// will return NULL/base values. or do no work at all. use at your own risk.
#ifndef STRING_BUILDER_PANIC
    #define STRING_BUILDER_PANIC(error_text)        ASSERT(false && error_text)
#endif


// String Builder internal structure
// holds the segments of the being built string.
typedef struct {
    char *data;
    u64 count;
    u64 capacity;
} Character_Buffer;

// String Builder internal structure
// Holds a bunch character buffers, and a pointer to the next Segment as well,
// Acts like a linked list node.
typedef struct Segment {
    struct Segment *next;
    Character_Buffer buffers[STRING_BUILDER_NUM_BUFFERS];
} Segment;

// A data-structure that can efficiently grow a string.
// good for medium to large strings. small strings (< 4096 bytes) are not recommended, as this structure will allocate once.
typedef struct String_Builder {
    // how much to allocate when a new segment is needed.
    u64 base_new_allocation;

    // the current segment were working on.
    // if its pointing to NULL, (aka you just zero initalized it), it will be set to a pointer to 'first_segment_holder'
    Segment *current_segment;

    // last buffer in use.
    u64 buffer_index;
    // the first in a linked list.
    Segment first_segment_holder;

    // set an allocator to use it to allocate, otherwise uses 'BESTED_MALLOC()'.
    Arena *allocator;
} String_Builder;


// current size of the string being built.
//
// this calculation runs though all of the segments, so dont spam it maybe.
u64 String_Builder_Count(String_Builder *sb);
// how much space the builder has.
// NOTE. some space will be wasted, for performance reasons.
u64 String_Builder_Total_Capacity(String_Builder *sb);

void String_Builder_Clear(String_Builder *sb);

// Dose not check for NULL byte
void String_Builder_Ptr_And_Size(String_Builder *sb, char *ptr, u64 size);
// Dose not check for NULL byte
void String_Builder_String(String_Builder *sb, String s);

u64 String_Builder_printf(String_Builder *sb, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

#define String_Builder_Struct_Bytes(sb, struct_ptr) String_Builder_Ptr_And_Size((sb), (void*) (struct_ptr), sizeof(*(struct_ptr)))
#define String_Builder_Array_Bytes(sb, ptr, count)  String_Builder_Ptr_And_Size((sb), (void*)(ptr), sizeof(*(ptr)) * (count))


// A trailing NULL byte is appended, so you can pass it into functions that expect a C String
String String_Builder_To_String(String_Builder *sb);

// Dose not write a trailing NULL byte, (as apposed to SB_to_SV)
void String_Builder_To_File(String_Builder *sb, FILE *file);

// only call if your not useing an allocator.
void String_Builder_Free(String_Builder *sb);


// ===================================================
//                    Dynamic Array
// ===================================================

#ifndef ARRAY_INITAL_CAPACITY
    #define ARRAY_INITAL_CAPACITY       32
#endif

//
// Example Array:
//
// struct float_Array {
//     _Array_Header_;
//     f32 *items;
// }
//
#define _Array_Header_ struct { u64 count; u64 capacity; Arena *allocator; }

typedef _Array_Header_ Array_Header;
void *Array_Grow(Array_Header *header, void *array, u64 item_size, u64 item_align, u64 count, b32 clear_to_zero);
void Array_Shift(Array_Header *header, void *array, u64 item_size, u64 from_index);

#define Array_Header_Cast(a)    ((Array_Header*)(a))
#define Array_Item_Size(a)      sizeof(*(a)->items)
#define Array_Item_Align(a)     Alignof(*(a)->items)

// add a single value
#define Array_Append(a, value)                                                                                                                  \
    (*((void **)&(a)->items) = Array_Grow(Array_Header_Cast(a), (a)->items, Array_Item_Size(a), Array_Item_Align(a), (a)->count + 1, false),    \
    (a)->items[(a)->count++] = (value))

// add 'n' unzero'd items, returns a pointer to the first element
#define Array_Add(a, n)                                                                                                                         \
    (*((void **)&(a)->items) = Array_Grow(Array_Header_Cast(a), (a)->items, Array_Item_Size(a), Array_Item_Align(a), (a)->count + (n), false),  \
    (a)->count += (n),                                                                                                                          \
    &(a)->items[(a)->count - (n)])


// Add 'n' zero'd items to the back of the list
#define Array_Add_Clear(a, n)                                                                                                                   \
    (*((void **)&(a)->items) = Array_Grow(Array_Header_Cast(a), (a)->items, Array_Item_Size(a), Array_Item_Align(a), (a)->count + (n), true),   \
    (a)->count += (n),                                                                                                                          \
    &(a)->items[(a)->count - (n)])

// make sure there is enough room to hold 'n' items, dose not increase count.
#define Array_Reserve(a, n)                                                                                             \
    (*((void **)&(a)->items) = Array_Grow(Array_Header_Cast(a), (a)->items, Array_Item_Size(a), Array_Item_Align(a), (n), false))

#define Array_Insert(a, i, value)                                                                                                               \
    (*((void **)&(a)->items) = Array_Grow(Array_Header_Cast(a), (a)->items, Array_Item_Size(a), Array_Item_Align(a), (a)->count + 1, false),    \
    Mem_Move((a)->items + (i), (a)->items + i - 1, ((a)->count - (i)) * Array_Item_Size(a)),                                                    \
    (a)->items[(i)] = (value),                                                                                                                  \
    (a)->count += 1)

#define Array_Remove(a, i, n)                                                                               \
    do {                                                                                                    \
        ASSERT((0 <= (i) && (i) < (a)->count) && (0 <= (n) && (n) <= (a)->count - (i)));                    \
        Mem_Move((a)->items + (i), (a)->items + (i) + (n), ((a)->count - (i) - (n)) * Array_Item_Size(a));  \
        (a)->count -= (n);                                                                                  \
    } while(0)

// Dose the full swap, so if you add +1 to the count, the item will return.
#define Array_Swap_And_Remove(array, index)                                 \
    do {                                                                    \
        ASSERT(0 <= (index) && (index) < (array)->count);                   \
        if ((index) != (array)->count-1) {                                  \
            Typeof(*(array)->items) tmp = (array)->items[(index)];          \
            (array)->items[(index)] = (array)->items[(array)->count-1];     \
            (array)->items[(array)->count-1] = tmp;                         \
        }                                                                   \
        (array)->count -= 1;                                                \
    } while (0)


// u64 index = it - array->items;
#define Array_For_Each(type, it, array)                                             \
    for (type *it = (array)->items; it < (array)->items + (array)->count; ++it)


#define Array_Free(array)                   \
    do {                                    \
        if ((array)->allocator != NULL) {   \
            fprintf(stderr, "=======================================================================================\n");       \
            fprintf(stderr, "Are you serious?\n");                                                                              \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "Did you just attempt to free an array that was allready given an allocator?\n");                   \
            fprintf(stderr, "Not just any allocator either, the only thing arrays accept are arena allocators.\n");             \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "You know I do this for you right?\n");                                                             \
            fprintf(stderr, "I give you all these tools and this is what you do with it?\n");                                   \
            fprintf(stderr, "Make a mistake that could have easily been ignored, look into this macro you just called.\n");     \
            fprintf(stderr, "I can check if you have an allocator and just ignore it, but I wont.\n");                          \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "I'm not gonna even ASSERT(false).\n");                                                             \
            fprintf(stderr, "I'm gonna let the segmentation falt, or the subtle memory bug do the talking for me.\n");          \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "I hope you have a terrible day.\n");                                                               \
            fprintf(stderr, "=======================================================================================\n");       \
        }                                   \
        BESTED_FREE((array)->items);        \
        (array)->items = NULL;              \
    } while (0)



// ===================================================
//                Dynamic HashMap
// ===================================================

//
// Example Array:
//
// struct Int_Float_Pair { u32 key; f32 value; }
//
// struct Int_To_Float_Map {
//     _Map_Header_;
//     Int_Float_Pair *items;
// }
//
#define _Map_Header_                    \
    struct {                            \
        u64 count;                      \
        u64 capacity;                   \
        Map_Hash_Entry *table;          \
        Arena *allocator;               \
    }
        // u64 insert_index;
        // u64 used_count_threshold;
        // u64 deleted_count;
        // u64 deleted_count_threshold;

typedef struct { u64 hash; u64 index; } Map_Hash_Entry;
typedef _Map_Header_ Map_Header;

#define Map_Header_Cast(m)          ((Map_Header*)(m))
#define Map_Key_Size(m)             sizeof((m)->items[0].key)
#define Map_Pair_Size(m)            sizeof((m)->items[0])

#define Map_Table_Size_Multiplier   4
#define Map_Table_Size(m)           ((m)->capacity * Map_Table_Size_Multiplier)


#define Map_Reserve(arena, m, n)        TODO("Map_Reserve")
#define Map_Default(arena, m, val)      TODO("Map_Default")
#define Map_Default_S(arena, m, s)      TODO("Map_Default_S")
#define Map_Clear(m)                    ((m)->count = 0)
#define Map_Clone(arena, m, result)     TODO("Map_Clone")


#define HASH_UNALLOCATED    (0)
#define HASH_DEAD           (0xFFFFFFFFFFFFFFFF)

u64   _Map_hash_ptr_and_size            (void *ptr, u64 size);
// returns the index of the key, or a place it can be placed, or -1 if capacity is zero.
s64   _Map_maybe_get_index_of_key       (Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size);
// gets the index of a key, -1 if the key is not in the array.
s64   _Map_get_index_of_key_in_table    (Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size);
s64   _Map_get_index_of_key_in_items    (Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size);
s64   _Map_get_index_of_key_in_items    (Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size);
void *_Map_get_pointer_to_pair_in_items (Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size);
void *_Map_Grow                         (Map_Header *header, void *kv_array, u64 key_size, u64 kv_pair_size, u64 count);
void *_Map_Put                          (Map_Header *header, void *kv_array, void *key, u64 key_size, void *kv_pair, u64 kv_pair_size);

// Generic map
#define Map_Put(m, k, val)                                                           \
    ((m)->items = _Map_Put(                                                                 \
                    Map_Header_Cast(m), (m)->items,                                         \
                    &(k), Map_Key_Size(m),                                                  \
                    &((Typeof(*(m)->items)){.key = (k), .value = val}), Map_Pair_Size(m)    \
                ))

#define Map_Index(m, k)             _Map_get_index_of_key_in_items((m)->table, Map_Table_Size(m), (m)->items, _Map_hash_ptr_and_size(&(k), Map_Key_Size(m)), &(k), Map_Key_Size(m), Map_Pair_Size(m))

#define Map_Get(m, type, k)                                                                 \
    ({                                                                                      \
        Typeof((m)->items) ptr = _Map_get_pointer_to_pair_in_items((m)->table, Map_Table_Size(m), (m)->items, _Map_hash_ptr_and_size(&(k), Map_Key_Size(m)), &(k), Map_Key_Size(m), Map_Pair_Size(m));  \
        ptr ? &ptr->value : NULL;                                                           \
    })

#define Map_Try_Get(m, k, outVal)       TODO("Map_Try_Get")
#define Map_Get_S(m, type, k)           TODO("Map_Get_S")
#define Map_Get_Ptr(m, k)               TODO("Map_Get_Ptr")
#define Map_Get_Ptr_Null(m, k)          TODO("Map_Get_Ptr_Null")
#define Map_Del(m, k)                   TODO("Map_Del")

#define Map_Free(m)                         \
    do {                                    \
        if ((m)->allocator != NULL) {       \
            fprintf(stderr, "=======================================================================================\n");       \
            fprintf(stderr, "Are you serious?\n");                                                                              \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "Did you just attempt to free a hashmap that was allready given an allocator?\n");                  \
            fprintf(stderr, "Not just any allocator either, the only thing hashmaps accept are arena allocators.\n");           \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "You know I do this for you right?\n");                                                             \
            fprintf(stderr, "I give you all these tools and this is what you do with it?\n");                                   \
            fprintf(stderr, "Make a mistake that could have easily been ignored, look into this macro you just called.\n");     \
            fprintf(stderr, "I can check if you have an allocator and just ignore it, but I wont.\n");                          \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "I'm not gonna even ASSERT(false).\n");                                                             \
            fprintf(stderr, "I'm gonna let the segmentation falt, or the subtle memory bug do the talking for me.\n");          \
            fprintf(stderr, "\n");                                                                                              \
            fprintf(stderr, "I hope you have a terrible day.\n");                                                               \
            fprintf(stderr, "=======================================================================================\n");       \
        }                                   \
        BESTED_FREE((m)->table);            \
        BESTED_FREE((m)->items);            \
        (m)->table = NULL;                  \
        (m)->items = NULL;                  \
    } while (0)


// ===================================================
//             Some Common Functions
// ===================================================

String Read_Entire_File(Arena *arena, String filename);




#endif // BESTED_H





#ifdef BESTED_IMPLEMENTATION


#include <string.h>
#include <stdarg.h>


// ===================================================
//        Useful Memory Manipulation Functions
// ===================================================

b32 Mem_Is_Aligned(void *ptr, u64 alignment) {
    static_assert(sizeof(u64) >= sizeof(void*), "doing math with pointers");
    return ((u64)ptr) % alignment == 0;
}

u64 Mem_Align_Forward(u64 size, u64 alignment) {
    return Div_Ceil(size, alignment)  * alignment;
}
u64 Mem_Align_Back(u64 size, u64 alignment) {
    return Div_Floor(size, alignment) * alignment;
}

void *Mem_Byte_Offset(void *ptr, s64 bytes) {
    u8 *ptr_u8 = (u8*)ptr;
    return (void*)(ptr_u8 + bytes);
}

s64 Mem_Ptr_Diff(void *ptr1, void *ptr2) {
    static_assert(sizeof(u64) >= sizeof(void*), "I HATE 128-Bit pointers");
    u64 ptr1_u64 = Ptr_To_U64(ptr1);
    u64 ptr2_u64 = Ptr_To_U64(ptr2);

    if (ptr1_u64 >= ptr2_u64) {
        return (s64) (ptr1_u64 - ptr2_u64);
    } else {
        u64 result = ptr2_u64 - ptr1_u64;
        return -((s64) result);
    }
}


void *Mem_Copy(void *dest, void *src, u64 size) {
    return memcpy(dest, src, size);
}

void *Mem_Move(void *dest, void *src, u64 size) {
    return memmove(dest, src, size);
}

void *Mem_Set (void *ptr, u8 value, u64 size) {
    return memset(ptr, value, size);
}

s32 Mem_Cmp (void *ptr1, void *ptr2, u64 count) {
    return memcmp(ptr1, ptr2, count);
}



// ===================================================
//                Misc Useful functions
// ===================================================

#ifdef _WIN32
    #error "We dont support windows currently, or at least not this one function. just delete it if you need to use the rest of this library"
#else
    #include <unistd.h>
    #include <time.h>
#endif


u64 nanoseconds_since_unspecified_epoch(void) {
#ifdef __unix__
    struct timespec ts;

    #ifndef CLOCK_MONOTONIC
        #define VSCODE_IS_DUMB
        #define CLOCK_MONOTONIC 69420
    #endif
    // dont worry, this will never trigger.
    ASSERT(CLOCK_MONOTONIC != 69420);

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return NANOSECONDS_PER_SECOND * ts.tv_sec + ts.tv_nsec;
#else
    #error "Sorry, haven't implemented this yet."
#endif
}

// https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
bool check_if_file_exists(const char *filepath) {
    return access(filepath, F_OK) == 0;
}


// ===================================================
//                      Arena
// ===================================================

internal Region *Arena_Internal_New_Region(u64 capacity_in_bytes) {
    Region *new_region = (Region*) BESTED_MALLOC(sizeof(Region) + capacity_in_bytes);
    if (new_region) {
        new_region->next                = NULL;
        new_region->count_in_bytes      = 0;
        new_region->capacity_in_bytes   = capacity_in_bytes;
        new_region->do_not_free_this    = false;
    }
    return new_region;
}

internal void Arena_Internal_Free_Region(Region *region) {
    if (!region->do_not_free_this) BESTED_FREE(region);
}

// inline because there is really nothing in this function.
// just some funny casts, and a call to Mem_Set()
internal inline void *Arena_Internal_Get_New_Memory_At_Last_Region(Arena *arena, u64 size_in_bytes, u64 alignment, b32 clear_to_zero) {
    u64 aligned_ptr_u64 = Mem_Align_Forward(Ptr_To_U64(arena->last->data + arena->last->count_in_bytes), alignment);
    s64 how_far_forward = Mem_Ptr_Diff(U64_To_Ptr(aligned_ptr_u64), arena->last->data + arena->last->count_in_bytes);
    ASSERT(how_far_forward >= 0);

    if (clear_to_zero) Mem_Zero(U64_To_Ptr(aligned_ptr_u64), size_in_bytes);

    arena->last->count_in_bytes += size_in_bytes + (u64) how_far_forward;
    return U64_To_Ptr(aligned_ptr_u64);
}


void *_Arena_Alloc(Arena *arena, u64 size_in_bytes, u64 alignment, b32 clear_to_zero) {
    u64 default_size = (arena->minimum_allocation_size != 0) ? arena->minimum_allocation_size : ARENA_REGION_DEFAULT_CAPACITY;

    // add the extra alignment because if you have to allocate the thing,
    // the alignment of the Region is kinda random, with + alignment, the allocation will always succeed.
    u64 to_alloc_if_no_room = Max(default_size, size_in_bytes + alignment);

    // if the arena currently holds no memory
    if (arena->last == NULL) {
        ARENA_ASSERT(arena->first == NULL);

        if (arena->dont_panic_when_allocation_failure) {
            ARENA_ASSERT(false && "Arena_alloc: attempted to allocate new memory, but that has been disallowed. (when there was no memory to begin with.)");
        }

        arena->last = Arena_Internal_New_Region(to_alloc_if_no_room);
        if (arena->last == NULL) {
            if (arena->dont_panic_when_allocation_failure) return NULL;
            ARENA_ASSERT(false && "Arena_alloc: attempted to allocate new memory, got null. (when there was no memory to begin with.)");
        }

        arena->first = arena->last;

        return Arena_Internal_Get_New_Memory_At_Last_Region(arena, size_in_bytes, alignment, clear_to_zero);
    }

    // find room, or find the end
    while ((arena->last->count_in_bytes + size_in_bytes > arena->last->capacity_in_bytes) && (arena->last->next != NULL)) {
        arena->last = arena->last->next;
        if (arena->last) {
            // if we just discoverd this, it must be zero'd.
            // this helps with our mark implementation as well.
            arena->last->count_in_bytes = 0;
        }
    }

    if (arena->last->count_in_bytes + size_in_bytes <= arena->last->capacity_in_bytes) {
        // if there is space alloc
        return Arena_Internal_Get_New_Memory_At_Last_Region(arena, size_in_bytes, alignment, clear_to_zero);

    } else {
        // we need a new region
        ARENA_ASSERT(arena->last->next == NULL);

        if (arena->panic_when_trying_to_allocate_new_page) {
            ARENA_ASSERT(false && "Arena_alloc: attempted to allocate new memory, but that has been disallowed.");
        }

        Region *last_last = arena->last;

        arena->last = Arena_Internal_New_Region(to_alloc_if_no_room);
        if (arena->last == NULL) {
            if (arena->dont_panic_when_allocation_failure) return NULL;
            ARENA_ASSERT(false && "Arena_alloc: attempted to allocate new memory, got null.");
        }
        last_last->next = arena->last;

        return Arena_Internal_Get_New_Memory_At_Last_Region(arena, size_in_bytes, alignment, clear_to_zero);
    }
}


Arena_Mark Arena_Get_Mark(Arena *arena) {
    Arena_Mark result = {
        .last = arena->last,
        .count = arena->last ? arena->last->count_in_bytes : 0,
    };
    return result;
}
void Arena_Set_To_Mark(Arena *arena, Arena_Mark mark) {
    arena->last = mark.last ? mark.last : arena->first;
    if (arena->last) {
        arena->last->count_in_bytes = mark.count;
    }
}


void Arena_Initialize_First_Page(Arena *arena, u64 first_page_size_in_bytes) {
    if (arena->first != NULL) return;

    u64 tmp_min_alloc_size = arena->minimum_allocation_size;
    arena->minimum_allocation_size = first_page_size_in_bytes;

    Arena_Alloc_Clear(arena, 0, false);

    arena->minimum_allocation_size = tmp_min_alloc_size;
}

void Arena_Add_Buffer_As_Storeage_Space(Arena *arena, void *buffer, u64 buffer_size_in_bytes) {
    ARENA_ASSERT(buffer != NULL && "why did you pass this to us.");
    ARENA_ASSERT(buffer_size_in_bytes > sizeof(Region) && "The passed in buffer must be big enough to contain the Region, preferably much bigger");

    u64 real_allocatable_space = buffer_size_in_bytes - sizeof(Region);

    Region *new_region = (Region*) buffer;
    new_region->count_in_bytes      = 0;
    new_region->capacity_in_bytes   = real_allocatable_space;
    new_region->next                = NULL;
    new_region->do_not_free_this    = true;


    if (arena->last == NULL) {
        ARENA_ASSERT(arena->first == NULL);
        arena->first = new_region;
        arena->last  = new_region;

    } else {
        Region *p;
        for (p = arena->last; p->next != NULL; p = p->next);
        p->next = new_region;
    }
}


const char *Arena_sprintf(Arena *arena, const char *format, ...) {
    char *buf    = NULL;
    u64 buf_size = 0;

    if (arena->last != NULL) {
        buf      = (char*) (arena->last->data + arena->last->count_in_bytes);
        buf_size = arena->last->capacity_in_bytes - arena->last->count_in_bytes;
    }

    va_list args;

    va_start(args, format);
        s64 formatted_size = vsnprintf(buf, buf_size, format, args);
    va_end(args);

    ARENA_ASSERT(formatted_size >= 0 && "format was successful");

    // i dont know if this should be <= or just <, hmm...
    if ((u64)formatted_size < buf_size) {
        // the string fits!
        // advance the count. +1 because we need the null terminator for this one.
        arena->last->count_in_bytes += (u64)formatted_size+1;
        return buf;


    } else {
        // else we need to allocate some space.

        buf      = (char*)Arena_Alloc_Clear(arena, (u64)formatted_size+1, false);
        buf_size = (u64)formatted_size+1;

        va_start(args, format);
            vsnprintf(buf, buf_size, format, args);
        va_end(args);

        return buf;
    }
}


void Arena_Clear(Arena *arena) {
    if (arena->first == NULL) return;

    for (Region *r = arena->first; r != arena->last; r = r->next) {
        r->count_in_bytes = 0;
    }
    arena->last->count_in_bytes = 0;

    arena->last = arena->first;
}

void Arena_Free (Arena *arena) {
    // this is slightly more complicated than it would be,
    // but you cant use the thing you just free'd, for some reason,
    // because it get's messed with by 'free'
    Region *r = arena->first;
    while (r) {
        Region *next = r->next;
        Arena_Internal_Free_Region(r);
        r = next;
    }

    arena->first = NULL;
    arena->last  = NULL;
}



// ===================================================
//                      Pool
// ===================================================

Arena *Pool_Get(Arena_Pool *pool) {
    while (true) {
        for (u32 i = 0; i < NUM_POOL_ARENAS; i++) {
            if (Has_Bit(Atomic_Load(&pool->in_use_flags), i)) continue;

            Pool_Flag_Type before = Atomic_Or(&pool->in_use_flags, Bit(i));
            if (Has_Bit(before, i)) continue; // someone got to it first,

            Arena *new_arena = &pool->arena_pool[i];
            Arena_Clear(new_arena);
            return new_arena;
        }

        Atomic(b32) *lock = &pool->creating_new_pool_in_chain_lock;
        Atomic_Capture_Lock(lock) {
            if (!pool->next_pool) {
                pool->next_pool = (Arena_Pool*)BESTED_MALLOC(sizeof(Arena_Pool));
                Mem_Zero(pool->next_pool, sizeof(Arena_Pool));
            }
            pool = pool->next_pool;
        }
    }

    UNREACHABLE();
}

void Pool_Release(Arena_Pool *pool, Arena *to_release) {
    while (pool) {
        s64 maybe_index = Mem_Ptr_Diff(to_release, pool->arena_pool);

        if (Is_Between(maybe_index, 0, (s64) ((NUM_POOL_ARENAS-1) * sizeof(Arena)))) {
            ASSERT(maybe_index % sizeof(Arena) == 0);
            s64 index = maybe_index / sizeof(Arena);

            ASSERT(Has_Bit( Atomic_Load(&pool->in_use_flags) , index));
            Atomic_Xor(&pool->in_use_flags, Bit(index)); // clear the flag atomically.
            return;
        }

        Atomic(b32) *lock = &pool->creating_new_pool_in_chain_lock;
        Atomic_Capture_Lock(lock) {
            pool = pool->next_pool;
        }
    }
    // Ran out of pools to check, to_release must not be an arena from this pool.
    UNREACHABLE();
}

void Pool_Free_Arenas(Arena_Pool *pool) {
    // the user just passed up the original pool.
    // dont free it, but free everything else

    Arena_Pool *original_pool = pool;

    // free all the arena's.
    while (pool) {
        for (u32 i = 0; i < NUM_POOL_ARENAS; i++) {
            Arena_Free(&pool->arena_pool[i]);
        }
        pool = pool->next_pool;
    }

    // free the pointers to the pools.
    pool = original_pool->next_pool;
    while (pool) {
        Arena_Pool *next_pool = pool->next_pool;
        BESTED_FREE(pool);
        pool = next_pool;
    }
}



// ===================================================
//                       String
// ===================================================

b32 Is_Whitespace(char c) {
    if (c == ' ' ) return true;
    if (c == '\f') return true;
    if (c == '\n') return true;
    if (c == '\r') return true;
    if (c == '\t') return true;
    if (c == '\v') return true;
    return false;
}


String String_From_C_Str(const char *str) {
    String result = {
        .data   = (char *) str,
        .length = strlen(str),
    };
    return result;
}
const char *String_To_C_Str(Arena *arena, String s) {
    return String_Duplicate(arena, s, .null_terminate = true).data;
}

#define TEMP_STRING_TO_C_STR_NUM_BUFFERS    64
#define TEMP_STRING_TO_C_STR_MAX_LENGTH     (4 * KILOBYTE)
const char *temp_String_To_C_Str(String s) {
    local_persist char buffers[TEMP_STRING_TO_C_STR_NUM_BUFFERS][TEMP_STRING_TO_C_STR_MAX_LENGTH];
    local_persist u32  current_buffer_index = 0;

    ASSERT(s.length + 1 < TEMP_STRING_TO_C_STR_MAX_LENGTH && "Your String is to big to fit into a temporary buffer");

    char *buf = buffers[current_buffer_index];
    current_buffer_index = (current_buffer_index + 1) % TEMP_STRING_TO_C_STR_NUM_BUFFERS;

    Mem_Copy(buf, s.data, s.length);
    buf[s.length] = 0;

    return buf;
}


String _String_Duplicate(Arena *arena, String s, String_Duplicate_Opt opt) {
    String result = { .length = s.length, };
    u64 alloc_size = s.length + (opt.null_terminate ? 1 : 0);
    if (arena) {
        result.data = (char*) Arena_Alloc(arena, alloc_size);
    } else {
        result.data = (char*) BESTED_MALLOC(alloc_size);
    }
    ASSERT(result.data);
    Mem_Copy(result.data, s.data, result.length);
    // dont need this because the Arena_Alloc dose it for us,
    // but this assignment is kinda free. (the branch isn't though)
    if (opt.null_terminate) result.data[result.length] = 0;
    return result;
}

void String_To_Upper(String *s) {
    for (u64 i = 0; i < s->length; i++) s->data[i] = To_Upper(s->data[i]);
}

b32 String_Eq(String s1, String s2) {
    if (s1.length != s2.length) return false;
    return Mem_Eq(s1.data, s2.data, s1.length);
}

b32 String_Starts_With(String s, String prefix) {
    if (s.length < prefix.length) return false;
    return Mem_Eq(s.data, prefix.data, prefix.length);
}

b32 String_Ends_With(String s, String postfix) {
    if (s.length < postfix.length) return false;
    return Mem_Eq(s.data + s.length - postfix.length, postfix.data, postfix.length);
}

b32 String_Contains_Char(String s, char c) {
    for (u64 i = 0; i < s.length; i++) {
        if (s.data[i] == c) return true;
    }
    return false;
}

s64 String_Find_Index_Of_Char(String s, char c) {
    for (u64 i = 0; i < s.length; i++) {
        // this could break if we have a string that is 2^63 characters long.
        // oh the tragedy.
        if (s.data[i] == c) return i;
    }
    return -1;
}

s64 String_Find_Index_Of(String s, String needle) {
    if (needle.length == 0) return -1;

    if (needle.length == 1) return String_Find_Index_Of_Char(s, needle.data[0]);

    while (true) {
        s64 index = String_Find_Index_Of_Char(s, needle.data[0]);
        if (index == -1) return -1;

        // check if not enough room for needle
        if (s.length - index < needle.length) return -1;

        b32 flag = true;
        for (u64 i = 1; i < needle.length; i++) {
            if (s.data[index+i] != needle.data[i]) {
                flag = false;
                break;
            }
        }

        if (flag) return index;

        s.data   += index + 1;
        s.length -= index + 1;
    }

    return -1;
}

String String_get_single_line(String s, u64 index) {
    // clamp i if its to big. this handles a lot more errors than your thinking about.
    if (index > s.length) index = s.length;

    String result = String_Advanced(s, index);

    s64 index_of_end = String_Find_Index_Of_Char(result, '\n');

    // clamp the length to the far newline
    if (index_of_end != -1) { result.length = index_of_end; }

    // go back until newline before result.data,
    // and make sure it doesn't go out of bounds
    while (result.data != s.data && result.data[-1] != '\n') {
        String_Advance(&result, -1);
    }

    return result;
}

void   String_Advance (String *s, s64 count) {
    s->data   += count;
    s->length -= count;
}
String String_Advanced(String s, s64 count) {
    return (String){.data = s.data + count, .length = s.length - count};
}

String String_Trim_Right(String s) {
    while (s.length > 0 && Is_Whitespace(s.data[s.length-1])) {
        s.length -= 1;
    }
    return s;
}

String String_Chop_While(String s, char_to_bool_func test_char_function) {
    u64 i;
    for (i = 0; i < s.length; i++) {
        if (!test_char_function(s.data[i])) break;
    }
    return String_Advanced(s, (s64)i);
}

String String_Path_to_Filename(String s) {
    while (true) {
        s64 index = String_Find_Index_Of_Char(s, '/');
        if (index == -1) break;
        String_Advance(&s, index+1);
    }
    return s;
}

String String_Remove_Extention(String s) {
    for (s64 i = s.length; i >= 0; i--) {
        if (s.data[i] == '.') {
            s.length = i;
            return s;
        }
    }
    return s;
}


String String_Get_Next_Line(String *parseing, u64 *line_num, String_Get_Next_Line_Flag flags) {
    b32 remove_comments = Flag_Exists(flags, SGNL_Remove_Comments);
    b32 trim            = Flag_Exists(flags, SGNL_Trim);
    b32 skip_empty      = Flag_Exists(flags, SGNL_Skip_Empty);

    String next_line = *parseing;

    while (parseing->length > 0) {
        s64 line_end = String_Find_Index_Of_Char(*parseing, '\n');

        next_line = *parseing;

        if (line_end != -1) {
            next_line.length = (u64)line_end;
            String_Advance(parseing, line_end+1);
        } else {
            String_Advance(parseing, (s64)parseing->length);
        }

        *line_num += 1;

        if (remove_comments) {
            s64 comment_index = String_Find_Index_Of(next_line, S("//"));
            if (comment_index != -1) { next_line.length = (u64)comment_index; }
        }

        if (trim) next_line = String_Trim_Right(next_line);

        if ((!skip_empty) || (next_line.length > 0)) break;
    }

    return next_line;
}


#define TEMP_SPRINTF_NUM_BUFFERS    (1<<6)
static_assert(Is_Pow_2(TEMP_SPRINTF_NUM_BUFFERS), "for well defined wrapping behaviour");

#define TEMP_SPRINTF_BUFFER_SIZE    512 // pretty sure this number is related to page size...

const char *temp_sprintf(const char *format, ...) {
    local_persist char          buffers[TEMP_SPRINTF_NUM_BUFFERS][TEMP_SPRINTF_BUFFER_SIZE];
    local_persist Atomic(u32)   current_buffer_index = 0;

    u32 buffer_index = Atomic_Add(&current_buffer_index, 1);
    char *buf = buffers[buffer_index % Array_Len(buffers)];

    va_list args;
    va_start(args, format);
        vsnprintf(buf, sizeof(buffers[0]), format, args);
    va_end(args);

    return buf;
}

String temp_String_sprintf(const char *format, ...) {
    local_persist char          buffers[TEMP_SPRINTF_NUM_BUFFERS][TEMP_SPRINTF_BUFFER_SIZE];
    local_persist Atomic(u32)   current_buffer_index = 0;

    u32 buffer_index = Atomic_Add(&current_buffer_index, 1);
    char *buf = buffers[buffer_index % Array_Len(buffers)];

    va_list args;
    va_start(args, format);
        vsnprintf(buf, sizeof(buffers[0]), format, args);
    va_end(args);

    // could be faster because we know how long it is. but w/e
    return String_From_C_Str(buf);
}



// ===================================================
//                    String Builder
// ===================================================

internal Character_Buffer *String_Builder_Internal_Maybe_Expand_To_Fit(String_Builder *sb, u64 size) {
    // set the current segment to the first segment if its null (aka just after zero initialization)
    if (sb->current_segment == NULL) sb->current_segment = &sb->first_segment_holder;

    Character_Buffer *last_buffer = &sb->current_segment->buffers[sb->buffer_index % STRING_BUILDER_NUM_BUFFERS];
    while (true) {
        // if the segment hasn't been allocated
        if (last_buffer->capacity == 0) break;

        // if the segent has enough room to hold the new thing
        if (size + last_buffer->count <= last_buffer->capacity) break;

        // else move onto the next segment
        sb->buffer_index += 1;

        if (sb->buffer_index % STRING_BUILDER_NUM_BUFFERS == 0) {
            if (!sb->current_segment->next) {
                if (sb->allocator) {
                    sb->current_segment->next = (Segment*) Arena_Alloc_Struct(sb->allocator, Segment);
                } else {
                    sb->current_segment->next = (Segment*) BESTED_MALLOC(sizeof(Segment));
                    Mem_Zero(sb->current_segment->next, sizeof(Segment));
                }

                if (!sb->current_segment->next) {
                    STRING_BUILDER_PANIC("String Builder - maybe_expand_to_fit: failed to malloc a new segment holder");
                    return NULL;
                }
            }

            sb->current_segment = sb->current_segment->next;
        }

        last_buffer = &sb->current_segment->buffers[sb->buffer_index % STRING_BUILDER_NUM_BUFFERS];
    }


    if (last_buffer->capacity == 0) {
        u64 default_size = sb->base_new_allocation ? sb->base_new_allocation : STRING_BUILDER_BUFFER_DEFAULT_SIZE;
        u64 to_add_size = Max(size, default_size);

        if (sb->allocator) {
            last_buffer->data = (char*) Arena_Alloc_Clear(sb->allocator, to_add_size * sizeof(char), true);
        } else {
            last_buffer->data = (char*) BESTED_MALLOC(to_add_size * sizeof(char));
            Mem_Zero(last_buffer->data, to_add_size * sizeof(char));
        }

        if (!last_buffer->data) {
            STRING_BUILDER_PANIC("maybe_expand_to_fit: failed to malloc a new segment with to_add_size");
            return NULL;
        }

        last_buffer->count = 0;
        last_buffer->capacity = to_add_size;
    }

    return last_buffer;
}

// this is not the most efficient thing ever.
// O(n^2) when looping over the entire array like this...
// but it will probably never come up as a performance problem.
internal inline Character_Buffer *String_Builder_Internal_Get_Character_Buffer_At_Index(String_Builder *sb, u64 buffer_index) {
    Segment *segment = &sb->first_segment_holder;
    while (buffer_index >= STRING_BUILDER_NUM_BUFFERS) {
        buffer_index -= STRING_BUILDER_NUM_BUFFERS;
        segment = segment->next;
    }
    return &segment->buffers[buffer_index];
}


u64 String_Builder_Count(String_Builder *sb) {
    u64 count = 0;
    for (u64 i = 0; i <= sb->buffer_index; i++) {
        Character_Buffer *buffer = String_Builder_Internal_Get_Character_Buffer_At_Index(sb, i);
        count += buffer->count;
    }
    return count;
}

u64 String_Builder_Total_Capacity(String_Builder *sb) {
    u64 capacity = 0;
    Segment *current_segment = &sb->first_segment_holder;
    while (current_segment) {
        for (u32 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            capacity += buffer->capacity;
        }
        current_segment = current_segment->next;
    }
    return capacity;
}

void String_Builder_Clear(String_Builder *sb) {
    for (u64 i = 0; i <= sb->buffer_index; i++) {
        Character_Buffer *buffer = String_Builder_Internal_Get_Character_Buffer_At_Index(sb, i);
        buffer->count = 0;
    }

    sb->buffer_index = 0;
    sb->current_segment  = &sb->first_segment_holder;
}

void String_Builder_Ptr_And_Size(String_Builder *sb, char *ptr, u64 size) {
    if (size == 0) return;

    Character_Buffer *buffer = String_Builder_Internal_Maybe_Expand_To_Fit(sb, size);

    if (!buffer) return; // String_Builder_Internal_Maybe_Expand_To_Fit() has already raised the panic.

    Mem_Copy(buffer->data + buffer->count, ptr, size);
    buffer->count += size;
}

void String_Builder_String(String_Builder *sb, String s) {
    String_Builder_Ptr_And_Size(sb, s.data, s.length);
}

u64 String_Builder_printf(String_Builder *sb, const char *format, ...) {
    va_list args;
    va_start(args, format);
        // TODO figure out how to do the thing we did in Arena.h for its sprintf
        s32 formatted_size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    // < 0 is an error from printf
    ASSERT(formatted_size >= 0);

    // early out.
    if (formatted_size == 0) return 0;

    // +1 because printf also puts a trailing '\0' byte.
    // we ignore that for the rest of the code.
    Character_Buffer *buffer = String_Builder_Internal_Maybe_Expand_To_Fit(sb, (u64)formatted_size+1);

    if (!buffer) return 0; // maybe_expand_to_fit has already raised the panic.

    va_start(args, format);
        vsprintf(buffer->data + buffer->count, format, args);
    va_end(args);

    buffer->count += (u64)formatted_size;
    return (u64)formatted_size;
}

String String_Builder_To_String(String_Builder *sb) {
    u64 count = String_Builder_Count(sb);

    String result = {
        .data   = NULL,
        .length = count,
    };

    if (sb->allocator) {
        result.data = (char*) Arena_Alloc_Clear(sb->allocator, count+1, false);
    } else {
        result.data = (char*) BESTED_MALLOC(count+1);
    }

    if (!result.data) {
        STRING_BUILDER_PANIC("Arena_Alloc failed when trying to allocate enough memory to hold to result string from String_Builder_To_String()");
        result.length = 0;
        return result;
    }

    u64 index = 0;
    for (u64 i = 0; i <= sb->buffer_index; i++) {
        Character_Buffer *buffer = String_Builder_Internal_Get_Character_Buffer_At_Index(sb, i);
        Mem_Copy(result.data + index, buffer->data, buffer->count);
        index += buffer->count;
    }

    result.data[result.length] = 0;
    return result;
}

void String_Builder_To_File(String_Builder *sb, FILE *file) {
    for (u64 i = 0; i <= sb->buffer_index; i++) {
        Character_Buffer *buffer = String_Builder_Internal_Get_Character_Buffer_At_Index(sb, i);
        fwrite(buffer->data, sizeof(char), buffer->count, file);
    }
}

void String_Builder_Free(String_Builder *sb) {
    if (sb->allocator) {
            fprintf(stderr, "=======================================================================================\n");
            fprintf(stderr, "Are you serious?\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Did you just attempt to free a string buffer that was allready given an allocator?\n");
            fprintf(stderr, "Not just any allocator either, the only thing string builders accept are arena allocators.\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "You know I do this for you right?\n");
            fprintf(stderr, "I give you all these tools and this is what you do with it?\n");
            fprintf(stderr, "Make a mistake that could have easily been ignored, look into this function you just called.\n");
            fprintf(stderr, "I can check if you have an allocator and just ignore it, but I wont.\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "I'm not gonna even ASSERT(false).\n");
            fprintf(stderr, "I'm gonna let the segmentation falt, or the subtle memory bug do the talking for me.\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "I hope you have a terrible day.\n");
            fprintf(stderr, "=======================================================================================\n");
    }

    Segment *segment = &sb->first_segment_holder;
    bool first_segment = true;

    while (segment) {
        for (u32 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
            Character_Buffer *buffer = &segment->buffers[i];
            if (buffer->data) BESTED_FREE(buffer->data);

            // only matters if its the first segment.
            buffer->count    = 0;
            buffer->capacity = 0;
            buffer->data     = NULL;
        }

        Segment *next_segment = segment->next;

        if (!first_segment) BESTED_FREE(segment);
        first_segment = false;

        segment = next_segment;
    }

    sb->first_segment_holder.next = NULL;
    sb->current_segment = &sb->first_segment_holder;
}



// ===================================================
//                Dynamic Arrays
// ===================================================

void *Array_Grow(Array_Header *header, void *array, u64 item_size, u64 item_align, u64 count, b32 clear_to_zero) {
    if (count > header->capacity) {
        header->capacity = header->capacity ? header->capacity * 2 : ARRAY_INITAL_CAPACITY;
        while (header->capacity < count) header->capacity *= 2;

        void *new_array;
        if (header->allocator) {
            new_array = _Arena_Alloc(header->allocator, item_size * header->capacity, item_align, false);
            Mem_Copy(new_array, array, item_size * header->count);
        } else {
            new_array = BESTED_ALIGNED_ALLOC(item_align, item_size * header->capacity);
            Mem_Copy(new_array, array, item_size * header->count);
            BESTED_FREE(array);
        }

        array = new_array;
    }

    if (clear_to_zero) Mem_Zero(((u8*)array) + (item_size * header->count), item_size * count);
    return array;
}

void Array_Shift(Array_Header *header, void *array, u64 item_size, u64 from_index) {
    Mem_Move(array, (u8*)array + from_index, (header->count - from_index) * item_size);
    header->count -= from_index;
}



// ===================================================
//                Dynamic HashMap
// ===================================================

u64 _Map_hash_ptr_and_size(void *ptr, u64 size) {
    u8 *u8_ptr = (u8*)ptr;
    u64 h = 1103;
    for (u64 i = 0; i < size; i++) h = h * 47 + u8_ptr[i];

    // Do not return tombstone hashs.
    if ((h == HASH_UNALLOCATED) || (h == HASH_DEAD)) {
        return 27644437;
    } else {
        return h;
    }
}

s64 _Map_maybe_get_index_of_key(Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size) {
    if (table_size == 0) return -1;

    // this probe strategy covers all positions when its a power of 2
    ASSERT(Is_Pow_2(table_size));
    u64 incr = 1;
    u64 pos = hash % table_size; // this could be (hash & (capacity-1)).

    while (table[pos].hash != HASH_UNALLOCATED) {
        Map_Hash_Entry entry = table[pos];

        // this is incorrect when the key is a string. maybe a bool?
        void *entry_key = ((u8*)kv_array) + (entry.index * kv_pair_size);

        // check the hashes for speed, then check the actual keys
        if ((entry.hash != HASH_DEAD) && (entry.hash == hash) && Mem_Eq(key, entry_key, key_size)) {
            return (s64) pos;
        }

        pos = (pos + incr) % table_size;
        incr += 1;
        ASSERT(incr < 512); // what are the odds for 512 hash collisions in a row?
    }

    return (s64) pos;
}

s64 _Map_get_index_of_key_in_table(Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size) {
    s64 index = _Map_maybe_get_index_of_key(table, table_size, kv_array, hash, key, key_size, kv_pair_size);

    if (index == -1)                            return -1;
    if (table[index].hash == HASH_UNALLOCATED)  return -1;
    if (table[index].hash == HASH_DEAD)         return -1;

    return index;
}

s64 _Map_get_index_of_key_in_items(Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size) {
    s64 index_in_table = _Map_get_index_of_key_in_table(table, table_size, kv_array, hash, key, key_size, kv_pair_size);

    if (index_in_table == -1) return -1;

    return (s64) table[index_in_table].index;
}

void *_Map_get_pointer_to_pair_in_items(Map_Hash_Entry *table, u64 table_size, void *kv_array, u64 hash, void *key, u64 key_size, u64 kv_pair_size) {
    s64 index_in_items = _Map_get_index_of_key_in_items(table, table_size, kv_array, hash, key, key_size, kv_pair_size);

    if (index_in_items == -1) return NULL;

    return ((u8*)kv_array) + ((u64)index_in_items * kv_pair_size);
}

void *_Map_Grow(Map_Header *header, void *kv_array, u64 key_size, u64 kv_pair_size, u64 count) {
    if (count > header->capacity) {
        u64 old_table_size = header->capacity * Map_Table_Size_Multiplier;

        u64 new_capacity = header->capacity == 0 ? 32 : header->capacity * 2;
        while (new_capacity < count) new_capacity *= 2;

        u64 new_table_size = new_capacity * Map_Table_Size_Multiplier;

        void *new_kv_array;
        Map_Hash_Entry *new_table;
        if (header->allocator) {
            // do we care about proper alignment? Nah
            new_kv_array = Arena_Alloc(header->allocator, new_capacity * kv_pair_size);
            new_table    = Arena_Alloc_Array(header->allocator, new_table_size, Map_Hash_Entry);
        } else {
            new_kv_array = BESTED_MALLOC(new_capacity * kv_pair_size);
            new_table    = (Map_Hash_Entry*) BESTED_MALLOC(new_table_size * sizeof(Map_Hash_Entry));
        }


        // we need to copy over all the keys and shit.
        Mem_Copy(new_kv_array, kv_array, kv_pair_size * header->count);

        // re-input the hashes into the new_table
        for (u64 i = 0; i < old_table_size; i++) {
            Map_Hash_Entry entry = header->table[i];
            // this is incorrect when the key is a string. maybe a bool?
            void *entry_key = ((u8*)kv_array) + (entry.index * kv_pair_size);

            if (entry.hash == HASH_UNALLOCATED || entry.hash == HASH_DEAD) continue;

            s64 new_table_index = _Map_maybe_get_index_of_key(new_table, new_table_size, kv_array, entry.hash, entry_key, key_size, kv_pair_size);
            ASSERT(new_table_index >= 0);

            ASSERT(new_table[new_table_index].hash == HASH_UNALLOCATED);
            new_table[new_table_index] = entry;
        }

        header->capacity = new_capacity;
        header->table = new_table;
        kv_array = new_kv_array;
    }

    return kv_array;
}

void *_Map_Put(Map_Header *header, void *kv_array, void *key, u64 key_size, void *kv_pair, u64 kv_pair_size) {
    kv_array = _Map_Grow(header, kv_array, key_size, kv_pair_size, header->count + 1);

    u64 hash = _Map_hash_ptr_and_size(key, key_size);
    s64 index_in_table = _Map_maybe_get_index_of_key(header->table, Map_Table_Size(header), kv_array, hash, key, key_size, kv_pair_size);

    if (header->table[index_in_table].hash == HASH_UNALLOCATED) {
        header->table[index_in_table].hash = hash;
        header->table[index_in_table].index = header->count;

        Mem_Copy(((u8*)kv_array) + (header->count * kv_pair_size), kv_pair, kv_pair_size);
        header->count += 1;


    } else {
        // this key was already in the hashmap.
        ASSERT(header->table[index_in_table].hash == hash);

        u64 index_in_kv_pair_array = header->table[index_in_table].index;
        Mem_Copy(((u8*)kv_array) + (index_in_kv_pair_array * kv_pair_size), kv_pair, kv_pair_size);

        // dont increment the header->count.
        // the array might erroneously grow from this. i do not care.
    }

    return kv_array;
}



// ===================================================
//             Some Common Functions
// ===================================================

String Read_Entire_File(Arena *arena, String filename) {
    // this might init a page that is to small to contain the file.
    // But I work with Strings now. not c_strings
    Arena_Mark mark = Arena_Get_Mark(arena);
        FILE *file = fopen(String_To_C_Str(arena, filename), "rb");
    Arena_Set_To_Mark(arena, mark);

    String result = ZEROED;

    if (file) {
        fseek(file, 0, SEEK_END);
        s64 length = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (length >= 0) {
            result.length = (u64)length;
            result.data = (char*) Arena_Alloc_Clear(arena, result.length+1, false);
            if (result.data) {
                u64 read_bytes = fread(result.data, 1, result.length, file);
                ASSERT(read_bytes == result.length);
                result.data[result.length] = 0; // zero terminate for the love of the game.
            }
        }

        fclose(file);
    }

    return result;
}




#endif // BESTED_IMPLEMENTATION


#ifdef __cplusplus
    }
#endif


