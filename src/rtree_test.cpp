/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2018 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#include <mdds/rtree.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "test_global.hpp"

using namespace mdds::draft;
using namespace std;

struct tiny_trait_1d
{
    constexpr static size_t dimensions = 1;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = false;
    constexpr static size_t reinsertion_size = 2;
};

struct tiny_trait_2d
{
    constexpr static size_t dimensions = 2;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = false;
    constexpr static size_t reinsertion_size = 2;
};

struct tiny_trait_2d_forced_reinsertion
{
    constexpr static size_t dimensions = 2;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = true;
    constexpr static size_t reinsertion_size = 2;
};

void rtree_test_intersection()
{
    stack_printer __stack_printer__("::rtree_test_intersection");
    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::extent_type;
    using detail::rtree::calc_linear_intersection;
    using detail::rtree::calc_intersection;

    struct check
    {
        bounding_box bb1;
        bounding_box bb2;
        int16_t expected_length1;
        int16_t expected_length2;
    };

    std::vector<check> checks =
    {
        // bounding box 1           bounding box 2
        { { { 0,  0}, { 3,   6} }, { {1,   2}, { 7,  5} },  2, 3 },
        { { { 3,  2}, { 7,  10} }, { {1,  10}, {10, 11} },  4, 0 },
        { { { 3,  2}, { 7,  10} }, { {1,   9}, {10, 11} },  4, 1 },
        { { { 3,  2}, { 7,   6} }, { {5,   4}, {11,  8} },  2, 2 },
        { { {-2, -8}, { 2,  -5} }, { {0, -10}, { 8, -1} },  2, 3 },
        { { { 2,  2}, {20,  12} }, { {5,   6}, {16,  9} }, 11, 3 },
        { { { 0,  0}, { 6,   6} }, { {0,   0}, { 2,  3} },  2, 3 },
    };

    for (const check& c : checks)
    {
        int16_t length1 = calc_linear_intersection<bounding_box>(0, c.bb1, c.bb2);
        assert(length1 == c.expected_length1);
        int16_t length2 = calc_linear_intersection<bounding_box>(1, c.bb1, c.bb2);
        assert(length2 == c.expected_length2);

        int16_t area = calc_intersection<bounding_box>(c.bb1, c.bb2);
        int16_t expected_area = c.expected_length1 * c.expected_length2;
        assert(area == expected_area);

        // Swap the boxes and run the same tests. We should get the same results.

        length1 = calc_linear_intersection<bounding_box>(0, c.bb2, c.bb1);
        assert(length1 == c.expected_length1);
        length2 = calc_linear_intersection<bounding_box>(1, c.bb2, c.bb1);
        assert(length2 == c.expected_length2);

        area = calc_intersection<bounding_box>(c.bb2, c.bb1);
        assert(area == expected_area);
    }
}

void rtree_test_square_distance()
{
    stack_printer __stack_printer__("::rtree_test_square_distance");
    using detail::rtree::calc_square_distance;

    {
        // 1 dimensional unsigned
        using rt_type = rtree<uint16_t, std::string, tiny_trait_1d>;
        using point_type = rt_type::point_type;

        struct test_case
        {
            point_type p1;
            point_type p2;
            uint16_t expected;
        };

        std::vector<test_case> tcs =
        {
            { {3}, {5},  4 },
            { {9}, {2}, 49 },
            { {0}, {0},  0 },
        };

        for (const test_case& tc : tcs)
        {
            cout << "p1: " << tc.p1.to_string() << "; p2: " << tc.p2.to_string() << endl;

            uint16_t dist = calc_square_distance<point_type>(tc.p1, tc.p2);
            assert(dist == tc.expected);

            // Flip the value to make sure we still get the same result.
            dist = calc_square_distance<point_type>(tc.p2, tc.p1);
            assert(dist == tc.expected);
        }
    }

    {
        // 2 dimensional unsigned
        using rt_type = rtree<uint16_t, std::string, tiny_trait_2d>;
        using point_type = rt_type::point_type;

        struct test_case
        {
            point_type p1;
            point_type p2;
            uint16_t expected;
        };

        std::vector<test_case> tcs =
        {
            { {0, 0}, {0, 0},  0 },
            { {0, 0}, {1, 1},  2 },
            { {0, 0}, {2, 2},  8 },
            { {3, 0}, {0, 4},  25 },
        };

        for (const test_case& tc : tcs)
        {
            cout << "p1: " << tc.p1.to_string() << "; p2: " << tc.p2.to_string() << endl;

            uint16_t dist = calc_square_distance<point_type>(tc.p1, tc.p2);
            assert(dist == tc.expected);

            // Flip the value to make sure we still get the same result.
            dist = calc_square_distance<point_type>(tc.p2, tc.p1);
            assert(dist == tc.expected);
        }
    }
}

