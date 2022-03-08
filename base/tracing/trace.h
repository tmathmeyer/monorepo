
#ifndef BASE_TRACING_TRACE_H_
#define BASE_TRACING_TRACE_H_

#include <cstddef>
#include <ctime>
#include <string>
#include <tuple>
#include <vector>

namespace base {

class Tracer {
 public:
  static Tracer* Get();

  size_t StartEvent(std::string e);
  void EndEvent(size_t id);

  void PrintOnExit();

 private:
  Tracer();
  ~Tracer();

  bool print_ = false;
  std::time_t initial_;
  std::vector<std::string> frames_;
  std::vector<std::tuple<bool, size_t, size_t>> events_;
};

template <typename T>
class TraceEvent {
 public:
  TraceEvent(T&& t) { key_ = Tracer::Get()->StartEvent(t); }

  ~TraceEvent() { Tracer::Get()->EndEvent(key_); }

 private:
  size_t key_;
};

}  // namespace base

#define TRACE_EVENT(name) ::base::TraceEvent<std::string> __##NAME##__(#name);

#endif  // BASE_TRACING_TRACE_H_