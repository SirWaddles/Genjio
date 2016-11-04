#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "videocheck.h"
#include <Windows.h>

#define FRAME_MULTIPLIER 10.0

void SendProgressMessage(WSserver* server, websocketpp::connection_hdl hdl, std::string message) {
	server->send(hdl, message.c_str(), message.length(), websocketpp::frame::opcode::text);
}

VideoAnalysisTask::VideoAnalysisTask(WSserver* server, websocketpp::connection_hdl hdl, std::string message) : Task(server, hdl, message), targetTemplate(emptyMat) {

}

void VideoAnalysisTask::Run() {
	VideoCheck();
	foundTemplate = false;
}

ImageTemplate::ImageTemplate() {

}

ImageTemplate::ImageTemplate(int width, int height) : image(height, width, CV_8UC3), mask(height, width, CV_8UC3){

}

void VideoAnalysisTask::CreateTemplates(double frameWidth, double frameHeight) {
	static const std::string templatePaths[] = {"bedouin.png", "chrome.png", "cinnabar.png", "malachite.png", "nihon.png", "nomad.png", "orchite.png", "young.png"};
	const int templateCount = 8;
	for (int i = 0; i < templateCount; i++) {
		std::string path = std::string("templates/") + templatePaths[i];
		cv::Mat templateImage = cv::imread(path, cv::IMREAD_UNCHANGED);
		double tWidth = templateImage.cols;
		double tHeight = templateImage.rows;
		double tgtWidth = (tWidth / 3840.0) * frameWidth;
		double tgtHeight = (tHeight / 2160.0) * frameHeight;
		cv::Mat result;
		cv::resize(templateImage, result, cv::Size(tgtWidth, tgtHeight), 0, 0);
		
		ImageTemplate fResult(tgtWidth, tgtHeight);

		cv::Mat out[] = { fResult.image, fResult.mask };
		cv::Mat in[] = { result };
		int from_to[] = { 0,0, 1,1, 2,2, 3,3, 3,4, 3,5 };
		cv::mixChannels(in, 1, out, 2, from_to, 6);

		templates.push_back(fResult);
	}
}

bool VideoAnalysisTask::MatchTemplates(cv::Mat& image) {
	if (foundTemplate) {
		return MatchTemplate(image, targetTemplate);
	}

	for (auto&& i : templates) {
		if (MatchTemplate(image, i)) {
			targetTemplate = i;
			foundTemplate = true;
			return true;
		}
	}
	return false;
}

bool VideoAnalysisTask::MatchTemplate(cv::Mat& image, ImageTemplate& templateImage) {
	cv::Mat result;
	cv::matchTemplate(image, templateImage.image, result, cv::TM_SQDIFF, templateImage.mask);
	double minVal, maxVal;
	cv::minMaxLoc(result, &minVal, &maxVal, nullptr, nullptr);
	printf("Match: %f\n", minVal);
	return minVal < 200.0;
}

void VideoAnalysisTask::VideoCheck() {
	cv::VideoCapture capture(contents, cv::CAP_FFMPEG);
	if (capture.isOpened()) {
		printf("Open!\n");
	}
	else {
		printf("Closed\n");
	}

	double frames = capture.get(cv::CAP_PROP_FRAME_COUNT) / FRAME_MULTIPLIER;
	double fps = capture.get(cv::CAP_PROP_FPS);
	double frameWidth = capture.get(cv::CAP_PROP_FRAME_WIDTH);
	double frameHeight = capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	CreateTemplates(frameWidth, frameHeight);

	cv::Mat image;
	int count = 0;
	int foundFrames = 0;

	for (;;) {
		count++;
		capture.set(cv::CAP_PROP_POS_FRAMES, double(count) * FRAME_MULTIPLIER);
		capture >> image;
		if (image.empty()) break;
		cv::Rect region(0, (frameHeight * 0.7), frameWidth * 0.3, frameHeight * 0.3);
		cv::Mat croppedImage(image, region);
		
		bool foundFrame = MatchTemplates(croppedImage);
		if (foundFrame) foundFrames++;

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