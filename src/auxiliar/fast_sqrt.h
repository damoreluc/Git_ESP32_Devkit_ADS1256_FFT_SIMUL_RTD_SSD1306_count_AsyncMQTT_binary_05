#ifndef _FAST_SQRT_H
#define _FAST_SQRT_H

/* John Carmack's Unusual Fast Inverse Square Root (Quake III) */
/* https://valinsky.me/posts/quake3-fast-inverse-square-root/  */

// fast 1/sqrt(x)
float Q_rsqrt( float number );

// fast sqrt(1/x)
float Q_sqrt( float number );

#endif