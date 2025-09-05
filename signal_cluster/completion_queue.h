//
// Created by viruser on 2025/9/5.
//

#ifndef SIGNALCLUSTER_COMPLETION_QUEUE_H_
#define SIGNALCLUSTER_COMPLETION_QUEUE_H_
#include "core/thread_pool.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <vector>

struct ICallData : public std::enable_shared_from_this<ICallData> {
  virtual ~ICallData() = default;
  virtual void Proceed(bool ok) = 0;
};

class CompletionQueueRunner {
public:
  CompletionQueueRunner(std::unique_ptr<grpc::ServerCompletionQueue> cq,
                        size_t thread_count = 1)
      : cq_(std::move(cq)), stop_(false) {
    pool_ = std::make_unique<ThreadPool>(thread_count);
  }

  void Start(size_t thread_count = 1) {
    for (size_t i = 0; i < thread_count; ++i) {
      threads_.emplace_back([this] { HandleEvents(); });
    }
  }

  void Shutdown() {
    stop_ = true;
    cq_->Shutdown();
    for (auto &t : threads_)
      if (t.joinable())
        t.join();
  }

private:
  void HandleEvents() {
    void *tag;
    bool ok;
    while (!stop_) {
      if (cq_->Next(&tag, &ok)) {
        auto *raw = static_cast<ICallData *>(tag);
        auto call = raw->shared_from_this(); // 提升生命周期
        pool_->Submit([call, ok] { call->Proceed(ok); });
      }
    }
  }

  std::unique_ptr<grpc::ServerCompletionQueue> cq_;
  std::unique_ptr<ThreadPool> pool_;
  std::vector<std::thread> threads_;
  std::atomic<bool> stop_;
};

#endif // SIGNALCLUSTER_COMPLETION_QUEUE_H_
