#pragma once


#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr {
public:
    // �������������� ArrayPtr ������� ����������
    ArrayPtr() = default;

    // ������ � ���� ������ �� size ��������� ���� Type.
    // ���� size == 0, ���� raw_ptr_ ������ ���� ����� nullptr
    explicit ArrayPtr(size_t size) {
        if (size == 0) {
            raw_ptr_ = nullptr;
        }
        else {
            raw_ptr_ = new Type[size];
        }
        // ���������� ����������� ��������������
    }

    // ����������� �� ������ ���������, ��������� ����� ������� � ���� ���� nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept :raw_ptr_(raw_ptr) {
    }

    // ��������� �����������
    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
        // �������� ���������� ��������������
    }

    // ��������� ������������
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    // ���������� ��������� �������� � ������, ���������� �������� ������ �������
    // ����� ������ ������ ��������� �� ������ ������ ����������
    [[nodiscard]] Type* Release() noexcept {
        Type* result = raw_ptr_;

        raw_ptr_ = nullptr;

        return result;
    }

    // ���������� ������ �� ������� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    // ���������� ����������� ������ �� ������� ������� � �������� index
    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    // ���������� true, ���� ��������� ���������, � false � ��������� ������
    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    // ���������� �������� ������ ���������, ��������� ����� ������ �������
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // ������������ ��������� ��������� �� ������ � �������� other
    void swap(ArrayPtr& other) noexcept {
        Type* other_ptr = other.Get();
        other.raw_ptr_ = this->raw_ptr_;
        this->raw_ptr_ = other_ptr;

    }

private:
    Type* raw_ptr_ = nullptr;
};