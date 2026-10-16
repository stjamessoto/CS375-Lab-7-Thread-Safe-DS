// CS375 - Lab 7 - Exercise 5
// Thread-Safe Double-Ended Queue
//
// Compile: g++ -o thread_safe_deque thread_safe_deque.cpp -pthread -std=c++11
// Run:     ./thread_safe_deque

#include <iostream>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

// Serializes console output so lines printed by different threads never interleave.
std::mutex coutMutex;

// One mutex guards BOTH ends. When the deque holds 0 or 1 elements the
// front and back are the same element, so separate per-end locks would race.
template <typename T>
class ThreadSafeDeque {
private:
    std::deque<T> deque;
    std::mutex mtx;

public:
    void push_front(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        deque.push_front(value);
    }

    void push_back(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        deque.push_back(value);
    }

    bool pop_front(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (deque.empty()) return false;
        value = deque.front();
        deque.pop_front();
        return true;
    }

    bool pop_back(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (deque.empty()) return false;
        value = deque.back();
        deque.pop_back();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return deque.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return deque.size();
    }
};

void dequeTest() {
    ThreadSafeDeque<int> dq;
    std::vector<std::thread> threads;
    std::atomic<int> pushed(0), popped(0);

    auto push_front = [&dq, &pushed](int id) {
        for (int i = 0; i < 5; ++i) {
            dq.push_front(id * 100 + i);
            pushed++;
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Thread " << id << " pushed front: " << (id * 100 + i) << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto push_back = [&dq, &pushed](int id) {
        for (int i = 0; i < 5; ++i) {
            dq.push_back(id * 100 + i);
            pushed++;
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Thread " << id << " pushed back: " << (id * 100 + i) << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto pop_front = [&dq, &popped]() {
        for (int i = 0; i < 10; ++i) {
            int value;
            if (dq.pop_front(value)) {
                popped++;
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "    Popped front: " << value << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    };

    auto pop_back = [&dq, &popped]() {
        for (int i = 0; i < 10; ++i) {
            int value;
            if (dq.pop_back(value)) {
                popped++;
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "    Popped back: " << value << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    };

    threads.emplace_back(push_front, 1);
    threads.emplace_back(push_back, 2);
    threads.emplace_back(pop_front);
    threads.emplace_back(pop_back);

    for (auto& t : threads) t.join();

    std::cout << "Summary: pushed " << pushed << ", popped " << popped
              << ", remaining " << dq.size()
              << (pushed == popped + (int)dq.size() ? "  -> consistent (no lost/duplicated items)" : "  -> INCONSISTENT")
              << "\n";
}

int main() {
    dequeTest();
    return 0;
}
