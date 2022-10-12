dnl @synopsis AC_COMPILE_ANSI
dnl
dnl Set compiler to ANSI mode.
dnl This macro must be put after AC_PROG_CC and AC_PROG_CXX in
dnl configure.in
dnl
dnl
AC_DEFUN([AC_COMPILE_ANSI],
[AC_MSG_CHECKING(ansi compiler mode)
if test -n "$CXX"
then
  if test "$GXX" = "yes"
  then
    ac_compile_ansi_opt='-ansi'
  fi
  CXXFLAGS="$CXXFLAGS $ac_compile_ansi_opt"
  ac_compile_ansi_msg="$ac_compile_ansi_opt for C++"
fi

if test -n "$CC"
then
  if test "$GCC" = "yes"
  then
    ac_compile_ansi_opt='-ansi'
  fi
  CFLAGS="$CFLAGS $ac_compile_ansi_opt"
  ac_compile_ansi_msg="$ac_compile_ansi_msg $ac_compile_ansi_opt for C"
fi
AC_MSG_RESULT($ac_compile_ansi_msg)
unset ac_compile_ansi_msg
unset ac_compile_ansi_opt
])
