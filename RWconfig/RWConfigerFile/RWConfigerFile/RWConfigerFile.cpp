// RWConfigerFile.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <limits.h>
#include "gtest/gtest.h"
#include "config_file.h"


class ConfigFileTest : public testing::Test
{
	void SetUp() override 
	{
		pconfig = new js::ConfigFile();
	}

	void TearDown() override
	{
		delete pconfig;
	}

public:
	void TestFileExist()
	{
		//绝对路径
		EXPECT_FALSE(pconfig->FileExist("F:/3D/RWconfig/IniFile/abc.ini"));
		EXPECT_STREQ("文件不存在！",pconfig->GetLastError());
		EXPECT_TRUE(pconfig->FileExist("F:/3D/RWconfig/IniFile/abc.txt"));
		EXPECT_TRUE(pconfig->FileExist("F:/3D/RWconfig/IniFile/SystemSetting.ini"));

		//相对路径,是指相对谁
		EXPECT_TRUE(pconfig->FileExist("../../IniFile/SystemSetting.ini"));
		EXPECT_TRUE(pconfig->FileExist("..\\..\\IniFile\\SystemSetting.ini"));

		//空
		EXPECT_FALSE(pconfig->FileExist(NULL));
		EXPECT_STREQ("输入为空指针！", pconfig->GetLastError());
	}

	void TestFormat()
	{
		//构造函数后，有缺省值，看get到的结果
		EXPECT_EQ(0, strcmp("#", pconfig->GetCommentSymbol()));
		EXPECT_EQ(0, strcmp("=", pconfig->GetDelimiter()));

		//我认为是在loadfile之前使用，之后使用意义？
		//set注释、分隔符后，再get
		const char *newCommonentSymblo = "//";
		pconfig->SetCommentSymbol(newCommonentSymblo);
		EXPECT_EQ(0, strcmp(pconfig->GetCommentSymbol(), newCommonentSymblo));

		const char *newDelimiter = "=:";
		pconfig->SetDelimiter(newDelimiter);
		EXPECT_EQ(0, strcmp(pconfig->GetDelimiter(), newDelimiter)); 

		EXPECT_FALSE(pconfig->SetCommentSymbol(NULL));
		EXPECT_FALSE(pconfig->SetDelimiter(NULL));
	}

	void TestLoadFile()
	{
		//测试文件内容是否load进入内存
		EXPECT_TRUE(pconfig->LoadFile("../../IniFile/SystemSetting_0.ini"));

		bool *bresult = new bool;
		EXPECT_TRUE(pconfig->GetBoolean("bUse", bresult));
		EXPECT_FALSE(*bresult);
		int *iresult = new int;
		EXPECT_TRUE(pconfig->GetInteger("nTrack", iresult, "track"));
		EXPECT_EQ(0, *iresult);
		double *fresult = new double;
		EXPECT_TRUE(pconfig->GetDouble("Port", fresult, "PLCInfo"));
		EXPECT_DOUBLE_EQ(503.67, *fresult);
		char cresult[50];
		EXPECT_TRUE(pconfig->GetString("sHost", cresult, 20, "database"));
		EXPECT_STREQ("localhost", cresult);

		////重复使用函数，测试结果保留情况
		EXPECT_TRUE(pconfig->LoadFile("../../IniFile/homeAddress.ini"));

		EXPECT_FALSE(pconfig->GetBoolean("bUse", bresult));
		EXPECT_TRUE(pconfig->GetString("add", cresult, 50, "HomeAddress"));
		EXPECT_STREQ("yunquyuan", cresult);

		//读取错误格式文件
		EXPECT_FALSE(pconfig->LoadFile("../../IniFile/SystemSetting_err.ini"));
		EXPECT_STREQ("当前组名已存在！", pconfig->GetLastError());
	}

