#
# Makefile for privfs
#

CFLAGS_privfs_core.o += -msse -mpreferred-stack-boundary=4
CFLAGS_privfs_lap.o += -msse -mpreferred-stack-boundary=4

obj-y += privfs.o

privfs-y += privfs_comm.o
privfs-y += privfs_proc.o
privfs-y += privfs_core.o
privfs-y += privfs_lap.o
