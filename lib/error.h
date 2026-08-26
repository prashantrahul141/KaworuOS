#ifndef _ERROR_H_
#define _ERROR_H_

#include "common_defs.h"
#include "types.h"

#define ERR_LIST                                                              \
	X(EOK, 0) /* ok! */                                                   \
	X(EPERM, 1) /* Operation not permitted */                             \
	X(ENOENT, 2) /* No such file or directory */                          \
	X(ESRCH, 3) /* No such process */                                     \
	X(EINTR, 4) /* Interrupted system call */                             \
	X(EIO, 5) /* Input/output error */                                    \
	X(ENXIO, 6) /* No such device or address */                           \
	X(E2BIG, 7) /* Argument list too long */                              \
	X(ENOEXEC, 8) /* Exec format error */                                 \
	X(EBADF, 9) /* Bad file descriptor */                                 \
	X(ECHILD, 10) /* No child processes */                                \
	/* yes, two error with same errno value */                            \
	X(EAGAIN, 11) /* Resource temporarily unavailable */                  \
	/* X(EWOULDBLOCK, 11) */ /* Resource temporarily unavailable */       \
	X(ENOMEM, 12) /* Cannot allocate memory */                            \
	X(EACCES, 13) /* Permission denied */                                 \
	X(EFAULT, 14) /* Bad address */                                       \
	X(ENOTBLK, 15) /* Block device required */                            \
	X(EBUSY, 16) /* Device or resource busy */                            \
	X(EEXIST, 17) /* File exists */                                       \
	X(EXDEV, 18) /* Invalid cross-device link */                          \
	X(ENODEV, 19) /* No such device */                                    \
	X(ENOTDIR, 20) /* Not a directory */                                  \
	X(EISDIR, 21) /* Is a directory */                                    \
	X(EINVAL, 22) /* Invalid argument */                                  \
	X(ENFILE, 23) /* Too many open files in system */                     \
	X(EMFILE, 24) /* Too many open files */                               \
	X(ENOTTY, 25) /* Inappropriate ioctl for device */                    \
	X(ETXTBSY, 26) /* Text file busy */                                   \
	X(EFBIG, 27) /* File too large */                                     \
	X(ENOSPC, 28) /* No space left on device */                           \
	X(ESPIPE, 29) /* Illegal seek */                                      \
	X(EROFS, 30) /* Read-only file system */                              \
	X(EMLINK, 31) /* Too many links */                                    \
	X(EPIPE, 32) /* Broken pipe */                                        \
	X(EDOM, 33) /* Numerical argument out of domain */                    \
	X(ERANGE, 34) /* Numerical result out of range */                     \
	/* yes same value again */                                            \
	/* X(EDEADLK, 35) */ /* Resource deadlock avoided */                  \
	X(EDEADLOCK, 35) /* Resource deadlock avoided */                      \
	X(ENAMETOOLONG, 36) /* File name too long */                          \
	X(ENOLCK, 37) /* No locks available */                                \
	X(ENOSYS, 38) /* Function not implemented */                          \
	X(ENOTEMPTY, 39) /* Directory not empty */                            \
	X(ELOOP, 40) /* Too many levels of symbolic links */                  \
	/* yes 41 is missing */                                               \
	X(ENOMSG, 42) /* No message of desired type */                        \
	X(EIDRM, 43) /* Identifier removed */                                 \
	X(ECHRNG, 44) /* Channel number out of range */                       \
	X(EL2NSYNC, 45) /* Level 2 not synchronized */                        \
	X(EL3HLT, 46) /* Level 3 halted */                                    \
	X(EL3RST, 47) /* Level 3 reset */                                     \
	X(ELNRNG, 48) /* Link number out of range */                          \
	X(EUNATCH, 49) /* Protocol driver not attached */                     \
	X(ENOCSI, 50) /* No CSI structure available */                        \
	X(EL2HLT, 51) /* Level 2 halted */                                    \
	X(EBADE, 52) /* Invalid exchange */                                   \
	X(EBADR, 53) /* Invalid request descriptor */                         \
	X(EXFULL, 54) /* Exchange full */                                     \
	X(ENOANO, 55) /* No anode */                                          \
	X(EBADRQC, 56) /* Invalid request code */                             \
	X(EBADSLT, 57) /* Invalid slot */                                     \
	X(EBFONT, 59) /* Bad font file format */                              \
	X(ENOSTR, 60) /* Device not a stream */                               \
	X(ENODATA, 61) /* No data available */                                \
	X(ETIME, 62) /* Timer expired */                                      \
	X(ENOSR, 63) /* Out of streams resources */                           \
	X(ENONET, 64) /* Machine is not on the network */                     \
	X(ENOPKG, 65) /* Package not installed */                             \
	X(EREMOTE, 66) /* Object is remote */                                 \
	X(ENOLINK, 67) /* Link has been severed */                            \
	X(EADV, 68) /* Advertise error */                                     \
	X(ESRMNT, 69) /* Srmount error */                                     \
	X(ECOMM, 70) /* Communication error on send */                        \
	X(EPROTO, 71) /* Protocol error */                                    \
	X(EMULTIHOP, 72) /* Multihop attempted */                             \
	X(EDOTDOT, 73) /* RFS specific error */                               \
	X(EBADMSG, 74) /* Bad message */                                      \
	X(EOVERFLOW, 75) /* Value too large for defined data type */          \
	X(ENOTUNIQ, 76) /* Name not unique on network */                      \
	X(EBADFD, 77) /* File descriptor in bad state */                      \
	X(EREMCHG, 78) /* Remote address changed */                           \
	X(ELIBACC, 79) /* Can not access a needed shared library */           \
	X(ELIBBAD, 80) /* Accessing a corrupted shared library */             \
	X(ELIBSCN, 81) /* .lib section in a.out corrupted */                  \
	X(ELIBMAX, 82) /* Attempting to link in too many shared libraries */  \
	X(ELIBEXEC, 83) /* Cannot exec a shared library directly */           \
	X(EILSEQ, 84) /* Invalid or incomplete multibyte or wide character */ \
	X(ERESTART, 85) /* Interrupted system call should be restarted */     \
	X(ESTRPIPE, 86) /* Streams pipe error */                              \
	X(EUSERS, 87) /* Too many users */                                    \
	X(ENOTSOCK, 88) /* Socket operation on non-socket */                  \
	X(EDESTADDRREQ, 89) /* Destination address required */                \
	X(EMSGSIZE, 90) /* Message too long */                                \
	X(EPROTOTYPE, 91) /* Protocol wrong type for socket */                \
	X(ENOPROTOOPT, 92) /* Protocol not available */                       \
	X(EPROTONOSUPPORT, 93) /* Protocol not supported */                   \
	X(ESOCKTNOSUPPORT, 94) /* Socket type not supported */                \
	/* yes */                                                             \
	/*	X(ENOTSUP, 95) */ /* Operation not supported */               \
	X(EOPNOTSUPP, 95) /* Operation not supported */                       \
	X(EPFNOSUPPORT, 96) /* Protocol family not supported */               \
	X(EAFNOSUPPORT, 97) /* Address family not supported by protocol */    \
	X(EADDRINUSE, 98) /* Address already in use */                        \
	X(EADDRNOTAVAIL, 99) /* Cannot assign requested address */            \
	X(ENETDOWN, 100) /* Network is down */                                \
	X(ENETUNREACH, 101) /* Network is unreachable */                      \
	X(ENETRESET, 102) /* Network dropped connection on reset */           \
	X(ECONNABORTED, 103) /* Software caused connection abort */           \
	X(ECONNRESET, 104) /* Connection reset by peer */                     \
	X(ENOBUFS, 105) /* No buffer space available */                       \
	X(EISCONN, 106) /* Transport endpoint is already connected */         \
	X(ENOTCONN, 107) /* Transport endpoint is not connected */            \
	X(ESHUTDOWN, 108) /* Cannot send after transport endpoint shutdown */ \
	X(ETOOMANYREFS, 109) /* Too many references: cannot splice */         \
	X(ETIMEDOUT, 110) /* Connection timed out */                          \
	X(ECONNREFUSED, 111) /* Connection refused */                         \
	X(EHOSTDOWN, 112) /* Host is down */                                  \
	X(EHOSTUNREACH, 113) /* No route to host */                           \
	X(EALREADY, 114) /* Operation already in progress */                  \
	X(EINPROGRESS, 115) /* Operation now in progress */                   \
	X(ESTALE, 116) /* Stale file handle */                                \
	X(EUCLEAN, 117) /* Structure needs cleaning */                        \
	X(ENOTNAM, 118) /* Not a XENIX named type file */                     \
	X(ENAVAIL, 119) /* No XENIX semaphores available */                   \
	X(EISNAM, 120) /* Is a named type file */                             \
	X(EREMOTEIO, 121) /* Remote I/O error */                              \
	X(EDQUOT, 122) /* Disk quota exceeded */                              \
	X(ENOMEDIUM, 123) /* No medium found */                               \
	X(EMEDIUMTYPE, 124) /* Wrong medium type */                           \
	X(ECANCELED, 125) /* Operation canceled */                            \
	X(ENOKEY, 126) /* Required key not available */                       \
	X(EKEYEXPIRED, 127) /* Key has expired */                             \
	X(EKEYREVOKED, 128) /* Key has been revoked */                        \
	X(EKEYREJECTED, 129) /* Key was rejected by service */                \
	X(EOWNERDEAD, 130) /* Owner died */                                   \
	X(ENOTRECOVERABLE, 131) /* State not recoverable */                   \
	X(ERFKILL, 132) /* Operation not possible due to RF-kill */           \
	X(EHWPOISON, 133) /* Memory page has hardware error */                \
	X(EMAXERR, 134) /* counter */

