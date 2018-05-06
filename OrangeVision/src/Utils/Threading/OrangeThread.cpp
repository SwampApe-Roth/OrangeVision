#include "OrangeThread.hpp"

OrangeThread::OrangeThread(std::shared_ptr<Updateable> updater) {
	m_updater = updater;
	m_thread = std::make_unique<std::thread>(&OrangeThread::update, this);
}

OrangeThread::~OrangeThread() {
	release();
}

void OrangeThread::start() {
	isRunning.store(true, std::memory_order_release);
	m_signal.notify_all();
}

void OrangeThread::stop() {
	isRunning.store(false, std::memory_order_release);
}


void OrangeThread::update() {
	while (isAlive.load(std::memory_order_acquire)) {
		if (isRunning.load(std::memory_order_acquire)) {
			m_updater->update();
		} else {
			std::unique_lock<std::mutex> lock(m_signalLock);
			m_signal.wait_for(lock, std::chrono::milliseconds(200), [this]() {return isRunning.load(std::memory_order_acquire); });
		}
	}
}

void OrangeThread::release() {
	isAlive.store(false, std::memory_order_release);
	m_thread->join();
}