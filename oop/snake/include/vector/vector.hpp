#pragma once 

//=========================================================

#include <math.h>

class Vector;

class Vector_posed;

//=========================================================

class Vector
{
    ssize_t x_ = 0, y_ = 0, z_ = 0;
    mutable ssize_t len_ = -1;

    public:

        Vector() {}

        Vector(const ssize_t x, const ssize_t y, const ssize_t z = 0):
            x_(x), 
            y_(y),
            z_(z),
            len_( ((!x_) && (!y_) && (!z_))? 0: -1 )
            {}

        ~Vector()             = default;
        Vector(const Vector&) = default; //copy ctor
        Vector(Vector&&)      = default; //move ctor

        Vector& operator= (const Vector&)  = default; //copy assignment
        Vector& operator= (Vector&&)       = default; //move assignment

        void set(const ssize_t x, const ssize_t y, const ssize_t z = 0)
            {
                len_ = ((abs(x_) == abs(x)) 
                     && (abs(y_) == abs(y))
                     && (abs(z_) == abs(z)))? len_: -1;
                x_ = x; 
                y_ = y;
                z_ = z;
            }

        void set_x(const ssize_t x)
            {
                len_ = (abs(x_) == abs(x))? len_: -1;
                x_ = x;
            }

        void set_y(const ssize_t y)
            {
                len_ = (abs(y_) == abs(y))? len_: -1;
                y_ = y;
            }

        void set_z(const ssize_t z)
            {
                len_ = (abs(z_) == abs(z))? len_: -1;
                z_ = z;
            }

        Vector& operator*= (const ssize_t mul)
            {
                set(x_ * mul, y_ * mul, z_ * mul);
                return *this;
            }

        Vector& operator+= (const Vector& v)
            {
                set(x_ + v.x_, y_ + v.y_, z_ + v.z_);
                return *this;
            }

        Vector& operator-= (const Vector& v)
            {
                set(x_ - v.x_, y_ - v.y_, z_ - v.z_);
                return *this;
            }

        ssize_t x() const { return x_; }
        ssize_t y() const { return y_; }
        ssize_t z() const { return z_; }

        ssize_t len() const 
            {
                if (len_ != -1)
                    return len_;

                len_ = (ssize_t) sqrt(pow(x_,2) + pow(y_,2) + pow(z_,2));
                return len_;
            }

        void rotate_z(const double rad_angle);
        void rotate_z(const double sin, double cos);

        void rotate_x(const double rad_angle);
        void rotate_x(const double sin, double cos);

        void rotate_y(const double rad_angle);
        void rotate_y(const double sin, double cos);

        void normalize();

        friend Vector orthogonal_2d_only(const Vector& v);

        friend Vector operator- (const Vector& a);
        friend Vector operator+ (const Vector& a,  const Vector& b);   
        friend Vector operator- (const Vector& a,  const Vector& b);  
        friend ssize_t operator* (const Vector& a,  const Vector& b);  
        friend Vector operator% (const Vector& a,  const Vector& b);
        friend Vector operator* (const Vector& v,  const ssize_t mul);
        friend Vector operator* (const ssize_t mul, const Vector& v);
};

//---------------------------------------------------------

class Vector_posed 
{
    Vector point_;
    Vector vector_;

    public:

        Vector_posed():
            point_(),
            vector_()
            {}
        
        Vector_posed(const Vector& r, const Vector& v):
            point_ (r),
            vector_(v)
            {}

        Vector_posed(const Vector_posed&) = default;
        Vector_posed(Vector_posed&&)      = default;

        Vector_posed& operator= (const Vector_posed&)  = default; //copy assignment
        Vector_posed& operator= (Vector_posed&&)       = default; //move assignment

        void set_point(const Vector& r)
            {
                point_ = r;
            }

        void set_vector(const Vector& v)
            {
                vector_ = v;
            }

        void rotate_z(const double rad_angle)
            {
                vector_.rotate_z(rad_angle);
            }

        void rotate_z(const double sin, const double cos)
            {
                vector_.rotate_z(sin, cos);
            }

        Vector vector() const { return vector_; }
        Vector point()  const { return point_;  }

        Vector_posed& operator*= (const ssize_t mul)
            {
                vector_ *= mul;
                return *this;
            }
        
        Vector_posed& operator+= (const Vector& v)
            {
                vector_ += v;
                return *this;
            }
        
        Vector_posed operator-()
            {
                return Vector_posed {point_, - vector_};
            }

        ssize_t len()
            {
                return vector_.len();
            }

        void normalize()
            {
                vector_.normalize();
            }

        void set_dir(const Vector& dir)
            {
                set_vector(dir - point_);
            }
};

//=========================================================