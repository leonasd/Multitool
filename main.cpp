#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <atlstr.h>
#include <windows.h>
#include "stdafx.h"
#include "tinystr.h"
#include "tinyxml.h"

#define random(x) (rand() % x)

using namespace std;

struct stMultiWriteOnceParam
{
	string FilePath;
	double SpeedResult;
	unsigned int FileSize;
	unsigned int BitRate;
};

double TestDiskSpeed()
{
	ofstream FileOut;
	clock_t begin, end;
	double calcbuff = 0;
	char *writedata = new char[10485760];
	memset(writedata, 55, sizeof(char) * 10485760);

	for (int i = 0; i < 100; i++)
	{
		begin = clock();
		FileOut.open("test.mp4");
		FileOut << writedata;
		FileOut.close();
		end = clock();
		calcbuff += ((double)(end - begin) / CLOCKS_PER_SEC);
	}
	calcbuff = calcbuff;
	delete[] writedata;
	system("del test.mp4");
	cout << "Max Speed Limiter: " << (1000 / calcbuff) << endl;
	return calcbuff;
}

static DWORD WINAPI MultiWriteOnce(LPVOID lpParameter)
{
	string path;
	ofstream FileOut;
	clock_t begin, end;
	double sleep_time = 0;
	size_t buffsize = 1048576;
	stMultiWriteOnceParam  pm;

	char* MBuff = new char[buffsize];
	int data = (int)(clock() % 255);

	memset(MBuff, data, sizeof(char)* buffsize);
	pm.FilePath = ((stMultiWriteOnceParam*)lpParameter)->FilePath;
	pm.FileSize = ((stMultiWriteOnceParam*)lpParameter)->FileSize;
	pm.BitRate = ((stMultiWriteOnceParam*)lpParameter)->BitRate;
	pm.SpeedResult = ((stMultiWriteOnceParam*)lpParameter)->SpeedResult;
	path = pm.FilePath;
	path = path + ".mp4";

	sleep_time = (1000 - pm.BitRate * pm.SpeedResult) / (pm.BitRate) + 2;
	
	if (sleep_time > 0)
	{

		try
		{
			begin = clock();
			FileOut.open(path.c_str());
			for (unsigned int i = 0; i < pm.FileSize; i++)
			{
				FileOut << MBuff;
				_sleep(sleep_time);
			}
			FileOut.close();
			end = clock();
			cout << "ThreadID: " << GetCurrentThreadId() << " Write Time: " << (double)(end - begin) / CLOCKS_PER_SEC << endl;
		}
		catch (stMultiWriteOnceParam& e)
		{
			FileOut.close();
		}
		delete[] MBuff;
	}
	else
	{
		cout << "Speed out of range!" << endl;
	}
	return 0;
}

CString GetAppPath()
{
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL, modulePath, MAX_PATH);
	CString strModulePath(modulePath);
	strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
	return strModulePath;
}

bool MyReadXmlFile(string& szFilename, string& path, int& multinum, int& filesizemin, int& filesizemax, int& bitratemin, int& bitratemax)
{
	try
	{
		CString appPath = GetAppPath();
		string seperator = "\\";
		string fullpath = appPath.GetBuffer(0) + seperator + szFilename;

		TiXmlDocument *myDocument = new TiXmlDocument(fullpath.c_str());
		myDocument->LoadFile();

		TiXmlElement *RootElement = myDocument->RootElement();
		TiXmlElement *Running_param = RootElement->FirstChildElement();
		TiXmlElement *Function_Choose = Running_param->FirstChildElement();
		string func = Function_Choose->GetText();
		TiXmlElement *funcbuff = Function_Choose->FirstChildElement();
		while ((string)funcbuff->Value() != func)
		{
			funcbuff = funcbuff->NextSiblingElement();
		}

		string funcbuffchose = funcbuff->GetText();
		funcbuff = funcbuff->FirstChildElement();

		while ((string)funcbuff->Value() != funcbuffchose)
		{
			funcbuff = funcbuff->NextSiblingElement();
		}
		funcbuff = funcbuff->FirstChildElement();
		string spath = funcbuff->GetText();
		funcbuff = funcbuff->NextSiblingElement();
		string smultinum = funcbuff->GetText();
		funcbuff = funcbuff->NextSiblingElement();
		string sfilesizemin = funcbuff->GetText();
		funcbuff = funcbuff->NextSiblingElement();
		string sfilesizemax = funcbuff->GetText();
		funcbuff = funcbuff->NextSiblingElement();
		string sbitratemin = funcbuff->GetText();
		funcbuff = funcbuff->NextSiblingElement();
		string sbitratemax = funcbuff->GetText();

		path = spath + "\\thread";
		multinum = atoi(smultinum.c_str());
		filesizemin = atoi(sfilesizemin.c_str());
		filesizemax = atoi(sfilesizemax.c_str());
		bitratemin = atoi(sbitratemin.c_str());
		bitratemax = atoi(sbitratemax.c_str());

		delete myDocument;
	}
	catch (string& e)
	{
		return false;
	}
	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
		
	string path;
	string fileName = "info.xml";
	double SpeedResult = TestDiskSpeed();
	int threadnum, filesizemin, filesizemax, bitratemin, bitratemax;
	//DWORD timeads;
	//DWORD increast;
	//BOOL dis;
	//GetSystemTimeAdjustment(&timeads, &increast, &dis);
	//cout << timeads << endl;
	//cout << increast << endl;
	//cout << dis << endl;
	MyReadXmlFile(fileName, path, threadnum, filesizemin, filesizemax, bitratemin, bitratemax);
	HANDLE *hThread = new HANDLE[threadnum];
	stMultiWriteOnceParam *myparam = new stMultiWriteOnceParam[threadnum];
	memset(myparam, 0, sizeof(stMultiWriteOnceParam)* threadnum);

	for (int i = 0; i < threadnum; i++)
	{
		char *index = new char[11];
		myparam[i].FilePath = path + itoa(i, index, 10);
		myparam[i].FileSize = filesizemin + random((filesizemax - filesizemin + 1));
		myparam[i].BitRate = bitratemin + random((bitratemax - bitratemin + 1));
		myparam[i].SpeedResult = SpeedResult;
		hThread[i] = CreateThread(NULL, 0, MultiWriteOnce, &myparam[i], 0, NULL);
		delete[] index;
	}
	WaitForMultipleObjects(threadnum, hThread, TRUE, INFINITE);
	delete[] hThread;
	delete[] myparam;
	system("pause");
	return 0;
}

http://blog.csdn.net/ithzhang/article/details/8373243

