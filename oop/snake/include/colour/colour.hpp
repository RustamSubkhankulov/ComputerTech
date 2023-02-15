#pragma once

//=========================================================

#include <stdio.h>

//---------------------------------------------------------

#include "../vector/vector.hpp"

//=========================================================

class Colour  
{
    Vector rgb_v_{};

    public: 

        Colour() 
            {}

        Colour(unsigned r, unsigned g, unsigned b):
            rgb_v_(r, g, b)
            {
                rgb_norm();
            }

        explicit Colour(const Vector& v):
            rgb_v_(v)
            {
                rgb_norm();
            }

        operator Vector() const
            {
                return rgb_v_;
            }

        ~Colour()             = default;
        Colour(const Colour&) = default;
        Colour(Colour&&)      = default;

        Colour& operator= (const Colour&) = default;
        Colour& operator= (Colour&&)      = default;

        void set(const unsigned r,
                 const unsigned g,
                 const unsigned b)
            {
                rgb_v_.set(r, g, b);
                rgb_norm();
            }

        inline void set_r(const int r);
        inline void set_g(const int g);
        inline void set_b(const int b);

        unsigned char get_r() const 
            {
                return (unsigned char)rgb_v_.x();
            };

        unsigned char get_g() const
            {
                return (unsigned char)rgb_v_.y();
            };

        unsigned char get_b() const 
            {
                return (unsigned char)rgb_v_.z();
            };

        Colour& operator*= (const double mul)
            {
                rgb_v_ *= mul;
                rgb_norm();
                return *this;
            }

        Colour& operator+= (const Colour& c)
            {
                rgb_v_ += c.rgb_v_;
                rgb_norm();
                return *this;
            }

        Colour& operator-= (const Colour& c)
            {
                rgb_v_ -= c.rgb_v_;
                rgb_norm();
                return *this;
            }

        Colour& operator%= (const Colour& c)
            {
                Vector col1 = rgb_v_;
                Vector col2 = c.rgb_v_;

                col1 = col1 * (1 / 255.0);
                col2 = col2 * (1 / 255.0);

                double r = 255 * (col1.x() * col2.x());
                double g = 255 * (col1.y() * col2.y());
                double b = 255 * (col1.z() * col2.z());

                rgb_v_.set(r, g, b);
                rgb_norm();
                
                return *this;
            }

        friend Colour operator-  (const Colour& c);
        friend Colour operator+  (const Colour& a, const Colour& b);
        friend Colour operator-  (const Colour& a, const Colour& b);
        friend Colour operator%  (const Colour& a, const Colour& b);
        friend Colour operator*  (const Colour& c, const double  m);
        friend Colour operator*  (const double  m, const Colour& c);
        friend bool   operator== (const Colour& a, const Colour& b);
        friend Colour col_average(const Colour& a, const Colour& b);

    private:

        void rgb_norm();
};

//---------------------------------------------------------

inline void Colour::set_r(const int r)
{
    unsigned r_ = r;

    if (r > 255)
        r_ = 255;
    else if (r < 0)
        r_ = 0;

    rgb_v_.set_x(double(r_));
}

//---------------------------------------------------------

inline void Colour::set_g(const int g)
{
    unsigned g_ = g;

    if (g > 255)
        g_ = 255;
    else if (g < 0)
        g_ = 0;

    rgb_v_.set_y(double(g_));
}

//---------------------------------------------------------

inline void Colour::set_b(const int b)
{
    unsigned b_ = b;

    if (b > 255)
        b_ = 255;
    else if (b < 0)
        b_ = 0;

    rgb_v_.set_z(double(b_));
}

//---------------------------------------------------------

static const Colour Colour_red    {255,   0,   0};
static const Colour Colour_green  {  0, 255,   0};
static const Colour Colour_blue   {  0,   0, 255};

static const Colour Colour_white  {255, 255, 255};
static const Colour Colour_black  {  0,   0,   0};

static const Colour Colour_yellow {255, 255,   0};
static const Colour Colour_cyan   {  0, 255, 255};
static const Colour Colour_magenta{255,   0, 255};