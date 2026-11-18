#include <mytimer.h>

MyTimer::MyTimer(){
    this->interval=0;
    this->lastmills=0;
}

MyTimer::~MyTimer(){
}

MyTimer::MyTimer(unsigned long interval){
    this->interval=interval;
    this->lastmills=millis();
}

MyTimer::MyTimer(const MyTimer& obj){
    this->interval=obj.interval;
    this->lastmills=obj.lastmills; // 还复制了拷贝对象的lastmills 
}

void MyTimer::set_interval(unsigned long interval){
    this->interval=interval;
}

unsigned long MyTimer::get_interval(){
    return this->interval;
}

bool MyTimer::is_period_inloop(){
    unsigned long nowt=millis();
    if(nowt-this->lastmills > this->interval){
        this->lastmills=nowt;
        return true;
    }
    return false;
}

MyTimer& MyTimer::operator=(unsigned long val){
    this->interval=val;
    return *this;
}

MyTimer& MyTimer::operator=(const MyTimer& obj){
    if(this == &obj)
        return *this;
    this->interval=obj.interval;    
    this->lastmills=obj.lastmills;
    return *this;
}

bool MyTimer::operator == ( bool b){
    return this->is_period_inloop() == b;
}

bool MyTimer::operator != ( bool b ){
    return this->is_period_inloop() != b;
}

MyTimer::operator bool(){
    return this->is_period_inloop();
}

//===============================


MyLoopCounter::MyLoopCounter()
    : _counter{0}
{
}

MyLoopCounter::~MyLoopCounter()
{
    // 无动态内存，空析构
}

MyLoopCounter::MyLoopCounter(const MyLoopCounter& obj)
    : _counter{obj._counter}
{
}

MyLoopCounter& MyLoopCounter::operator=(uint64_t val)
{
    _counter = val;
    return *this;
}

MyLoopCounter& MyLoopCounter::operator=(const MyLoopCounter& obj)
{
    if(this == &obj)
        return *this;

    _counter = obj._counter;
    return *this;
}

bool MyLoopCounter::operator==(uint64_t val) const
{
    return _counter == val;
}

bool MyLoopCounter::operator!=(uint64_t val) const
{
    return _counter != val;
}

MyLoopCounter::operator uint64_t() const
{
    return _counter;
}

bool MyLoopCounter::operator==(const MyLoopCounter& obj) const
{
    return _counter == obj._counter;
}

bool MyLoopCounter::operator!=(const MyLoopCounter& obj) const
{
    return _counter != obj._counter;
}

MyLoopCounter MyLoopCounter::operator+(uint64_t val) const
{
    MyLoopCounter tmp = *this;
    tmp += val;
    return tmp;
}

MyLoopCounter MyLoopCounter::operator-(uint64_t val) const
{
    MyLoopCounter tmp = *this;
    tmp -= val;
    return tmp;
}

MyLoopCounter& MyLoopCounter::operator+=(uint64_t val)
{
    if (_counter > UINT64_MAX - val)
    {
        _counter = UINT64_MAX;
    }
    else
    {
        _counter += val;
    }
    return *this;
}

MyLoopCounter& MyLoopCounter::operator-=(uint64_t val)
{
    if (_counter < val)
    {
        _counter = 0;
    }
    else
    {
        _counter -= val;
    }
    return *this;
}

// 前缀 ++
MyLoopCounter& MyLoopCounter::operator++()
{
    if(_counter < UINT64_MAX)
    {
        _counter++;
    }
    return *this;
}

// 后缀 ++
const MyLoopCounter MyLoopCounter::operator++(int)
{
    MyLoopCounter tmp = *this;
    if(_counter < UINT64_MAX)
    {
        _counter++;
    }
    return tmp;
}

// 前缀 --
MyLoopCounter& MyLoopCounter::operator--()
{
    if(_counter > 0)
    {
        _counter--;
    }
    return *this;
}

// 后缀 --
const MyLoopCounter MyLoopCounter::operator--(int)
{
    MyLoopCounter tmp = *this;
    if(_counter > 0)
    {
        _counter--;
    }
    return tmp;
}
