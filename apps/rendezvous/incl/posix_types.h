#ifndef _ASM_X86_64_POSIX_TYPES_H
#define _ASM_X86_64_POSIX_TYPES_H

typedef unsigned long   __kernel_dev_t;
typedef unsigned long   __kernel_ino_t;
typedef unsigned int    __kernel_mode_t;
typedef unsigned long   __kernel_nlink_t;
typedef long            __kernel_off_t;
typedef int             __kernel_pid_t;
typedef int             __kernel_ipc_pid_t;
typedef unsigned int    __kernel_uid_t;
typedef unsigned int    __kernel_gid_t;
typedef unsigned long   __kernel_size_t;
typedef long            __kernel_ssize_t;
typedef long            __kernel_ptrdiff_t;
typedef long            __kernel_time_t;
typedef long            __kernel_suseconds_t;
typedef long            __kernel_clock_t;
typedef int             __kernel_daddr_t;
typedef char *          __kernel_caddr_t;
typedef unsigned short  __kernel_uid16_t;
typedef unsigned short  __kernel_gid16_t;

//Ron Add
typedef unsigned long   size_t;
#define ssize_t size_t
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int socklen_t;

#endif
