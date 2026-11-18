#ifndef MYTIMER_H_
#define MYTIMER_H_
#include <mybase.h>


class MyTimer{
    private:
        volatile unsigned long lastmills;
        unsigned long interval;
    public:
        MyTimer();
        //interval单位毫秒
        MyTimer(unsigned long interval); 
        MyTimer(const MyTimer& obj);
        ~MyTimer();
        void set_interval(unsigned long interval);
        unsigned long get_interval(); 
        bool is_period_inloop();  //需要在loop 每次判断一次以更新判断   
        MyTimer& operator = (unsigned long val);
        MyTimer& operator = (const MyTimer& obj);
        bool operator == ( bool b);
        bool operator != ( bool b );
        explicit operator bool();      
    };

//基于loop()函数的计数器 每一次loop被调用 MyLoopCounter内部计数器+1      
class MyLoopCounter
{
private:
    uint64_t _counter;
public:
    MyLoopCounter();
    ~MyLoopCounter();
    MyLoopCounter(const MyLoopCounter& obj);

    MyLoopCounter& operator = (uint64_t val);
    MyLoopCounter& operator = (const MyLoopCounter& obj);

    bool operator == (uint64_t val) const;
    bool operator != (uint64_t val) const;

    explicit operator uint64_t() const;

    bool operator == (const MyLoopCounter& obj) const;
    bool operator != (const MyLoopCounter& obj) const;

    // + 生成新对象，不修改this，返回值，const
    MyLoopCounter operator + (uint64_t val) const;
    // - 生成新对象，不修改this，返回值，const
    MyLoopCounter operator - (uint64_t val) const;

    MyLoopCounter& operator += (uint64_t val);
    MyLoopCounter& operator -= (uint64_t val);

    // 前缀 ++obj
    MyLoopCounter& operator ++ ();
    // 后缀 obj++，哑元int
    const MyLoopCounter operator ++ (int);

    // 前缀 --obj
    MyLoopCounter& operator --();
    // 后缀 obj--
    const MyLoopCounter operator --(int);
};



#endif //MYTIMER_H_
