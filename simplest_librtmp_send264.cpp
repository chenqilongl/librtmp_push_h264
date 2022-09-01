#include <stdio.h>
#include "librtmp_send264.h"
#include <iostream>
#include <sstream>
#include <fstream>


FILE *fp_send1;
using namespace std;
string push_file;
string push_ip;

//读文件的回调函数
//we use this callback function to read data from buffer
int read_buffer1(unsigned char *buf, int buf_size ){
	if(!feof(fp_send1)){
		int true_size=fread(buf,1,buf_size,fp_send1);
		return true_size;
	}else{
		return -1;
	}
}
void readTxt()
{
    ifstream fin("./push_info.txt");
    string s;
    int i = 0;
    while (fin>>s) {
        if(i == 0)
        {
            cout<<"push file is : "<<s<<endl;
            push_file = s;
        }else{
            cout<<"push ip is : "<<s<<endl;
            push_ip = s;
        }
        i++;
    }
    fin.close();
    s.clear();
}

int main(int argc, char* argv[])
{
    readTxt();
    fp_send1 = fopen(push_file.data(), "rb");

	//初始化并连接到服务器
    RTMP264_Connect(push_ip.data());
	
	//发送
	RTMP264_Send(read_buffer1);

	//断开连接并释放相关资源
	RTMP264_Close();

	return 0;
}

