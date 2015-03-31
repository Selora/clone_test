# Lttng traces samples for Container Analysis
Code used to test the lttng/tracecompass container analysis


#HOWTO :
There is traces samples with nested LXC containers as well as traces taken with a simple programm creating nested tasks in different PID namespaces.
To use those traces using tracecompass, get the latest container analysis patch from :
https://github.com/Selora/tracecompass/tree/container

#The program pid_init_sleep
Compile it using gcc :
```
gcc pidns_init_sleep.c -o pidns_init_sleep
```

#Creating custom traces:
To run a program creating pid namespace, you need to be root or to have user namespace enabled.

To have the necessary informations in lttng events, you need this version of lttng-modules :

Before doing the following commands, ensure that you have removed ALL of lttng-modules packages before.
Also, check that the session deamon is not running (lsmod | grep lttng) -> should not return anything

```
git clone https://github.com/Selora/lttng-modules
make
sudo make modules_install
sudo depmod -a
```

First, setup LTTng (as root) with :
```
lttng create namespace_session -o /path/to/trace
lttng enable-event -k -a
lttng start
```
Note that the method above uses all events. This could lead to really big traces.

Then run the program you just compiled (as root too):
```
./pidns_init_sleep
```

Stop lttng :
```
lttng stop
```

If everything is fine, you should events that looks like this:
```
babeltrace /path/to/trace 

(...) sched_process_fork: { cpu_id = 1 }, { parent_comm = "bash", parent_tid = 30332, parent_pid = 30332, parent_ns_inum = 4026532363, child_comm = "bash", child_tid = 3568, child_vtid = 2338, child_pid = 3568, child_ns_inum = 4026532363 }
(...) lttng_statedump_process_state: { cpu_id = 2 }, { tid = 3, vtid = 3, pid = 3, vpid = 3, ppid = 2, vppid = 2, name = "ksoftirqd/0", type = 1, mode = 5, submode = 0, status = 5, ns_level = 0, ns_inum = 4026531836 }
```
Note that in the events above, the INode of the PID namespaces has been added over the sched_process_fork events and the lttng_statedump_process_state events
