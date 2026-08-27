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
        zx::mat::vector_t<2, float> location = {};
        zx::mat::vector_t<2, float> velocity = {};
        zx::mat::vector_t<2, float> accelarion = {};
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
    std::vector<zx::mat::vector_t<2, float>> points = {};
    std::optional<zx::geometry::dcel_t<float>> dcel = {};
    std::optional<zx::geometry::dcel_t<float>> voronoi = {};

    void update()
    {
        dcel = std::nullopt;
        voronoi = std::nullopt;

        zx::geometry::triangulate(points)
            .transform(
                [&](const zx::geometry::dcel_t<float>& t)
                {
                    dcel = t;
                    return t;
                })
            .and_then(zx::geometry::tesselate)
            .transform([&](const zx::geometry::dcel_t<float>& t) { voronoi = t; });
    }
};

struct PointsModel
{
    struct Point
    {
        float y;
        anim::animation<float> animation = anim::constant(0.F, 100.F);
        zx::mat::vector_t<2, float> pos = {};
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
    zx::mat::vector_t<2, float> pos;
};

}  // namespace Commands

using Command = std::variant<  //
    Commands::Init,
    Commands::Exit,
    Commands::AddPoint>;
