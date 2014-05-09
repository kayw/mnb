#ifndef BUILDCONFIG_H_56701A13_F72B_49CD_A03F_3D010C8FF612
#define BUILDCONFIG_H_56701A13_F72B_49CD_A03F_3D010C8FF612

// grab from QT/corelib/global/qglobal.h
/*
   The operating system, must be one of: (_OS_x)

     DARWIN   - Darwin OS (synonym for _OS_MAC)
     //SYMBIAN  - Symbian
     //MSDOS    - MS-DOS and Windows
     //OS2      - OS/2
     //OS2EMX   - XFree86 on OS/2 (not PM)
     WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
     //WINCE    - WinCE (Windows CE 5.0)
     //CYGWIN   - Cygwin
     //SOLARIS  - Sun Solaris
     //HPUX     - HP-UX
     //ULTRIX   - DEC Ultrix
     LINUX    - Linux
     //FREEBSD  - FreeBSD
     //NETBSD   - NetBSD
     //OPENBSD  - OpenBSD
     //BSDI     - BSD/OS
     //IRIX     - SGI Irix
     //OSF      - HP Tru64 UNIX
     //SCO      - SCO OpenServer 5
     //UNIXWARE - UnixWare 7, Open UNIX 8
     //AIX      - AIX
     //HURD     - GNU Hurd
     //DGUX     - DG/UX
     //RELIANT  - Reliant UNIX
     //DYNIX    - DYNIX/ptx
     //QNX      - QNX
     //LYNX     - LynxOS
     //BSD4     - Any BSD 4.4 system
     //UNIX     - Any UNIX BSD/SYSV system
*/

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define _OS_DARWIN
#  define _OS_BSD4
#  ifdef __LP64__
#    define _OS_DARWIN64
#  else
#    define _OS_DARWIN32
#  endif
//#elif defined(__SYMBIAN32__) || defined(SYMBIAN)
//#  define Q_OS_SYMBIAN
//#  define Q_NO_POSIX_SIGNALS
//#  define QT_NO_GETIFADDRS
//#elif defined(__CYGWIN__)
//#  define Q_OS_CYGWIN
//#elif defined(MSDOS) || defined(_MSDOS)
//#  define Q_OS_MSDOS
//#elif defined(__OS2__)
//#  if defined(__EMX__)
//#    define Q_OS_OS2EMX
//#  else
//#    define Q_OS_OS2
//#  endif
#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define _OS_WIN32
#  define _OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  if defined(WINCE) || defined(_WIN32_WCE)
#    define _OS_WINCE
#  else
#    define _OS_WIN32
#  endif
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define _OS_WIN32
//#elif defined(__sun) || defined(sun)
//#  define Q_OS_SOLARIS
//#elif defined(hpux) || defined(__hpux)
//#  define Q_OS_HPUX
//#elif defined(__ultrix) || defined(ultrix)
//#  define Q_OS_ULTRIX
//#elif defined(sinix)
//#  define Q_OS_RELIANT
//#elif defined(__native_client__)
//#  define Q_OS_NACL
#elif defined(__linux__) || defined(__linux)
#  define _OS_LINUX
//#elif defined(__FreeBSD__) || defined(__DragonFly__)
//#  define Q_OS_FREEBSD
//#  define Q_OS_BSD4
//#elif defined(__NetBSD__)
//#  define Q_OS_NETBSD
//#  define Q_OS_BSD4
//#elif defined(__OpenBSD__)
//#  define Q_OS_OPENBSD
//#  define Q_OS_BSD4
//#elif defined(__bsdi__)
//#  define Q_OS_BSDI
//#  define Q_OS_BSD4
//#elif defined(__sgi)
//#  define Q_OS_IRIX
//#elif defined(__osf__)
//#  define Q_OS_OSF
//#elif defined(_AIX)
//#  define Q_OS_AIX
//#elif defined(__Lynx__)
//#  define Q_OS_LYNX
//#elif defined(__GNU__)
//#  define Q_OS_HURD
//#elif defined(__DGUX__)
//#  define Q_OS_DGUX
//#elif defined(__QNXNTO__)
//#  define Q_OS_QNX
//#elif defined(_SEQUENT_)
//#  define Q_OS_DYNIX
//#elif defined(_SCO_DS) /* SCO OpenServer 5 + GCC */
//#  define Q_OS_SCO
//#elif defined(__USLC__) /* all SCO platforms + UDK or OUDK */
//#  define Q_OS_UNIXWARE
//#elif defined(__svr4__) && defined(i386) /* Open UNIX 8 + GCC */
//#  define Q_OS_UNIXWARE
//#elif defined(__INTEGRITY)
//#  define Q_OS_INTEGRITY
//#elif defined(VXWORKS) /* there is no "real" VxWorks define - this has to be set in the mkspec! */
//#  define Q_OS_VXWORKS
//#elif defined(__MAKEDEPEND__)
#else
#  error "unsupport OS by MNB - talk to "
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64) || defined(Q_OS_WINCE)
#  define Q_OS_WIN
#endif

