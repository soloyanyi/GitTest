//
// Created by root on 7/19/18.
//

#include <iomanip>
#include <samplerate.h>
#include "Features.h"

static clock_t startTime, endTime;

float euclideanDistance(const Eigen::MatrixXi &s1, const Eigen::MatrixXi &s2){
    int distance = 0;
    int tmp;
    int length = s1.size() > s2.size() ? s2.size() : s1.size();
    const int *data1 = s1.data();
    const int *data2 = s2.data();
    for (int i = START_INDEX; i < length; ++i) {
        if(data1[i] == data2[i])
            continue;
        else {
            tmp = data1[i] - data2[i];
            if(tmp > 0)
                distance += tmp;
            else
                distance -= tmp;
        }
    }
    return distance;
}

int hammingDistance(const Eigen::MatrixXi &s1, const Eigen::MatrixXi &s2){
    int distance = 0;
    int length = s1.size() > s2.size() ? s2.size() : s1.size();
    const int *data1 = s1.data();
    const int *data2 = s2.data();
    for (int i = START_INDEX; i < length; ++i) {
        if(data1[i] == data2[i])
            continue;
        else
            distance += 1;
    }
    return distance;
}

int editDistance(const Eigen::MatrixXi &s1, const Eigen::MatrixXi &s2){
    int n = s1.size();
    int m = s2.size();
    if (m == 0) return n;
    if (n == 0) return m;
    const int *data1 = s1.data() + START_INDEX;
    const int *data2 = s2.data() + START_INDEX; //此处从START_INDEX处开始，这个矩阵是一维的，即使按列存储也是可以的
    //Construct a matrix

    //从START_INDEX处开始计算
    n -= START_INDEX;
    m -= START_INDEX;

    typedef std::vector<std::vector<int> >  Tmatrix;
    Tmatrix matrix(n + 1);
    for (int i = 0; i <= n; i++)  matrix[i].resize(m + 1);

    //step 2 Initialize

    for (int i = 0; i <= n; i++) matrix[i][0] = i;
    for (int i = 0; i <= m; i++) matrix[0][i] = i;

    //step 3
    for (int i = 1; i <= n; i++)
    {
//        float si = data1[i-1];
        //step 4
        for (int j = 1; j <= m; j++)
        {
            if(data1[i-1] == data2[j-1])
                matrix[i][j] = matrix[i-1][j-1];
            else{
                int nm =  std::min(matrix[i-1][j], matrix[i][j-1]);
                matrix[i][j] = 1 + std::min(matrix[i-1][j-1], nm);
            }

        }
    }//step7
    return matrix[n][m];
}

Eigen::MatrixXi featureDecoding(const Eigen::MatrixXf &input){
    Eigen::MatrixXi result;
    result.resize(1, input.size());
    const  float *data = input.data();
    float base = data[0];
    result(0,0) = 0;
    for (int i = 1; i < input.size(); ++i) {
        if(data[i] > base){
            result(0,i) = 1;
        } else if(data[i] < base){
            result(0,i) = -1;
        } else  if(data[i]-base<MIN_THR && base-data[i]<MIN_THR){
            result(0,i) = 0;
        }
        base = data[i];
    }
    return result;
}

