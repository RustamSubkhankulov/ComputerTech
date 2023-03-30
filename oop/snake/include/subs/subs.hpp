#pragma once

//=========================================================

class Subscriber_on_key
{
    public:

        virtual ~Subscriber_on_key() {}

    protected:

        virtual void on_key(int key) = 0;
};

class Subscriber_on_timer
{
    public:

        Subscriber_on_timer(unsigned timeout_):
        timeout(timeout_) {}

        virtual ~Subscriber_on_timer() {}

    protected:

        unsigned timeout = 0;
        virtual void on_timer() = 0;
};