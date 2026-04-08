#pragma once

#include "app_runner.hpp"
#include "canvas.hpp"
#include "model.hpp"

struct Render
{
    sf::Color voronoi_outline_color = sf::Color::Red;
    sf::Color dcel_outline_color = sf::Color::White;
    sf::Color point_fill_color = sf::Color::Yellow;

    canvas::DrawOp operator()(const DcelModel& m, fps_t fps) const
    {
        std::vector<canvas::DrawOp> items;
        if (m.voronoi)
        {
            for (const auto& face : m.voronoi->faces())
            {
                items.push_back(
                    canvas::polygon(face.as_polygon())            //
                    | canvas::outline_thickness(1.F)              //
                    | canvas::fill_color(sf::Color::Transparent)  //
                    | canvas::outline_color(voronoi_outline_color));
            }
        }
        if (m.dcel)
        {
            for (const auto& face : m.dcel->faces())
            {
                items.push_back(
                    canvas::polygon(face.as_polygon())            //
                    | canvas::outline_thickness(1.5F)             //
                    | canvas::fill_color(sf::Color::Transparent)  //
                    | canvas::outline_color(dcel_outline_color));
            }
        }
        items.push_back(canvas::transform(
            [this](const zx::mat::vector_t<float, 2>& p) -> canvas::DrawOp
            { return canvas::point(p, 5.F) | canvas::fill_color(point_fill_color); },
            m.points));

        return canvas::group(std::move(items));
    }

    canvas::DrawOp operator()(const PointsModel& m, fps_t fps) const
    {
        return canvas::transform(
            [this](const PointsModel::Point& point) -> canvas::DrawOp
            { return canvas::point(point.pos, 5.F) | canvas::fill_color(point_fill_color); },
            m.points);
    }

    canvas::DrawOp operator()(const Model& m, fps_t fps) const
    {
        return canvas::group((*this)(m.dcel_model, fps), (*this)(m.points_model, fps));
    }
};
