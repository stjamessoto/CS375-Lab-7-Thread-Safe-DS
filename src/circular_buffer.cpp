// CS375 - Lab 7 - Exercise 4
// Thread-Safe Circular (Bounded) Buffer
//
// Compile: g++ -o circular_buffer circular_buffer.cpp -pthread -std=c++11
// Run:     ./circular_buffer

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>

#define BUFFER_SIZE 5

// Serializes console output so lines printed by different threads never interleave.
std::mutex coutMutex;

class ThreadSafeCircularBuffer {
private:
    int buffer[BUFFER_SIZE];          // fixed-size array, no dynamic allocation
    int in = 0, out = 0, count = 0;   // write index, read index, number of items
    std::mutex mtx;
    std::condition_variable not_full, not_empty;

public:
    // Blocks while the buffer is full.
    void push(int value) {
        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, [this] { return count < BUFFER_SIZE; });
        buffer[in] = value;
        in = (in + 1) % BUFFER_SIZE;  // wrap around
        count++;
        lock.unlock();
        not_empty.notify_one();       // wake one waiting consumer
    }

    // Blocks while the buffer is empty.
    bool pop(int& value) {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [this] { return count > 0; });
        value = buffer[out];
        out = (out + 1) % BUFFER_SIZE;  // wrap around
        count--;
        lock.unlock();
        not_full.notify_one();          // wake one waiting producer
        return true;
    }

    // Non-blocking variant: returns false immediately if empty.
    bool try_pop(int& value) {
        std::unique_lock<std::mutex> lock(mtx);
        if (count == 0) return false;
        value = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        lock.unlock();
        not_full.notify_one();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return count == 0;
    }

    bool full() {
        std::lock_guard<std::mutex> lock(mtx);
        return count == BUFFER_SIZE;
    }

    int size() {
        std::lock_guard<std::mutex> lock(mtx);
        return count;
    }
};

void circularBufferTest() {
    ThreadSafeCircularBuffer cb;
    std::vector<std::thread> producers, consumers;
    const int NUM_PRODUCERS = 2;
    const int NUM_CONSUMERS = 2;
    const int NUM_ITEMS = 10;  // per producer; consumers split the total evenly

    // Producers are faster than consumers, so the buffer fills up and
    // producers block on not_full -- this demonstrates back-pressure.
    auto producer = [&cb](int id) {
        for (int i = 0; i < NUM_ITEMS; ++i) {
            int value = id * 100 + i;
            cb.push(value);
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Producer " << id << " produced: " << value
                          << "  [buffer " << cb.size() << "/" << BUFFER_SIZE
                          << (cb.full() ? " FULL" : "") << "]\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    auto consumer = [&cb](int id) {
        const int toConsume = NUM_PRODUCERS * NUM_ITEMS / NUM_CONSUMERS;
        for (int i = 0; i < toConsume; ++i) {
            int value;
            if (cb.pop(value)) {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "    Consumer " << id << " consumed: " << value
                          << "  [buffer " << cb.size() << "/" << BUFFER_SIZE << "]\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    };

    for (int i = 1; i <= NUM_PRODUCERS; ++i) producers.emplace_back(producer, i);
    for (int i = 1; i <= NUM_CONSUMERS; ++i) consumers.emplace_back(consumer, i);

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    std::cout << "Done. Buffer empty: " << (cb.empty() ? "yes" : "no") << "\n";
}

int main() {
    circularBufferTest();
    return 0;
}
