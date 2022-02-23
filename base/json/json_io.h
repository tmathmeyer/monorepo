
#include "base/json/json.h"

#ifndef BASE_JSON_JSON_IO_H_
#define BASE_JSON_JSON_IO_H_

namespace base {
namespace json {

std::ostream& operator<<(std::ostream&, const JSON&);
std::ostream& operator<<(std::ostream&, const Object&);
std::ostream& operator<<(std::ostream&, const Array&);

}
}

#endif  // BASE_JSON_JSON_IO_H_
