#!/bin/bash

if [ -z "${PATH}" ]
then
    PATH="/opt/icc-8.1.038/bin:/opt/i386-unknown-linux-gnu/bin"; export PATH
else
    PATH="/opt/icc-8.1.038/bin:/opt/i386-unknown-linux-gnu/bin:$PATH"; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]
then
    LD_LIBRARY_PATH="/opt/icc-8.1.038/lib:/opt/i386-unknown-linux-gnu/lib"; export LD_LIBRARY_PATH
else
    LD_LIBRARY_PATH="/opt/icc-8.1.038/lib:/opt/i386-unknown-linux-gnu/lib:$LD_LIBRARY_PATH"; export LD_LIBRARY_PATH
fi

if [ -z "${MANPATH}" ]
then
    MANPATH="/opt/i386-unknown-linux-gnu/man":$(manpath); export MANPATH
else
    MANPATH="/opt/i386-unknown-linux-gnu/man:${MANPATH}"; export MANPATH
fi

