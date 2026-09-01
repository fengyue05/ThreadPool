#ifndef _ANY_H_
#define _ANY_H_

#include <memory>

class Any {
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;

    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    template<typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data)) 
    {}

    template <typename T>
    T cast_() {
        Derive<T>* pd = dynamic_cast<Derive<T>*>(base_.get());
        if (pd == nullptr) {
            throw "type is incompatiable";
        }
        return pd->data_;
    }

private:
    class Base {
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base {
    public:
        Derive(T data) : data_(data) 
        {}

        T data_;
    };

    std::unique_ptr<Base> base_;
};

#endif