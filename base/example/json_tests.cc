
#include "base/json/json.h"
#include "base/json/json_rectify.h"
#include "base/tuple.h"

int main() {
  std::map<std::string, base::json::JSON> keys = {};
  keys.insert({"bar", "bar"});
  base::json::Object object(std::move(keys));

  auto foo = base::json::Rectify<std::string, std::optional<std::string>>(
      object, "bar", "foo");

  if (!foo.has_value()) {
    puts("rectify failed!");
    return 0;
  }

  std::cout << "opt = " << *foo << "\n";

  puts("succeeded");
}