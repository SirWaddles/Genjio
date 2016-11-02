#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "videocheck.h"

void SendProgressMessage(WSserver* server, websocketpp::connection_hdl hdl, std::string message) {
	server->send(hdl, message.c_str(), message.length(), websocketpp::frame::opcode::text);
}

void VideoAnalysisTask::Run() {
	VideoCheck();
}

void VideoAnalysisTask::VideoCheck() {
	cv::VideoCapture capture(contents, cv::CAP_FFMPEG);
	if (capture.isOpened()) {
		printf("Open!\n");
	}
	else {
		printf("Closed\n");
	}

	double frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
	double fps = capture.get(cv::CAP_PROP_FPS);
	double frameWidth = capture.get(cv::CAP_PROP_FRAME_WIDTH);
	double frameHeight = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
	cv::Mat templateImage = cv::imread("template.png");
	double templateWidth = templateImage.cols;
	double templateHeight = templateImage.rows;

	double tgtTmplWidth = (templateWidth / 1920.0) * frameWidth; // Template is designed for 1080p
	double tgtTmplHeight = (templateHeight / 1080.0) * frameHeight; // This will only work for 16:9 resolutions
	cv::Mat tgtTemplate;
	cv::resize(templateImage, tgtTemplate, cv::Size(tgtTmplWidth, tgtTmplHeight), 0, 0);
	cv::Mat image;
	int count = 0;
	int foundFrames = 0;
	for (;;) {
		count++;
		capture >> image;
		if (image.empty()) break;
		cv::Rect region(0, (frameHeight * 0.7), frameWidth * 0.3, frameHeight * 0.3);
		cv::Mat croppedImage(image, region);
		cv::Mat result;
		cv::matchTemplate(croppedImage, tgtTemplate, result, cv::TM_CCOEFF);

		double minVal, maxVal;
		cv::Point minLoc, maxLoc;
		cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
		bool foundFrame = false;
		if (maxVal > 60000000.0) { // 60m is pretty confident
			foundFrames++;
			foundFrame = true;
		}

		char buffer[500]; // this is dumb
		int length = sprintf(buffer, "{\"progress\": %f,\"frame\":%d,\"genji\":%s}", double(count) / frames, count, foundFrame ? "true" : "false");
		mServer->send(mHdl, buffer, length, websocketpp::frame::opcode::text);
	}

	capture.release();

	std::string message;
	if (double(foundFrames) / frames > 0.6) {
		message = std::string("{\"success\":true, \"filename\":\"") + contents + "\"}";
	} else {
		remove(contents.c_str());
		message = std::string("{\"success\":false}");
	}
	mServer->send(mHdl, message.c_str(), message.length(), websocketpp::frame::opcode::text);
}