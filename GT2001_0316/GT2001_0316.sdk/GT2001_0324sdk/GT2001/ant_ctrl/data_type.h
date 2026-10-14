
#ifndef __DATA_TYPE_H
#define __DATA_TYPE_H

#include <stdint.h>
#include <stdbool.h>


#define BOOL bool
//#define FALSE	false
//#define TRUE	true

typedef int32_t         s32;
typedef int16_t         s16;
typedef int8_t          s8;

typedef const int32_t   sc32;  
typedef const int16_t   sc16;  
typedef const int8_t    sc8;  

typedef uint32_t        u32;
typedef uint16_t        u16;
typedef uint8_t         u8;

typedef const uint32_t  uc32;  
typedef const uint16_t  uc16;  
typedef const uint8_t   uc8; 


//typedef enum {FALSE = 0, TRUE = !FALSE} BOOL, bool;
//typedef enum {DISABLE = 0U, ENABLE = !DISABLE} MotorStatue;

#endif
