#ifndef BASE_STATUS_H_
#define BASE_STATUS_H_

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "json.h"
#include "location.h"
#include "serialize_base.h"

namespace base {

using StatusCodeType = uint16_t;
using StatusGroupType = std::string_view;

namespace internal {

struct StatusData {
  StatusData();
  StatusData(const StatusData&);
  StatusData(StatusGroupType group, StatusCodeType code, std::string message);
  ~StatusData();
  StatusData& operator=(const StatusData&);

  std::unique_ptr<StatusData> copy() const;
  void AddLocation(const Location&);
  std::string group;
  StatusCodeType code;
  std::string message;
  std::vector<Value> frames;
  std::vector<StatusData> causes;
  Object data;
};

template <typename T>
struct StatusTraitsHelper {
  static constexpr std::optional<typename T::Codes> DefaultEnumValue() {
    return DefaultEnumValueImpl(0);
  }

 private:
  template <typename X = T>
  static constexpr typename std::enable_if<
      std::is_pointer<decltype(&X::DefaultEnumValue)>::value,
      std::optional<typename T::Codes>>::type
  DefaultEnumValueImpl(int) {
    static_assert(
        std::is_same<decltype(T::DefaultEnumValue), typename T::Codes()>::value,
        "TypedStatus Traits::DefaultEnumValue() must return Traits::Codes.");
    return T::DefaultEnumValue();
  }

  static constexpr std::optional<typename T::Codes> DefaultEnumValueImpl(...) {
    return {};
  }
};

}  // namespace internal

template <typename T>
class TypedStatus {
  static_assert(std::is_enum<typename T::Codes>::value,
                "TypedStatus Traits::Codes must be an enum type.");
  static_assert(std::is_same<decltype(T::Group), StatusGroupType()>::value,
                "TypedStatus Traits::Group() must return StatusGroupType.");

 public:
  using Traits = T;
  using Codes = typename T::Codes;
  TypedStatus(Codes code,
              std::string message = "",
              const Location& location = Location::Current()) {
    
    if (code == internal::StatusTraitsHelper<T>::DefaultEnumValue()) {
      CHECK(!!message.empty());
      return;
    }
    data_ = std::make_unique<internal::StatusData>(
        T::Group(), static_cast<StatusCodeType>(code),
        std::string(message));
    data_->AddLocation(location);
  }

  TypedStatus(const TypedStatus<T>& copy) { *this = copy; }

  TypedStatus<T>& operator=(const TypedStatus<T>& copy) {
    if (!copy.data_) {
      data_.reset();
      return *this;
    }
    data_ = copy.data_->copy();
    return *this;
  }

  Codes code() const {
    if (!data_)
      return *internal::StatusTraitsHelper<T>::DefaultEnumValue();
    return static_cast<Codes>(data_->code);
  }

  const std::string group() const {
    return data_ ? data_->group : T::Group();
  }

  const std::string& message() const {
    CHECK(data_);
    return data_->message;
  }
  
  TypedStatus<T>&& AddHere(
      const Location& location = Location::Current()) && {
    CHECK(data_);
    data_->AddLocation(location);
    return std::move(*this);
  }

  template <typename D>
  TypedStatus<T>&& WithData(const char* key, const D& value) && {
    CHECK(data_);
    data_->data.SetKey(key, Serialize(value));
    return std::move(*this);
  }

  template <typename AnyTraitsType>
  TypedStatus<T>&& AddCause(TypedStatus<AnyTraitsType>&& cause) && {
    CHECK(data_ && cause.data_);
    data_->causes.push_back(*cause.data_);
    return std::move(*this);
  }

  inline bool operator==(T code) const { return code == this->code(); }

  inline bool operator!=(T code) const { return code != this->code(); }

  inline bool operator==(const TypedStatus<T>& other) const {
    return other.code() == code();
  }

  inline bool operator!=(const TypedStatus<T>& other) const {
    return other.code() != code();
  }

  template <typename OtherType>
  class Or {
   public:
    ~Or() = default;
    Or(TypedStatus<T>&& error) : error_(std::move(error)) {
      CHECK(!internal::StatusTraitsHelper<T>::DefaultEnumValue() ||
            *internal::StatusTraitsHelper<T>::DefaultEnumValue() !=
                 code());
    }
    Or(const TypedStatus<T>& error) : error_(error) {
      CHECK(!internal::StatusTraitsHelper<T>::DefaultEnumValue() ||
            *internal::StatusTraitsHelper<T>::DefaultEnumValue() !=
                 code());
    }

    Or(OtherType&& value) : value_(std::move(value)) {}
    Or(typename T::Codes code,
       const Location& location = Location::Current())
        : error_(TypedStatus<T>(code, "", location)) {
      CHECK(!internal::StatusTraitsHelper<T>::DefaultEnumValue() ||
            *internal::StatusTraitsHelper<T>::DefaultEnumValue() != code);
    }

    Or(Or&&) = default;
    Or& operator=(Or&&) = default;

    bool has_value() const { return value_.has_value(); }
    bool has_error() const { return error_.has_value(); }

    inline bool operator==(typename T::Codes code) const {
      return code == this->code();
    }

    inline bool operator!=(typename T::Codes code) const {
      return code != this->code();
    }

    TypedStatus<T> error() && {
      CHECK(error_);
      auto error = std::move(*error_);
      error_.reset();
      return error;
    }

    OtherType value() && {
      CHECK(value_);
      auto value = std::move(*value_);
      value_.reset();
      return value;
    }

    typename T::Codes code() const {
      CHECK(error_ || (
        value_ && internal::StatusTraitsHelper<Traits>::DefaultEnumValue()));
      return error_ ? error_->code()
                    : *internal::StatusTraitsHelper<Traits>::DefaultEnumValue();
    }

   private:
    std::optional<TypedStatus<T>> error_;
    std::optional<OtherType> value_;
  };

 public:
  std::unique_ptr<internal::StatusData> data_;

  template <typename StatusEnum>
  friend class TypedStatus;

  template<typename Any>
  friend struct Serializer;
};

template <typename T>
inline bool operator==(typename T::Codes code, const TypedStatus<T>& status) {
  return status == code;
}

template <typename T>
inline bool operator!=(typename T::Codes code, const TypedStatus<T>& status) {
  return status != code;
}

}  // namespace base

#endif  // BASE_STATUS_H_