static void stft(std::vector<Real> &audio, Eigen::MatrixXf &out){
    if(!IF_INIT)
        frequencyInit();
    //Construct a matrix
    typedef std::vector<std::vector<float> >  Tmatrix;
    ///可以根据audio的长度估计矩阵的行数
    Tmatrix matrix;
//    matrix.resize(MATRIX_ROW);
//    for (int i = 0; i < MATRIX_ROW; i++)  matrix[i].resize(MATRIX_COL);
    //矩阵行数根据audio的长度进行预设
//            std::cout << audio.size() << std::endl;
    int matrix_row = audio.size() / MATRIX_AUDIO;
    matrix.resize(matrix_row);
    for (int i = 0; i < matrix_row; i++)  matrix[i].resize(MATRIX_COL);

    essentia::init();
    int framesize = FRAMESIZE;
    int hopsize = HOPSIZE;
    int sr = SAMPLERATE;
    AlgorithmFactory& factory = AlgorithmFactory::instance();

    // create algorithms
    Algorithm* frameCutter = factory.create("FrameCutter",
                                            "frameSize", framesize,
                                            "hopSize", hopsize,
                                            "startFromZero", false);

    Algorithm* window = factory.create("Windowing",
                                       "type", "hann",
                                       "size",framesize,
                                       "zeroPadding", 0);

    Algorithm* spectrum = factory.create("Spectrum",
                                         "size", framesize);

    // configure frameCutter:
    std::vector<Real> frame;
    frameCutter->input("signal").set(audio);
    frameCutter->output("frame").set(frame);

    // configure windowing:
    std::vector<Real> windowedframe;
    window->input("frame").set(frame);
    window->output("frame").set(windowedframe);

    // configure spectrum:
    std::vector<Real> spec;
    spectrum->input("frame").set(windowedframe);
    spectrum->output("spectrum").set(spec);

    // configure pitch extraction:
//    Real thisPitch = 0., thisConf = 0;
//    Real localTime=0.;
//    std::vector<Real> allPitches, allConf, time;
//    pitchDetect->input("spectrum").set(spec);
//    pitchDetect->output("pitch").set(thisPitch);
//    pitchDetect->output("pitchConfidence").set(thisConf);

    // process:
    int i = 0;
    while (true) {
        frameCutter->compute();

        if (!frame.size())
            break;

        if (isSilent(frame))
            continue;

        window->compute();
        spectrum->compute();
//        pitchDetect->compute();
//        allPitches.push_back(thisPitch);
//        localTime+=float(hopsize)/float(sr);
//        time.push_back(localTime);
//        allConf.push_back(thisConf);

        for (int j = 0; j < spec.size(); ++j) {
            matrix[i][j] = spec[j];
        }

        ++i;
    }
//    std::cout << spec.size() << std::endl;
//    std::cout << i << std::endl;
    // clean up
    delete frameCutter;
    delete spectrum;
//    delete pitchDetect;

    out.resize(i, MATRIX_COL);
    float *dataOut = out.data();
    for (int k = 0; k < i; ++k) {
        for (int j = 0; j < MATRIX_COL; ++j) {
            out(k, j) = matrix[k][j];
        }
    }
}

