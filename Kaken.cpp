#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "wave.h"

// peakの値と周波数、さらに配列のインデックスをセットで管理するための構造体
struct PeakInfo {
    double peak;
    double frequency;
    int index; // 追加: 前後の要素にアクセスするため、元の配列のインデックスを保存
};

int main() {
    // 1. WAVファイルの読み込み
    Wave::Wavedata wave = Wave::Wavedata::load("garage.wav");
    if (wave.getlength() == 0) {
        std::cout << "ファイルの読み込みに失敗したか、無効なWAVデータです。" << std::endl;
        return -1;
    }

    // 2. WavedataをMonoAudioに変換
    Wave::MonoAudio mono = Wave::MonoAudio::fromWavedata(wave);

    // 3. FFTにかけるデータサイズ N (2のべき乗)
    int N = 4096;
    
    // --- 読み込み時のエラー判定 ---
    if (mono.getlength() < N) {
        std::cout << "エラー: WAVファイルの長さがFFTサイズより短いです。" << std::endl;
        return -1;
    }

    // 4. FFT用の配列を確保
    Wave::cmpnum* fft_result = new Wave::cmpnum[N];
    double samplerate = mono.getsamplerate();

    int offset = 0; 
    
    // 要望1: 新たな変数counterの定義
    int counter = 0;
    
    // 要望3: 全ての計算結果を保存する2次元配列 (sound[counter][i])
    std::vector<std::vector<PeakInfo>> sound;

    // --- FFT解析のループ ---
    while (true) {
        if (mono.getlength() < N + (int)(N * offset / 2)) {
            break;
        }

        // FFT実行
        Wave::FFT(mono.getdata() + (int)(N * offset / 2), fft_result, N);

        std::vector<double> frequencies;
        std::vector<double> magnitudes;

        for (int i = 0; i < N / 2; i++) {
            double freq = (double)i * samplerate / N;
            double mag = std::sqrt(fft_result[i].real * fft_result[i].real + fft_result[i].imag * fft_result[i].imag);
            frequencies.push_back(freq);
            magnitudes.push_back(mag);
        }
        
        // 要望2: 2倍音、3倍音のmagnitudesを掛ける操作
        // 小さい周波数から操作するため、新しいメモリは使わずに上書き
        int max_idx = (N / 2) - 1;
        for (int i = 0; i <= max_idx; i++) {
            // 限界を超えた場合は一番大きい周波数のindex(max_idx)を採用する
            int idx2 = std::min(i * 2, max_idx);
            int idx3 = std::min(i * 3, max_idx);
            magnitudes[i] = magnitudes[i] * magnitudes[idx2] * magnitudes[idx3];
        }

        std::vector<PeakInfo> peak_list;

        for (size_t i = 0; i < frequencies.size() - 2; i++) {
            double mag_i   = magnitudes[i];
            double mag_i1  = magnitudes[i + 1];
            double mag_i2  = magnitudes[i + 2];

            double fre_i1  = frequencies[i + 1];

            // 山になっている部分をリストに追加
            if (mag_i1 - mag_i > 0 && mag_i1 - mag_i2 > 0) {
                PeakInfo info;
                info.peak = mag_i1;
                info.frequency = fre_i1;
                info.index = i + 1; // 元の配列のインデックスも記憶させておく
                peak_list.push_back(info);
            }
        }

        // このループ(counter)用の音データを格納する配列（10個分を0で初期化）
        std::vector<PeakInfo> current_sound(10, {0.0, 0.0, 0});

        if (!peak_list.empty()) {
            // peakの値が大きい順（降順）にソート
            std::sort(peak_list.begin(), peak_list.end(), [](const PeakInfo& a, const PeakInfo& b) {
                return a.peak > b.peak; 
            });
            
            // 要望3: 前後の周波数とpeak値から加重平均で計算して current_sound に入れる
            int limit = std::min((int)peak_list.size(), 10);
            for (int i = 0; i < limit; i++) {
                int idx = peak_list[i].index;
                
                // 元の配列から1段階下(freq0, mag0)と1段階上(freq1, mag1)を取得
                double freq0 = frequencies[idx - 1];
                double mag0  = magnitudes[idx - 1];
                double freq1 = frequencies[idx + 1];
                double mag1  = magnitudes[idx + 1];
                
                double peak_center = peak_list[i].peak;
                double freq_center = peak_list[i].frequency;

                // 新たなpeakとfrequencyの計算
                current_sound[i].peak = peak_center + mag0 + mag1;
                if (current_sound[i].peak > 0) { // ゼロ除算防止
                    current_sound[i].frequency = (freq_center * peak_center + freq0 * mag0 + freq1 * mag1) / current_sound[i].peak;
                }
            }
        }
        
        // 完成したこのタイミングのデータを全体配列に追加
        sound.push_back(current_sound);
        
        offset++;
        // 要望1: ループを1回行うごとにcounterをカウントアップ
        counter++; 
    }
    
    // メモリの解放
    delete[] fft_result;

    // 要望4: 全てのcounterの sound[counter][0].peak の最大値 (maxsound) を求める
    double maxsound = 0.0;
    for (int c = 0; c < counter; c++) {
        if (sound[c][0].peak > maxsound) {
            maxsound = sound[c][0].peak;
        }
    }

    // maxsoundの1/128より小さい場合、鳴っていないものとしてpeakを0にする
    double threshold = maxsound / 128;
    for (int c = 0; c < counter; c++) {
        for (int i = 0; i < 10; i++) {
            if (sound[c][i].peak < threshold) {
                sound[c][i].peak = 0.0;
                // ピークが0なら周波数も0にする（ノイズカット）
                sound[c][i].frequency = 0.0;
            }
        }
    }

    // 要望5: 全てのcounterの sound[counter][0].peak と frequency を表示
    for (int c = 0; c < counter; c++) {
        std::cout  << c*N/samplerate/2 << " | peak: " << sound[c][0].peak << " | freq: " << sound[c][0].frequency << std::endl;
    }//なぜか2で割るとちょうどいい

    return 0;
}