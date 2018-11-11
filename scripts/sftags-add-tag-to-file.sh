#!/bin/bash

# ver: 2018-11-11-0

# $1 : filename
# $2 : tag

g_debug=0
#g_debug=1 # :debug:

g_sftags_file="$HOME/.sftags2"

file_get_length()
{
    local len=$(stat -c "%s" "$1")
    if ((rc)); then
        echo "Error getting length for file $1"
        exit 2
    fi
    echo "$len"
}


file_get_sha256sum()
{
    local checksum=$(sha256sum "$1")
    if ((rc)); then
        echo "Error getting sha256sum for file $1"
        exit 2
    fi
    echo "$checksum" | sed "s/ .*//"
}


sftags_display_fatal_error()
{
    kdialog --error "$@"
    exit 2
}


sftags_prerequisites()
{
    which kdialog > /dev/null || return 1
}


# $1 - tag
# $2 - file
sftags_apply_tag_to_file()
{
    [ -z "$1" ] && exit 5
    [ -z "$2" ] && exit 5
    if [ ! -f "$2" ]; then
        echo "No such file: $2"
        return 1
    fi
    local len=$(file_get_length "$2")
    local sum=$(file_get_sha256sum "$2")
    echo "f|$sum|$len|$1|$2" >> "$g_sftags_file"
}


main()
{
    if sftags_prerequisites; then
        fpath=$(realpath "$1")
        if [ "$?" -ne 0 ]; then
            sftags_display_fatal_error "Invalid path: $1"
        fi
        tag=$(kdialog --inputbox "tag")
        if [ "$?" -ne 0 ]; then
            sftags_display_fatal_error "Error getting tag for file $fpath"
        fi
        ((g_debug)) && echo "applying tag $tag to file $fpath"
        sftags_apply_tag_to_file "$tag" "$fpath" || sftags_display_fatal_error "Error applying tag $tag to file $fpath"
    else
        sftags_display_fatal_error "Missing prerequisites"
    fi
}


run_tests()
{
    file_get_length "/etc/passwd" || echo Error
    file_get_sha256sum "/etc/passwd" || echo Error
}

if [ "$1"_ == "-t_" ]; then
    run_tests
else
    main "$@"
fi
