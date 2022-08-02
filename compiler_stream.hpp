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
class object_stream;

template <class T>
class is_object_stream: std::false_type {};

template <class T>
class is_object_stream<object_stream<T>>: std::true_type {};

template<class T>
constexpr bool is_object_stream_v = is_object_stream<T>::value;

template <class T>
class object_stream{

    std::vector<std::future<T>> stream;

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

    object_stream &operator<<(T const &obj){
        stream.emplace_back(obj);
        return *this;
    }

    object_stream &operator<<(T &&obj){
        stream.emplace_back(std::move(obj));
        return *this;
    }

    template<class F>
    requires (std::is_invocable_v<F, T>)
    auto transform(F const &f){
        using result_object_type = std::invoke_result_t<F, T>;
        auto result_stream = object_stream<result_object_type>{};

        for (auto &&obj: *this)
            result_stream << std::async([](auto const &f, auto &&obj){return std::invoke(f, std::move(obj).get());}, f, std::move(obj));
        return result_stream;
    }

    template<class F>
    requires (std::is_invocable_v<F, T> && is_object_stream_v<std::invoke_result_t<F, T>>)
    auto transform_and_combine(F const &f){
        using result_stream_type = std::invoke_result_t<F, T>;
        auto intermediate_stream = object_stream<result_stream_type>{};
        auto result_stream = result_stream_type{};

        for (auto &&obj: *this)
            intermediate_stream << std::async([](auto const &f, auto &&obj){return std::invoke(f, std::move(obj).get());}, f, std::move(obj));
        for (auto &stream: intermediate_stream)
            result_stream.insert(result_stream.end(), stream.begin(), stream.end());
        return result_stream;
    }

    std::vector<T> get(){
        auto result = std::vector<T>(stream.size());
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