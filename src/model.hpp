#pragma once

#include <variant>
#include <vector>
#include <zx/dcel.hpp>
#include <zx/functional.hpp>
#include <zx/sequence.hpp>
#include <zx/triangulation.hpp>

struct Model
{
    std::vector<zx::mat::vector_t<float, 2>> points = {};
    std::optional<zx::geometry::dcel_t<float>> dcel = {};
    std::optional<zx::geometry::dcel_t<float>> voronoi = {};

    void update()
    {
        try
        {
            dcel = zx::geometry::triangulate(points);
        }
        catch (const std::exception& e)
        {
            // std::cout << "error on triangulation: " << e.what() << '\n';
            dcel = std::nullopt;
        }
        if (dcel)
        {
            try
            {
                voronoi = zx::geometry::voronoi(*dcel);
            }
            catch (const std::exception& e)
            {
                // std::cout << "error on voronoi: " << e.what() << '\n';
                voronoi = std::nullopt;
            }
        }
    }
};

namespace Commands
{

struct Init
{
};
struct Exit
{
};
struct AddPoint
{
    zx::mat::vector_t<float, 2> pos;
};

}  // namespace Commands

using Command = std::variant<  //
    Commands::Init,
    Commands::Exit,
    Commands::AddPoint>;
