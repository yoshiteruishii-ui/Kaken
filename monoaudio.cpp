#include "wave.h"

namespace Wave {
	MonoAudio::MonoAudio(int length, int samplerate, cmpnum* data) {
		this->length = length;
		this->samplerate = samplerate;
		this->data = data;
	}
	MonoAudio::MonoAudio(int length, int samplerate) {
		this->length = length;
		this->samplerate = samplerate;
		this->data = new cmpnum[length];
	}
	MonoAudio::~MonoAudio() {
		delete[] this->data;
	}
	MonoAudio MonoAudio::fromWavedata(Wavedata data) {
		Wavedata mono = data.getmono();
		int length = mono.getlength();
		int samplerate = mono.getsamplerate();
		cmpnum* newdata = new cmpnum[length];
		short* olddata = mono.getdata();
		for (int i = 0; i < length; i++) {
			newdata[i].real = olddata[i] / 32768.0;
			newdata[i].imag = 0;
		}
		return MonoAudio(length, samplerate, newdata);
	}

	// --- 追加分 ---
	cmpnum* MonoAudio::getdata() {
		return this->data;
	}
	int MonoAudio::getlength() {
		return this->length;
	}
	int MonoAudio::getsamplerate() {
		return this->samplerate;
	}
}