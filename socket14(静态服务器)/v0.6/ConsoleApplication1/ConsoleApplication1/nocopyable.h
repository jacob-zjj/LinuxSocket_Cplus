/*hpp，其实质就是将.cpp的实现代码混入.h头文件当中，定义与实现都包含在同一文件，则该类的调用者只需要include该hpp文件即可，
无需再 将cpp加入到project中进行编译。而实现代码将直接编译到调用者的obj文件中，不再生成单独的obj,采用hpp将大幅度减少调用
project中的cpp文件数与编译次数，也不用再发布烦人的lib与dll,因此非常适合用来编写公用的开源库。*/

//创建非拷贝构造 非赋值基类 其它类继承该类即具有该类功能  参考陈硕的linux多线程服务器端编程
#pragma once
class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:
	noncopyable(const noncopyable&);
	const noncopyable& operator=(const noncopyable&);
};