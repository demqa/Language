#ifndef DEBUG_LIB_H

#define DEBUG_LIB_H

#include <stdio.h>

#define __OUT__ stderr

#define PRINT_LINE     do{ fprintf(__OUT__, "I'm at %s at line %d in %s\n", __PRETTY_FUNCTION__, __LINE__, __FILE__); }while(0)

#define PRINT_PTR(ptr) do{ fprintf(__OUT__, "pointer %s at %p at line %d\n", #ptr, ptr, __LINE__);                    }while(0)

#define PRINT_C(char)  do{ fprintf(__OUT__, "[%s:%d] %s = %c\n",  __PRETTY_FUNCTION__, __LINE__, #char, char);        }while(0)

#define PRINT_S(str)   do{ fprintf(__OUT__, "[%s:%d] %s = %s\n",  __PRETTY_FUNCTION__, __LINE__, #str, str);          }while(0)

#define PRINT(str)     do{ fprintf(__OUT__, "%s\n", #str); }while(0);

#define PRINT_UL(num)  do{ fprintf(__OUT__, "[%s:%d] %s = %lu\n", __PRETTY_FUNCTION__, __LINE__, #num, num);          }while(0)

#define PRINT_D(num)   do{ fprintf(__OUT__, "[%s:%d] %s = %d\n",  __PRETTY_FUNCTION__, __LINE__, #num, num);          }while(0)

#define PRINT_X(num)   do{ fprintf(__OUT__, "[%s:%d] %s = %x\n",  __PRETTY_FUNCTION__, __LINE__, #num, num);          }while(0)

#define PRINT_SM(s, n) do{ fprintf(__OUT__, "[%s:%d] = %.*s\n",   __PRETTY_FUNCTION__, __LINE__, n, s);                                         }while(0)

#endif // DEBUG_LIB_H
