/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/common.h>
#include <darts/fwd.h>
#include <darts/math.h>
#include <darts/point_kdtree.h>
#include <darts/ray.h>

/** \addtogroup Integrators
    @{
*/

//! Compactly stores the direction and power of a photon along with auxiliary kd-tree information
struct Photon
{
    enum Flags
    {
        SPLITTING_PLANE = (1 << 0) | (1 << 1),
        RADIANCE_PHOTON = 1 << 2
    };

    uint8_t  rgbe[4]; ///< Photon power stored in Greg Ward's RGBE format
    uint16_t flags;   ///< splitting plane for kd-tree
    uint8_t  theta;   ///< Discretized photon direction (\p theta component)
    uint8_t  phi;     ///< Discretized photon direction (\p phi component)

    uint16_t axis() const
    {
        return flags & SPLITTING_PLANE;
    }
    void set_axis(uint16_t a)
    {
        flags &= ~SPLITTING_PLANE;
        flags |= (a & SPLITTING_PLANE);
    }

    /// Dummy constructor
    Photon()
    {
    }

    /// Initialize a photon with the specified direction of propagation and power
    Photon(const Vec3f &dir, const Color3f &power);

    /**
        Convert the stored photon direction from quantized spherical coordinates to a Vector3f value. Uses
        precomputation similar to that of Henrik Wann Jensen's implementation.
    */
    Vec3f direction() const
    {
        return Vec3f(m_cos_phi[phi] * m_sin_theta[theta], m_sin_phi[phi] * m_sin_theta[theta], m_cos_theta[theta]);
    }

    /// Convert the photon power from RGBE to floating point
    Color3f power() const
    {
        return Color3f(rgbe[0], rgbe[1], rgbe[2]) * m_exp_table[rgbe[3]];
    }

protected:
    // ======================================================================
    /// @{ \name Precomputed lookup tables
    // ======================================================================

    static float m_cos_theta[256];
    static float m_sin_theta[256];
    static float m_cos_phi[256];
    static float m_sin_phi[256];
    static float m_exp_table[256];
    static bool  m_precomp_table_ready;

    /// @}
    // ======================================================================

    /// Initialize the precomputed lookup tables
    static bool initialize();
};

/// A kd-tree storing photons in 3D
using PhotonMap = PointKDTree<3, float, Photon>;

//! Structure to store both the photon and its (squared) distance to a query_position location
struct SearchResult
{
    SearchResult(const PhotonMap::Node *p = nullptr, float dist2 = 0.f) : photon(p), dist2(dist2)
    {
    }

    bool operator<(const SearchResult &p2) const
    {
        return dist2 < p2.dist2;
    }

    const PhotonMap::Node *photon;
    float                  dist2;
};

//! Base class for kd-tree search operations
/**
    The #PhotonMap::find() function takes in a Search object which allows it to influence which nodes to visit during
    the search and to accumulate results during the search.

    The object needs to have two functions defined:
      - check(): check the photon being visited and potentially accumulate it into the result
      - #check_children(): \copybrief check_children

    This base class defines the #check_children function, and we provide two derived classes, one which performs a
    simple fixed-radius search (processing *all* photons that are within a fixed radius of the search query_position),
    and a k-nn search process which processes only the k-nearest neighboring photons to the query_position location.
*/
struct SearchBase
{
    Vec3f query_position; ///< The search query position
    float max_dist2;      ///< Search for photons only within this maximum squared distance to #query_position

    SearchBase(const Vec3f &q, float md2) : query_position(q), max_dist2(md2)
    {
    }

    /**
        Determine whether we should continue the search on the left side, right side, or both.

        \param [in] photon          The kd-tree node
        \param [in] bounds          The bounding box of the containing kd-tree node
        \param [in] lo              The start index of the subtree of \p photon
        \param [in] median          The index of \p photon
        \param [in] hi              The end index of the subtree of \p photon
        \param [out] search_left    Set to true if the query overlaps the left child
        \param [out] search_right   Set to true if the query overlaps the right child
        \return                     True if we should check the left child first
    */
    bool check_children(const PhotonMap::Node &photon, const Box3f &bounds, int lo, int median, int hi,
                        bool *search_left, bool *search_right) const
    {
        // compute distance along axis
        auto  axis  = photon.data.axis();
        float dist1 = query_position[axis] - photon.position[axis];

        if (dist1 < 0)
        { // query_position located to the left of the node
            *search_left &= true;
            *search_right &= dist1 * dist1 < max_dist2;
            return true;
        }
        else
        { // query_position located to the right of the node
            *search_right &= true;
            *search_left &= dist1 * dist1 < max_dist2;
            return false;
        }
    }
};

//! A search process which finds just the k-nearest neighboring photons using a max-heap
struct KNNSearch : public SearchBase
{
    std::vector<SearchResult> results;         ///< The collection of up to k photons found
    bool                      is_heap = false; ///< Have we made the heap yet, or is it an unordered array

    KNNSearch(const Vec3f &q, float md2, unsigned max_count) : SearchBase(q, md2)
    {
        results.reserve(max_count);
    }

    void reset()
    {
        results.resize(0);
    }

    void check(const PhotonMap::Node &photon)
    {
        Vec3f to_node = photon.position - query_position;
        float dist2   = length2(to_node);

        // process the current node if it is within the radius
        if (dist2 >= max_dist2)
            return;

        // switch to a max-heap when we run out of available search result space
        if (results.size() < results.capacity())
            // There is still room, just add photon to unordered array of photons
            results.push_back(SearchResult(&photon, dist2));
        else
        {
            if (!is_heap)
            {
                // turn unordered list into a max heap
                std::make_heap(results.begin(), results.end());
                max_dist2 = results.front().dist2;
                is_heap   = true;
            }

            // Remove most distant photon from heap and add new photon
            std::pop_heap(results.begin(), results.end());
            results.back() = SearchResult(&photon, dist2);
            std::push_heap(results.begin(), results.end());
            max_dist2 = results.front().dist2;
        }
    }
};

//! A search process which finds *all* photons within a maximum radius
struct FixedRadiusProcess : public SearchBase
{
    std::function<void(const SearchResult &p)> func; ///< Function to apply to each photon within the radius

    FixedRadiusProcess(const Vec3f &q, float md2, std::function<void(const SearchResult &p)> f) :
        SearchBase(q, md2), func(f)
    {
    }

    void check(const PhotonMap::Node &photon)
    {
        Vec3f to_node = photon.position - query_position;
        float dist2   = length2(to_node);

        // process the current node if it is within the radius
        if (dist2 >= max_dist2)
            return;

        func(SearchResult(&photon, dist2));
    }
};

/** @}*/

/**
    \file
    \brief Common include files and various utility functions.
*/