void rtree_test_center_point()
{
    stack_printer __stack_printer__("::rtree_test_center_point");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;
    using detail::rtree::get_center_point;
    using extent_type = rt_type::extent_type;
    using point_type = rt_type::point_type;

    struct test_case
    {
        extent_type extent;
        point_type expected;
    };

    std::vector<test_case> tcs =
    {
        { {{0, 0}, {2, 2}}, {1, 1} },
        { {{-2, -4}, {2, 4}}, {0, 0} },
        { {{3, 5}, {8, 10}}, {5, 7} },
    };

    for (const test_case& tc : tcs)
    {
        cout << "extent: " << tc.extent.to_string() << endl;
        auto pt = get_center_point(tc.extent);
        assert(pt == tc.expected);
    }
}

void rtree_test_area_enlargement()
{
    stack_printer __stack_printer__("::rtree_test_area_enlargement");
    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::extent_type;
    using detail::rtree::calc_area_enlargement;

    struct check
    {
        bounding_box host;
        bounding_box guest;
        int16_t expected_area;
    };

    std::vector<check> checks =
    {
        { {{0,0}, {2,2}}, {{ 2, 2}, { 3, 3}},  5 }, // 3x3 - 2x2 = 5
        { {{0,0}, {2,2}}, {{ 0, 1}, { 1, 2}},  0 }, // no enlargement
        { {{0,0}, {3,3}}, {{-3, 1}, { 7, 2}}, 21 }, // 10x3 - 3x3 = 21
        { {{0,0}, {0,0}}, {{-1,-1}, { 0, 0}},  1 }, // 1x1 = 1
        { {{0,0}, {1,1}}, {{-4,-3}, {-3,-2}}, 19 }, // 5x4 - 1x1 = 19
    };

    for (const check& c : checks)
    {
        int16_t area = calc_area_enlargement(c.host, c.guest);
        assert(area == c.expected_area);
    }
}

void rtree_test_basic_search()
{
    stack_printer __stack_printer__("::rtree_test_basic_search");
    using rt_type = rtree<int16_t, std::string>;

    rt_type tree;
    rt_type::extent_type expected_bb;

    tree.insert({0, 0}, {2, 2}, "test");
    expected_bb = {{0, 0}, {2, 2}};
    assert(tree.get_root_extent() == expected_bb);
    assert(tree.size() == 1);

    tree.insert({3, 3}, {5, 5}, "test again");
    expected_bb = {{0, 0}, {5, 5}};
    assert(tree.get_root_extent() == expected_bb);
    assert(tree.size() == 2);

    tree.insert({-2, 1}, {3, 6}, "more test");
    expected_bb = {{-2, 0}, {5, 6}};
    assert(tree.get_root_extent() == expected_bb);
    assert(tree.size() == 3);

    tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);

    // Verify the search method works.

    rt_type::const_search_results res = tree.search({1, 1});

    auto it = res.cbegin(), it_end = res.cend();

    size_t n = std::distance(it, it_end);
    assert(n == 2);

    std::unordered_map<std::string, rt_type::extent_type> expected_values =
    {
        { "test",      {{ 0, 0}, {2, 2}} },
        { "more test", {{-2, 1}, {3, 6}} },
    };

    for (; it != it_end; ++it)
    {
        cout << "bounding box: " << it->box.to_string() << "; value: " << it->value << "; depth: " << it->depth << endl;
        auto itv = expected_values.find(it->value);
        assert(itv != expected_values.end());
        assert(itv->second == it->box);
        assert(it->depth == 1);
    }

    // Perform an out-of-bound search by point.
    std::vector<rt_type::point_type> pts =
    {
        {-10, -10},
        {1, 7},
        {6, 3},
    };

    for (const rt_type::point_type& pt : pts)
    {
        res = tree.search(pt);
        assert(res.cbegin() == res.cend());
    }
}

