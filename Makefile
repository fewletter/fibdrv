CONFIG_MODULE_SIG = n
TARGET_MODULE := fibdrv

obj-m := $(TARGET_MODULE).o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) client
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) client out
load:
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null

client: client.c
	$(CC) -o $@ $^

test: all
	sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"
	sudo sh performance.sh
	sudo sh -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
	$(MAKE) unload
	$(MAKE) load
	sudo taskset 0x1 ./test_time
	$(MAKE) unload

test_all:
	sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"
	sudo sh performance.sh
	sudo sh -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
	$(MAKE) unload
	$(MAKE) load
	python3 scripts/auto_run.py
	$(MAKE) unload

compile_test:
	gcc -c test_time.c -o test_time.o
	gcc test_time.o -o test_time

PRINTF = env printf
PASS_COLOR = \e[32;01m
NO_COLOR = \e[0m
pass = $(PRINTF) "$(PASS_COLOR)$1 Passed [-]$(NO_COLOR)\n"

check: all
	$(MAKE) unload
	$(MAKE) load
	sudo ./client > out
	$(MAKE) unload
	@diff -u out scripts/expected.txt && $(call pass)
	@scripts/verify.py
