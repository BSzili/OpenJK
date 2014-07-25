#include <math.h>
#if _MSC_VER
# include <float.h>
# pragma fenv_access (on)
#elif defined(__amigaos4__)
// from glibc 2.19
typedef enum { FE_TONEAREST = 0, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD } eRoundType;
static inline int fegetround(void)
{
	int result;
	asm volatile("mcrfs 7,7\n\t"
		"mfcr	%0" : "=r"(result) : : "cr7");
	return result & 3;
}

static inline int fesetround(int round)
{
	if ((unsigned int)round < 2)
	{
		asm volatile("mtfsb0 30");
		if ((unsigned int)round == 0)
			asm volatile("mtfsb0 31");
		else
			asm volatile("mtfsb1 31");
	}
	else
	{
		asm volatile ("mtfsb1 30");
		if ((unsigned int)round == 2)
			asm volatile("mtfsb0 31");
		else
			asm volatile("mtfsb1 31");
	}

	return 0;
}
#else
# include <fenv.h>
#endif

#if _MSC_VER
static inline float roundfloat(float n)
{
	return (n < 0.0f) ? ceilf(n - 0.5f) : floorf(n + 0.5f);
}
#endif

extern "C"
void Sys_SnapVector(float *v)
{
#if _MSC_VER
	unsigned int oldcontrol;
	unsigned int newcontrol;

	_controlfp_s(&oldcontrol, 0, 0);
	_controlfp_s(&newcontrol, _RC_NEAR, _MCW_RC);

	v[0] = roundfloat(v[0]);
	v[1] = roundfloat(v[1]);
	v[2] = roundfloat(v[2]);

	_controlfp_s(&newcontrol, oldcontrol, _MCW_RC);
#else
	// pure C99
	int oldround = fegetround();
	fesetround(FE_TONEAREST);

	v[0] = nearbyintf(v[0]);
	v[1] = nearbyintf(v[1]);
	v[2] = nearbyintf(v[2]);

	fesetround(oldround);
#endif
}
