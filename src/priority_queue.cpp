// CS375 - Lab 7 - Exercise 3
// Thread-Safe Priority Queue
//
// Compile: g++ -o priority_queue priority_queue.cpp -pthread -std=c++11
// Run:     ./priority_queue

#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <random>

// Serializes console output so lines printed by different threads never interleave.
std::mutex coutMutex;

template <typename T>
class ThreadSafePriorityQueue {
private:
    std::priority_queue<T> pq;  // max-heap: top() is always the largest element
    std::mutex mtx;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        pq.push(value);  // O(log n) sift-up restores the heap property
    }

    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (pq.empty()) return false;
        value = pq.top();
        pq.pop();        // O(log n) sift-down restores the heap property
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return pq.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return pq.size();
    }
};

void priorityQueueTest() {
    ThreadSafePriorityQueue<int> pq;
    std::vector<std::thread> threads;
    const int NUM_THREADS = 4;
    const int PUSHES_PER_THREAD = 5;
    const int TOTAL = NUM_THREADS * PUSHES_PER_THREAD;

    // Each pusher gets its own random generator (rand() is not guaranteed thread-safe).
    auto pusher = [&pq](int id) {
        std::mt19937 gen(std::random_device{}() + id);
        std::uniform_int_distribution<int> dist(0, 99);
        for (int i = 0; i < PUSHES_PER_THREAD; ++i) {
            int priority = dist(gen);
            pq.push(priority);
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Thread " << id << " pushed: " << priority << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    // The popper keeps going until it has removed every pushed element.
    auto popper = [&pq]() {
        int popped = 0;
        while (popped < TOTAL) {
            int value;
            if (pq.pop(value)) {
                popped++;
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Popped: " << value << " (remaining: " << pq.size() << ")\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(pusher, i);
    }
    threads.emplace_back(popper);

    for (auto& t : threads) t.join();

    std::cout << "All " << TOTAL << " elements popped. Queue empty: "
              << (pq.empty() ? "yes" : "no") << "\n";
}

int main() {
    priorityQueueTest();
    return 0;
}