#if defined(Q_OS_DARWIN)
#  define Q_OS_MAC /* Q_OS_MAC is mostly for compatibility, but also more clear */
#  define Q_OS_MACX /* Q_OS_MACX is only for compatibility.*/
#  if defined(Q_OS_DARWIN64)
#     define Q_OS_MAC64
#  elif defined(Q_OS_DARWIN32)
#     define Q_OS_MAC32
#  endif
#endif

#ifdef QT_AUTODETECT_COCOA
#  ifdef Q_OS_MAC64
#    define QT_MAC_USE_COCOA 1
#    define QT_BUILD_KEY QT_BUILD_KEY_COCOA
#  else
#    define QT_BUILD_KEY QT_BUILD_KEY_CARBON
#  endif
#endif

#if defined(Q_WS_MAC64) && !defined(QT_MAC_USE_COCOA) && !defined(QT_BUILD_QMAKE) && !defined(QT_BOOTSTRAPPED)
#error "You are building a 64-bit application, but using a 32-bit version of Qt. Check your build configuration."
#endif

#if defined(Q_OS_MSDOS) || defined(Q_OS_OS2) || defined(Q_OS_WIN)
#  undef Q_OS_UNIX
#elif !defined(Q_OS_UNIX)
#  define Q_OS_UNIX
#endif

#if defined(Q_OS_DARWIN) && !defined(QT_LARGEFILE_SUPPORT)
#  define QT_LARGEFILE_SUPPORT 64
#endif

#ifdef Q_OS_DARWIN
#  ifdef MAC_OS_X_VERSION_MIN_REQUIRED
#    undef MAC_OS_X_VERSION_MIN_REQUIRED
#  endif
#  define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_4
#  include <AvailabilityMacros.h>
#  if !defined(MAC_OS_X_VERSION_10_3)
#     define MAC_OS_X_VERSION_10_3 MAC_OS_X_VERSION_10_2 + 1
#  endif
#  if !defined(MAC_OS_X_VERSION_10_4)
#       define MAC_OS_X_VERSION_10_4 MAC_OS_X_VERSION_10_3 + 1
#  endif
#  if !defined(MAC_OS_X_VERSION_10_5)
#       define MAC_OS_X_VERSION_10_5 MAC_OS_X_VERSION_10_4 + 1
#  endif
#  if !defined(MAC_OS_X_VERSION_10_6)
#       define MAC_OS_X_VERSION_10_6 MAC_OS_X_VERSION_10_5 + 1
#  endif
#  if !defined(MAC_OS_X_VERSION_10_7)
#       define MAC_OS_X_VERSION_10_7 MAC_OS_X_VERSION_10_6 + 1
#  endif
#  if !defined(MAC_OS_X_VERSION_10_8)
#       define MAC_OS_X_VERSION_10_8 MAC_OS_X_VERSION_10_7 + 1
#  endif
#  if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_8)
#    warning "This version of Mac OS X is unsupported"
#  endif
#endif

#ifdef __LSB_VERSION__
#  if __LSB_VERSION__ < 40
#    error "This version of the Linux Standard Base is unsupported"
#  endif
#ifndef QT_LINUXBASE
#  define QT_LINUXBASE
#endif
#endif

