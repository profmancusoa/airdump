#ifndef __HELPER_H
#define __HELPER_H

#include <stdio.h>
#include <stdbool.h>

/**
 * Compute exponentially weighted moving average
 * @mavg:	old value of the moving average
 * @sample:	new sample to update @mavg
 * @weight:	decay factor for new samples, 0 < weight <= 1
 */
static inline double ewma(double mavg, double sample, double weight)
{
	return mavg == 0 ? sample : weight * mavg + (1.0 - weight) * sample;
}

/* Integer units - similar to %g for float. */
static inline char *int_counts(uint32_t count)
{
	static char result[0x10];

	if (count < 1e3)
		sprintf(result, "%u", count);
	else if (count < 1e9)
		sprintf(result, "%.0fk", count/1e3);
	else
		sprintf(result, "%.1G", (double)count);

	return result;
}

/* SI units -- see units(7) */
static inline char *byte_units(const double bytes)
{
	static char result[0x100];

	if (bytes >= 1 << 30)
		sprintf(result, "%0.2lf GiB", bytes / (1 << 30));
	else if (bytes >= 1 << 20)
		sprintf(result, "%0.2lf MiB", bytes / (1 << 20));
	else if (bytes >= 1 << 10)
		sprintf(result, "%0.2lf KiB", bytes / (1 << 10));
	else
		sprintf(result, "%.0lf B", bytes);

	return result;
}

/* map 0.0 <= ratio <= 1.0 into min..max */
static inline double map_val(double ratio, double min, double max)
{
	return min + ratio * (max - min);
}

extern const char *conf_ifname(void);

#endif