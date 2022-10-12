AC_DEFUN(AC_SYS_POSIX,
[AC_REQUIRE([AC_CANONICAL_SYSTEM])dnl

AC_MSG_CHECKING(for POSIX System)

case "${host}" in
        *-*-solaris*)
		if grep _POSIX_VERSION [/usr/include/sys/unistd.h] >/dev/null 2>&1
		then
		  AC_MSG_RESULT(yes)
		  sys_posix=yes # If later tests want to check for Posix.
		  AC_DEFINE(_POSIX_C_SOURCE)
		else
		  AC_MSG_RESULT(no)
		  sys_posix=
		fi ;;

        *-*-linux*)
		if grep _POSIX_VERSION [/usr/include/unistd.h] >/dev/null 2>&1
		then
		  AC_MSG_RESULT(yes)
		  sys_posix=yes # If later tests want to check for ISC.
		  AC_DEFINE(_POSIX_SOURCE)
		else
		  AC_MSG_RESULT(no)
		  sys_posix=
		fi ;;
	*)
		AC_MSG_RESULT(no)
		sys_posix=no ;;
esac
])
