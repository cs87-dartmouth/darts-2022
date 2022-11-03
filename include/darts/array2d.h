/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/math.h>
#include <utility>
#include <vector>

/// Generic, resizable, 2D array class.
template <typename T>
class Array2d
{
public:
    Array2d();                                               ///< empty array, 0 by 0 elements
    Array2d(int size_x, int size_y, const T &value = T(0.)); ///< size_x by size_y elements
    Array2d(const Array2d &);                                ///< copy constructor
    Array2d &operator=(const Array2d &);                     ///< assignment operator
    ~Array2d();                                              ///< destructor

    ///\{ \name Element access
    T       &at(int x, int y);               ///< bounds-checked 2d access
    const T &at(int x, int y) const;         ///< bounds-checked 2d access
    T       &operator()(int x, int y);       ///< unchecked 2d access
    const T &operator()(int x, int y) const; ///< unchecked 2d access
    T       &at(int i);                      ///< bounds-checked linear access
    const T &at(int i) const;                ///< bounds-checked linear access
    T       &operator()(int i);              ///< unchecked linear access
    const T &operator()(int i) const;        ///< unchecked linear access
    const T *row(int y) const;               ///< pointer to `y`-th row
    ///\}

    /// Convert the 2d index into a linear index
    int index_1d(int x, int y) const
    {
        return y * m_size.x + x;
    }

    /// Convert the 1d index into a 2d index
    std::pair<int, int> index_2d(int i) const
    {
        return std::make_pair(i % m_size.x, i / m_size.x);
    }

    ///\{ \name Dimension sizes
    // clang-format off
    int   width() const  {return m_size.x;}             ///< size of first dimension
    int   height() const {return m_size.y;}             ///< size of second dimension
    Vec2i size() const   {return m_size;}               ///< size along both directions
    int   length() const {return m_size.x * m_size.y;}  ///< total number of elements
    // clang-format on
    ///\}

    void resize(const Vec2i &size);    ///< Resize to (size.x) x (size.y)
    void reset(const T &value = T(0)); ///< Set all elements to `value`
    void operator=(const T &);         ///< Set all elements to `value`

protected:
    std::vector<T> m_data; ///< the data
    Vec2i          m_size; ///< size along both directions
};

template <typename T>
inline Array2d<T>::Array2d() : m_data(0), m_size(0, 0)
{
    // empty
}

template <typename T>
inline Array2d<T>::Array2d(int size_x, int size_y, const T &value) :
    m_data(size_x * size_y, value), m_size(size_x, size_y)
{
    // empty
}

template <class T>
Array2d<T>::Array2d(const Array2d<T> &other) : m_data(other.m_data), m_size(other.m_size)
{
}

template <class T>
Array2d<T> &Array2d<T>::operator=(const Array2d<T> &other)
{
    m_size = other.m_size;
    m_data = other.m_data;
    return *this;
}

template <typename T>
inline Array2d<T>::~Array2d()
{
    // empty
}

template <typename T>
inline T &Array2d<T>::operator()(int x, int y)
{
    return m_data[index_1d(x, y)];
}

template <typename T>
inline const T &Array2d<T>::operator()(int x, int y) const
{
    return m_data[index_1d(x, y)];
}

template <typename T>
inline T &Array2d<T>::at(int x, int y)
{
    return m_data.at(index_1d(x, y));
}

template <typename T>
inline const T &Array2d<T>::at(int x, int y) const
{
    return m_data.at(index_1d(x, y));
}

template <typename T>
inline T &Array2d<T>::operator()(int i)
{
    return m_data[i];
}

template <typename T>
inline const T &Array2d<T>::operator()(int i) const
{
    return m_data[i];
}

template <typename T>
inline T &Array2d<T>::at(int i)
{
    return m_data.at(i);
}

template <typename T>
inline const T &Array2d<T>::at(int i) const
{
    return m_data.at(i);
}

template <typename T>
inline const T *Array2d<T>::row(int y) const
{
    return &m_data[index_1d(0, y)];
}

template <typename T>
inline void Array2d<T>::resize(const Vec2i &size)
{
    if (size == m_size)
        return;

    m_data.resize(size.x * size.y);
    m_size = size;
}

template <typename T>
inline void Array2d<T>::reset(const T &value)
{
    for (auto &e : m_data)
        e = value;
}

template <typename T>
inline void Array2d<T>::operator=(const T &value)
{
    reset(value);
}

/**
    \file
    \brief Class #Array2d
*/
