#pragma once
#ifndef CONCURRENT_STREAM_HPP
#define CONCURRENT_STREAM_HPP
#include <mutex>
#include <atomic>
#include <opencv2/opencv.hpp>

namespace ov {
	template <typename T>
	class ConcurrentStream {
	public:
		void write(const T& data) {
			std::unique_lock<std::mutex> dataLock(m_dataLock);
			m_lastData = data;
			dataLock.unlock();
			m_dataId.fetch_add(1, std::memory_order_release);
			signalNew.notify_all();
		}

		void write(T&& data) {
			std::unique_lock<std::mutex> dataLock(m_dataLock);
			std::swap(m_lastData, data);
			dataLock.unlock();
			m_dataId.fetch_add(1, std::memory_order_release);
			signalNew.notify_all();
		}

		int getId() {
			return m_dataId.load(std::memory_order_acquire);
		}

		void read(T& data) const {
			std::unique_lock<std::mutex> dataLock(m_dataLock);
			data = m_lastData;
		}

		void waitForNextWrite(unsigned int currentId) {
			std::unique_lock<std::mutex> lock(m_signalLock);
			signalNew.wait_for(lock, std::chrono::milliseconds(200), [&currentId, this]() {return currentId != m_dataId.load(std::memory_order_acquire); });
		}

	private:
		mutable std::mutex m_dataLock;
		mutable std::mutex m_signalLock;
		T m_lastData;
		std::atomic<unsigned int> m_dataId;
		std::condition_variable signalNew;
	};
}
#endif
