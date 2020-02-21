#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define handle_perror(msg)				\
	do {						\
		fprintf(stderr,				\
			"%s:%d:%s(): PERROR: ",		\
			__FILE__, __LINE__, __func__);	\
		perror(msg);				\
	} while (0)

static inline
void delta_timespec(struct timespec *interval,
		    const struct timespec *start,
		    const struct timespec *end)
{
	interval->tv_sec = end->tv_sec - start->tv_sec;
	interval->tv_nsec = end->tv_nsec - start->tv_nsec;
	if (interval->tv_nsec < 0) {
		interval->tv_nsec += 1000000000;
		interval->tv_sec -= 1;
	}
}

static inline
unsigned long long timespec_to_llu(const struct timespec *ts)
{
	return ts->tv_sec * 1000000000ULL + ts->tv_nsec;
}

static inline
void timespec_copy(struct timespec *dest, const struct timespec *src)
{
	dest->tv_sec = src->tv_sec;
	dest->tv_nsec = src->tv_nsec;
}


#define NR_CPU 8
#define NR_ITER 5

#define low_cpu 4
#define high_cpu 8

int main(int argc, char *argv[])
{
	char *on = "1";
	char *off = "0";

	struct timespec timing_on[NR_ITER];
	struct timespec timing_off[NR_ITER];
	
	char path[NR_CPU][1024];
	int fd[NR_CPU];
	for (size_t idx_cpu = 0; idx_cpu < NR_CPU; ++idx_cpu) {
		snprintf(path[idx_cpu], sizeof path[idx_cpu],
			 "/sys/devices/system/cpu/cpu%zu/online",
			 idx_cpu);
		
		if ((fd[idx_cpu] = open(path[idx_cpu], O_WRONLY)) < 0) {
			char error_msg[sizeof path[idx_cpu] + 8];
			snprintf(error_msg, sizeof error_msg,
				 "open(%s)", path[idx_cpu]);
			handle_perror(error_msg);
		}
	}

	for (size_t idx_iter = 0; idx_iter < NR_ITER; ++idx_iter) {
		///
		sleep(2);
		///
	
		struct timespec start_time_off;
		clock_gettime(CLOCK_REALTIME, &start_time_off);

/* #pragma GCC unroll 8 */
		for (size_t idx_cpu_ = low_cpu; idx_cpu_ < high_cpu; ++idx_cpu_) {
			if (write(fd[idx_cpu_], off, sizeof off) < 0) {
				char error_msg[sizeof path[idx_cpu_] + 8];
				snprintf(error_msg, sizeof error_msg,
					 "write(%s)", path[idx_cpu_]);
				handle_perror(error_msg);
			}
		}
		struct timespec stop_time_off;
		clock_gettime(CLOCK_REALTIME, &stop_time_off);

		struct timespec delta_off;
		delta_timespec(&delta_off, &start_time_off, &stop_time_off);
		timespec_copy(&timing_off[idx_iter], &delta_off);
		/* printf("%10ld.%09ld\n", timing_off[idx_iter].tv_sec, timing_off[idx_iter].tv_nsec); */

		///
		sleep(2);
		/// 
		
		struct timespec start_time_on;
		clock_gettime(CLOCK_REALTIME, &start_time_on);

/* #pragma GCC unroll 8 */
		for (size_t idx_cpu_ = low_cpu; idx_cpu_ < high_cpu; ++idx_cpu_) {
			if (write(fd[idx_cpu_], on, sizeof on) < 0) {
				char error_msg[sizeof path[idx_cpu_] + 8];
				snprintf(error_msg, sizeof error_msg,
					 "write(%s)", path[idx_cpu_]);
				handle_perror(error_msg);
			}
		}
		struct timespec stop_time_on;
		clock_gettime(CLOCK_REALTIME, &stop_time_on);

		struct timespec delta_on;
		delta_timespec(&delta_on, &start_time_on, &stop_time_on);
		timespec_copy(&timing_on[idx_iter], &delta_on);
		/* printf("%10ld.%09ld\n", timing_on[idx_iter].tv_sec, timing_on[idx_iter].tv_nsec); */
	}
	
	for (size_t idx_cpu = 0; idx_cpu < NR_CPU; ++idx_cpu) {
		close(fd[idx_cpu]);
	}

	double sum_off = 0.;
	double sum_on = 0.;
	
	for (size_t idx_iter = 0; idx_iter < NR_ITER; ++idx_iter) {
		printf("%10ld.%09ld %10ld.%09ld\n",
		       timing_off[idx_iter].tv_sec, timing_off[idx_iter].tv_nsec,
		       timing_on[idx_iter].tv_sec, timing_on[idx_iter].tv_nsec);

		sum_off += timespec_to_llu(&timing_off[idx_iter]);
		sum_on += timespec_to_llu(&timing_on[idx_iter]);
	}

	sum_off = sum_off / NR_ITER;
	sum_on = sum_on / NR_ITER;
	
	struct timespec sum_off_;
	sum_off_.tv_sec = sum_off / 1000000000;
	sum_off_.tv_nsec = ((int)sum_off % 1000000000);
	printf("avg off => %10ld.%09ld\n", sum_off_.tv_sec, sum_off_.tv_nsec);

	struct timespec sum_on_;
	sum_on_.tv_sec = sum_on / 1000000000;
	sum_on_.tv_nsec = ((int)sum_on % 1000000000);
	printf("avg on => %10ld.%09ld\n", sum_on_.tv_sec, sum_on_.tv_nsec);

	return EXIT_SUCCESS;
}

