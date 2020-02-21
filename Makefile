main: main.c
	gcc -Wall -Wextra -O3 -funroll-loops main.c -o main

all:main

run: all
	echo "performance" | sudo tee -a /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
	echo 1400000 | sudo tee -a /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq
	echo "performance" | sudo tee -a /sys/devices/system/cpu/cpufreq/policy4/scaling_governor
	echo 2000000 | sudo tee -a /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq

	sudo taskset -c 0 ./main

	echo "ondemand" | sudo tee -a /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
	echo 1400000 | sudo tee -a /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq
	echo "ondemand" | sudo tee -a /sys/devices/system/cpu/cpufreq/policy4/scaling_governor
	echo 2000000 | sudo tee -a /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq

