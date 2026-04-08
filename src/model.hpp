#pragma once

#include <variant>
#include <vector>
#include <zx/dcel.hpp>
#include <zx/functional.hpp>
#include <zx/sequence.hpp>
#include <zx/triangulation.hpp>

#include "animation.hpp"

struct Boid
{
    struct Linear
    {
        zx::mat::vector_t<float, 2> location = {};
        zx::mat::vector_t<float, 2> velocity = {};
        zx::mat::vector_t<float, 2> accelarion = {};
    };
    struct Angular
    {
        float location = 0.F;
        float velocity = 0.F;
        float acceleration = 0.F;
    };

    Linear linear;
    Angular angular;

    void update(duration_t dt, float max_angular_velocity)
    {
        linear.velocity += linear.accelarion * dt;
        linear.location += linear.velocity * dt;

        angular.velocity += angular.acceleration * dt;
        angular.velocity = std::min(angular.velocity, +max_angular_velocity);
        angular.velocity = std::max(angular.velocity, -max_angular_velocity);
        angular.location += angular.velocity * dt;
    }
};

struct DcelModel
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
                voronoi = std::nullopt;
            }
        }
    }
};

struct PointsModel
{
    struct Point
    {
        float y;
        anim::animation<float> animation = anim::constant(0.F, 100.F);
        zx::mat::vector_t<float, 2> pos = {};
    };

    std::vector<Point> points = {};
    anim::time_point_t time_point = {};
};

struct Model
{
    DcelModel dcel_model = {};
    PointsModel points_model = {};
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
