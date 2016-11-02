#pragma once

#include "task.h"

class VideoAnalysisTask : public Task {
public:
	using Task::Task;

	virtual void Run();
	void VideoCheck();
};