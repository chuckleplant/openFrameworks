#pragma once

#include "ofBaseMainLoop.h"

class ofMainLoop : public ofBaseMainLoop{
public:
	ofMainLoop();
	virtual ~ofMainLoop();
	std::shared_ptr<ofAppBaseWindow> createWindow(const ofWindowSettings & settings);
	void run(std::shared_ptr<ofAppBaseWindow> window, std::shared_ptr<ofBaseApp> && app);
	void run(std::shared_ptr<ofBaseApp> && app);
	int loop();
	void loopOnce();
	void pollEvents();
	void exit();
	void shouldClose(int status);
	std::shared_ptr<ofAppBaseWindow> getCurrentWindow();
	void setCurrentWindow(std::shared_ptr<ofAppBaseWindow> window);
	void setCurrentWindow(ofAppBaseWindow * window);
	std::shared_ptr<ofBaseApp> getCurrentApp();
	void setEscapeQuitsLoop(bool quits);

private:
	void keyPressed(ofKeyEventArgs & key);
	bool bShouldClose;
	std::weak_ptr<ofAppBaseWindow> currentWindow;
	int status;
	bool escapeQuits;
};
