#!/bin/bash

# acquire the inhibition
exec 9> >(./inhibit-screensaver bash-inhibitor-example testing)

# do work
echo Working...
sleep 10

# still works if we die
kill -9 $$

# otherwise, uninhibit like so (or just exit)
exec 9>&-
