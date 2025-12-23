#include "ThreadPool.h"

ThreadPool::ThreadPool(int capacity) : stop(false) {
  // intialize pool with give capacity
  for (int i = 0; i < capacity; i++) {
    workers.emplace_back([this] {
      while (true) {
        // endless function that will keep checking queue for a task
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(taskMtx);

          // wait till queue is not empty
          condition.wait(lock, [this] { return !tasks.empty() || stop; });

          // if stop and empty exit thread - happens during destruction
          if (stop && tasks.empty()) return;
          task = tasks.front();
          tasks.pop();
        }
        task();
      }
    });
  }
  std::cout << "Thread pool created successfully" << "\n";
}

void ThreadPool::enqueue(std::function<void()> task) {
  {
    std::lock_guard lock(taskMtx);
    tasks.push(task);
    condition.notify_one();
    // std::cout << "Added new request to queue" << "\n";
  }
}
ThreadPool::~ThreadPool() {
  {
    std::lock_guard lock(taskMtx);
    stop = true;
  }
  condition.notify_all();  // wake all up so they can finish
  for (auto& worker : workers) {
    if (worker.joinable()) {
      worker.join();  // wait for them to finish
    }
  }
}