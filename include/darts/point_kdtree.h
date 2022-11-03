/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <algorithm>
#include <darts/box.h>
#include <darts/math.h>
#include <utility>
#include <vector>

/**
    An example axis operator that simply forwards the #axis and #set_axis calls to the template type Data.

    \tparam Data    The data associated with a #PointKDTree node #PointKDTree::Node
*/
template <typename Data>
class AxisOperator
{
public:
    /// Return the axis of the node
    static int axis(const Data &data)
    {
        return data.axis();
    }

    /// Set the axis of the node
    static void set_axis(Data &data, int axis)
    {
        return data.set_axis(axis);
    }
};

/**
    A KD-Tree data structure that stores, and accelerates nearest-neighbor queries, of N-dimensional points with
    associated data.

    \tparam N
        The number of dimensions (e.g. 2 or 3)
    \tparam T
        The underlying type of #PointKDTree::Position (e.g. float or double)
    \tparam Data
        Any data type to associate with each Position.
    \tparam AxisOp
        A functor that provides functions to set and retrieve the axis information stored in \p Data
*/
template <int N, class T, typename Data, typename AxisOp = AxisOperator<Data>>
class PointKDTree
{
public:
    /**
        A general \p N -dimensional vector with elements of type \p T.

        Element access is performed using the array [] operator.
     */
    using Position = Vec<N, T>;

    /// A PointKDTree node consisting of a position and some data
    struct Node
    {
        Node() : position(), data()
        {
        }
        Node(const Position &p, const Data &d) : position(p), data(d)
        {
            // empty
        }
        Position position; ///< The position (typically some sort of n-dimensional vector)
        Data     data;     ///< The data to associate with this position
    };
    using Nodes = std::vector<Node>; ///< An array of #PointKDTree::Node
    Nodes     nodes;                 ///< All nodes of the tree.
    Box<N, T> bounds;                ///< Bounding box containing all the elements

    PointKDTree()
    {
    }

    static unsigned dimensions()
    {
        return N;
    }

    /// Return the number of #nodes in the kd-tree
    size_t size() const
    {
        return nodes.size();
    }

    /// Resize the kd-tree to have \p t #nodes
    void resize(size_t t)
    {
        nodes.resize(t);
    }

    /// Ensure that the #nodes vector can store at least \p t elements
    void reserve(size_t t)
    {
        nodes.reserve(t);
    }

    /// Clear all the #nodes of the kd-tree
    void clear()
    {
        nodes.clear();
    }

    /**
        Build (or balance) the kd-tree.

        Calling this function re-arranges the elements of #nodes so that they form a proper kd-tree.
    */
    void build()
    {
        build(0, int(size()) - 1);
    }

    /**
        Set the specified element within the #nodes vector, and updated the #bounds.

        \param [in] index   The index within the #nodes vector to update.
        \param [in] k       The position of the data point
        \param [in] v       Additional data to store with the point.

        \see #insert
    */
    void set(int index, const Position &k, const Data &v)
    {
        nodes[index] = Node(k, v);

        // update the bounding box
        bounds.enclose(k);
    }

    /**
        Insert a data point at the end of the #nodes vector (increasing #size), and updated the #bounds.

        \param [in] k       The position of the data point
        \param [in] v       Additional data to store with the point.

        \see #set
    */
    void insert(const Position &k, const Data &v)
    {
        nodes.push_back(Node(k, v));

        // update the bounding box
        bounds.enclose(k);
    }

    /**
        Perform a search to find nearby data points by traversing the tree.

        The kd-tree must first have been constructrd by calling the #build method.

        \tparam Search  A helper object (see #SearchBase) used to determine which nodes to visit during the traversal
        and to accumulate results during the search.
    */
    template <typename Search>
    void find(Search &search) const
    {
        if (size() <= 0)
            return;

        int lo = 0, hi = int(size() - 1);

        // keep track of bounding box as we traverse
        Box<N, T> box(bounds);

        // Custom stack item
        struct KDTreeStackItem
        {
            int       lo, hi;
            Box<N, T> box;
        };
        // custom stack instead of recursion
        KDTreeStackItem todo[64];
        int             todo_pos = 0;

        while (true)
        {
            int         median = (lo + hi) / 2;
            const Node &node   = nodes[median];

            int axis  = AxisOp::axis(node.data);
            T   split = node.position[axis];

            // process the current node
            search.check(node);

            if (hi - lo >= 1)
            { // the node has children

                bool search_left = median - lo >= 1, search_right = hi - median >= 1;

                if (search.check_children(node, box, lo, median, hi, &search_left, &search_right))
                { // check left child first
                    if (search_left && search_right)
                    {
                        // Enqueue right side in todo list
                        todo[todo_pos].lo            = median + 1;
                        todo[todo_pos].hi            = hi;
                        todo[todo_pos].box           = {box.min, box.max};
                        todo[todo_pos].box.min[axis] = split;
                        ++todo_pos;

                        hi            = median - 1;
                        box.max[axis] = split;
                        continue;
                    }
                }
                else
                { // check right child first
                    if (search_right && search_left)
                    {
                        // Enqueue left side in todo list
                        todo[todo_pos].lo            = lo;
                        todo[todo_pos].hi            = median - 1;
                        todo[todo_pos].box           = {box.min, box.max};
                        todo[todo_pos].box.max[axis] = split;
                        ++todo_pos;

                        lo            = median + 1;
                        box.min[axis] = split;
                        continue;
                    }
                }

                if (search_left)
                {
                    hi            = median - 1;
                    box.max[axis] = split;
                    continue;
                }
                else if (search_right)
                {
                    lo            = median + 1;
                    box.min[axis] = split;
                    continue;
                }
            }

            // Grab next node to search from todo list
            if (todo_pos > 0)
            {
                --todo_pos;
                lo  = todo[todo_pos].lo;
                hi  = todo[todo_pos].hi;
                box = todo[todo_pos].box;
            }
            else
                break;
        }
    }

private:
    //! Recursive function to build the PointKDTree structure.
    void build(int lo, int hi)
    {
        if (hi - lo <= 0)
            return;

        int median = (lo + hi) / 2;

        // find axis to split along (split along the biggest axis)
        Position range = bounds.diagonal();
        unsigned axis  = la::argmax(range);

        // split about the median element
        std::nth_element(nodes.begin() + lo, nodes.begin() + median, nodes.begin() + hi + 1,
                         [&axis](const Node &k0, const Node &k1) { return k0.position[axis] < k1.position[axis]; });

        // store the splitting axis
        Node &node = nodes[median];
        AxisOp::set_axis(node.data, axis);

        // build the left subtree, if there is more then 1 node in it
        T tmp_max        = bounds.max[axis];
        bounds.max[axis] = node.position[axis];
        build(lo, median - 1);
        bounds.max[axis] = tmp_max;

        // build the right subtree, if there is more then 1 node in it
        T tmp_min        = bounds.min[axis];
        bounds.min[axis] = node.position[axis];
        build(median + 1, hi);
        bounds.min[axis] = tmp_min;
    }
};

/**
    \file
    \brief Class #PointKDTree and #AxisOperator
*/