#pragma once

//=========================================================

class Subscriber_on_key
{
    protected:

        virtual void on_key(int key) = 0;
};

class Subscriber_on_timer
{
    protected:

        unsigned timeout = 0;
        virtual void on_timer() = 0;
};