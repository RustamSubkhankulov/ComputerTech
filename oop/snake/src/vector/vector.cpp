#include <math.h>

//=========================================================

#include "../../include/vector/vector.hpp" 

//=========================================================

// Works only for 2d
Vector orthogonal_2d_only(const Vector& v)
{
    return Vector {v.y_, -v.x_};
}

//---------------------------------------------------------

Vector operator+ (const Vector& a, const Vector& b)
{
    Vector res = a;
    res += b;
    return res;
}

Vector operator- (const Vector& a, const Vector& b)
{
    Vector res = a;
    res -= b;
    return res;
}

Vector operator* (const Vector& v, const ssize_t mul)
{
    return Vector {v.x_ * mul, v.y_ * mul, v.z_ * mul};
}

Vector operator* (const ssize_t mul, const Vector& v)
{
    return Vector {v.x_ * mul, v.y_ * mul, v.z_ * mul};
}

ssize_t operator* (const Vector& a, const Vector& b)
{
    return a.x_ * b.x_ + a.y_ * b.y_ + a.z_ * b.z_;
}

//---------------------------------------------------------

Vector operator- (const Vector& v)
{
    return Vector{ - v.x_, - v.y_, - v.z_ };
}

//---------------------------------------------------------

void Vector::rotate_z(const double rad_angle)
{
    double sinus  = sin(rad_angle);
    double cosine = cos(rad_angle);

    rotate_z(sinus, cosine);
}

void Vector::rotate_z(const double sin, const double cos)
{
    ssize_t x1 = x_;
    ssize_t y1 = y_;

    set_x((ssize_t) (cos * (double) x1 - sin * (double) y1));
    set_y((ssize_t) (sin * (double) x1 + cos * (double) y1));
}

//---------------------------------------------------------

void Vector::rotate_x(const double rad_angle)
{
    double sinus  = sin(rad_angle);
    double cosine = cos(rad_angle);

    rotate_x(sinus, cosine);
}

void Vector::rotate_x(const double sin, const double cos)
{
    ssize_t y1 = y_;
    ssize_t z1 = z_;

    set_y((ssize_t) (cos * (double) y1 - sin * (double) z1));
    set_z((ssize_t) (sin * (double) y1 + cos * (double) z1));
}

//---------------------------------------------------------


void Vector::rotate_y(const double rad_angle)
{
    double sinus  = sin(rad_angle);
    double cosine = cos(rad_angle);

    rotate_y(sinus, cosine);
}

void Vector::rotate_y(const double sin, const double cos)
{
    ssize_t x1 = x_;
    ssize_t z1 = z_;

    set_x((ssize_t)(cos * (double) x1 - sin * (double) z1));
    set_z((ssize_t)(sin * (double) x1 + cos * (double) z1));
}

//---------------------------------------------------------

void Vector::normalize()
{
    ssize_t length = len();

    if (length != 0)
        (*this) *= (1 / length);
}

//---------------------------------------------------------