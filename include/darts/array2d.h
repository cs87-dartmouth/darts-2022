/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

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
        return y * m_size_x + x;
    }

    /// Convert the 1d index into a 2d index
    std::pair<int, int> index_2d(int i) const
    {
        return std::make_pair(i % m_size_x, i / m_size_x);
    }

    ///\{ \name Dimension sizes
    int width() const
    {
        return m_size_x;
    } ///< size of first dimension
    int height() const
    {
        return m_size_y;
    } ///< size of second dimension

    int size() const
    {
        return m_size_x * m_size_y;
    } ///< total number of elements
    int size_x() const
    {
        return m_size_x;
    } ///< size of first dimension
    int size_y() const
    {
        return m_size_y;
    } ///< size of first dimension
    ///\}

    void resize(int size_x, int size_y); ///< Resize to `size_x` x `size_y`
    void reset(const T &value = T(0));   ///< Set all elements to `value`
    void operator=(const T &);           ///< Set all elements to `value`

protected:
    std::vector<T> m_data;   ///< the data
    int            m_size_x; ///< size of first dimension
    int            m_size_y; ///< size of second dimension
};

template <typename T>
inline Array2d<T>::Array2d() : m_data(0), m_size_x(0), m_size_y(0)
{
    // empty
}

template <typename T>
inline Array2d<T>::Array2d(int size_x, int size_y, const T &value) :
    m_data(size_x * size_y, value), m_size_x(size_x), m_size_y(size_y)
{
    // empty
}

template <class T>
Array2d<T>::Array2d(const Array2d<T> &other) : m_data(other.m_data), m_size_x(other.m_size_x), m_size_y(other.m_size_y)
{
}

template <class T>
Array2d<T> &Array2d<T>::operator=(const Array2d<T> &other)
{
    m_size_x = other.m_size_x;
    m_size_y = other.m_size_y;
    m_data   = other.m_data;
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
inline void Array2d<T>::resize(int size_x, int size_y)
{
    if (size_x == m_size_x && size_y == m_size_y)
        return;

    m_data.resize(size_x * size_y);
    m_size_x = size_x;
    m_size_y = size_y;
}

template <typename T>
inline void Array2d<T>::reset(const T &value)
{
    int size = m_size_x * m_size_y;
    for (int i = 0; i < size; i++)
        m_data[i] = value;
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
