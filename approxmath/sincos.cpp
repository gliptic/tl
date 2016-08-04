#include "am.hpp"

namespace tl {

static double const PI4A = 7.85398125648498535156E-1;                             // 0x3fe921fb40000000, Pi/4 split into three parts
static double const PI4B = 3.77489470793079817668E-8;                             // 0x3e64442d00000000,
static double const PI4C = 2.69515142907905952645E-15;                            // 0x3ce8469898cc5170,
static double const M4PI = 1.273239544735162542821171882678754627704620361328125; // 4/pi

static double const sintab[6] = {
	1.58962301576546568060E-10, // 0x3de5d8fd1fd19ccd
	-2.50507477628578072866E-8, // 0xbe5ae5e5a9291f5d
	2.75573136213857245213E-6,  // 0x3ec71de3567d48a1
	-1.98412698295895385996E-4, // 0xbf2a01a019bfdf03
	8.33333333332211858878E-3,  // 0x3f8111111110f7d0
	-1.66666666666666307295E-1, // 0xbfc5555555555548
};

// cos coefficients
static double const costab[6] = {
	-1.13585365213876817300E-11, // 0xbda8fa49a0861a9b
	2.08757008419747316778E-9,   // 0x3e21ee9d7b4e3f05
	-2.75573141792967388112E-7,  // 0xbe927e4f7eac4bc6
	2.48015872888517045348E-5,   // 0x3efa01a019c844f5
	-1.38888888888730564116E-3,  // 0xbf56c16c16c14f91
	4.16666666666665929218E-2,   // 0x3fa555555555554b
};

VectorD2 sincos(double x) {
	f64 sin_sign = 1.0, cos_sign = 1.0;

	if (x < 0.0) {
		x = -x;
		sin_sign = -1.0;
	}

	int j = int(x * M4PI);
	f64 y = j;

	if (j & 1) {
		++j;
		y += 1.0;
	}

	j &= 7;
	if (j > 3) { // reflect in x axis
		j -= 4;
		sin_sign = -sin_sign;
		cos_sign = -cos_sign;
	}

	if (j > 1) {
		cos_sign = -cos_sign;
	}
	
	f64 z = ((x - y*PI4A) - y*PI4B) - y*PI4C; // Extended precision modular arithmetic
	f64 zz = z * z;
	f64 cos = 1.0 - 0.5*zz + zz*zz*((((((costab[0]*zz)+costab[1])*zz+costab[2])*zz+costab[3])*zz+costab[4])*zz+costab[5]);
	f64 sin = z + z*zz*((((((sintab[0]*zz)+sintab[1])*zz+sintab[2])*zz+sintab[3])*zz+sintab[4])*zz+sintab[5]);

	if (j == 1 || j == 2) {
		return VectorD2(sin * cos_sign, cos * sin_sign);
	} else {
		return VectorD2(cos * cos_sign, sin * sin_sign);
	}
}

}
