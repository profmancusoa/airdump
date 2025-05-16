#ifndef __ERROR_H
#define __ERROR_H

#include <stdarg.h>
#include <sys/capability.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
 
extern bool has_net_admin_capability(void);
extern void err_msg(const char *format, ...);
extern void err_quit(const char *format, ...);
extern void err_sys(const char *format, ...);

#endif