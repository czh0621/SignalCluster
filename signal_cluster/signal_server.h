//
// Created by viruser on 2025/9/5.
//
#ifndef SIGNALCLUSTER_SIGNAL_SERVER_H_
#define SIGNALCLUSTER_SIGNAL_SERVER_H_
#include "data.h"
#include "proto/gen/signal/gignal.grpc.pb.h"
#include "signal_proxy.h"

class SignalServer {
public:
  SignalServer(const std::string &addr) : server_address_(addr) {}

  void Run(size_t cq_threads = 2, size_t work_threads = 4) {
    // 1. 构建 gRPC Server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address_,
                             grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);

    // 2. 创建 CompletionQueue
    auto cq = builder.AddCompletionQueue();

    // 3. 构建 Server
    server_ = builder.BuildAndStart();

    // 4. 注册 RPC CallData 实例
    RegisterRpcs(cq.get());

    // 5. 封装 CompletionQueueRunner + 线程池
    cq_runner_ =
        std::make_unique<CompletionQueueRunner>(std::move(cq), work_threads);
    cq_runner_->Start(cq_threads);
  }

  void Shutdown() {
    cq_runner_->Shutdown();
    server_->Shutdown();
  }

private:
  void RegisterRpcs(grpc::ServerCompletionQueue *cq) {
    RegisterUnary<signal::SubscribeSignalReq, signal::SubscribeSignalRsp,
                  signal::SignalManager::AsyncService>(
        service_, // YourAsyncService* 实例
        cq,       // grpc::ServerCompletionQueue* 实例
        &signal::SignalManager::AsyncService::
            RequestSubscribeSignal, // 请求函数指针
        [](const SubscribeSignalReq &request,
           SubscribeSignalRsp &response) -> grpc::Status {
          // 这里是您的业务处理逻辑
          return HandleSubscribeSignal(request, response);
        });
  }

private:
  std::string server_address_;
  signal::SignalManager::AsyncService service_;
  std::unique_ptr<grpc::Server> server_;
  std::shared_ptr<>
  std::unique_ptr<CompletionQueueRunner> cq_runner_;
};

#endif // SIGNALCLUSTER_SIGNAL_SERVER_H_