static void readFile(std::string path, std::vector<Real> &audio){
    audio.clear();

    FILE *f;
    f = fopen(path.c_str(), "rb");
    if(f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    unsigned int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    //FIXME::这块考虑是否可以使用静态的
    char *buffer = (char *)malloc(len);
    len = fread(buffer, 1, len, f);
//    std::cout << len << std::endl;
    fclose(f);

    int input_len = len;
    float *in = (float *)malloc(input_len*sizeof(float));
    SRC_DATA data;
    float *out = (float *)malloc(input_len*3*sizeof(float));
    data.src_ratio = SAMPLERATE / ORRSAMPLE;
    for (int j = 0; j < len/2; ++j) {
        char bl = buffer[2*j];
        char bh = buffer[2*j+1];

        short s = (short)((bh & 0x00FF) << 8 | bl & 0x00FF);
        float ft = s / 32768.0;
        in[j] = ft;
//        std::cout << f << std::endl;
    }
    data.input_frames = input_len;
    data.output_frames = input_len*3;
    data.data_in = in;
    data.data_out = out;
    int error;
    if(error = src_simple(&data, SRC_SINC_FASTEST, 1)){
        std::cout << src_strerror(error) << std::endl;
    }

    for (int i = 0; i < data.output_frames_gen; ++i) {
        audio.push_back(out[i]);
    }
    free(buffer);
    free(in);
    free(out);
}

void bufferToFloat(const char *buffer, unsigned int len, std::vector<Real> &audio){
    audio.clear();

//    std::cout << "len: " << len << std::endl;
    ////使用静态分配的内存，看是否比动态分配的速度快   FIME:在buffer过长时会出现问，此处可以根据len的长度进行动态分配
    static float in[MAXBUFFLEN];
    static float out[RESAMBUFFLEN];
    memset(in, 0, sizeof(float)*MAXBUFFLEN);
    memset(out, 0, sizeof(float)*RESAMBUFFLEN);
    SRC_DATA data;
    data.src_ratio = SAMPLERATE / ORRSAMPLE;
//    std::cout << len/2 << std::endl;
    int j;
    for (j = 22*1025; j < len/2; ++j) {
        char bl = buffer[2*j];
        char bh = buffer[2*j+1];

        short s = (short)((bh & 0x00FF) << 8 | bl & 0x00FF);
        float ft = s / 32768.0;
        in[j-22*1025] = ft;
//        std::cout << f << std::endl;
    }
    data.input_frames = j-22*1025;
//    data.input_frames = MAXBUFFLEN;
    data.output_frames = RESAMBUFFLEN;
    data.data_in = in;
    data.data_out = out;
    int error;
    if(error = src_simple(&data, SRC_SINC_FASTEST, 1)){
        std::cout << src_strerror(error) << std::endl;
    }
//    std::cout << data.output_frames_gen << std::endl;

    for (int i = 0; i < data.output_frames_gen; ++i) {
        audio.push_back(out[i]);
    }
}


void bufferToFloat(const char *buffer, unsigned int len, std::vector<Real> &audio, float pre, float cmp_length){
    audio.clear();
    //使用静态分配的内存，看是否比动态分配的速度快
//    static float in[MAXBUFFLEN];
//    static float out[RESAMBUFFLEN];
//    memset(in, 0, sizeof(float)*MAXBUFFLEN);
//    memset(out, 0, sizeof(float)*RESAMBUFFLEN);
    SRC_DATA data;
    data.src_ratio = SAMPLERATE / ORRSAMPLE;
    len = (len - pre * FRAMELEN) * cmp_length;
    if(len&1)
        ++len;
//    std::cout << len/2 << std::endl;
    //动态分配内存   除2是因为两个char对应一个float
    float *in = (float *)malloc(len / 2 * sizeof(float));
    float *out = (float *)malloc(len / 2 * sizeof(float) * SAMPLEBUFF);

    int j = (int)(pre*FRAMELEN/2);
    for (; j < (len+pre*FRAMELEN)/2; ++j) {
        char bl = buffer[2*j];
        char bh = buffer[2*j+1];

        short s = (short)((bh & 0x00FF) << 8 | bl & 0x00FF);
        float ft = s / 32768.0;
        in[j-(int)(pre*FRAMELEN/2)] = ft;
//        std::cout << f << std::endl;
    }
//    std::cout << "len:" << len/2 << "  j:" << j-pre*FRAMELEN/2 << std::endl;
//    data.input_frames = j-pre*FRAMELEN/2;
//    data.output_frames = RESAMBUFFLEN;
    data.input_frames = len/2;
    data.output_frames = len*SAMPLEBUFF/2;
    data.data_in = in;
    data.data_out = out;
    int error;
    if(error = src_simple(&data, SRC_SINC_FASTEST, 1)){
        std::cout << src_strerror(error) << std::endl;
    }

    for (int i = 0; i < data.output_frames_gen; ++i) {
        audio.push_back(out[i]);
    }

    free(in);
    free(out);
}

Eigen::MatrixXf featuresBuffer(const char *ori_buffer, unsigned int ori_len, float cmp_length){
    if(!IF_INIT)
        frequencyInit();

    Eigen::MatrixXf output;
    std::vector<Real> audio;

//    startTime = clock();
    bufferToFloat(ori_buffer, ori_len*cmp_length, audio);
//    endTime = clock();
//    std::cout << "bufferToFloat Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;

//    startTime = clock();
    stft(audio, output);
//    endTime = clock();
//    std::cout << "stft Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;
//    stft(path3, output);
//    startTime = clock();
    output = ampliTudeToDb(output);
//    endTime = clock();
//    std::cout << "ampliTudeToDb Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;
//    std::cout << output.rows() << "   " << output.cols() << std::endl;

    std::vector<std::pair<int, int> > idx = getIdx(output, SMALL, MIN_HUMAN_DB);
    setIdx(output, idx, SET_MIN_DB);

//    Eigen::MatrixXi mat_index = argSort(output, 1);
//    mat_index.transposeInPlace();
#if 1
//    startTime = clock();
    output.transposeInPlace();
    Eigen::MatrixXi mat_index = argSort(output, 0);
//    endTime = clock();
//    std::cout << "argsort Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;
//    mat_index.transposeInPlace();
#endif

    int cur_col = mat_index.cols();
//    std::cout << mat_index.rows() << "   " << mat_index.cols() << std::endl;
//    std::cout << MATRIX_COL-MATRIX_RANGE << std::endl;
    Eigen::MatrixXf result(MATRIX_RANGE, cur_col);
    int index;
    for (int i = MATRIX_COL-MATRIX_RANGE; i < MATRIX_COL; ++i) {
        for (int j = 0; j < cur_col; ++j) {
            //从FFT_FRE中获取对应排序索引的频率,目前取最大的6列，即最后6列
            index = mat_index(i, j);
            result(i-(MATRIX_COL-MATRIX_RANGE), j) = FFT_FRE(0, index);
        }
    }

//    idx = getIdx(result, GREATE, MAX_DB);
//    setIdx(result, idx, 0);


    return result;
}

Eigen::MatrixXf featuresBuffer(const char *ori_buffer, unsigned int ori_len, float pre, float cmp_length){
    if(!IF_INIT)
        frequencyInit();

    Eigen::MatrixXf output;
    std::vector<Real> audio;

    startTime = clock();
    bufferToFloat(ori_buffer, ori_len, audio, pre, cmp_length);
    endTime = clock();
    std::cout << "bufferToFloat Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;

    startTime = clock();
    stft(audio, output);
    endTime = clock();
    std::cout << "stft Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;
    startTime = clock();
    output = ampliTudeToDb(output);
    endTime = clock();
    std::cout << "ampliTudeToDb Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;

    startTime = clock();
//    std::vector<std::pair<int, int> > idx = getIdx(output, SMALL, MIN_HUMAN_DB);
//    setIdx(output, idx, SET_MIN_DB);
    getAndSetIdx(output, SMALL, MIN_HUMAN_DB, SET_MIN_DB);
    endTime = clock();
    std::cout << "setIdx Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;

//    startTime = clock();
    output.transposeInPlace();
//    endTime = clock();
//    std::cout << "trans Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;

    startTime = clock();
    Eigen::MatrixXi mat_index = argSort(output, 0);
    endTime = clock();
    std::cout << "argsort Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;
//    Eigen::MatrixXi mat_index = argSort(output, 1);
//    mat_index.transposeInPlace();
//    mat_index.transposeInPlace();

//    startTime = clock();
    int cur_col = mat_index.cols();
    Eigen::MatrixXf result(MATRIX_RANGE, cur_col);
    int index;
    for (int i = MATRIX_COL-MATRIX_RANGE; i < MATRIX_COL; ++i) {
        for (int j = 0; j < cur_col; ++j) {
            //从FFT_FRE中获取对应排序索引的频率,目前取最大的6列，即最后6列
            index = mat_index(i, j);
            result(i-(MATRIX_COL-MATRIX_RANGE), j) = FFT_FRE(0, index);
        }
    }
//    endTime = clock();
//    std::cout << "getFre Time: " << (double)(endTime - startTime) /CLOCKS_PER_SEC<< "s" << std::endl;;
    return result;
}
