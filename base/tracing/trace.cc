
#include "trace.h"
#include "base/json/json.h"
#include "base/json/json_io.h"

#include <chrono>
#include <iostream>

namespace base {

namespace {

size_t ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

Tracer::Tracer() {
  initial_ = ms();
}

Tracer::~Tracer() {
  if (!print_)
    return;
  std::map<std::string, json::JSON> schema;
  schema.insert({"exporter", "base/trace"});
  schema.insert({"name", "trace.json"});
  schema.insert({"activeProfileIndex", 0});
  schema.insert(
      {"$schema", "https://www.speedscope.app/file-format-schema.json"});

  std::map<std::string, json::JSON> shared;
  std::vector<json::JSON> frames;
  for (const std::string& frname : frames_) {
    std::map<std::string, json::JSON> blob;
    blob.insert({"name", frname});
    frames.emplace_back(std::move(blob));
  }
  shared.insert({"frames", std::move(frames)});
  schema.insert({"shared", std::move(shared)});

  std::vector<json::JSON> profiles;
  std::map<std::string, json::JSON> profile_map;
  profile_map.insert({"type", "evented"});
  profile_map.insert({"name", "trace"});
  profile_map.insert({"unit", "none"});
  profile_map.insert({"startValue", 0});
  ssize_t last_event_frame_no = 0;
  std::vector<json::JSON> events;
  for (const auto& ev : events_) {
    last_event_frame_no =
        std::max(static_cast<ssize_t>(std::get<1>(ev)), last_event_frame_no);
    std::map<std::string, json::JSON> blob;
    blob.insert({"type", std::get<0>(ev) ? "O" : "C"});
    blob.insert({"frame", static_cast<ssize_t>(std::get<1>(ev))});
    blob.insert({"at", static_cast<ssize_t>(std::get<2>(ev))});
    events.emplace_back(std::move(blob));
  }
  profile_map.insert({"endValue", last_event_frame_no});
  profile_map.insert({"events", std::move(events)});
  profiles.emplace_back(std::move(profile_map));
  schema.insert({"profiles", std::move(profiles)});

  json::Object report(std::move(schema));
  std::cout << report << "\n";
}

// static
Tracer* Tracer::Get() {
  static Tracer tracer;
  return &tracer;
}

size_t Tracer::StartEvent(std::string e) {
  size_t vec_size = frames_.size();
  std::time_t current = ms() - initial_;
  frames_.push_back(e);
  events_.push_back(
      std::make_tuple<bool, size_t, size_t>(true, vec_size + 0, current));
  return vec_size;
}

void Tracer::EndEvent(size_t key) {
  std::time_t current = ms() - initial_;
  events_.push_back(
      std::make_tuple<bool, size_t, size_t>(false, std::move(key), current));
}

void Tracer::PrintOnExit() {
  print_ = true;
}

}  // namespace base