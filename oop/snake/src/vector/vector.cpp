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

void Vector::rotate_z(const ssize_t rad_angle)
{
    ssize_t sinus  = sin(rad_angle);
    ssize_t cosine = cos(rad_angle);

    rotate_z(sinus, cosine);
}

void Vector::rotate_z(const ssize_t sin, const ssize_t cos)
{
    ssize_t x1 = x_;
    ssize_t y1 = y_;

    set_x(cos * x1 - sin * y1);
    set_y(sin * x1 + cos * y1);
}

//---------------------------------------------------------

void Vector::rotate_x(const ssize_t rad_angle)
{
    ssize_t sinus  = sin(rad_angle);
    ssize_t cosine = cos(rad_angle);

    rotate_x(sinus, cosine);
}

void Vector::rotate_x(const ssize_t sin, const ssize_t cos)
{
    ssize_t y1 = y_;
    ssize_t z1 = z_;

    set_y(cos * y1 - sin * z1);
    set_z(sin * y1 + cos * z1);
}

//---------------------------------------------------------


void Vector::rotate_y(const ssize_t rad_angle)
{
    ssize_t sinus  = sin(rad_angle);
    ssize_t cosine = cos(rad_angle);

    rotate_y(sinus, cosine);
}

void Vector::rotate_y(const ssize_t sin, const ssize_t cos)
{
    ssize_t x1 = x_;
    ssize_t z1 = z_;

    set_x(cos * x1 - sin * z1);
    set_z(sin * x1 + cos * z1);
}

//---------------------------------------------------------

void Vector::normalize()
{
    ssize_t length = len();
    (*this) *= (1 / length);
}

//---------------------------------------------------------