	void TestModifyFile()
	{
		EXPECT_TRUE(pconfig->LoadFile("../../IniFile/SystemSetting_0.ini"));

		//item存在，set成功，返回true。这里先不看是否修改
		EXPECT_TRUE(pconfig->SetBoolean("bUse",true));
		EXPECT_TRUE(pconfig->SetInteger("nTrack",1,"track")); 
		EXPECT_TRUE(pconfig->SetDouble("Port",502.68,"PLCInfo"));
		EXPECT_TRUE(pconfig->SetString("sHost","bozhonlocal","database"));

		EXPECT_FALSE(pconfig->SetBoolean("srt1", true, "Lights"));  //该item不存在
		EXPECT_STREQ("key值没有匹配上", pconfig->GetLastError());
		EXPECT_FALSE(pconfig->SetInteger("uh", 3, "Glasses"));        
		EXPECT_STREQ("group值没有匹配上", pconfig->GetLastError());
		EXPECT_FALSE(pconfig->SetDouble("uio", 9.8));               
		EXPECT_STREQ("key值没有匹配上", pconfig->GetLastError());
		EXPECT_FALSE(pconfig->SetString("juib8","iu98"));           
		EXPECT_STREQ("key值没有匹配上", pconfig->GetLastError());


		//保存并打开新文件，查看是否修改成功
		pconfig->SetFileName("../../IniFile/SystemSetting_1.ini");
		EXPECT_TRUE(pconfig->SaveFile());
		EXPECT_TRUE(pconfig->LoadFile("../../IniFile/SystemSetting_1.ini"));

		bool *bresult= new bool;
		pconfig->GetBoolean("bUse", bresult);
		EXPECT_TRUE(bresult);
		int *iresult = new int;
		pconfig->GetInteger("nTrack", iresult, "track");
		EXPECT_EQ(1,*iresult);
		double *fresult = new double;
		pconfig->GetDouble("Port", fresult, "PLCInfo");
		EXPECT_DOUBLE_EQ(502.68, *fresult);
		char cresult[20];
		pconfig->GetString("sHost", cresult,20,"database");
		EXPECT_STREQ("bozhonlocal", cresult);


		//Add正常情况，group有，key无。之后get到值，且与add值相等
		EXPECT_TRUE(pconfig->AddInteger("NameNameNameName",0));
		EXPECT_TRUE(pconfig->AddString("NameNameNameName", "wenjingliu","track")); 
		EXPECT_TRUE(pconfig->AddDouble("NameNameNameDouble", 0.35, "track"));
		EXPECT_TRUE(pconfig->AddBoolean("NameNameNameName", false, "Lights"));

		pconfig->GetBoolean("NameNameNameName", bresult,"Lights");
		EXPECT_FALSE(*bresult);
		pconfig->GetInteger("NameNameNameName", iresult);
		EXPECT_EQ(0, *iresult);
		pconfig->GetDouble("NameNameNameDouble", fresult, "track");
		EXPECT_DOUBLE_EQ(0.35, *fresult);
		pconfig->GetString("NameNameNameName", cresult, 20, "track");
		EXPECT_STREQ("wenjingliu", cresult);

		//add已经存在的key
		EXPECT_FALSE(pconfig->AddInteger("offset", 67, "sensorOffset"));      //group有、key有
		EXPECT_FALSE(pconfig->AddBoolean("bDisplayImage", true, "display"));         //group有、key有
		EXPECT_TRUE(pconfig->AddString("notheKey", "addvalue", "notHavetheGroup"));  //group无，key无 
		EXPECT_TRUE(pconfig->AddDouble("notheKeydouble", 0.987, "notHavetheGroup"));  //group无，key无


		//删除,之后再get应该返回false，且lasterror
		EXPECT_TRUE(pconfig->Remove("NameNameNameName","Lights"));
		EXPECT_TRUE(pconfig->Remove("NameNameNameName"));
		EXPECT_FALSE(pconfig->Remove("Skyskyskysky","Lights"));  //删除不存在的item
		EXPECT_TRUE(pconfig->Remove("NameNameNameDouble", "track"));
		EXPECT_TRUE(pconfig->Remove("NameNameNameName", "track"));

		pconfig->SaveFile();
		pconfig->LoadFile("../../IniFile/SystemSetting_1.ini");

		EXPECT_FALSE(pconfig->GetBoolean("NameNameNameName", bresult, "Lights"));
		EXPECT_FALSE(pconfig->GetInteger("NameNameNameName", iresult));
		EXPECT_FALSE(pconfig->GetDouble("NameNameNameDouble", fresult, "track"));
		EXPECT_FALSE(pconfig->GetString("NameNameNameName", cresult, 20, "track"));

	}

	js::ConfigFile  *pconfig;

};


TEST_F(ConfigFileTest, FileExist)
{
	TestFileExist();
}

TEST_F(ConfigFileTest, Format)
{
	TestFormat();
}

TEST_F(ConfigFileTest, LoadFile)
{
	TestLoadFile();
}

TEST_F(ConfigFileTest, ModifyFile)
{
	TestModifyFile();
}

int main()
{
	testing::InitGoogleTest();
	RUN_ALL_TESTS();
	getchar();
    return 0;
}

