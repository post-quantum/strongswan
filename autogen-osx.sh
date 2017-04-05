
LIBTOOLIZE=`which glibtoolize 2>/dev/null`
case "$LIBTOOLIZE" in
    /* )    ;;
    *  )    LIBTOOLIZE=`which libtoolize 2>/dev/null`
        case "$LIBTOOLIZE" in
            /* )    ;;
            *  )    LIBTOOLIZE=libtoolize
                ;;
        esac
        ;;
esac

autoheader &&
$LIBTOOLIZE --force &&
aclocal -I /usr/local/Cellar/gettext/0.19.8.1/share/aclocal &&
automake -a &&
autoconf

