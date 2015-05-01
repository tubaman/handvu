dnl
dnl ACC_SETVAR(variable, value)
dnl
dnl  Set variable no matter what
dnl
AC_DEFUN([ACC_SETVAR],[
#  echo "  forcing $1 to \"$2\""
  $1="$2"
])dnl

dnl
dnl ACC_ADDTO(variable, value)
dnl
dnl  Add value to variable
dnl
AC_DEFUN([ACC_ADDTO],[
  if test "x$$1" = "x"; then
#    echo "  setting $1 to \"$2\""
    $1="$2"
  else
    apr_addto_bugger="$2"
    for i in $apr_addto_bugger; do
      apr_addto_duplicate="0"
      for j in $$1; do
        if test "x$i" = "x$j"; then
          apr_addto_duplicate="1"
          break
        fi
      done
      if test $apr_addto_duplicate = "0"; then
#        echo "  adding \"$i\" to $1"
        $1="$$1 $i"
      fi
    done
  fi
])dnl

dnl
dnl ACC_REMOVEFROM(variable, value)
dnl
dnl Remove a value from a variable
dnl
AC_DEFUN([ACC_REMOVEFROM],[
  if test "x$$1" = "x$2"; then
#    echo "  nulling $1"
    $1=""
  else
    apr_new_bugger=""
    apr_removed=0
    for i in $$1; do
      if test "x$i" != "x$2"; then
        apr_new_bugger="$apr_new_bugger $i"
      else
        apr_removed=1
      fi
    done
    if test $apr_removed = "1"; then
a#      echo "  removed \"$2\" from $1"
      $1=$apr_new_bugger
    fi
  fi
]) dnl

dnl
dnl ACC_FIND_FILE(filename, directory_list, postfix_list, resultvar, warn)
dnl
dnl look for a file in the directory_list, possibly postpending one of the
dnl items from the postfix_list, return the first match in resultvar;
dnl if warn is not empty, no success results in a warning only, otherwise
dnl an unsuccessful search is a fatal error
dnl
AC_DEFUN([ACC_FIND_FILE_IN_DIRS],
[
  AC_MSG_CHECKING(for $1)
  $4=""
  for root in $2; do
        if test -f "$root/$1"; then
            $4="$root"
            break
        fi
        for postfix in $3; do
            if test -f "$root/$postfix/$1"; then
                $4="$root/$postfix"
                break
            fi
        done
        if test "x$$4" != "x"; then
            break;
        fi
  done
  if test "x$$4" = "x"; then
	if test "x$5" = "x"; then
		echo no
        	AC_MSG_ERROR([$1 not found])
	else
		AC_MSG_RESULT(no)
	fi
  else
	AC_MSG_RESULT(yes: $$4)
  fi
]) dnl

dnl
dnl ACC_FIND_DIRS(message, directory_list, resultvar)
dnl
dnl look for existance of the first directory from the list
dnl
AC_DEFUN([ACC_FIND_DIRS],
[
  AC_MSG_CHECKING(for $1)
  $3=""
  for p in $2; do
    if test -d "$p"; then
        $3="$$3 $p"
    fi
  done
  if test "x$$3" = "x"; then
	echo no
	AC_MSG_ERROR([$1 not found])
  fi
  AC_MSG_RESULT(found:$$3)
]) dnl

