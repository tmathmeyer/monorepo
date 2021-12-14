#ifndef BASE_JSON_H_
#define BASE_JSON_H_

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <iostream>

#include "check.h"

namespace base {

template<typename T>
class ReaderStream {
 public:
  ReaderStream(T data) {
    backing_ = data;
    index_ = 0;
    eos_ = backing_.length() == 0;
  }

  ~ReaderStream() = default;

  void SkipWhiteSpace() {
    while(!eos_) {
      switch (backing_[index_]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
          index_++;
          if (index_ >= backing_.length())
            eos_ = true;
          break;
        default:
          return;
      }
    }
  }

  char NextChar() {
    index_++;
    return eos_ ? 0 : backing_[index_-1];
  }

  void DropChars(size_t count) {
    index_ += count;
    if (index_ >= backing_.length())
      eos_ = true;
  }

  char Peek() {
    return eos_ ? 0 : backing_[index_];
  }

 private:
  T backing_;
  size_t index_;
  bool eos_;
};

// Forward delcare.
class Value;

class Object {
 public:
  Object(std::map<std::string, Value>&&);
  Object() = default;
  ~Object() = default;

  const std::map<std::string, Value>& Items() const;
  void SetKey(std::string, Value&&);
  Object Clone() const;

 private:
  std::map<std::string, Value> store_;
};

class Array {
 public:
  Array(std::vector<Value>&&);
  Array() = default;
  ~Array() = default;

  const std::vector<Value>& Values() const;
  void Append(Value&&);
  Array Clone() const;

 private:
  std::vector<Value> store_;
};

class Value {
 public:
  enum class Type {
    kString,
    kObject,
    kArray,
    kBoolean,
    kInteger,
    kFloating,
    kNull,
  };

  template<typename T,
           std::enable_if_t<std::is_integral<T>::value, bool> = true>
  Value(T integer_type) {
    type_ = Type::kInteger;
    integer_ = static_cast<ssize_t>(integer_type);
  }

  Value(const Value& copy);
  Value(std::vector<Value>&&);
  Value(Array&&);
  Value(const Array&);
  Value(std::map<std::string, Value>&&);
  Value(Object&&);
  Value(const Object&);
  Value(std::string);
  Value(const char*);
  Value(bool);
  Value(double);
  Value();
  ~Value() = default;

  Array AsArray() &&;
  const Array& AsArray() const &;
  Object AsObject() &&;
  const Object& AsObject() const &;
  std::string AsString() &&;
  const std::string& AsString() const &;
  bool AsBool() const;
  ssize_t AsInteger() const;
  double AsDouble() const;

  Value Clone() const;

  Type type() const;

  void WriteToStream(std::ostream& stream,
                     std::optional<int> indent=std::nullopt) const;

  template<typename T>
  static Value Parse(ReaderStream<T>* stream) {
    stream->SkipWhiteSpace();
    auto c = stream->NextChar();
    switch(c) {
      case '[': {
        return ParseArray(stream);
      }
      case '{': {
        return ParseObject(stream);
      }
      case 'n': {
        stream->DropChars(3);  // drop "ull"
        stream->SkipWhiteSpace();
        return Value();
      }
      case 't': {
        stream->DropChars(3);  // drop "rue"
        stream->SkipWhiteSpace();
        return Value(true);
      }
      case 'f': {
        stream->DropChars(4);  // drop "alse"
        stream->SkipWhiteSpace();
        return Value(false);
      }
      case '"': {
        return Value(ParseString(stream));
      }
      case '-':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        double d;
        ssize_t i;
        switch(ParseNumeric(c, &d, &i, stream)) {
          case Type::kFloating:
            return Value(d);
          case Type::kInteger:
            return Value(i);
          default:
            MCHECK(false, "Expected numeric type");
            return Value();
        }
      }
      default: {
        printf("Go char: <%c>\n", c);
        MCHECK(false, "Invalid JSON");
      }
    }
    return Value();
  }

  template<typename T>
  static Value ParseArray(ReaderStream<T>* stream) {
    std::vector<Value> values;
    stream->SkipWhiteSpace();
    if (stream->Peek() == ']') {
      stream->DropChars(1);
      return Value(std::move(values));
    }

    while (true) {
      values.push_back(Parse(stream));
      switch(stream->NextChar()) {
        case ']': {
          stream->SkipWhiteSpace();
          return Value(std::move(values));
        }
        case ',': {
          break;
        }
        default: {
          MCHECK(false, "Expected , or ]");
          return Value();
        }
      }
    }
  }

  template<typename T>
  static Value ParseObject(ReaderStream<T>* stream) {
    std::map<std::string, Value> values;
    std::string key = "";

    stream->SkipWhiteSpace();
    if (stream->Peek() == '}') {
      stream->DropChars(1);
      return Value(std::move(values));
    }

    while (true) {
      char c = stream->NextChar();
      MCHECK_EQ(c, '"', "Expected '\"', got '%c'\n", c);
      key = ParseString(stream);
      c = stream->NextChar();
      MCHECK_EQ(c, ':', "Expected colon, got '%c'\n", c);
      values.insert({key, Parse(stream)});
      c = stream->NextChar();
      switch(c) {
        case '}': {
          stream->SkipWhiteSpace();
          return Value(std::move(values));
        }
        case ',': {
          break;
        }
        default: {
          MCHECK(false, "Expected , or }, got '%c'\n", c);
          return Value();
        }
      }
    }
  }

  template<typename T>
  static std::string ParseString(ReaderStream<T>* stream) {
    std::string result = "";
    while (true) {
      auto c = stream->NextChar();
      if (c == '"') {
        stream->SkipWhiteSpace();
        return result;
      }
      if (!c)
        return "";
      result.append(1, c);
    }
  }

  template<typename T>
  static Type ParseNumeric(
      char c, double* dbl, ssize_t* it, ReaderStream<T>* stream) {
    *dbl = 0.0;
    *it = 0;
    size_t decimal_places = 0;
    Type result = Type::kInteger;
    if (c != '-')
      *it = c - '0';

    while (true) {
      char c = stream->Peek();
      switch(c) {
        case '0':
          MCHECK_NE(c, '-', "0 can't be the first digit!");
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          *it = (*it * 10) + (c - '0');
          decimal_places++;
          stream->DropChars(1);
          break;
        }
        case '.': {
          result = Type::kFloating;
          *dbl = *it;
          *it = 0;
          decimal_places = 0;
          stream->DropChars(1);
          break;
        }
        default: {
          // End of the number - time to fix it up!
          if (result == Type::kFloating) {
            double lt1 = *it;
            while (decimal_places --> 0)
              lt1 /= 10.0;
            *dbl += lt1;
            if (c == '-')
              *dbl *= -1.0;
            return result;
          }
          if (result == Type::kInteger) {
            if (c == '-')
              *it *= -1;
            return result;
          }
        }
      }
    }
  }

 private:
  std::optional<std::string> string_;
  std::optional<bool> bool_;
  std::optional<Object> object_;
  std::optional<Array> array_;
  std::optional<double> double_;
  std::optional<ssize_t> integer_;
  Type type_;
};

std::ostream& operator<<(std::ostream&, const Value&);

}  // namespace base

#endif  //BASE_JSON_H_