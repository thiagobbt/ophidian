#include "random_purple_cell_painter.h"

namespace uddac2016 {

typedef struct {
    double r;       // percent
    double g;       // percent
    double b;       // percent
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, v is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
        if( in.g >= max )
            out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
        else
            out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

random_purple_cell_painter::random_purple_cell_painter(std::function<bool(const ophidian::entity_system::entity&)> is_fixed) :
    m_is_fixed(is_fixed),
    m_distribution(0.5, 0.05)
{

}

sf::Color random_purple_cell_painter::color(ophidian::entity_system::entity cell)
{
    if(m_is_fixed(cell))
        return sf::Color(150,150,150);
    hsv color;
    color.h = 300.0;
    color.s = 0.8;
    color.v = std::max(0.0, std::min(m_distribution(m_engine), 1.0));
    rgb rgb_color = hsv2rgb(color);
    return sf::Color(static_cast<sf::Uint8>(rgb_color.r*255), static_cast<sf::Uint8>(rgb_color.g*255), static_cast<sf::Uint8>(rgb_color.b*255));
}


void range_painter::negative(const std::pair<const sf::Color, const sf::Color> &neg)
{
    m_negative = neg;
}

void range_painter::zero(const sf::Color &color)
{
    m_zero = color;
}

void range_painter::positive(const sf::Color &color)
{
    m_positive = color;
}

void range_painter::default_(const sf::Color &color)
{
    m_default = color;
}

void range_painter::color_map(const std::unordered_map<ophidian::entity_system::entity, std::pair<double, bool> > &map)
{
    m_color_map = &map;
    double worst = std::numeric_limits<double>::infinity();
    double best = -std::numeric_limits<double>::infinity();
    for(auto slack : *m_color_map)
    {
        if(std::isinf(slack.second.first)) continue;
        worst = std::min(worst, slack.second.first);
        best = std::max(best, slack.second.first);
    }
    m_range.first = worst;
    m_range.second = best;
}

sf::Color range_painter::color(ophidian::entity_system::entity cell)
{
    auto cell_it = m_color_map->find(cell);

    if(cell_it == m_color_map->end())
        return m_default;

    double cell_value = cell_it->second.first;


    if(std::isinf(cell_value)) return m_default;

    std::pair<double, double> bounds(0.0, 0.0);

    std::pair<sf::Color, sf::Color> color_range;

    if(cell_value < 0)
    {
        bounds.first = m_range.first;
        bounds.second = 0.0;
        color_range.first = m_negative.first;
        color_range.second = m_negative.second;
    }
    else
    {
        bounds.second = m_range.second;
        color_range.first = m_zero;
        color_range.second = m_positive;
    }

    cell_value = (cell_value - bounds.first)/(bounds.second-bounds.first);
    std::pair<hsv, hsv> color_range_hsv;
    std::pair<rgb, rgb> color_range_rgb;
    color_range_rgb.first = rgb{static_cast<double>(color_range.first.r)/255.0, static_cast<double>(color_range.first.g)/255.0, static_cast<double>(color_range.first.b)/255.0};
    color_range_rgb.second = rgb{static_cast<double>(color_range.second.r)/255.0, static_cast<double>(color_range.second.g)/255.0, static_cast<double>(color_range.second.b)/255.0};
    color_range_hsv.first = rgb2hsv(color_range_rgb.first);
    color_range_hsv.second = rgb2hsv(color_range_rgb.second);
    hsv final_color_hsv = color_range_hsv.first;
    final_color_hsv.h += cell_value * (color_range_hsv.second.h-color_range_hsv.first.h);

    if(cell_it->second.second) {
        final_color_hsv.s = .3;
        final_color_hsv.v = .75;
    }
    rgb final_color_rgb = hsv2rgb(final_color_hsv);
    return sf::Color(final_color_rgb.r*255, final_color_rgb.g*255, final_color_rgb.b*255);
}

}
