#pragma once

#include <compare>
#include <memory>

namespace util
{

/**
 * Вспомогательный шаблонный класс "Маркированный тип".
 * С его помощью можно описать строгий тип на основе другого типа.
 */
template <typename Value, typename Tag>
class Tagged
{
public:
    using ValueType = Value;
    using TagType = Tag;

    explicit Tagged(Value&& v) : m_value(std::move(v))
    {
    }
    explicit Tagged(const Value& v) : m_value(v)
    {
    }

    const Value& operator*() const
    {
        return m_value;
    }

    Value& operator*()
    {
        return m_value;
    }

    // Spaceship opertor
    // Так в C++20 можно объявить оператор сравнения Tagged-типов
    // Будет просто вызван соответствующий оператор для поля m_value
    auto operator<=>(const Tagged<Value, Tag>&) const = default;
private:
    Value m_value;
};

// Хешер для Tagged-типа, чтобы Tagged-объекты можно было хранить в unordered-контейнерах
template <typename TaggedValue>
struct TaggedHasher
{
    size_t operator()(const TaggedValue& value) const
    {
        // Возвращает хеш значения, хранящегося внутри value
        return std::hash<typename TaggedValue::ValueType>{}(*value);
    }
};

} // namespace util
