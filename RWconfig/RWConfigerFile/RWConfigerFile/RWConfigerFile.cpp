// RWConfigerFile.cpp : �������̨Ӧ�ó������ڵ㡣
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
		//����·��
		EXPECT_FALSE(pconfig->FileExist("F:/3D/RWconfig/IniFile/abc.ini"));
		EXPECT_STREQ("�ļ������ڣ�",pconfig->GetLastError());
		EXPECT_TRUE(pconfig->FileExist("F:/3D/RWconfig/IniFile/abc.txt"));
		EXPECT_TRUE(pconfig->FileExist("F:/3D/RWconfig/IniFile/SystemSetting.ini"));

		//���·��,��ָ���˭
		EXPECT_TRUE(pconfig->FileExist("../../IniFile/SystemSetting.ini"));
		EXPECT_TRUE(pconfig->FileExist("..\\..\\IniFile\\SystemSetting.ini"));

		//��
		EXPECT_FALSE(pconfig->FileExist(NULL));
		EXPECT_STREQ("����Ϊ��ָ�룡", pconfig->GetLastError());
	}

	void TestFormat()
	{
		//���캯������ȱʡֵ����get���Ľ��
		EXPECT_EQ(0, strcmp("#", pconfig->GetCommentSymbol()));
		EXPECT_EQ(0, strcmp("=", pconfig->GetDelimiter()));

		//����Ϊ����loadfile֮ǰʹ�ã�֮��ʹ�����壿
		//setע�͡��ָ�������get
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
		//�����ļ������Ƿ�load�����ڴ�
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

		////�ظ�ʹ�ú��������Խ���������
		EXPECT_TRUE(pconfig->LoadFile("../../IniFile/homeAddress.ini"));

		EXPECT_FALSE(pconfig->GetBoolean("bUse", bresult));
		EXPECT_TRUE(pconfig->GetString("add", cresult, 50, "HomeAddress"));
		EXPECT_STREQ("yunquyuan", cresult);

		//��ȡ�����ʽ�ļ�
		EXPECT_FALSE(pconfig->LoadFile("../../IniFile/SystemSetting_err.ini"));
		EXPECT_STREQ("��ǰ�����Ѵ��ڣ�", pconfig->GetLastError());
	}

	void TestModifyFile()
	{
		EXPECT_TRUE(pconfig->LoadFile("../../IniFile/SystemSetting_0.ini"));

		//item���ڣ�set�ɹ�������true�������Ȳ����Ƿ��޸�
		EXPECT_TRUE(pconfig->SetBoolean("bUse",true));
		EXPECT_TRUE(pconfig->SetInteger("nTrack",1,"track")); 
		EXPECT_TRUE(pconfig->SetDouble("Port",502.68,"PLCInfo"));
		EXPECT_TRUE(pconfig->SetString("sHost","bozhonlocal","database"));

		EXPECT_FALSE(pconfig->SetBoolean("srt1", true, "Lights"));  //��item������
		EXPECT_STREQ("keyֵû��ƥ����", pconfig->GetLastError());
		EXPECT_FALSE(pconfig->SetInteger("uh", 3, "Glasses"));        
		EXPECT_STREQ("groupֵû��ƥ����", pconfig->GetLastError());
		EXPECT_FALSE(pconfig->SetDouble("uio", 9.8));               
		EXPECT_STREQ("keyֵû��ƥ����", pconfig->GetLastError());
		EXPECT_FALSE(pconfig->SetString("juib8","iu98"));           
		EXPECT_STREQ("keyֵû��ƥ����", pconfig->GetLastError());


		//���沢�����ļ����鿴�Ƿ��޸ĳɹ�
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


		//Add���������group�У�key�ޡ�֮��get��ֵ������addֵ���
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

		//add�Ѿ����ڵ�key
		EXPECT_FALSE(pconfig->AddInteger("offset", 67, "sensorOffset"));      //group�С�key��
		EXPECT_FALSE(pconfig->AddBoolean("bDisplayImage", true, "display"));         //group�С�key��
		EXPECT_TRUE(pconfig->AddString("notheKey", "addvalue", "notHavetheGroup"));  //group�ޣ�key�� 
		EXPECT_TRUE(pconfig->AddDouble("notheKeydouble", 0.987, "notHavetheGroup"));  //group�ޣ�key��


		//ɾ��,֮����getӦ�÷���false����lasterror
		EXPECT_TRUE(pconfig->Remove("NameNameNameName","Lights"));
		EXPECT_TRUE(pconfig->Remove("NameNameNameName"));
		EXPECT_FALSE(pconfig->Remove("Skyskyskysky","Lights"));  //ɾ�������ڵ�item
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

