#pragma once
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>


class ThreadPool {
 public:
  ThreadPool(int capacity);
  void enqueue(std::function<void()> task);
  ~ThreadPool();

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex taskMtx;
  std::condition_variable condition;
  bool stop;
};
