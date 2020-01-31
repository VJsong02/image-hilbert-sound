#include <chrono>
#include <CImg.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <sstream>

#define verbose

constexpr auto size = 64;
constexpr auto TAU = 6.283185307179586;
constexpr auto multiplier = 4096.0 / (size * size);

int xy2d(int n, int x, int y);
void rot(int n, int* x, int* y, int rx, int ry);
void showprogress(int current, int max, int fps);

int main() {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(NULL);
	omp_set_num_threads(omp_get_num_procs());

	cv::VideoCapture in("input.mp4");
	if (!in.isOpened()) { std::cout << "Error opening video stream or file" << std::endl; return -1; }

#ifdef verbose
	cv::namedWindow("source", CV_WINDOW_NORMAL);
	cv::namedWindow("frame", CV_WINDOW_NORMAL);
	cv::resizeWindow("source", 256, 144);
	cv::resizeWindow("frame", 256, 144);
#endif

	double fps = in.get(CV_CAP_PROP_FPS);
	double length = 44100 / fps;
	int limit = 120 * fps;
	if (limit > in.get(cv::CAP_PROP_FRAME_COUNT)) limit = in.get(cv::CAP_PROP_FRAME_COUNT);
	std::cout << length << " " << limit << std::endl;

	std::ofstream file;
	file.open("dump.txt");
	std::stringstream output;
	output.precision(15);

	std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
	int frames = 0;
	for (int framecount = 0; framecount < limit; ++framecount) {
#ifdef verbose
		if (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - t).count() >= 1) {
			showprogress(framecount, limit, framecount - frames);
			t = std::chrono::steady_clock::now();
			frames = framecount;
		}
#endif

		cv::Mat frame, resized;
		in.read(frame);
		if (frame.empty()) break;

#ifdef verbose
		cv::imshow("source", frame);
#endif

		cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
		cv::resize(frame, resized, cv::Size(), double(size) / frame.cols, (double)size / frame.rows, cv::INTER_AREA);

#ifdef verbose
		cv::imshow("frame", resized);
		cv::waitKey(1);
#endif

		unsigned char pixels[size * size];
		for (int y = 0; y < resized.cols; ++y)
			for (int x = 0; x < resized.rows; ++x)
				pixels[xy2d(size, x, y)] = resized.at<unsigned char>(x, y);

		std::vector<double> audio(length);
		for (int i = 0; i < length; ++i) audio[i] = 0;

#pragma omp parallel for schedule(dynamic)
		for (int f = 0; f < size * size; ++f) {
			double freq = f * multiplier + 50;
			double amplitude = (double)pixels[f] / 255.0;
			if (amplitude < 0.004) continue;
			amplitude *= amplitude;

			for (int x = 0; x < length; ++x)
				audio[x] += amplitude * sin(freq * TAU * x / 44100.0);
		}

		for (int i = 0; i < length; ++i)
			audio[i] /= size * size * 0.75;

		for (double f : audio) output << (int)(f * 32767.0) << "\n";
	}

	in.release();

#ifdef verbose
	cv::destroyAllWindows();
#endif

	file << output.str();
	file.close();
}

int xy2d(int n, int x, int y) {
	int rx, ry, s, d = 0;
	for (s = n / 2; s > 0; s /= 2) {
		rx = (x & s) > 0;
		ry = (y & s) > 0;
		d += s * s * ((3 * rx) ^ ry);
		rot(n, &x, &y, rx, ry);
	}
	return d;
}

void rot(int n, int* x, int* y, int rx, int ry) {
	if (ry == 0) {
		if (rx == 1) {
			*x = n - 1 - *x;
			*y = n - 1 - *y;
		}

		int t = *x;
		*x = *y;
		*y = t;
	}
}

void showprogress(int current, int max, int fps) {
	std::cout << "\r[";
	for (int i = 0; i < current * 50 / max; ++i)
		std::cout << "=";
	for (int i = 0; i < 50 - current * 50 / max; ++i)
		std::cout << " ";
	std::cout << "] " << current * 100 / max << "% | " << fps << "fps";
}
