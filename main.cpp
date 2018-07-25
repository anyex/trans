#include <cstdio>
#include <sstream>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <thread>
#include <set>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sem.h>
#include <dirent.h>


static int processNum = 0;

class MyString {
public:
	MyString(char* s) :str(s) {  };
	MyString() :str() {};
	MyString(std::string&A) :str(A) {};

	void  DeleteMark(const char *mark) {

		str.erase(std::remove(str.begin(), str.end(), *mark), str.end());

	}

	void  Split(std::string &s1, std::string &s2, const std::string &flag)
	{
		int pos = str.find(flag);
		if (pos != -1)
		{
			s2 = str.substr(pos + 1, str.size() - 1);
			s1 = str.substr(0, pos);
		}
	}

	void Split(std::set<std::string>& s, const std::string&c) {
		std::string::size_type pos1, pos2;
		pos2 = str.find(c);
		pos1 = 0;
		while (std::string::npos != pos2)
		{
			std::string t = str.substr(pos1, pos2 - pos1);
			if (t.find("-"))
			{
				break;
			}
			s.insert(t);
			pos1 = pos2 + c.size();
			pos2 = str.find(c, pos1);

		}
		if (pos1 != str.length())
			s.insert(str.substr(pos1));

	}

	friend std::ostream& operator<<(std::ostream&, MyString&);
	int find(const std::string &s)
	{
		return str.find(s);
	}

	std::string str;

};
std::ostream & operator<<(std::ostream&output, MyString& str)
{
	output << str.str;
}

class Shell {
public:
	Shell(std::string &cmd):cmdline(cmd) {
		doShell();
	};
	void doShell() {
		FILE *stream;
		FILE *wstream;
		char buf[1024];
		stream = popen(cmdline.c_str(), "r");
		fread(buf, 1, 1024, stream);
		ret = buf;
		pclose(stream);
	}
public:
	std::string cmdline;
	std::string ret;
	
};

class FileSet {
public:
	FileSet(std::string &s) :str(s) {
		str.Split(file, "\n");
	};
	
	MyString str;
	std::set<std::string>file;
	
};

class File {

};




class Ini {

public:
	Ini(const std::string& file) :filename(file) {
		getIni();
	};
	void getIni() {
		std::ifstream inflie(filename.data());
		if (inflie)
		{
			MyString s;
			std::string s1, s2,root;
			while (getline(inflie, s.str))
			{
				int pos = s.find("//");
				if (pos == 0)
				{
					continue;
				}
				s.DeleteMark(" ");
				s.DeleteMark("\r");
				s.DeleteMark("\t");

				if (s.find("[") != -1)
				{
					s.DeleteMark("[");
					s.DeleteMark("]");
					root = s.str;
					continue;
				}
				s.Split(s1, s2, "=");
				Inid[root][s1] = s2;
			}
		}
		else
		{
			std::cout << "can't open file!" << std::endl;
		}
		inflie.close();
	};
public:

	std::map <std::string, std::map<std::string, std::string> > Inid;
	const std::string& filename;
};

void doTranslate(std::set<std::string>& set,Ini& ini)
{
	for (auto a : set)
	{
		std::string inputFilename = ini.Inid["set"]["input"];
		std::string outputFilename = ini.Inid["set"]["output"];

		//设置lame的输入输出参数
		inputFilename.append("/");
		std::string inputPath = inputFilename;
		inputFilename.append(a);
		outputFilename.append("/");
		std::string b = a.substr(0, a.find("."));
		outputFilename.append(b);
		outputFilename.append(".mp3");

		std::string cmdlame = "lame --preset fast standard ";
		cmdlame.append(inputFilename);
		cmdlame.append(" ");
		cmdlame.append(outputFilename);


		std::cout << cmdlame << std::endl;
		/*使用lame转换文件*/
		Shell lame(cmdlame);

		/*修改转换后的名称*/
		inputPath.append("-");
		inputPath.append(a);

		std::string rename = "rename ";
		rename.append(inputFilename);
		rename.append(" ");
		rename.append(inputPath);
		rename.append(" ");
		rename.append(inputFilename);

		//std::cout << rename << std::endl;
		Shell srename(rename);

		set.erase(a);
	}
}

void readFile(std::set<std::string>&set, std::string&path)
{
	std::string Path = path;
	DIR *input = opendir(Path.c_str());
	std::string file;
	struct dirent *direntp;
	if (input)
	{
		while ((direntp = readdir(input)) != NULL)
		{
			if (direntp->d_type == 8)//文件
			{
				file.append(Path);
				file.append("/");
				file.append(direntp->d_name);
				
				set.insert(file);
				file.clear();
			}
			if (direntp->d_type == 4)//目录
			{
				std::string s = direntp->d_name;
				
				if (s.find(".")==-1)
				{
					Path.append("/");
					Path.append(direntp->d_name);
					readFile(set, Path);
					Path = path;
				}
				
			}
		}
	}
}
void sig_handle(int num)
{
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if (WIFEXITED(status))
		{
			printf("child process exit pid[%6d],exit code[%d]\n",pid,WEXITSTATUS(status));
			processNum--;
		}
		else {
			printf("child process exit but...\n");
		
		}
	}
}


void SetMp3(char* oldname,char* newName,char* out)//设置转换后的mp3的文件名称
{
	char newname[128],aname[128];
	std::string name(oldname);
	int a = name.find_last_of("/");
	int b = name.find_last_of(".");
		
	name = name.substr(a+1, b - a-1);

	std::cout << name << std::endl;
	sprintf(newname, "%s/%s%s",out,name.c_str(), ".mp3");
	strcpy(newName, newname);
}

void Rename(char *oldname,char *flag)
{
	char newName[128],name[128];
	sscanf(oldname, "%[^.]", name);
	sprintf(newName, "%s%s%s", name, ".wav",flag);
	printf("%s", newName);
	rename(oldname, newName);
}



int main()
{
	   
	Ini ini("/root/task.ini");
	int pid;
	std::set<std::string> files;
	std::string file;
	signal(SIGCHLD, sig_handle);
	std::string input = ini.Inid["set"]["input"];

	

	//while (true)
	{
		if (files.empty())
		{
			readFile(files, input);
		}
		
		for (auto a : files)
		{
			if (processNum < 4)
			{
				//if (vfork() == 0)
				{
					processNum++;

					char mp3[128];
					char wav[128] = {};
					char out[128];
					strcpy(out, ini.Inid["set"]["output"].c_str());
					strcpy(wav, a.c_str());
					SetMp3(wav, mp3, out);

				/*	char *cmd[] = { "--preset fast standard",wav,mp3,NULL };
					if (execv("/usr/bin/lame", cmd) <0)
					{
						perror("error on exec");
						exit(0);
					}
					files.erase(a);
					std::cout << a << std::endl;*/
					Rename(wav, "_");

				}
			}
			else
			{
				sleep(1);
			}
			
		}
	}

	return 0;
}