/*
   The compiler, must be one of: (Q_CC_x)

     SYM      - Digital Mars C/C++ (used to be Symantec C++)
     MWERKS   - Metrowerks CodeWarrior
     MSVC     - Microsoft Visual C/C++, Intel C++ for Windows
     BOR      - Borland/Turbo C++
     WAT      - Watcom C++
     GNU      - GNU C++
     COMEAU   - Comeau C++
     EDG      - Edison Design Group C++
     OC       - CenterLine C++
     SUN      - Forte Developer, or Sun Studio C++
     MIPS     - MIPSpro C++
     DEC      - DEC C++
     HPACC    - HP aC++
     USLC     - SCO OUDK and UDK
     CDS      - Reliant C++
     KAI      - KAI C++
     INTEL    - Intel C++ for Linux, Intel C++ for Windows
     HIGHC    - MetaWare High C/C++
     PGI      - Portland Group C++
     GHS      - Green Hills Optimizing C++ Compilers
     GCCE     - GCCE (Symbian GCCE builds)
     RVCT     - ARM Realview Compiler Suite
     NOKIAX86 - Nokia x86 (Symbian WINSCW builds)
     CLANG    - C++ front-end for the LLVM compiler


   Should be sorted most to least authoritative.
*/

#if defined(__ghs)
# define Q_OUTOFLINE_TEMPLATE inline

/* the following are necessary because the GHS C++ name mangling relies on __*/
# define Q_CONSTRUCTOR_FUNCTION0(AFUNC) \
   static const int AFUNC ## _init_variable_ = AFUNC();
# define Q_CONSTRUCTOR_FUNCTION(AFUNC) Q_CONSTRUCTOR_FUNCTION0(AFUNC)
# define Q_DESTRUCTOR_FUNCTION0(AFUNC) \
    class AFUNC ## _dest_class_ { \
    public: \
       inline AFUNC ## _dest_class_() { } \
       inline ~ AFUNC ## _dest_class_() { AFUNC(); } \
    } AFUNC ## _dest_instance_;
# define Q_DESTRUCTOR_FUNCTION(AFUNC) Q_DESTRUCTOR_FUNCTION0(AFUNC)

#endif

/* Symantec C++ is now Digital Mars */
#if defined(__DMC__) || defined(__SC__)
#  define Q_CC_SYM
/* "explicit" semantics implemented in 8.1e but keyword recognized since 7.5 */
#  if defined(__SC__) && __SC__ < 0x750
#    define Q_NO_EXPLICIT_KEYWORD
#  endif
#  define Q_NO_USING_KEYWORD

#elif defined(__MWERKS__)
#  define Q_CC_MWERKS
#  if defined(__EMU_SYMBIAN_OS__)
#    define Q_CC_NOKIAX86
#  endif
/* "explicit" recognized since 4.0d1 */

#elif defined(_MSC_VER)
#  define Q_CC_MSVC
#  define Q_CC_MSVC_NET
#  define Q_CANNOT_DELETE_CONSTANT
#  define Q_OUTOFLINE_TEMPLATE inline
#  define Q_NO_TEMPLATE_FRIENDS
#  define Q_ALIGNOF(type) __alignof(type)
#  define Q_DECL_ALIGN(n) __declspec(align(n))
/* Intel C++ disguising as Visual C++: the `using' keyword avoids warnings */
#  if defined(__INTEL_COMPILER)
#    define Q_CC_INTEL
#  endif
/* MSVC does not support SSE/MMX on x64 */
#  if (defined(Q_CC_MSVC) && defined(_M_X64))
#    undef QT_HAVE_SSE
#    undef QT_HAVE_MMX
#    undef QT_HAVE_3DNOW
#  endif

#if defined(Q_CC_MSVC) && _MSC_VER >= 1600
#      define Q_COMPILER_RVALUE_REFS
#      define Q_COMPILER_AUTO_TYPE
#      define Q_COMPILER_LAMBDA
#      define Q_COMPILER_DECLTYPE
//  MSCV has std::initilizer_list, but do not support the braces initialization
//#      define Q_COMPILER_INITIALIZER_LISTS
#  endif


#elif defined(__BORLANDC__) || defined(__TURBOC__)
#  define Q_CC_BOR
#  define Q_INLINE_TEMPLATE
#  if __BORLANDC__ < 0x502
#    define Q_NO_BOOL_TYPE
#    define Q_NO_EXPLICIT_KEYWORD
#  endif
#  define Q_NO_USING_KEYWORD

#elif defined(__WATCOMC__)
#  define Q_CC_WAT

