#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/laplace_pri.h>
#include <asm/i387.h>

unsigned int max = 0xffffffff;

static inline float fastlog2 (float x)
{
  union { float f; unsigned int i; } vx = { x };
  union { unsigned int i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
  float y = vx.i;
  y *= 1.1920928955078125e-7f;

  return y - 124.22551499f
           - 1.498030302f * mx.f 
           - 1.72587999f / (0.3520887068f + mx.f);
}

static inline float fastlog (float x)
{
  return 0.69314718f * fastlog2 (x);
}

float get_fast_laplace(float u, float b)
{
	/*
	 * generating the laplace variables. 
	 * pdf: f(x) = (1/2b) * exp(-|x - u|/b) 
	 * x_x
	 */
	float result;
	float e = 0;
	unsigned int ran;
	kernel_fpu_begin();
	while(e == 0){
		get_random_bytes(&ran, sizeof(unsigned int));
		e = (float)ran / max;
	}
	e -= 0.5; /* (-1/2, 1/2] */
	if(e > 0){ /* X = u - (b * sgn(U) * ln(1 - 2|U|))  */
		result = u - b * fastlog(1 - 2 * e); 
	}
	else if(e == 0){
		result = u;
	}
	else{
		result = u + b * fastlog(1 + 2 * e);
	}
	kernel_fpu_end();
	return result;
}
EXPORT_SYMBOL(get_fast_laplace);
