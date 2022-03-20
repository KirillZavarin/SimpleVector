#pragma once


#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <iterator>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj() = default;

    ReserveProxyObj(size_t capacity) :reserve_capacity(capacity) {};

    size_t getcapacity() {
        return reserve_capacity;
    }
private:
    size_t reserve_capacity = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) :items_(size), size_(size), capacity_(size) {
        for (auto It = begin(); It < end(); It++) {
            *It = Type();
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) :items_(size), size_(size), capacity_(size) {
        for (auto It = begin(); It < end(); It++) {
            *It = std::move(value);
        }
    }

    explicit  SimpleVector(ReserveProxyObj obj) : items_(obj.getcapacity()) {
        size_ = 0;
        capacity_ = obj.getcapacity();
        std::fill(begin(), end(), Type());
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :items_(init.size()) {
        if (init.size() == 0) {
            SimpleVector(0);
        }
        else {
            std::copy(init.begin(), init.end(), begin());
            size_ = init.size();
            capacity_ = static_cast<size_t>(init.size());
        }
    }

    SimpleVector(const SimpleVector<Type>& other) {   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        SimpleVector<Type> temp(other.size_);
        for (size_t i = 0; i < other.capacity_; i++) {
            temp[i] = other[i];
        }
        swap(temp);
    }

    SimpleVector(SimpleVector<Type>&& other) {
        swap(other);
    }

    SimpleVector& operator=(SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector temp(rhs);
        items_.swap(temp.items_);

        std::swap(size_, temp.size_);
        std::swap(capacity_, temp.capacity_);

        return *this;
    }

    SimpleVector& operator=(const SimpleVector&& rhs) {
        if (this == &rhs) {
            return *this;
        }

        SimpleVector temp = SimpleVector(rhs);
        swap(temp);

        return *this;

    }


    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = item;
    }

    void PushBack(Type&& item) {
        if (size_ >= capacity_) {
            // выделяем новый массив с удвоенной вместимостью
            SimpleVector temp_vector(((capacity_ * 2) == 0) ? 1 : capacity_ * 2);
            std::move(begin(),
                end(),
                temp_vector.begin());
            *(temp_vector.items_.Get() + size_) = std::move(item);
            temp_vector.size_ = ++size_;
            swap(temp_vector);
        }
        else {
            // иначе просто добавляем элемент
            *(items_.Get() + size_) = std::move(item);
            ++size_;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        // Напишите тело самостоятельно
        assert(pos >= begin() && pos <= end());
        auto delta_iter = pos - begin();
        if (size_ >= capacity_) {
            SimpleVector temp(((capacity_ * 2) == 0) ? 1 : capacity_ * 2);
            std::move(begin(), begin() + delta_iter, temp.begin());
            *(temp.begin() + delta_iter) = value;
            std::move(begin() + delta_iter, end(), temp.begin() + delta_iter + 1);
            temp.size_ = ++size_;
            swap(temp);
        }
        else {
            std::move_backward(begin() + delta_iter, end(), begin() + delta_iter + 1);
            *(begin() + delta_iter) = value;
            ++size_;
        }
        return begin() + delta_iter;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());

        auto delta_iter = pos - begin();
        if (size_ >= capacity_) {
            SimpleVector temp(((capacity_ * 2) == 0) ? 1 : capacity_ * 2);
            std::move(begin(), begin() + delta_iter, temp.begin());
            *(temp.begin() + delta_iter) = std::move(value);
            std::move(begin() + delta_iter, end(), temp.begin() + delta_iter + 1);
            temp.size_ = ++size_;
            swap(temp);
        }
        else {
            std::move(const_cast<Iterator>(pos), end(), const_cast<Iterator>(pos) + 1);
            *(const_cast<Iterator>(pos)) = std::move(value);
            ++size_;
        }
        return begin() + delta_iter;
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            size_--;
        }
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        std::move(const_cast<Iterator>(pos) + 1, end(), const_cast<Iterator>(pos));
        size_--;
        return const_cast<Iterator>(pos);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index > size");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index > size");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {

            capacity_ = std::max(new_size, capacity_ * 2);

            ArrayPtr<Type> new_items(capacity_);

            std::move(begin(), end(), new_items.Get());
            for (auto It = new_items.Get() + size_; It < new_items.Get() + new_size; It++) {
                *It = Type();
            }

            items_.swap(new_items);
            size_ = new_size;
            return;
        }
        if ((new_size < capacity_) && (new_size >= size_)) {
            for (auto It = end(); It < begin() + new_size; It++) {
                *It = Type();
            }
            size_ = new_size;
        }
        else {
            size_ = new_size;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector temp(new_capacity);
            std::copy(begin(), end(), temp.begin());
            temp.size_ = size_;
            swap(temp);
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return &items_[size_];
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend()const noexcept {
        return &items_[size_];
    }

    ConstIterator begin() const noexcept {
        return cbegin();
    }

    ConstIterator end() const noexcept {
        return cend();
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
private:

    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()
        && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}