void rtree_test_basic_erase()
{
    stack_printer __stack_printer__("::rtree_test_basic_erase");
    using rt_type = rtree<int16_t, std::string>;

    rt_type tree;
    tree.insert({-2,-2}, {2,2}, "erase me");
    assert(!tree.empty());
    assert(tree.size() == 1);

    rt_type::const_search_results res = tree.search({0,0});

    size_t n = std::distance(res.begin(), res.end());
    assert(n == 1);

    rt_type::const_iterator it = res.begin();
    assert(it != res.end());

    tree.erase(it);
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(rt_type::extent_type() == tree.get_root_extent());

    tree.insert({0,0}, {2,2}, "erase me");
    tree.insert({-10,-4}, {0,0}, "erase me");
    rt_type::extent_type expected_bb({-10,-4}, {2,2});
    assert(tree.get_root_extent() == expected_bb);
    assert(tree.size() == 2);

    res = tree.search({-5,-2});
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    tree.erase(it);
    assert(!tree.empty()); // there should be one value stored in the tree.
    assert(tree.size() == 1);
    expected_bb = {{0,0}, {2,2}};
    assert(tree.get_root_extent() == expected_bb);

    tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);
}

void rtree_test_node_split()
{
    stack_printer __stack_printer__("::rtree_test_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;

    rt_type tree;

    // Inserting 6 entries should cause the root directory node to split.
    // After the split, the root node should become a non-leaf directory
    // storing two leaf directory nodes as its children.

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "foo" << i;
        tree.insert({i, i}, {int16_t(i+w), int16_t(i+w)}, os.str());
    }

    assert(tree.size() == 6);

    cout << tree.export_tree(rt_type::export_tree_type::formatted_node_properties) << endl;

    size_t count_values = 0;
    size_t count_leaf = 0;
    size_t count_nonleaf = 0;

    auto walker = [&](const rt_type::node_properties& np)
    {
        switch (np.type)
        {
            case rt_type::node_type::value:
                ++count_values;
                break;
            case rt_type::node_type::directory_leaf:
                ++count_leaf;
                break;
            case rt_type::node_type::directory_nonleaf:
                ++count_nonleaf;
                break;
            default:
                ;
        }
    };

    tree.walk(walker);

    assert(count_values == 6);
    assert(count_leaf == 2);
    assert(count_nonleaf == 1);

    tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);

    // Adding two more entries will cause one of the leaf directory nodes
    // below the root node to split.

    for (int16_t i = 6; i < 8; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "bar" << i;
        tree.insert({i, i}, {int16_t(i+w), int16_t(i+w)}, os.str());
    }

    assert(tree.size() == 8);
    tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);

    // Count all the nodes again.
    count_values = 0;
    count_leaf = 0;
    count_nonleaf = 0;

    tree.walk(walker);

    assert(count_values == 8);
    assert(count_leaf == 3);
    assert(count_nonleaf == 1);

    // Erase the entry at (0, 0).  There should be only one match.  Erasing
    // this entry will cause the node to be underfilled.

    rt_type::const_search_results res = tree.search({0,0});
    auto it = res.cbegin();
    assert(it != res.cend());
    assert(std::distance(it, res.cend()) == 1);
    tree.erase(it);

    assert(tree.size() == 7);
    tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);

    // Count all the nodes again.
    count_values = 0;
    count_leaf = 0;
    count_nonleaf = 0;

    tree.walk(walker);

    assert(count_values == 7);
    assert(count_leaf == 2);
    assert(count_nonleaf == 1);
}

