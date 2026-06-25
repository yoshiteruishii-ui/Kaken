#include "wave.h"

namespace Wave {
	cmpnum addCmp(cmpnum A, cmpnum B) {
		cmpnum result;
		result.real = A.real + B.real;
		result.imag = A.imag + B.imag;
		return result;
	}

	cmpnum scCmp(cmpnum A, double x) {
		cmpnum result;
		result.real = A.real * x;
		result.imag = A.imag * x;
		return result;
	}

	cmpnum mulCmp(cmpnum A, cmpnum B) {
		cmpnum result;
		result.real = A.real * B.real - A.imag * B.imag;
		result.imag = A.real * B.imag + A.imag * B.real;
		return result;
	}

	int bitrev(int num, int size) {
		int result = 0;
		for (int i = 0; i < size; i++) {
			result = (result << 1) | ((num >> i) & 1);
		}
		return result;
	}

	void FFT(cmpnum* data, cmpnum* result, int N) {
		cmpnum* tempdata = (cmpnum*)malloc(sizeof(cmpnum) * N);
		for (int i = 0; i < N; i++) {
			tempdata[i] = scCmp(data[i], (1.0 - cos(2.0 * PI * i / N))/2.0);
		}
		cmpnum* Wtable = (cmpnum*)malloc(sizeof(cmpnum) * N / 2);
		for (int i = 0; i < N / 2; i++) {
			Wtable[i].real = cos(2 * PI * i / N);
			Wtable[i].imag = sin(2 * PI * i / N);
		}

		int stages = log2(N);
		for (int i = 0; i < stages; i++) {
			int bscale = 1 << (stages - (i + 1));
			for (int j = 0; j < (1 << i); j++) {
				for (int k = 0; k < bscale; k++) {
					tempdata[2 * bscale * j + k] = addCmp(tempdata[2 * bscale * j + k], tempdata[2 * bscale * j + k + bscale]);
					tempdata[2 * bscale * j + k + bscale] = mulCmp(addCmp(tempdata[2 * bscale * j + k], scCmp(tempdata[2 * bscale * j + k + bscale], -2)), Wtable[k * (1 << i)]);
				}
			}
		}

		for (int i = 0; i < N; i++) {
			result[bitrev(i, stages)] = scCmp(tempdata[i], 2.0 / N);
		}
		free(Wtable);
		free(tempdata);
	}
}