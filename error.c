 #include "error.h"
/*
 * For displaying warning messages that are unrelated to system calls,
 * outside ncurses mode for %WARN_DISPLAY_DELAY seconds.
 */
void err_msg(const char *format, ...) {
	va_list argp;

	va_start(argp, format);
	vwarnx(format, argp);
	va_end(argp);
}


/*
 * Abort on fatal error unrelated to system call.
 */
void err_quit(const char *format, ...) {
	va_list argp;

	va_start(argp, format);
	vwarnx(format, argp);
	va_end(argp);
	exit(EXIT_FAILURE);
}

/*
 * Abort on fatal error related to system call.
 */
void err_sys(const char *format, ...) {
	err_quit(format);
}
