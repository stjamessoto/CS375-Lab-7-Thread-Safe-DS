// CS375 - Lab 7 - Exercise 6
// Thread-Safe Singly Linked List
//
// Compile: g++ -o thread_safe_linked_list thread_safe_linked_list.cpp -pthread -std=c++11
// Run:     ./thread_safe_linked_list

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

// Serializes console output so lines printed by different threads never interleave.
std::mutex coutMutex;

template <typename T>
class ThreadSafeLinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node(T value) : data(value), next(nullptr) {}
    };
    Node* head = nullptr;
    std::mutex mtx;  // coarse-grained: one lock protects the whole list

public:
    ThreadSafeLinkedList() {}
    ThreadSafeLinkedList(const ThreadSafeLinkedList&) = delete;            // a copy would share nodes
    ThreadSafeLinkedList& operator=(const ThreadSafeLinkedList&) = delete; // and double-delete them

    ~ThreadSafeLinkedList() {
        while (!empty()) {
            T value;
            pop_front(value);
        }
    }

    // Reading head and writing head must happen atomically; otherwise two
    // pushers could both link to the same old head and one node would be lost.
    void push_front(T value) {
        Node* newNode = new Node(value);  // allocate outside the lock (no shared state touched)
        std::lock_guard<std::mutex> lock(mtx);
        newNode->next = head;
        head = newNode;
    }

    // Unlinking and deleting under the lock prevents two poppers from
    // removing (and double-deleting) the same node.
    bool pop_front(T& value) {
        Node* temp;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!head) return false;
            temp = head;
            value = temp->data;
            head = head->next;
        }
        delete temp;  // node is unreachable by other threads now
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return head == nullptr;
    }

    // O(n) traversal while holding the lock, so no node can be freed mid-walk.
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        size_t count = 0;
        Node* current = head;
        while (current) {
            count++;
            current = current->next;
        }
        return count;
    }
};

void linkedListTest() {
    ThreadSafeLinkedList<int> list;
    std::vector<std::thread> threads;

    auto pusher = [&list](int id) {
        for (int i = 0; i < 5; ++i) {
            list.push_front(id * 100 + i);
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Thread " << id << " pushed: " << (id * 100 + i) << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto popper = [&list]() {
        for (int i = 0; i < 10; ++i) {
            int value;
            if (list.pop_front(value)) {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "    Popped: " << value << "  (list size now " << list.size() << ")\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    };

    threads.emplace_back(pusher, 1);
    threads.emplace_back(pusher, 2);
    threads.emplace_back(popper);

    for (auto& t : threads) t.join();

    std::cout << "Final list size: " << list.size()
              << " (remaining nodes are freed by the destructor)\n";
}

int main() {
    linkedListTest();
    return 0;
}
