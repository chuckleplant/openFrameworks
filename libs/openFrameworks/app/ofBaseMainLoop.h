#pragma once
#include "ofConstants.h"
#include "ofAppBaseWindow.h"
#include "ofBaseApp.h"
#include "ofEvents.h"

class ofBaseMainLoop {
public:
	virtual ~ofBaseMainLoop() {}
	virtual std::shared_ptr<ofAppBaseWindow> createWindow(const ofWindowSettings & settings) = 0;

	template<typename Window>
	void addWindow(std::shared_ptr<Window> window) {
		allowMultiWindow = Window::allowsMultiWindow();
		if (Window::doesLoop()) {
			windowLoop = Window::loop;
		}
		if (Window::needsPolling()) {
			windowPollEvents = Window::pollEvents;
		}
		if (!allowMultiWindow) {
			windowsApps.clear();
		}
		windowsApps[window] = std::shared_ptr<ofBaseApp>();
		currentWindow = window;
		ofAddListener(window->events().keyPressed, this, &ofBaseMainLoop::keyPressed);
	}

	virtual void run(std::shared_ptr<ofAppBaseWindow> window, std::shared_ptr<ofBaseApp> && app){}
	virtual void run(std::shared_ptr<ofBaseApp> && app){}
	virtual int loop() = 0;
	virtual void loopOnce(){}
	virtual void pollEvents(){}
	virtual void exit(){}
	virtual void shouldClose(int status){}
	virtual std::shared_ptr<ofAppBaseWindow> getCurrentWindow() = 0;
	virtual void setCurrentWindow(std::shared_ptr<ofAppBaseWindow> window){}
	virtual void setCurrentWindow(ofAppBaseWindow * window){}
	virtual std::shared_ptr<ofBaseApp> getCurrentApp() = 0;
	virtual void setEscapeQuitsLoop(bool quits){}
	ofEvent<void> exitEvent;
protected:
	bool allowMultiWindow;
	virtual void keyPressed(ofKeyEventArgs & key) {}
	map<std::shared_ptr<ofAppBaseWindow>, std::shared_ptr<ofBaseApp> > windowsApps;
	std::shared_ptr<ofAppBaseWindow> currentWindow;
	std::function<void()> windowLoop;
	std::function<void()> windowPollEvents;
};