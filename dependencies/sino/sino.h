/*
 * A speed-improved simplex noise algorithm for 2D, 3D and 4D in C.
 * Ported to C by Bram Stolk.
 *
 * Based on SimplexNoise.java from Stefan Guvstavson, which was:
 * Based on example code by Stefan Gustavson (stegu@itn.liu.se).
 *   Optimisations by Peter Eastman (peastman@drizzle.stanford.edu).
 *   Better rank ordering method by Stefan Gustavson in 2012.
 *
 * This code was placed in the public domain by its original author,
 * Stefan Gustavson. You may use it as you see fit, but
 * attribution is appreciated.
 */

typedef float scalar;

extern void sino_init( void );
extern void sino_exit( void );
extern scalar sino_2d( scalar xin, scalar yin );
extern scalar sino_3d( scalar xin, scalar yin, scalar zin );
extern scalar sino_4d( scalar x, scalar y, scalar z, scalar w );

