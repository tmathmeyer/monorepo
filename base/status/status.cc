#include "base/status/status.h"

#include <memory>

namespace base {

namespace internal {

StatusData::StatusData() = default;

StatusData::StatusData(const StatusData& copy) {
  *this = copy;
}

StatusData::StatusData(StatusGroupType group,
                       StatusCodeType code,
                       std::string message)
    : group(group), code(code), message(std::move(message)) {}

std::unique_ptr<StatusData> StatusData::copy() const {
  auto result = std::make_unique<StatusData>(group, code, message);
  for (const auto& frame : frames)
    result->frames.push_back(frame);
  return result;
}

StatusData::~StatusData() = default;

StatusData& StatusData::operator=(const StatusData& copy) {
  group = copy.group;
  code = copy.code;
  message = copy.message;
  for (const auto& frame : copy.frames)
    frames.push_back(frame);
  return *this;
}

void StatusData::AddLocation(const base::Location& location) {
  frames.push_back(location);
}

}  // namespace internal

}  // namespace base