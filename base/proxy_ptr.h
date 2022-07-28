#pragma once

#include <atomic>

namespace base {

template <typename T>
class proxy_ptr {
 public:
  proxy_ptr(T* inst) {
    internal_ = std::make_shared<proxy_internal>({
      new std::unique_ptr<T>(inst)
    });
  }

  proxy_ptr(std::unique_ptr<T> inst) {
    internal_ = std::make_shared<proxy_internal>(std::move(inst));
  }

  std::unique_ptr<T> Claim() && {
    if (internal_->claimed_.exchange(true))
      return nullptr;
    return std::move(internal_->data);
  }

  bool Claimed() const {
    return internal_->claimed_.load();
  }

  T* Get() const {
    if (Claimed())
      return nullptr;
    return internal_->data.get();
  }

 private:
  struct proxy_internal {
    std::unique_ptr<T> data;
    std::atomic<bool> claimed_ = false;
  };

  std::shared_ptr<proxy_internal> internal_;
};

}  // namespace base