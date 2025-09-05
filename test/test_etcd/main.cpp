#include <condition_variable>
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>
#include <etcd/Watcher.hpp>
#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void CallBack(const etcd::Response &resp) {
  if (resp.is_ok() == false) {
    spdlog::error("收到一个错误的事件通知: {}", resp.error_message());
    return;
  } else {
    for (const auto &ev : resp.events()) {
      if (ev.event_type() == etcd::Event::EventType::PUT) {
        spdlog::info("服务信息发生了改变:");
        spdlog::info("当前的值：{}-{}", ev.kv().key(), ev.kv().as_string());
        spdlog::info("原来的值：{}-{}", ev.prev_kv().key(),
                     ev.prev_kv().as_string());
      } else if (ev.event_type() == etcd::Event::EventType::DELETE_) {
        spdlog::warn("服务信息下线被删除:");
        spdlog::warn("当前的值：{}-{}", ev.kv().key(), ev.kv().as_string());
        spdlog::warn("原来的值：{}-{}", ev.prev_kv().key(),
                     ev.prev_kv().as_string());
      }
    }
  }
}
void test_etcd() {
  using namespace etcd;
  Client client(
      "http://127.0.0.1:2379,http://127.0.0.1:22379,http://127.0.0.1:32379");
  client.put("/test_etcd", "test_value1").wait();
  client.get("/test_etcd").then([](pplx::task<etcd::Response> resp_task) {
    try {
      etcd::Response resp = resp_task.get();
      if (resp.is_ok()) {
        spdlog::info("key {} value {}", resp.value().key(),
                     resp.value().as_string());
      }
    } catch (std::exception const &ex) {
      spdlog::error("error:{}", ex.what());
    }
  });

  std::shared_ptr<etcd::Watcher> watcher =
      std::make_shared<etcd::Watcher>(client, "/service", CallBack, true);
  watcher->Wait();
  spdlog::info("服务下线");
}

int main() {
  test_etcd();
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait(lock, [] { return ready; });
  return 0;
}