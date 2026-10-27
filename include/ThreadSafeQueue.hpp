#pragma once

#include <queue>
#include <vector>
#include <mutex>

template <typename T>
class ThreadSafeQueue
{
    private:
        std::queue<T> data_;
        std::mutex mutex_;

    public:
        void push(const T& value)
        {
            std::lock_guard lock(mutex_);        
            data_.push(value);
        }

        std::vector<T> takeAll()
        {
            std::lock_guard lock(mutex_);

            std::vector<T> dataAvailable;
            while (!data_.empty())
            {
                dataAvailable.push_back(data_.front());
                data_.pop();
            }

            return dataAvailable;
        }
};
