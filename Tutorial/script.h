#pragma once
//�����_��e��ԭ��
class script
{
public:
	script();
	~script();
	virtual void onInit()=0;
	virtual void onRender() = 0;
	virtual void onKeyDown(unsigned char key, int mx, int my){
		printf("onKeyDown:%c \n", key);
	}
		 
};

