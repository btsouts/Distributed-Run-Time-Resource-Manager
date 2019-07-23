#! /bin/sh

if [ -z "${PATH}" ]
then
    PATH="/opt/intel_cc_80/bin:/opt/i386-unknown-linux-gnu/bin"; export PATH
else
    PATH="/opt/intel_cc_80/bin:/opt/i386-unknown-linux-gnu/bin:$PATH"; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]
then
    LD_LIBRARY_PATH="/opt/intel_cc_80/lib:/opt/i386-unknown-linux-gnu/lib"; export LD_LIBRARY_PATH
else
    LD_LIBRARY_PATH="/opt/intel_cc_80/lib:/opt/i386-unknown-linux-gnu/lib:$LD_LIBRARY_PATH"; export LD_LIBRARY_PATH
fi

if [ -z "${MANPATH}" ]
then
    MANPATH="/opt/intel_cc_80/man:/opt/i386-unknown-linux-gnu/man":$(manpath); export MANPATH
else
    MANPATH="/opt/intel_cc_80/man:/opt/i386-unknown-linux-gnu/man:${MANPATH}"; export MANPATH
fi

if [ -z "${INTEL_LICENSE_FILE}" ]
then
	INTEL_LICENSE_FILE="/opt/intel_cc_80/licenses:/opt/intel/licenses:${HOME}/intel/licenses"; export INTEL_LICENSE_FILE 
else
	INTEL_LICENSE_FILE="${INTEL_LICENSE_FILE}:/opt/intel_cc_80/licenses:${HOME}/intel/licenses"; export INTEL_LICENSE_FILE
fi

