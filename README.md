The repository contains all the necessary files to execute DRTRM,
a Distributed Run-Time Resource Management framework for parallel
applications on many-core systems [1]. The target platform of DRTRM,
is Intel Single Chip Cloud Computer (SCC), although the design is intended
for portability. In addition, it can be compiled to simulate execution
of a many-core system, using a process per core on a Linux system.
The design parameters and extensions of DRTRM are published at [1], [2], [3].

In order to be executed on Intel SCC the requirement is that Linux
is running on every SCC core and RCCEv2.0 has been installed. For
a successful compilation, the relevant crosscompile.sh file must
be sourced. For a successul compilation for Intel SCC the flags 
PLATFORM=SCC and API=gory are required. For the rest of the compilation 
options please contact billtsou AT microlab.ntua.gr.

The folder experiments-input contains all the input data that were used
in the experimental evaluation of DRTRM at [1], [2], [3]. They contain
application input data, application arrival scenarios, topology parameters
of DRTRM and Controller cores topology (See [1]). In addition scripts for
the parsing of execution log files are included.

The folder scripts includes scripts for execution different scenarios of DRTRM.
For example 'exec_scr_multi_multiple.sh' takes as parameters: (1) the name of
the executable file, (2) the path of examined experimental scenario, (3) the
name of the examined experimental scenario, (4) the number of controller cores of DRTRM,
(5) the type of input application, (6) the workload intensity of incoming applications,
(7) the number of incoming applications, (8) the operating frequency of SCC cores, (9)
initial cores of the scenario and (10) input application arrival rate.

[1] Tsoutsouras, V., Anagnostopoulos, I., Masouros, D. and Soudris, D., 2018. A Hierarchi-
cal Distributed Runtime Resource Management Scheme for NoC-Based Many-Cores.
ACM Transactions on Embedded Computing Systems (TECS), 17(3), p.65.

[2] Tsoutsouras, V., Xydis, S. and Soudris, D.J., 2018. Application-Arrival Rate Aware Dis-
tributed Run-Time Resource Management for Many-core Computing Platforms. IEEE
Transactions on Multi-Scale Computing Systems (TMSCS).

[3] Tsoutsouras, V., Masouros, D., Xydis, S. and Soudris, D., 2017. SoftRM: Self-Organized
Fault-Tolerant Resource Management for Failure Detection and Recovery in NoC
Based Many-Cores. ACM Transactions on Embedded Computing Systems (TECS),
16(5s), p.144.
