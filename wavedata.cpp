#include "wave.h"


namespace Wave {
	Wavedata::Wavedata(int length, int channels, int samplerate, short* data) {
		this->data = data;
		this->length = length;
		this->channels = channels;
		this->samplerate = samplerate;
	}

	Wavedata::Wavedata(int length, int channels, int samplerate) {
		this->data = new short[length * channels];
		this->length = length;
		this->channels = channels;
		this->samplerate = samplerate;
	}

	Wavedata::~Wavedata() {
		delete[] this->data;
	}

	int Wavedata::getchannels() {
		return this->channels;
	}

	int Wavedata::getlength() {
		return this->length;
	}

	int Wavedata::getsamplerate() {
		return this->samplerate;
	}

	short* Wavedata::getdata() {
		return this->data;
	}

	void Wavedata::setdata(short* data) {
		delete[] this->data;
		this->data = data;
	}

	void Wavedata::derive() {
		for (int s = 0; s < this->length; s++) {
			for (int c = 0; c < this->channels; c++) {
				this->data[this->channels * s + c] = 4 * (this->data[this->channels * (s + 1) + c] - this->data[this->channels * s + c]);
			}
		}
	}

	Wavedata Wavedata::getmono() {
		short* data = new short[this->length];
		double temp;
		for (int s = 0; s < this->length; s++) {
			temp = 0;
			for (int c = 0; c < this->channels; c++) {
				temp += this->data[this->channels * s + c];
			}
			temp /= this->channels;
			data[s] = temp;
		}
		return Wavedata(this->length, 1, this->samplerate, data);
	}

	void Wavedata::save(std::string path) {
		std::ofstream ofs(path, std::ios_base::binary);

		if (ofs) {
			unsigned long size = 36 + 2 * this->channels * this->length; // ファイルサイズ - 8
			unsigned long fmtsize = 0x10; // リニアPCM
			unsigned short fmt = 0x01; // PCM
			unsigned long bytespersec = 2 * this->samplerate * this->channels; // 16bitなので2bytes * samplerate * channels
			unsigned short blocksize = 2 * this->channels;
			unsigned short bitspersample = 16;
			unsigned long datasize = this->length * blocksize;
			ofs.write("RIFF", 4);
			ofs.write((const char*)&size, 4);
			ofs.write("WAVE", 4);
			ofs.write("fmt ", 4);
			ofs.write((const char*)&fmtsize, 4);
			ofs.write((const char*)&fmt, 2);
			ofs.write((const char*)&this->channels, 2);
			ofs.write((const char*)&this->samplerate, 4);
			ofs.write((const char*)&bytespersec, 4);
			ofs.write((const char*)&blocksize, 2);
			ofs.write((const char*)&bitspersample, 2);
			ofs.write("data", 4);
			ofs.write((const char*)&datasize, 4);

			for (int i = 0; i < this->length * this->channels; i++) {
				ofs.write((const char*)(this->data + i), 2);
			}
			ofs.close();
		}
		else {
			std::cout << "ファイルオープンに失敗しました。(" << path << ")" << std::endl;
		}
	}

	Wavedata Wavedata::load(std::string path) {
		std::ifstream ifs(path, std::ios_base::binary);

		if (ifs) {
			unsigned long sRIFF = 0x46464952;
			unsigned long sWAVE = 0x45564157;
			unsigned long sfmt_ = 0x20746d66;
			unsigned long sdata = 0x61746164;

			char fourbytes[5];
			fourbytes[4] = 0;

			unsigned long riffsize;
			unsigned long chunksize;

			unsigned short fmt;
			unsigned short channels;
			unsigned long samplerate;
			unsigned long bytespersec;
			unsigned short blocksize;
			unsigned short bitspersample;

			ifs.read(fourbytes, 4);
			if (*((int*)fourbytes) != 0x46464952) { // "RIFF"判定
				std::cout << "?????" << std::endl;
				return Wavedata(0, 0, 0);
			}
			ifs.read((char*)&riffsize, 4);
			ifs.read(fourbytes, 4);
			if (*((int*)fourbytes) != 0x45564157) { // "WAVE"判定
				std::cout << "not Wave" << std::endl;
				return Wavedata(0, 0, 0);
			}

			std::streampos pos;
			int loop = 1;

			while (loop) {
				ifs.read(fourbytes, 4);
				switch (*((int*)fourbytes)) {
				case 0x20746d66: // "fmt "判定
					ifs.read((char*)&chunksize, 4);
					std::cout << "[fmt ]: " << chunksize << std::endl;
					ifs.read((char*)&fmt, 2);
					ifs.read((char*)&channels, 2);
					ifs.read((char*)&samplerate, 4);
					ifs.read((char*)&bytespersec, 4);
					ifs.read((char*)&blocksize, 2);
					ifs.read((char*)&bitspersample, 2);

					std::cout << "fmt        : " << fmt << std::endl;
					std::cout << "channels   : " << channels << std::endl;
					std::cout << "samplerate : " << samplerate << std::endl;
					std::cout << "bytes/sec  : " << bytespersec << std::endl;
					std::cout << "blocksize  : " << blocksize << std::endl;
					std::cout << "bits/sample: " << bitspersample << std::endl;

					pos = ifs.tellg();
					ifs.seekg(pos + std::streampos(chunksize - 16)); // もし拡張パラメータがあったら飛ばす。
					break;
				case 0x61746164: // "data"判定
					goto read_data;
				default: // それか、それ以外かのそれ以外
					ifs.read((char*)&chunksize, 4);
					std::cout << "[" << fourbytes << "]: " << chunksize << std::endl;
					pos = ifs.tellg();
					ifs.seekg(pos + std::streampos(chunksize));
				}
			}
		read_data:
			ifs.read((char*)&chunksize, 4);
			std::cout << "[data]: " << chunksize << std::endl;

			short* data = new short[chunksize / 2];


			for (int i = 0; i < chunksize / 2; i++) {
				ifs.read((char*)(data + i), 2);
			}

			ifs.close();
			return Wavedata(chunksize / 2 / channels, channels, samplerate, data);
		}
		else {
			std::cout << "ファイルオープンに失敗しました。(" << path << ")" << std::endl;
			return Wavedata(0, 0, 0);
		}
	}
}