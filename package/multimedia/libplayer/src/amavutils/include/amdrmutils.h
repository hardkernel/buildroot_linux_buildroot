#ifndef AMDRM_UTILS_H
#define AMDRM_UTILS_H

struct tvp_region
{
	unsigned long long start;
	unsigned long long end;
	int mem_flags;
};

#define TVP_MM_ENABLE_FLAGS_FOR_4K 0x02

extern int tvp_mm_enable(int flags);
extern int tvp_mm_disable(int flags);
extern int tvp_mm_get_mem_region(struct tvp_region* region, int region_size);

#endif

