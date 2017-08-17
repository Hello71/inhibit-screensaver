#!/bin/sh

# POSIX shell doesn't have >() so we need a subshell
exec 3>&1
{
    exec 9>&1 1>&3
    # do work
    echo Working...
    sleep 10
    # still works if we die!
    kill -9 $$
} | ./inhibit-screensaver posix-sh-inhibitor-example testing

# otherwise, here the inhibition would be lifted
echo test
