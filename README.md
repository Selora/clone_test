# clone_test
Code used to test the lttng/tracecompass LXC/namespace analysis


#HOWTO :
Compile this program using gcc :
gcc pidns_init_sleep.c -o pidns_init_sleep

#Trace this program :
To run a program creating pid namespace, you need to be root (or to have user namespace enabled)

To have the VPID and VPPID of cloned children's task in lttng event, you need this version of lttng-modules :

Before doing the following commands, ensure that you have removed ALL of lttng-modules packages before.
Also, check that the session deamon is not running (lsmod | grep lttng) -> should not return anything

git clone https://github.com/Selora/lttng-modules
make
sudo make modules_install
sudo depmod -a

First, setup LTTng (as root) with :
lttng create namespace_session -o /path/to/trace
lttng enable-event --kernel --channel container_info sched_process_fork
lttng start

Then run the program you just compiled (as root too):
./pidns_init_sleep

Stop lttng :
lttng stop

You should get an output similar to this :
babeltrace /path/to/trace 

[03:02:40.979995502] (+0.001726402) redwind sched_process_fork: { cpu_id = 0 }, { parent_comm = "pidns_init_slee", parent_tid = 19562, parent_pid = 19562, child_comm = "pidns_init_slee", child_tid = 19563, child_vtid = 1, child_pid = 19563, child_vppid = 0 }
[03:02:40.981285037] (+0.001289535) redwind sched_process_fork: { cpu_id = 0 }, { parent_comm = "pidns_init_slee", parent_tid = 19562, parent_pid = 19562, child_comm = "pidns_init_slee", child_tid = 19564, child_vtid = 1, child_pid = 19564, child_vppid = 0 }
[03:02:40.982209282] (+0.000924245) redwind sched_process_fork: { cpu_id = 1 }, { parent_comm = "pidns_init_slee", parent_tid = 19563, parent_pid = 19563, child_comm = "pidns_init_slee", child_tid = 19565, child_vtid = 2, child_pid = 19565, child_vppid = 1 }
[03:02:40.983831464] (+0.001622182) redwind sched_process_fork: { cpu_id = 0 }, { parent_comm = "pidns_init_slee", parent_tid = 19565, parent_pid = 19565, child_comm = "pidns_init_slee", child_tid = 19566, child_vtid = 1, child_pid = 19566, child_vppid = 0 }

