
//=========================================================

#include "../../include/colour/colour.hpp"

//=========================================================

Colour col_average(const Colour& a, const Colour& b)
{
    Vector sum = a.rgb_v_ + b.rgb_v_;
    sum *= 0.5;

    return Colour{(unsigned) sum.x(), (unsigned) sum.y(), (unsigned) sum.z()};
}

//---------------------------------------------------------

void Colour::rgb_norm()
{
    double r = rgb_v_.x();
    double g = rgb_v_.y();
    double b = rgb_v_.z();

    if (r > 255) r = 255;
    if (r < 0)   r = 0;

    if (g > 255) g = 255;
    if (g < 0)   g = 0;

    if (b > 255) b = 255;
    if (b < 0)   b = 0;

    rgb_v_.set(r, g, b);
}

//---------------------------------------------------------

bool operator== (const Colour& a, const Colour& b)
{
    bool r_equal = (a.get_r() == b.get_r());
    bool g_equal = (a.get_g() == b.get_g());
    bool b_equal = (a.get_b() == b.get_b());

    return (r_equal && g_equal && b_equal);
}

Colour operator- (const Colour& c)
{
    return Colour{ - c.rgb_v_ };
}

Colour operator+ (const Colour& a, const Colour& b)
{
    return Colour{a.rgb_v_ + b.rgb_v_};
}

Colour operator- (const Colour& a, const Colour& b)
{
    return Colour{a.rgb_v_ - b.rgb_v_};
}

Colour operator% (const Colour& a, const Colour& b)
{
    Colour c = a;
    c %= b;

    return c;
}

Colour operator* (const Colour& c, const double m)
{
    return Colour{c.rgb_v_ * m};
}

Colour operator* (const double m, const Colour& c)
{
    return Colour{m * c.rgb_v_};
}
