//
// Created by viruser on 2025/9/5.
//

#ifndef SIGNALCLUSTER_DATA_H_
#define SIGNALCLUSTER_DATA_H_

#include "completion_queue.h"
#include <functional>

template <typename Request, typename Response, typename AsyncService>
class UnaryCallData : public ICallData {
public:
  using RpcHandler = std::function<grpc::Status(const Request &, Response &)>;

  UnaryCallData(AsyncService *service, grpc::ServerCompletionQueue *cq,
                RpcHandler handler,
                std::function<void(UnaryCallData *)> requestFn)
      : service_(service), cq_(cq), responder_(&ctx_),
        handler_(std::move(handler)), requestFn_(std::move(requestFn)) {
    // 初始注册一次
    Proceed(true);
  }

  void Proceed(bool ok) override {
    if (status_ == CREATE) {
      status_ = PROCESS;
      auto self = this->shared_from_this();
      requestFn_(this);

    } else if (status_ == PROCESS) {
      if (!ok) {
        status_ = FINISH;
        return;
      }
      // 执行业务逻辑
      auto status = handler_(request_, reply_);
      responder_.Finish(reply_, status, this);
      status_ = FINISH;

    } else {
      // FINISH: 完成，释放 shared_ptr
    }
  }

  grpc::ServerContext ctx_;
  Request request_;
  Response reply_;
  grpc::ServerAsyncResponseWriter<Response> responder_;

private:
  enum CallStatus { CREATE, PROCESS, FINISH };
  CallStatus status_ = CREATE;

  AsyncService *service_;
  grpc::ServerCompletionQueue *cq_;
  RpcHandler handler_;
  std::function<void(UnaryCallData *)> requestFn_;
};

template <typename Request, typename Response, typename AsyncService>
void RegisterUnary(
    AsyncService *service, grpc::ServerCompletionQueue *cq,
    void (AsyncService::*requestFn)(grpc::ServerContext *, Request *,
                                    grpc::ServerAsyncResponseWriter<Response> *,
                                    grpc::ServerCompletionQueue *,
                                    grpc::ServerCompletionQueue *, void *),
    std::function<grpc::Status(const Request &, Response &)> handler) {
  auto call = std::make_shared<UnaryCallData<Request, Response, AsyncService>>(
      service, cq, handler,
      [=](UnaryCallData<Request, Response, AsyncService> *callData) {
        (service->*requestFn)(&callData->ctx_, &callData->request_,
                              &callData->responder_, cq, cq, callData);
      });
}

#endif // SIGNALCLUSTER_DATA_H_
