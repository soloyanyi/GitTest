//
// Created by root on 7/19/18.
//

#ifndef SONGCOMPARE_FEATURES_H
#define SONGCOMPARE_FEATURES_H

#include <vector>
#include <algorithm>
#include <eigen3/Eigen/Dense>
#include "Num.h"

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h> // for the isSilent function
#include <essentia/pool.h>

#include <samplerate.h>

using namespace essentia;
using namespace standard;

//开始计算距离的起始位置
const int START_INDEX = 0;
//原唱与伴奏之间的差距
const float TIMES = 1.4;

//float比较相等的阈值范围
const float MIN_THR = 0.0000001f;
//预设的帧范围
const int MATRIX_ROW = 1800;
const int MATRIX_COL = 1025;
const int MATRIX_RANGE = 6;

//对分贝的过滤值
const int MAX_DB = 8000;
const int MIN_ORI_DB = -16;     //原唱
const int MIN_HUMAN_DB = -20;   //人声
const int SET_MIN_DB = -100;

const int FRAMESIZE = 2048;
const int HOPSIZE = 512;
const float SAMPLERATE = 44100.0;
const float ORRSAMPLE = 16000.0;

const int MAXBUFFLEN = 400000;
const int RATIOSIZE = 3;

enum {GREATE = 0, SMALL = 1};

//欧氏距离
float euclideanDistance(const Eigen::MatrixXi &s1, const Eigen::MatrixXi &s2);
//汉明距离
int hammingDistance(const Eigen::MatrixXi &s1, const Eigen::MatrixXi &s2);
//编辑距离
int editDistance(const Eigen::MatrixXi &s1, const Eigen::MatrixXi &s2);

//将特征进行编码
Eigen::MatrixXi featureDecoding(const Eigen::MatrixXf &input);

//  特征处理模块的函数
Eigen::MatrixXf original_music(std::string path1, std::string path2);
Eigen::MatrixXf original_music_buffer(std::string path1, std::string path2);

Eigen::MatrixXf features(std::string path3);
Eigen::MatrixXf features_buffer(std::string path3);
Eigen::MatrixXf features_buffer(const char *org_buffer, unsigned int org_len, float cmp_length);

//获取stft后的矩阵
static void stft(std::string path, Eigen::MatrixXf &out);
static void stft(std::vector<Real> &audio, Eigen::MatrixXf &out);

static void readFile(std::string path, std::vector<Real> &audio);

void bufferToFloat(const char *buffer, unsigned int len, std::vector<Real> &audio);

#endif //SONGCOMPARE_FEATURES_H
