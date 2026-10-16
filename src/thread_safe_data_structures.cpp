// CS375 - Lab 7 - Exercise 1
// Thread-Safe Queue and Stack with Producer-Consumer and Undo-Redo
//
// Compile: g++ -o thread_safe_data_structures thread_safe_data_structures.cpp -pthread -std=c++11
// Run:     ./thread_safe_data_structures

#include <iostream>
#include <queue>
#include <stack>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>

// Serializes console output so lines printed by different threads never interleave.
std::mutex coutMutex;

// Thread-safe Queue class
template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;

public:
    ThreadSafeQueue() {}

    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);  // locked here, unlocked at end of scope
        queue.push(value);
    }

    // Check-and-remove happens under a single lock, so two consumers can
    // never both see the same front element.
    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) return false;
        value = queue.front();
        queue.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

// Thread-safe Stack class
template <typename T>
class ThreadSafeStack {
private:
    std::stack<T> stack;
    std::mutex mtx;

public:
    ThreadSafeStack() {}

    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        stack.push(value);
    }

    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (stack.empty()) return false;
        value = stack.top();
        stack.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return stack.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return stack.size();
    }
};

// Problem 1: Producer-Consumer Simulation
void producerConsumerProblem() {
    ThreadSafeQueue<std::string> messageQueue;
    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 2;
    const int MESSAGES_PER_PRODUCER = 5;
    const int TOTAL_MESSAGES = NUM_PRODUCERS * MESSAGES_PER_PRODUCER;
    std::atomic<int> messages_produced(0);
    std::atomic<int> messages_consumed(0);
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    auto producer = [&messageQueue, &messages_produced](int id) {
        for (int i = 0; i < MESSAGES_PER_PRODUCER; ++i) {
            std::string message = "Producer " + std::to_string(id) + " Message " + std::to_string(i);
            messageQueue.push(message);
            messages_produced++;
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Produced: " << message << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto consumer = [&messageQueue, &messages_produced, &messages_consumed](int id) {
        int consumed = 0;
        while (true) {
            std::string message;
            // Read the counter BEFORE trying to pop. Producers increment it only
            // after pushing, so if it already equals TOTAL_MESSAGES every message
            // is in the queue; a failed pop afterwards means the queue is truly
            // drained. (Checking after the pop could miss a message pushed in between.)
            bool allProduced = (messages_produced == TOTAL_MESSAGES);
            if (messageQueue.pop(message)) {
                consumed++;
                messages_consumed++;
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Consumer " << id << " processed: " << message << std::endl;
            } else if (allProduced) {
                break;  // All messages produced and queue empty
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Consumer " << id << " finished after processing " << consumed << " messages" << std::endl;
    };

    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer, i);
    }
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(consumer, i);
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    std::cout << "Summary: produced " << messages_produced << ", consumed " << messages_consumed
              << ", left in queue " << messageQueue.size() << std::endl;
}

// Problem 2: Undo-Redo System
void undoRedoProblem() {
    ThreadSafeStack<std::string> undoStack;
    ThreadSafeStack<std::string> redoStack;
    std::string currentText = "";

    // Each edit saves the old state on the undo stack and clears the redo
    // history (a new edit invalidates anything that could have been redone).
    auto editText = [&undoStack, &redoStack, &currentText](const std::string& newText) {
        undoStack.push(currentText);
        std::string temp;
        while (redoStack.pop(temp)) {
            // discard redo history
        }
        currentText = newText;
        std::cout << "Text updated to: " << currentText << std::endl;
    };

    auto undo = [&undoStack, &redoStack, &currentText]() {
        std::string prevText;
        if (undoStack.pop(prevText)) {
            redoStack.push(currentText);
            currentText = prevText;
            std::cout << "Undo - Current text: " << currentText << std::endl;
        } else {
            std::cout << "Undo - Nothing to undo" << std::endl;
        }
    };

    auto redo = [&undoStack, &redoStack, &currentText]() {
        std::string nextText;
        if (redoStack.pop(nextText)) {
            undoStack.push(currentText);
            currentText = nextText;
            std::cout << "Redo - Current text: " << currentText << std::endl;
        } else {
            std::cout << "Redo - Nothing to redo" << std::endl;
        }
    };

    editText("Hello");
    editText("Hello World");
    editText("Hello Universe");
    undo();
    undo();
    redo();
    editText("Hello Galaxy");
    redo();  // redo history was cleared by the edit above

    std::cout << "Final text: " << currentText
              << " (undo stack size: " << undoStack.size()
              << ", redo stack size: " << redoStack.size() << ")" << std::endl;
}

int main() {
    std::cout << "Problem 1: Producer-Consumer Simulation\n";
    producerConsumerProblem();

    std::cout << "\nProblem 2: Undo-Redo System\n";
    undoRedoProblem();
    return 0;
}
