#ifndef COMPILER_STREAM_HPP
#define COMPILER_STREAM_HPP

#include <vector>
#include <future>
#include <type_traits>
// #include <concepts> // std::invokable

namespace utlang{


/*
    A stream of objects of type T
    Created by one process with object_stream<T>{} << t1 << t2 << ...
    transformed/created by another process with objs.tranform() or objs.tranform_and_combine()
    returns stored values (and waits completion) with .get()
*/

template <class T>
class object_pipeline;

template <class T>
class is_object_stream: public std::false_type {};

template <class T>
class is_object_stream<object_pipeline<T>>: public std::true_type {};

template<class T>
constexpr bool is_object_stream_v = is_object_stream<T>::value;

template<class T>
constexpr std::remove_const_t<std::remove_reference_t<T>> identity(T t){
    return std::move(t);
}

template <class T>
class object_pipeline{
    public:

    // static constexpr auto identity = [](T const &t){return t;};

    std::vector<std::future<T>> stream;

    object_pipeline(){};
    object_pipeline(object_pipeline const &) = delete;
    object_pipeline(object_pipeline &&) = default;
    object_pipeline &operator=(object_pipeline const &) = delete;
    object_pipeline &operator=(object_pipeline &&) = default;

    auto begin(){
        return stream.begin();
    }

    auto begin() const{
        return stream.begin();
    }

    auto end(){
        return stream.end();
    }

    auto end() const{
        return stream.end();
    }

    
    object_pipeline &operator<<(T const &obj) /*requires(std::is_copy_constructible_v<T>)*/ {
        stream.push_back(std::async(identity<decltype(obj)>, obj));
        return *this;
    }

    // object_stream &operator<<(T const &obj) requires(not std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T> and std::is_default_constructible_v<T>) {
    //     T copy{};
    //     copy = obj;
    //     stream.push_back(std::async(identity<decltype(obj)>, std::move(copy)));
    //     return *this;
    // }

    object_pipeline &operator<<(T &&obj){
        stream.push_back(std::async(identity<decltype(std::move(obj))>, std::move(obj)));
        return *this;
    }

    template<class F>
    requires (std::is_invocable_v<F, T>)
    auto transform(F const &f){
        using result_object_type = std::invoke_result_t<F, T>;
        auto result_stream = object_pipeline<result_object_type>{};

        for (auto &obj: *this)
            result_stream.stream.push_back(std::async([](auto const &f, auto &&obj){return std::invoke(f, std::move(obj).get());}, f, std::move(obj)));
        return result_stream;
    }

    template<class F>
    requires (std::is_invocable_v<F, T> && is_object_stream_v<std::invoke_result_t<F, T>>)
    auto transform_and_combine(F const &f){
        using result_stream_type = std::invoke_result_t<F, T>;
        auto intermediate_stream = object_pipeline<result_stream_type>{}; // problem
        // auto gg = object_stream<object_stream<int>>{};
        auto result_stream = result_stream_type{};

        for (auto &obj: *this)
            intermediate_stream.stream.push_back(std::async([](auto const &f, auto &&obj){return std::invoke(f, std::move(obj).get());}, f, std::move(obj)));
        for (auto &substream: intermediate_stream)
            for (auto &obj: std::move(substream).get())
                result_stream.stream.push_back(std::move(obj));
            // result_stream.stream.insert(result_stream.end(), substream.get().begin(), substream.get().end());
        return result_stream;
    }

    std::vector<T> get(){
        auto result = std::vector<T>{};
        result.reserve(stream.size());
        for (auto &obj: *this){
            if (obj.valid())[[likely]]
                result.push_back(obj.get());
            else
                throw std::future_error(std::future_errc::no_state);
        }
        return result;
    }
};


}

#endif