/* Symbian GCCE */
#elif defined(__GCCE__)
#  define Q_CC_GCCE
#  define QT_VISIBILITY_AVAILABLE
#  if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__)
#    define QT_HAVE_ARMV6
#  endif

/* ARM Realview Compiler Suite
   RVCT compiler also defines __EDG__ and __GNUC__ (if --gnu flag is given),
   so check for it before that */
#elif defined(__ARMCC__) || defined(__CC_ARM)
#  define Q_CC_RVCT
#  if __TARGET_ARCH_ARM >= 6
#    define QT_HAVE_ARMV6
#  endif
/* work-around for missing compiler intrinsics */
#  define __is_empty(X) false
#  define __is_pod(X) false
#elif defined(__GNUC__)
#  define Q_CC_GNU
#  define Q_C_CALLBACKS
#  if defined(__MINGW32__)
#    define Q_CC_MINGW
#  endif
#  if defined(__INTEL_COMPILER)
/* Intel C++ also masquerades as GCC 3.2.0 */
#    define Q_CC_INTEL
#  endif
#  if defined(__clang__)
/* Clang also masquerades as GCC 4.2.1 */
#    define Q_CC_CLANG
#  endif
#  ifdef __APPLE__
#    define Q_NO_DEPRECATED_CONSTRUCTORS
#  endif
#  if __GNUC__ == 2 && __GNUC_MINOR__ <= 7
#    define Q_FULL_TEMPLATE_INSTANTIATION
#  endif
/* GCC 2.95 knows "using" but does not support it correctly */
#  if __GNUC__ == 2 && __GNUC_MINOR__ <= 95
#    define Q_NO_USING_KEYWORD
#    define QT_NO_STL_WCHAR
#  endif
#  if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#    define Q_ALIGNOF(type)   __alignof__(type)
#    define Q_TYPEOF(expr)    __typeof__(expr)
#    define Q_DECL_ALIGN(n)   __attribute__((__aligned__(n)))
#  endif
#  if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#    define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#    define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)
#  endif
/* GCC 3.1 and GCC 3.2 wrongly define _SB_CTYPE_MACROS on HP-UX */
#  if defined(Q_OS_HPUX) && __GNUC__ == 3 && __GNUC_MINOR__ >= 1
#    define Q_WRONG_SB_CTYPE_MACROS
#  endif
/* GCC <= 3.3 cannot handle template friends */
#  if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)
#    define Q_NO_TEMPLATE_FRIENDS
#  endif
/* Apple's GCC 3.1 chokes on our streaming qDebug() */
#  if defined(Q_OS_DARWIN) && __GNUC__ == 3 && (__GNUC_MINOR__ >= 1 && __GNUC_MINOR__ < 3)
#    define Q_BROKEN_DEBUG_STREAM
#  endif
#  if (defined(Q_CC_GNU) || defined(Q_CC_INTEL)) && !defined(QT_MOC_CPP)
#    define Q_PACKED __attribute__ ((__packed__))
#    define Q_NO_PACKED_REFERENCE
#    ifndef __ARM_EABI__
#      define QT_NO_ARM_EABI
#    endif
#  endif
#  if defined(__GXX_EXPERIMENTAL_CXX0X__)
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 403
       /* C++0x features supported in GCC 4.3: */
#      define Q_COMPILER_RVALUE_REFS
#      define Q_COMPILER_DECLTYPE
#    endif
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 404
       /* C++0x features supported in GCC 4.4: */
#      define Q_COMPILER_VARIADIC_TEMPLATES
#      define Q_COMPILER_AUTO_TYPE
#      define Q_COMPILER_EXTERN_TEMPLATES
#      define Q_COMPILER_DEFAULT_DELETE_MEMBERS
#      define Q_COMPILER_CLASS_ENUM
#      define Q_COMPILER_INITIALIZER_LISTS
#    endif
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 405
       /* C++0x features supported in GCC 4.5: */
#      define Q_COMPILER_LAMBDA
#      define Q_COMPILER_UNICODE_STRINGS
#    endif
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406
       /* C++0x features supported in GCC 4.6: */
#      define Q_COMPILER_CONSTEXPR
#    endif


#endif /* BUILDCONFIG_H_56701A13_F72B_49CD_A03F_3D010C8FF612 */

