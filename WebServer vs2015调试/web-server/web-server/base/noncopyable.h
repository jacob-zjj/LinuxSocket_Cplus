#pragma once

class noncopyable
{
protected:
    noncopyable() {}
	~noncopyable() {}
	/*
	对于构造函数
	为什么声明成protected呢？
	首先肯定不能为private，不然无法构造子类实例。
	如果为public，那么外部是可以创建noncopyable这么一个实例的，可是这个实例是完全没有意义的，该类只有在被继承之后才有意义。
	所以此处声明为protected是非常恰当合适的，既保证外部无法直接构造一个无意义的noncopyable实例，又不影响构造子类实例。
	*/
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
	/*上面拷贝构造函数和赋值构造函数都声明为private，这样不论什么派生方式，子类对此都是无权访问的，从而达到禁止拷贝的目的。*/
};