void rtree_test_directory_node_split()
{
    stack_printer __stack_printer__("::rtree_test_directory_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;

    rt_type tree;
    using point = rt_type::point_type;
    using bounding_box = rt_type::extent_type;

    for (int16_t x = 0; x < 10; ++x)
    {
        for (int16_t y = 0; y < 10; ++y)
        {
            std::ostringstream os;
            os << "(x=" << x << ",y=" << y << ")";
            std::string v = os.str();
            int16_t xe = x + 1, ye = y + 1;
            point s({x,y}), e({xe,ye});
            bounding_box bb(s, e);
            cout << "Inserting value '" << v << "' to {" << bb.to_string() << "} ..." << endl;
            tree.insert(s, e, v);
            tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);
        }
    }

    assert(tree.size() == 100);

    // All value nodes in this tree should be at depth 4 (root having the
    // depth of 0).  Just check a few of them.

    std::vector<point> pts =
    {
        { 5, 5 },
        { 2, 3 },
        { 7, 9 },
    };

    for (const point& pt : pts)
    {
        auto res = tree.search(pt);
        auto it = res.cbegin();
        assert(it != res.cend());
        assert(it->depth == 4);
    }
}

void rtree_test_erase_directories()
{
    stack_printer __stack_printer__("::rtree_test_erase_directories");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;

    rt_type tree;
    using point = rt_type::point_type;
    using bounding_box = rt_type::extent_type;

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            std::ostringstream os;
            int16_t x2 = x * 2;
            int16_t y2 = y * 2;
            os << "(x=" << x2 << ",y=" << y2 << ")";
            std::string v = os.str();
            int16_t xe = x2 + 2, ye = y2 + 2;
            point s({x2, y2}), e({xe, ye});
            bounding_box bb(s, e);
            tree.insert(s, e, v);
        }
    }

    assert(tree.size() == 25);
    tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);
    cout << tree.export_tree(rt_type::export_tree_type::formatted_node_properties) << endl;

    size_t expected_size = 25;

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            int16_t x2 = x * 2 + 1;
            int16_t y2 = y * 2 + 1;

            cout << "erase at (" << x2 << ", " << y2 << ")" << endl;

            auto res = tree.search({x2, y2});
            auto it = res.begin(), ite = res.end();
            size_t n = std::distance(it, ite);
            assert(n == 1);

            tree.erase(it);

            assert(tree.size() == --expected_size);
            tree.check_integrity(rt_type::integrity_check_type::throw_on_fail);

            res = tree.search({x2, y2});
            n = std::distance(res.begin(), res.end());
            assert(n == 0);
        }
    }

    assert(tree.empty());
    assert(tree.size() == 0);
}

void rtree_test_forced_reinsertion()
{
    stack_printer __stack_printer__("::rtree_test_forced_reinsertion");

    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;

    rt_type tree;

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "foo" << i;
        tree.insert({i, i}, {int16_t(i+w), int16_t(i+w)}, os.str());
    }

    assert(tree.size() == 6);
    tree.check_integrity(rt_type::integrity_check_type::whole_tree);

    tree.clear();
    assert(tree.empty());
    assert(tree.size() == 0);
    tree.check_integrity(rt_type::integrity_check_type::whole_tree);

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            std::ostringstream os;
            int16_t x2 = x * 2;
            int16_t y2 = y * 2;
            os << "(x=" << x2 << ",y=" << y2 << ")";
            std::string v = os.str();
            int16_t xe = x2 + 2, ye = y2 + 2;
            tree.insert({x2, y2}, {xe, ye}, v);
        }
    }

    tree.check_integrity(rt_type::integrity_check_type::whole_tree);
    assert(tree.size() == 25);
}

int main(int argc, char** argv)
{
    rtree_test_intersection();
    rtree_test_square_distance();
    rtree_test_center_point();
    rtree_test_area_enlargement();
    rtree_test_basic_search();
    rtree_test_basic_erase();
    rtree_test_node_split();
    rtree_test_directory_node_split();
    rtree_test_erase_directories();
    rtree_test_forced_reinsertion();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

