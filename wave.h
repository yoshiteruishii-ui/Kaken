#ifndef WAVEH
#define WAVEH

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdlib.h>

#define PI 3.14159265358979

namespace Wave {
	// --- 1. まず cmpnum を定義する ---
	typedef struct {
		double real;
		double imag;
	} cmpnum;

	// ヘルパー関数のプロトタイプ宣言
	cmpnum addCmp(cmpnum A, cmpnum B);
	cmpnum scCmp(cmpnum A, double x);
	cmpnum mulCmp(cmpnum A, cmpnum B);
	int bitrev(int num, int size);
	void FFT(cmpnum* data, cmpnum* result, int N);

	// --- 2. 次に Wavedata クラス ---
	class Wavedata {
	public:
		Wavedata(int length, int channels, int samplerate, short* data);
		Wavedata(int length, int channels, int samplerate);
		~Wavedata();
		int getchannels();
		int getlength();
		int getsamplerate();
		short* getdata();
		void setdata(short* data);
		void save(std::string path);
		void derive();
		Wavedata getmono();
		static Wavedata load(std::string path);
	private:
		short* data;
		int length;
		int channels;
		int samplerate;
	};

	// --- 3. 最後に MonoAudio クラス (cmpnum を使用可能) ---
	class MonoAudio {
	public:
		MonoAudio(int length, int samplerate, cmpnum* data);
		MonoAudio(int length, int samplerate);
		~MonoAudio();
		static MonoAudio fromWavedata(Wavedata wave);
		
		// 追加したメソッド
		cmpnum* getdata();
		int getlength();
		int getsamplerate();
	private:
		cmpnum* data;
		int length;
		int samplerate;
	};
}

#endif