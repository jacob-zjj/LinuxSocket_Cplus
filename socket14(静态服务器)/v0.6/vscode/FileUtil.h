#pragma once
#include "nocopyable.h"
#include <string>
class AppendFile : noncopyable
{
public:
	explicit AppendFile(std::string filename);
	~AppendFile();
	void append(const char* logfile, const size_t len);
	void flush();

private:
	size_t write(const char* logfile, const size_t len);
	FILE* fp_;
	char buffer_[64 * 1024];
};