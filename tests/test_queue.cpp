#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

#include "ThreadSafeQueue.hpp"

// Test 1: Empty queue
void testEmptyQueue()
{
    ThreadSafeQueue<int> queue;

    auto data = queue.takeAll();

    assert(data.empty());
}

// Test 2: FIFO order
void testFIFO()
{
    ThreadSafeQueue<int> queue;

    queue.push(10);
    queue.push(20);
    queue.push(30);

    auto data = queue.takeAll();

    assert(data.size() == 3);
    assert(data[0] == 10);
    assert(data[1] == 20);
    assert(data[2] == 30);
}

// Test 3: Queue clearing
void testQueueClearing()
{
    ThreadSafeQueue<int> queue;

    queue.push(1);
    queue.push(2);

    auto first = queue.takeAll();
    auto second = queue.takeAll();

    assert(first.size() == 2);
    assert(second.empty());
}

// Test 4: Concurrent access
void testConcurrentAccess()
{
    ThreadSafeQueue<int> queue;

    constexpr int NUM_THREADS = 4;
    constexpr int VALUES_PER_THREAD = 1000;

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&queue, i]()
        {
            for (int j = 0; j < VALUES_PER_THREAD; ++j)
            {
                queue.push(i * VALUES_PER_THREAD + j);
            }
        });
    }

    for (auto& thread : threads)
        thread.join();

    auto data = queue.takeAll();

    assert(data.size() == NUM_THREADS * VALUES_PER_THREAD);

    // Verify that all expected values were inserted exactly once
    std::vector<bool> found(NUM_THREADS * VALUES_PER_THREAD, false);

    for (int value : data)
    {
        assert(value >= 0);
        assert(value < NUM_THREADS * VALUES_PER_THREAD);
        assert(!found[value]);

        found[value] = true;
    }

    for (bool value : found)
        assert(value);
}

int main()
{
    testEmptyQueue();
    testFIFO();
    testQueueClearing();
    testConcurrentAccess();

    std::cout << "All ThreadSafeQueue tests passed!\n";

    return 0;
}
