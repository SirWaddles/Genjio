#pragma once

#include <opencv2/imgproc.hpp>
#include <vector>
#include "task.h"

struct ImageTemplate {
	cv::Mat image;
	cv::Mat mask;
	std::string templatePath;
	ImageTemplate();
	ImageTemplate(int width, int height);
};

class VideoAnalysisTask : public Task {
public:
	VideoAnalysisTask(WSserver* server, websocketpp::connection_hdl hdl, std::string message);

	virtual void Run();
	void VideoCheck();
	void CreateTemplates(double frameWdith, double frameHeight);
	bool MatchTemplate(cv::Mat& image, ImageTemplate& img);
	bool MatchTemplates(cv::Mat& image);
private:
	std::vector<ImageTemplate> templates;
	bool foundTemplate;
	ImageTemplate& targetTemplate;
	ImageTemplate emptyMat; // hacks? How else do I do this?
};