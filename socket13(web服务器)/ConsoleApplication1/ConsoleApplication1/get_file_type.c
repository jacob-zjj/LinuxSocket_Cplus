//通过文件名获取文件的类型
const char *get_file_type(const char* name)
{
	char* dot;
	//自右向左查找'.'字符，如果不存在则返回NULL
	dot = strrchr(name, '.');
	if (dot == NULL) 
	{
		return "text/plain; charset=utf-8";
	}
	else if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) 
	{
		return "text/html; charset=utf-8";
	}
	else if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jepg") == 0) 
	{
		return "image/jpeg";
	}
	else if (strcmp(dot, ".gif") == 0) 
	{
		return "image/gif";
	}
	else if (strcmp(dot, ".png") == 0) 
	{
		return "image/png";
	}
	else if (strcmp(dot, ".css") == 0) 
	{
		return "text/css";
	}
	else if (strcmp(dot, ".au") == 0)
	{
		return "audio/basic";
	}
	else if (strcmp(dot, ".wav") == 0)
	{
		return "audio/wav";
	}
	else if (strcmp(dot, ".avi") == 0)
	{
		return "video/x-msvideo";
	}
	else if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
	{
		return "video/quicktime";
	}
	else if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
	{
		return "video/mpeg";
	}
	else if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
	{
		return "model/vrml";
	}
	else if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
	{
		return "audio/midi";
	}
	else if (strcmp(dot, ".mp3") == 0) 
	{
		return "audio/mpeg";
	}
	else 
	{
		return "text/plain; charset=iso-8859-1";
	}
}