#define X(variant, num) variant = (num),
typedef isize errno_t;
enum : i32 { ERR_LIST };
#undef X

/*
 * Returns a string for the given error value.
 * The value could be negative in which case its magnitude will be taken.
 */
const i8 *str_err(isize err);

/*
 * Returns a string for the given error value.
 * Its the calle's responsibility to make sure the error code does exist.
 */
const i8 *str_err_raw(errno_t err);

/*
 * converting the value to u64 gives a small optimisation.
 * Any value between -1 to -EMAXERR will roll to (U64::MAX - value) which are
 * invalid pointer values in themselves, so we dont have to do double comparison
 * to check wehther the value lies between -1 and -EMAXERR, single comparison is
 * enough.
 */
#define IS_ERR_VALUE(x) unlikely((u64)(void *)(x) >= (u64) - EMAXERR)

/*
 * check whether the given value is an error or not
 */
static inline MUST_CHECK bool IS_ERR(const void *value)
{
	// NOLINTNEXTLINE
	return IS_ERR_VALUE((u64)value);
}

/*
 * Converts a negative error value to an error pointer
 */
static inline MUST_CHECK void *ERR_TO_PTR(isize error)
{
	return (void *)error;
}

/*
 * Converts a pointer value to an error
 */
static inline MUST_CHECK isize PTR_TO_ERR(const void *ptr)
{
	return (isize)ptr;
}

/*
 * helper for this pattern:
 *      if (IS_ERR(ptr))
 *          return PTR_TO_ERR(ptr);
 *      else
 *          return EOK;
 *
 */
static inline isize MUST_CHECK PTR_ERR_OR_OK(const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_TO_ERR(ptr);
	else
		return EOK;
}

#endif // _ERROR_H_
