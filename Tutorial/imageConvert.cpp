#include "stdafx.h"
#include "imageConvert.h"
#include "WireComposition.h"
imageConvert::imageConvert(string filename)
{
	image = cv::imread(filename, cv::IMREAD_GRAYSCALE);
	if (!image.data)
	{
		cout << "�xȡ�DƬ" << filename << "ʧ��!"<<endl;
		system("pause");
	}
	//cout << "Mat type:" << image.type()<<"size:"<<image.size();
	if (useSkeleton){
		calSkeleton();
		cv::imwrite("../Resources/skel.png", skeletonImg);
	}
	else
	{
		cv::threshold(image, skeletonImg, skeletonThreshold, 255, cv::THRESH_BINARY);
		cv::imshow("after threshold", skeletonImg);
		cv::waitKey(0);
		cv::destroyAllWindows();
	}
	createData();

}
bool undirectContain(vector<pair<pixelPoint, pixelPoint>> edgeSet, pair<pixelPoint, pixelPoint> edge){
	for each (pair <pixelPoint,pixelPoint> e in edgeSet)
	{
		//cout << "e:(" << e.first.x << "," << e.first.y << ") >> (" << e.second.x << "," << e.second.y << "),    ";
		if (e.first.x == edge.first.x&&e.first.y == edge.first.y&&e.second.x == edge.second.x&&e.second.y == edge.second.y){
			//cout << endl;
			return true;
		}
		if (e.first.x == edge.second.x&&e.first.y == edge.second.y&&e.second.x == edge.first.x&&e.second.y == edge.first.y){
			//cout << endl;
			return true;
		}
	}
	//cout << endl;
	return false;
}
int indexof(vector<pixelPoint*> list, pixelPoint* elem){
	//cout << "list:";
	for (size_t i = 0; i < list.size(); i++)
	{
		//cout << "(" << list[i]->x << "," << list[i]->y << ")";

		if ((list[i]->x == elem->x) && (list[i]->y == elem->y)){
			//cout << "return" << i;
			return i;
		}
	}
	return -1;
}
int indexof(vector<int> list, int elem){
	for (size_t i = 0; i < list.size(); i++)
	{
		//cout << "(" << list[i]->x << "," << list[i]->y << ")";

		if (list[i] == elem){
			//cout << "return" << i;
			return i;
		}
	}
	return -1;
}
vector<pixelPoint*> imageConvert::findLastConnectPoint(int posx, int posy){
	vector<pixelPoint*> result;
	int x = posx - 1;
	col lastcol = datas[x];

	//cout<< "in findLastConnectPoint x:" << posx << "y:" << posy <<endl;
	if (x < 0){
		return result;
	}
	for (int y = posy - 1; y <= posy+1; y++){//����c��һ��col�����������c
		
		if (y >= 0 && y < skeletonImg.size().height){
			
			if (skeletonImg.at<unsigned char>(y, x)>0){//�ҵ�һ���c
				int closePtIdx = -1;
				for (size_t i = 0; i < lastcol.height; i++)//���c��һ��(col)�����c�M�Ќ���
				{
					//cout  <<"lastcol address:" << &lastcol<<"height:"<<lastcol.height<<endl;
					
					pixelPoint* pt = lastcol.points[i];
					//cout << "i="<<i<<" pt address" << pt<<", ";
					if (y-pt->y <=mergeYThreshold&&y-pt->y>=0){
						if (closePtIdx<0||abs(pt->y - y) < abs(lastcol.points[closePtIdx]->y-y)){//��y�����С��pixelPoint
							closePtIdx = i;
						}
					}
				}
				if (indexof(result, lastcol.points[closePtIdx]) < 0){
					
					result.push_back(lastcol.points[closePtIdx]);
				}
			}

		}
	}
	//cout << endl;
	return result;
}
void imageConvert::createData(){
	
	width = skeletonImg.size().width;
	int height = skeletonImg.size().height;
	mergeYThreshold = 0.5*height;
	datas = new col[width];
	totalPixelPointNum=0;
	for ( int x = 0; x < width; x++)
	{
		//cout << "x=" << x << ":";
		vector<pixelPoint*> list;
		pixelPoint *firstMeetPoint = NULL;//��׃��case2�鿴��һ��case1���������Ă�pixelPoint,֮���B��
		for (int y = 0; y < height; y++){
			//case1:�������µ�һ��������ֵ����
			if ((y == 0 && skeletonImg.at<unsigned char>(y, x)>0) || (skeletonImg.at<unsigned char>(y, x)>0 && skeletonImg.at<unsigned char>(y - 1, x) == 0)){
				pixelPoint *newPt = new pixelPoint(x, y, (float)x / (float)(width - 1), (float)y / (float)(height - 1), totalPixelPointNum++);
				//cout << "case1 newPt x:" << x << " y:" << y;
				flatArray.push_back(newPt);
				int suby = y;
				vector<pixelPoint*>last;
				while (suby<height&&skeletonImg.at<unsigned char>(suby, x)>0){//����������ͬ�Ӵ��0���c,ֱ��ĳ���c���B�ӵ�����pixelPoint���·��]���κ�>0���c��
					//cout << "suby:" << suby;
					last = findLastConnectPoint(x, suby);
					if (last.size() > 0){//�ҵ����B�ӵ����c
						//newPt->lastPoints = last;
						for each(pixelPoint *pt in last){
							bool found = false;
							for each (pixelPoint *ptHave in newPt->lastPoints)
							{
								if (pt == ptHave)
								{
									found = true;
									break;
								}
							}
							if (!found){
								newPt->lastPoints.push_back(pt);
							}
						}
					}
					suby++;
					if (suby - y > mergeYThreshold){//�������ֵ��Ȼ��1���ǳ������J����ͬһ���c�ľ��x
						y = suby;	
						break;
					}
				}
				//cout << "ͣ��y=" << y << "suby=" << suby<<" ";
				y = suby;
				//cout << "�������lastPoint";
				for each(pixelPoint* pt in newPt->lastPoints){
					//cout << "(" << pt->x << "," << pt->y << ")";
					pt->nextPoints.push_back(newPt);
				}
				//cout<< endl;
				firstMeetPoint = newPt;
				list.push_back(newPt);
			}
			/*
			else if ((y == height - 1 && skeletonImg.at<unsigned char>(y, x)>0) || (skeletonImg.at<unsigned char>(y, x)>0 && skeletonImg.at<unsigned char>(y+1,x)==0))//��Փ�ρ��f,�@�N��rֻ�����F����һ����r֮��,�@�e���Ǔ�©,�������ֵ>�K���������һ�����؞�0��Ԓ�f�����_��>0�����е�β��,�t���һ������
			{
				pixelPoint *newPt = new pixelPoint(x, y, (float)x / (float)(width - 1), (float)y / (float)(height - 1), totalPixelPointNum++);
				//cout << "case2 newPt x:" << x << " y:" << y;
				flatArray.push_back(newPt);
				vector<pixelPoint*> last = findLastConnectPoint(x, y);
				if (last.size() > 0){//�ҵ����B�ӵ����c
					//newPt->lastPoints = last;
					for each(pixelPoint *pt in last){
						bool found = false;
						for each (pixelPoint *ptHave in newPt->lastPoints)
						{
							if (pt == ptHave)
							{
								found = true;
								break;
							}
						}
						if (!found){
							newPt->lastPoints.push_back(pt);
						}
					}
				}
				for each(pixelPoint* pt in newPt->lastPoints){
					
					pt->nextPoints.push_back(newPt);
				}
				if (firstMeetPoint != NULL){//��Փ�ρ��f�@�eһ��������,���case2һ������case1֮��l��
					firstMeetPoint->lastPoints.push_back(newPt);
					//newPt->nextPoint.push_back(firstMeetPoint);
				}
				list.push_back(newPt);
			}
			*/

		}
		
		col nowCol(list.data(), list.size());
		datas[x] = nowCol;
	}

	//����yֵ.������㷨���ԏ��ϵ��µ�һ���|�����ķ�0�c�齻�c,���Ԯ�ʹ�õ�elem����һ�����ظ��ֵĕr��,�׶˵Ľ��c�͕����Fy!=1����r(�����������һ�е����ز������齻�c���˵�����)
	bool pixelMeetEnd = false;
	for (int x = 0; x < width; x++)//�����ǲ����������|��
	{
		if (skeletonImg.at<unsigned char>(height - 1, width)>0){
			pixelMeetEnd = true;
			break;
		}
	}
	if (pixelMeetEnd){//�_ʼ����,��y�������ص�percent_y����1,�������ظ����Լ���percent_y�֔��@��׃����
		//cout << "����datas:";
		float maxPercent_y=0;
		for (int x = 0; x < width;x++)
		{
			pixelPoint* colEnd = datas[x].points[datas[x].height - 1];
			if (maxPercent_y < colEnd->percentage_y){
				maxPercent_y = colEnd->percentage_y;
			}
		}
		for (int x = 0; x < width; x++){
			for (int y = 0; y < datas[x].height; y++)
			{
				datas[x].points[y]->percentage_y /= maxPercent_y;
			}
		}
	}

	//�ҽ��c,�Ȳ����c��lastPoints��nextPoints���һ�����c,�����ⲿ���c��percent_y=0��1��percent_x�����0��1���c
	for (size_t x = 0; x < width; x++)
	{
		col colx = datas[x];
		for (size_t y = 0; y < colx.height; y++)
		{
			pixelPoint* pt = colx.points[y];
			//�҃Ȳ����c
			//if (pt->lastPoints.size()>1 || pt->nextPoint.size()>1)
			if (pt->nextPoints.size()+pt->lastPoints.size()>2)
			{
				pt->intersection = true;
				intersectionPoint.push_back(pt);

			}
			//�ҝ����ⲿ���c,��ʲ�N�����ⲿ���c�Ķ��x��percent_y=0��1��percent_x�����0��1���c
			//ԭ���ǝ����ⲿ���c�Ǟ�����sample�ĕr���~���г���,�@�������ڄ���pathGraph�ĕr��ͬdistance�ăɂ�ring֮�g�Ľ��c�ŕ��w�F����
		}
	}
}
void imageConvert::calSkeleton(){
	cv::Mat im;
	cv::threshold(image, im, skeletonThreshold, 255, cv::THRESH_BINARY);
	//cout << "skeletonImg type:" << skeletonImg.type() << "image type:" << image.type();
	cv::Mat morp_elem = getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

	cv::Mat skel = cv::Mat::zeros(im.size(), CV_8UC1);
	cv::Mat temp = cv::Mat::zeros(im.size(), CV_8UC1);
	//cout << "0.skel type:" << skel.type()<<"temp type:"<<temp.type() << endl;
	int i = 0;
	while (true){
		cout << "i=" << i<<" ";
		cv::morphologyEx(im, temp, cv::MORPH_OPEN, morp_elem);
		//cout << "1.skel type:" << skel.type() << "temp type:" << temp.type() << endl;
		cv::bitwise_not(temp, temp);
		//cout << "2.skel type:" << skel.type() << "temp type:" << temp.type() << endl;
		cv::bitwise_and(im, temp, temp);
		//cout << "3.skel type:" << skel.type() << "temp type:" << temp.type() << endl;
		//cv::imshow("��i����ȡ��݆��", temp);
		//cv::waitKey(0);
		//cout << "skel size:" << skel.size()<<" type:"<<skel.type()<< " temp size:" << temp.size()<<"type:"<<temp.type()<<endl;
		cv::bitwise_or(skel, temp, skel);
		
		cv::erode(im, im, morp_elem);

		double max_val = -1;
		cv::minMaxLoc(im, NULL, &max_val, NULL, NULL);
		if (max_val == 0){
			break;
		}
		i++;
	}
	skeletonImg = skel;
	cv::imwrite("skel.jpg", skeletonImg);
	//cv::imshow("��ȡ�Ĺ���:", skel);
	//cv::waitKey(0);
	//cv::destroyAllWindows();
}


vector<vector<pixelPoint>> imageConvert::sample(int Number){
	float interval = (float)width / (float)Number;
	vector<int> intervalHitX;
	for (int i = 1; i < Number; i++){//�e���������width��5,interval��1,�tNumber��5,�t��Ҫӛ䛵�HitXֻ��{1,2,3,4},̎�x=0��x=5���c��sample�����c�K�c,һ������ӛ䛵�
		intervalHitX.push_back(interval*i);
	}

	//vector<pixelPoint*> haveExtend;
	vector<pair<pixelPoint, pixelPoint>> extendRecord;//���ӛ��ѽ�����^�˵��B��,���÷������}���
	vector<pair<pixelPoint*,pixelPoint*>> header;//��ӵ����c,first�ǵ�һ��pixelPoint,second�t��header�����ԵĽ��c
	//vector<bool> headerMarker;//��춘�ӛ����������header����next߀��last,��next�r��true,��֮��false
	vector<vector<pixelPoint>> result;
	for (int i = 0; i < datas[0].height; i++){
		//if (datas[0].points[i]->intersection){
			for each (pixelPoint* next in datas[0].points[i]->nextPoints)
			{
				//cout << "add next:(" << next->x << "," << next->y << ")";
				header.push_back(pair<pixelPoint*, pixelPoint*>(next, datas[0].points[i]));
			}
	}
	while (header.size() > 0){
		
		pixelPoint* now = header[0].first;
		pixelPoint* lineStartPoint = now;

		vector<pixelPoint> line;
		line.push_back(*header[0].second);
		//cout << "now->nextPoints.size:" << now->nextPoint.size()<<" intersection:"<<now->intersection;
		bool directToNext = header[0].first->x > header[0].second->x;//�f��ǰ�M�������,���ǳ�node��next����ǰ�M
		if (directToNext)
		{
			while (now->nextPoints.size() > 0 && !now->intersection){//�����ǰ�Ƕ��c�tnextpoint��0��,�����ǰ�ǽ��c�tnextpoint���1��

				if (indexof(intervalHitX, now->x)>=0){
					line.push_back(*now);
				}
				else if (now->percentage_y == 0||now->percentage_y==1)
				{
					line.push_back(*now);
				}
				now = now->nextPoints[0];//�Ƅӵ���һ��
			}
		}
		else
		{
			while (now->lastPoints.size() > 0 && !now->intersection){//�����ǰ�Ƕ��c�tnextpoint��0��,�����ǰ�ǽ��c�tnextpoint���1��
				if (indexof(intervalHitX, now->x)>=0){
					line.push_back(*now);
				}
				else if (now->percentage_y == 0 || now->percentage_y == 1)
				{
					line.push_back(*now);
				}
				now = now->lastPoints[0];//�Ƅӵ���һ��
			}
		}
		if (now != lineStartPoint){//�п��ܵ�һ���c��ֱ���ǽ��c,�@�N��r�����Mwhile,����now��==lineStartPoint,�@�N��r�������~����Լ����Mȥ,���һ�_ʼ�ѽ����^��
			line.push_back(*now);//��齻�c�����M�����whileޒȦ�����@�eҪ�~��ѽ��c���Mȥ
		}
		
		header.erase(header.begin());
		//headerMarker.erase(headerMarker.begin());
		//cout << "�Kֹ���c(" << now->x << "," << now->y << ") intersection" << now->intersection << "nextPoint size:" << now->nextPoint.size();
		pair<pixelPoint, pixelPoint> edge(line.front(),line.back());
		if (!undirectContain(extendRecord, edge))
		{

			for each(pixelPoint* p in now->nextPoints)
			{
				header.push_back(pair<pixelPoint*, pixelPoint*>(p, now));//first�ǔU��·�������c,second�Ǵ����c���Ɂ�
				//headerMarker.push_back(true);
			}
			for each(pixelPoint* p in now->lastPoints)
			{
				header.push_back(pair<pixelPoint*, pixelPoint*>(p, now));//first�ǔU��·�������c,second�Ǵ����c���Ɂ�
				//headerMarker.push_back(false);
			}
			result.push_back(line);
			extendRecord.push_back(edge);
		}
		else{//����@���K�c�ѽ����U���^��,line���U��,���϶���һ�lline�ķ������

		}
		//cout << endl;
	}
	return result;
}
void imageConvert::debugDatas(){
	for (int y = 0; y < width; y++){
		cout << y << ":";
		for (int x = 0; x < datas[y].height; x++){
			pixelPoint* pt = datas[y].points[x];
			cout << "(" << pt->x << "," << pt->y << ");";
		}
		cout << "     ";
	}
	cout << "debug datas �Y��";
}
imageConvert::~imageConvert()
{
}

bool leftKeyDown = false;
int activePointIndex =-1;
controlGraph *mainGraph = NULL;
const int TRAGET_INDEX=2;
void mouseEventInConnect(int event, int x, int y, int flags, void *param)
{
	switch (event)
	{
		case CV_EVENT_LBUTTONDOWN:{
			leftKeyDown = true;
			break;
		}
		case CV_EVENT_LBUTTONUP:{
			leftKeyDown = false;
			break;
		}

	}
	if (leftKeyDown){
		int width = (float)mainGraph->Image->size().width / 3;
		float percent_x = (float)(x-width) / width;
		percentPoint tragetPos(percent_x, (float)y / (float)mainGraph->Image->size().height);
		//mainGraph->contorlPoints[TRAGET_INDEX]->line->deform(mainGraph->contorlPoints[TRAGET_INDEX]->point, tragetPos);
		imageConvert::drawBaseGraph(mainGraph->Image,mainGraph->mainGraphs);
		imageConvert::drawGraphPoints(mainGraph->Image,mainGraph->mainGraphs);
		imageConvert::drawControlPoints(mainGraph->Image, mainGraph->contorlPoints);
		//cv::circle(*mainGraph->Image, cv::Point2f(x, y), 4, cv::Scalar(0, 255, 0), 2);
		cv::imshow("graphShow", *mainGraph->Image);
	}
}
void imageConvert::drawBaseGraph(cv::Mat *image, vector<graphLine*> graph){
	(*image) = cv::Scalar(0, 0, 0);//����

	int width = image->size().width / 3; int height = image->size().height;
	for (size_t t = 0; t < 3; t++)
	{
		for each (graphLine *line in graph)
		{
			int begin_x = t*width;
			for (size_t i = 0; i < line->points->size() - 1; i++)
			{
				cv::Point2i p1(begin_x + width*line->points->operator[](i).percentage_x, height*line->points->operator[](i).percentage_y);
				cv::Point2i p2(begin_x + width*line->points->operator[](i + 1).percentage_x, height*line->points->operator[](i+1).percentage_y);
				cv::line(*image, p1, p2, cv::Scalar(225, 255, 255), 1);
			}
		}
	}
}
void imageConvert::drawBaseGraph(cv::Mat *image, vector<graphLine*> graph, int width, int height){
	(*image) = cv::Scalar(0, 0, 0);//����
	for (size_t t = 0; t < 3; t++)
	{
		for each (graphLine *line in graph)
		{
			int begin_x = t*width;
			for (size_t i = 0; i < line->points->size() - 1; i++)
			{
				cv::Point2i p1(begin_x + width*line->points->operator[](i).percentage_x, height*line->points->operator[](i).percentage_y);
				cv::Point2i p2(begin_x + width*line->points->operator[](i + 1).percentage_x, height*line->points->operator[](i+1).percentage_y);
				cv::line(*image, p1, p2, cv::Scalar(225, 255, 255), 1);
			}
		}
	}
}
void  imageConvert::drawControlPoints(cv::Mat *image, vector<controlPoint*> cpts){
	int width = image->size().width / 3; int height = image->size().height;
	for each (controlPoint* cpt in cpts)
	{
		if (cpt->matchPoint!=NULL)//�ѽ����䌦���c
			cv::circle(*image, cv::Point2i(width + cpt->points[0]->percentage_x*width, cpt->points[0]->percentage_y*height),3,cv::Scalar(0,255,0),2);
		else
			cv::circle(*image, cv::Point2i(width + cpt->points[0]->percentage_x*width, cpt->points[0]->percentage_y*height), 3, cv::Scalar(0, 0, 255), 2);
	}
}
void imageConvert::drawGraphPoints(cv::Mat *image, vector<graphLine*>graph){
	int width = image->size().width / 3; int height = image->size().height;
	for each (graphLine *line in graph)
	{
		int begin_x = width;

		for (size_t i = 0; i < line->points->size(); i++)
		{
			cv::Point2i pos(begin_x + width*line->points->operator[](i).percentage_x, height*line->points->operator[](i).percentage_y);
			cv::circle(*image,pos ,2,cv::Scalar(255,0,0),2);

		}
	}
}
void imageConvert::drawNextGraph(cv::Mat *image, vector<graphLine*> graph, float start_percentX, int width, int height){
	for each (graphLine *line in graph)
	{
		int begin_x = start_percentX*width;
		for (size_t i = 0; i < line->points->size() - 1; i++)
		{
			cv::Point2i p1(begin_x + width*line->points->operator[](i).percentage_x, height*line->points->operator[](i).percentage_y + height);
			cv::Point2i p2(begin_x + width*line->points->operator[](i + 1).percentage_x, height*line->points->operator[](i+1).percentage_y + height);
			cv::line(*image, p1, p2, cv::Scalar(225, 255, 255), 1);
		}
	}
}
controlPointGroup imageConvert::horizontalMatch(vector<controlPoint*> controls, float margeThreshold){
	controlPointGroup result;
	//cout << "controls:";
		
	for each (controlPoint* cpt in controls)
	{
		//cout << "(" << cpt->points[0]->percentage_x << "," << cpt->points[0]->percentage_y << ")";
		if (cpt->points[0]->percentage_x == 0.0f){
			result.group1.push_back(cpt);
		}
		else if (cpt->points[0]->percentage_x == 1.0f)
		{
			result.group2.push_back(cpt);
		}
	}

	for (int i = 0; i < result.group1.size(); i++){
		//cout << "group1 ��" << i << "�c:(" << result.group1[i]->points[0]->percentage_x << "," << result.group1[i]->points[0]->percentage_y << ")";
		for each (controlPoint* testPoint in result.group2)//
		{
			if (abs(testPoint->points[0]->percentage_y - result.group1[i]->points[0]->percentage_y) < margeThreshold){
				if (abs(testPoint->points[0]->percentage_y - 0.5f) > abs(result.group1[i]->points[0]->percentage_y - 0.5f)){//testPoint���^�ӽ��DƬ�Ľ���
					percentPoint pos(result.group1[i]->points[0]->percentage_x, testPoint->points[0]->percentage_y);
					result.group1[i]->deform(pos);
					//cout << "׃�ε�:(" << pos.x << "," << pos.y << ");";
				}
				else if (abs(testPoint->points[0]->percentage_y - 0.5f) < abs(result.group1[i]->points[0]->percentage_y - 0.5f)){//��ǰ�c���^�ӽ��DƬ����
					percentPoint pos(testPoint->points[0]->percentage_x, result.group1[i]->points[0]->percentage_y);
					testPoint->deform(pos); //line->deform(testPoint->point, pos);
				}
				result.group1[i]->matchPoint = testPoint;
				testPoint->matchPoint = result.group1[i];
			}
		}
	}
	return result;
}
bool imageConvert::horizonalIsOk(controlPointGroup match){
	//cout << "horizonalIsOk�_ʼ" << endl;
	int connectCount = 0;
	for each (controlPoint* point in match.group1)
	{
		if (point->matchPoint!=NULL)
		{
			connectCount++;
		}
	}
	if (connectCount>0)//�M���B�ӳ����ėl����������һ���B���c�����B�ӵ�
	{
		return true;
	}
	else
	{
		return false;
	}
}
controlPointGroup imageConvert::verticalMatch(vector<controlPoint*> controls, float margeThreshold,bool& success){
	controlPointGroup result;
	//cout << "controls:";
	for each (controlPoint* cpt in controls)
	{
		//cout << "(" << cpt->points[0]->percentage_x << "," << cpt->points[0]->percentage_y << ")";
		if (cpt->points[0]->percentage_y == 0.0f){
			int insertIdx=0;
			for (; insertIdx <result.group1.size(); insertIdx++)
			{
				if (result.group1[insertIdx]->points[0]->percentage_x >= cpt->points[0]->percentage_x){
					break;
				}

			}
			//cout << "group1 insert at:" << insertIdx;
			result.group1.insert(result.group1.begin()+insertIdx,cpt);
			//result.group1.push_back(cpt);
		}
		else if (cpt->points[0]->percentage_y == 1.0f)
		{
			int insertIdx = 0;
			for (; insertIdx <result.group2.size(); insertIdx++)
			{
				if (result.group2[insertIdx]->points[0]->percentage_x >= cpt->points[0]->percentage_x){
					break;
				}
			}
			//cout << "group2 insert at:" << insertIdx;
			result.group2.insert(result.group2.begin() + insertIdx, cpt);
			//result.group2.push_back(cpt);
		}
	}
	percentPoint upLeft(0, 0);
	percentPoint upRight(1, 0);
	percentPoint btmLeft(0, 1);
	percentPoint btmRight(1, 1);
	if (result.group1.size() == result.group2.size() || abs((int)result.group1.size() - (int)result.group2.size()) == 1){
		success = false;
		if (result.group1.size() == result.group2.size()){
			success = true;
		}
		else{
			if (result.group1.size() > (int)result.group2.size()){//����DƬ�Ϸ����c�������·���1���f����Փ����һ���غ��cλ�y=0 ��y=1,ֻ���@��ÿһ�ӽ��c�����ŕ�һ��
				if (result.group1.front()->points[0]->percentage_x <= margeThreshold){//����`��,�п��ܽ��c�K���Ǆ�����λ�0,���С���������,�t��ԓ�cyֵ������0
					result.group1.front()->deform(upLeft);
				}
				if (1 - result.group1.back()->points[0]->percentage_x <= margeThreshold){
					result.group1.back()->deform(upRight);
				}
				if (result.group1.front()->points[0]->percentage_x == 0.0f&&result.group1.back()->points[0]->percentage_x == 1.0f){
				//if (result.group1.front()->points[0]->percentage_x == result.group1.back()->points[0]->percentage_y){//�Д��ǲ�����ȫ�����ɹ���,����Ѓɂ��c���������һ���]��������ԒҲ����������ȫ�ɹ���
					success = true;
				}
			}
			else//����DƬ�·����c�������Ϸ���1
			{
				if (result.group2.front()->points[0]->percentage_x <= margeThreshold){
					result.group2.front()->deform(btmLeft);
				}
				if (1 - result.group2.back()->points[0]->percentage_x <= margeThreshold){
					result.group2.back()->deform(btmRight);
				}
				if (result.group2.front()->points[0]->percentage_x == 0.0f&&result.group2.back()->points[0]->percentage_x == 1.0f){
					success = true;
				}
			}
		}
		if (success){
			bool margeEndPoint = result.group2.front()->points[0]->percentage_x == 0.0f&&result.group2.back()->points[0]->percentage_x == 1.0f;//result.group2.front()->points[0]->percentage_x == result.group2.back()->points[0]->percentage_y;
			vector<controlPoint*> splicingControlPoints;//ƴ�Ӄɂ�
			splicingControlPoints.insert(splicingControlPoints.end(), result.group2.begin(), result.group2.end());
			if (margeEndPoint)//���^һ��y=0���c�팍�F�ρ���ͬ�c��Ч��
				splicingControlPoints.insert(splicingControlPoints.end(), result.group2.begin()+1, result.group2.end());
			else
				splicingControlPoints.insert(splicingControlPoints.end(), result.group2.begin(), result.group2.end());
			int pointCanChoose = splicingControlPoints.size() - result.group1.size() + 1;
			if (result.group2.size()==1)//���������cֻ��һ����Ԓ�����Ă�����
			{
				result.group2[0]->matchPoint = result.group1[0];
				for each (controlPoint* cpt in result.group1)
				{
					cpt->matchPoint = result.group2[0];
				}
			}

			for (size_t i = 0; i < pointCanChoose; i++)
			{
				float error = 0;
				for (size_t j = 0; j < result.group1.size()-1; j++)
				{
					
					float splicing2 = splicingControlPoints[i + j + 1]->points[0]->percentage_x;
					if (i + j + 1 >= result.group2.size()){
						splicing2 += 1;
					}
					float splicing1 = splicingControlPoints[i + j]->points[0]->percentage_x;
					if (i + j >= result.group2.size()){
						splicing1 += 1;
					}

					error += abs((splicing2 - splicing1) - (result.group1[j + 1]->points[0]->percentage_x - result.group1[j]->points[0]->percentage_x));

				}
				if (error == 0){
					for (size_t j = 0; j < result.group1.size(); j++){
						controlPoint* matchPoint = NULL;
						if (margeEndPoint){
							matchPoint = result.group2[i + j%(result.group2.size()-1)];//�@�����DƬ�·��Ŀ����c������һ�cһ���]��matchpoint,���^���@����r������һ�c������ǰһ�c
						}
						else
						{
							matchPoint = result.group2[i + j%result.group2.size()];
						}
						result.group1[j]->matchPoint = matchPoint;
						matchPoint->matchPoint = result.group1[j];
					}
					break;
				}
				else if (error<mergeVerticalThreshold)//ͨ�^�Ą�group1��percent_x�폊��match
				{
					for (size_t j = 0; j < result.group1.size() - 1; j++){
						float splicing2 = splicingControlPoints[i + j + 1]->points[0]->percentage_x;
						if (i + j + 1 >= result.group2.size()){
							splicing2 += 1;
						}
						float splicing1 = splicingControlPoints[i + j]->points[0]->percentage_x;
						if (i + j >= result.group2.size()){
							splicing1 += 1;
						}
						float correctInterval = splicing2 - splicing1;
						result.group1[j + 1]->deform(percentPoint(result.group1[j]->points[0]->percentage_x + correctInterval,0));
					}
					for (size_t j = 0; j < result.group1.size(); j++){
						controlPoint* matchPoint = NULL;
						
						if (margeEndPoint){
							matchPoint = result.group2[(i + j) % (result.group2.size() - 1)];//�@�����DƬ�·��Ŀ����c������һ�cһ���]��matchpoint,���^���@����r������һ�c������ǰһ�c
						}
						else
						{
							//cout << "i+j%result.group2.size()=" << i + j%result.group2.size();
							matchPoint = result.group2[(i + j)%result.group2.size()];
						}
						//cout << "�cj=" << j << "matchpoint address:" << matchPoint << endl;
						result.group1[j]->matchPoint = matchPoint;
						matchPoint->matchPoint = result.group1[j];
					}
					break;

				}
			}

		}
		else
		{
			return result;
		}
	}
	return result;
}
bool imageConvert::inExtraStartDirect(int id1, int id2){
	for each (pair<int,int> direct in extraStartDirect)
	{
		if (direct.first==id1&&direct.second ==id2)
		{
			return true;
		}
	}
	return false;
}
void imageConvert::showGraphicConnect(vector<vector<pixelPoint>> &graph){
	int ori_w = skeletonImg.size().width;
	int ori_h = skeletonImg.size().height;
	cout << "ori_w:" << ori_w << ",ori_h:" << ori_h;
	float ratio = (float)ori_w / (float)ori_h;
	int height = 300;//200;
	int width = ratio*height;
	cv::Mat  newone(height, width*3, CV_8UC3, cv::Scalar(0,0,0));
	int count = 0;

	//�и�ݔ���graph,��lineGraphs��keyPoints������и��Ԓ�����nodeGraph�o���ҵ������B�ӵ�node
	int origrn_graph_size = graph.size();
	for (size_t i = 0; i <origrn_graph_size; i++)
	{
		vector<pixelPoint*> interKpts;
		for (size_t j = 0; j < graph[i].size(); j++)
		{
			pixelPoint *kpt = &graph[i][j];
			if (kpt->percentage_y==0)
			{
				//cout << "kpt:(" << kpt->percentage_x << "," << kpt->percentage_y << "):last size:" << kpt->lastPoints.size() << " next size:" << kpt->nextPoint.size() << ";";
			}
			//λ춈DƬ�׶˻���픶˵��ֲ���λ���������ҵ����g���c��Ҫ�~���и����
			if ((kpt->percentage_y == 0 || kpt->percentage_y == 1) && (kpt->percentage_x != 0 && kpt->percentage_x != 1) && (kpt->lastPoints.size()>0 && kpt->nextPoints.size()>0))
			{
				interKpts.push_back(kpt);
			}
		}
		if (interKpts.size() > 0){//��Ҫ�и�
			vector<vector<pixelPoint>> segmPart;
			int beginSegmIdx = 0;
			for each (pixelPoint* ikpt in interKpts)
			{
				int endSegmIdx = -1;
				for (int j = 0; j < graph[i].size(); j++){
					if (graph[i][j].percentage_x == ikpt->percentage_x&&graph[i][j].percentage_y == ikpt->percentage_y){
						endSegmIdx = j;
						break;
					}
				}
				if (endSegmIdx < 0){
					cout << "�e�`���и�!endSegmIdx:" << endSegmIdx;
					system("pause");
				}
				vector<pixelPoint> newPart;
				for (size_t j = beginSegmIdx; j <= endSegmIdx; j++)
				{
					newPart.push_back(graph[i][j]);
				}
				segmPart.push_back(newPart);
				beginSegmIdx = endSegmIdx;
			}
			vector<pixelPoint> newPart;
			for (size_t j = beginSegmIdx; j < graph[i].size(); j++)
			{
				newPart.push_back(graph[i][j]);
			}
			segmPart.push_back(newPart);
			//�и���ɵ���Ҫ�Ż�ȥ���]�����N����,�S��ŕ���׃graph�ĽY��,�e������ graph��3��line ��̖0,1,2,����и���0,����3,4,���ֱ���Ƴ�0,��3,4push back
			//graph�ĽY����׃��:1,2,3,4,������]���},��������l�FlineGraphs[1] interKpts>0��Ҫ�и�,�˕r����ȥ��graph[1],���Ǵ˕r����ֵ1ָ�����ԭ���2!
			graph[i] = segmPart[0];
			for (int j = 1; j < segmPart.size(); j++)
			{
				graph.push_back(segmPart[j]);
			}
			//ͬ��������case 0,1,2�и��a�� 3,4,���@���������Y������3,1,2,4,����Ӱ���һ���и�
		}
	}

	vector<graphLine*> lineGraphs(graph.size());
	//cout << "lineGraphs������" << graph.size() << "�lline";
	for (size_t i = 0; i < graph.size(); i++)
	{
		lineGraphs[i] = new graphLine(&graph[i],ori_w,ori_h);
		//cout << "lineGraph address:" << lineGraphs[i] << endl;
		//cout <<"i="<<i <<"points:";
		for each (pixelPoint point in graph[i])
		{
			//cout << "(" << point.x << "," << point.y << ")";
		}
	}
	//cout << endl;
	for (size_t i = 0; i < lineGraphs.size(); i++)
	{
		for (size_t j = 0; j < lineGraphs.size(); j++)
		{
			if (i != j){
				lineGraphs[i]->tryConnect(lineGraphs[j]);
			}
		}
		lineGraphs[i]->finshConnect();
	}
	vector<pixelPoint*> keyPoints;
	vector<controlPoint*> controlPoints;

	for each (graphLine *lineGraph in lineGraphs)
	{
		//keyPoints.insert(keyPoints.end(), lineGraphs->keyPoints.begin(), lineGraphs->keyPoints.end());
		for each (pixelPoint* kpt in lineGraph->keyPoints)
		{
			bool haveControlPoint = false;
			for each (controlPoint* cpt in controlPoints)
			{
				float cpt_x = cpt->points[0]->percentage_x;
				float cpt_y = cpt->points[0]->percentage_y;
				if (cpt_x == kpt->percentage_x&&cpt_y == kpt->percentage_y){
					cpt->points.push_back(kpt);
					cpt->lines.push_back(lineGraph);
					haveControlPoint = true;
				}
			}
			if (!haveControlPoint){//����]���ҵ�������������ͬ�Ŀ����c
				controlPoint *cpt = new controlPoint();
				cpt->points.push_back(kpt);
				cpt->lines.push_back(lineGraph);
				controlPoints.push_back(cpt);
			}
		}

	}
	/*cout << "ӡ��control Points:";
	for each (controlPoint* cpt in controlPoints)
	{
		cout << "(" << cpt->points[0]->percentage_x << "," << cpt->points[0]->percentage_y << ") point num:" << cpt->points.size() <<"match:"<<cpt->matchPoint<< endl;
	}*/
	
	controlPointGroup matchResult_horizontal = horizontalMatch(controlPoints, 0.02f);

	bool horizontal_ok = horizonalIsOk(matchResult_horizontal);
	mainGraph = new controlGraph();
	mainGraph->mainGraphs = lineGraphs;
	mainGraph->Image = &newone;
	mainGraph->contorlPoints = controlPoints;
	imageConvert::drawBaseGraph(mainGraph->Image, lineGraphs);
	imageConvert::drawGraphPoints(mainGraph->Image, lineGraphs);
	imageConvert::drawControlPoints(mainGraph->Image, controlPoints);

	
	//cout << "horizontal_ok:" << horizontal_ok;
	if (!horizontal_ok){
		cv::putText(newone,"error! none horizontal connect",cv::Point2i(10,height/2),cv::FONT_HERSHEY_SIMPLEX,0.7,cv::Scalar(75,75,255));
	}
	cv::imshow("graphShow", newone);
	//cv::setMouseCallback("graphShow",mouseEventInConnect);
	cv::waitKey(0);
	bool vertical_ok = false;
	controlPointGroup matchResult_vertical = verticalMatch(controlPoints, 0.02f, vertical_ok);
	cv::Mat image2(height*2, width * 3, CV_8UC3, cv::Scalar(0, 0, 0));
	imageConvert::drawBaseGraph(&image2, lineGraphs,width,height);
	float start_x = matchResult_vertical.group1[0]->matchPoint->points[0]->percentage_x - matchResult_vertical.group1[0]->points[0]->percentage_x + 1;//������@�e��Access violation reading location 0x0000000000000000.һ���ǿv���B���Ҳ���ƥ����c matchResult_vertical.group1[0]->matchPoint ==NULL����
	imageConvert::drawNextGraph(&image2, lineGraphs,start_x,width,height);
	float firstPointOffset = matchResult_vertical.group1[0]->points[0]->percentage_x;
	//cout << "���c��(" << start_x << "," << height << ")";
	if (!vertical_ok){
		cv::putText(image2, "error! none  vertical connect", cv::Point2i(10, height / 2), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(75, 75, 255));
		cv::circle(image2, cv::Point2i((start_x+firstPointOffset)*width, height), 3, cv::Scalar(0, 0, 255), 2);
	}
	else{
		cv::circle(image2, cv::Point2i((start_x+firstPointOffset)*width, height), 3, cv::Scalar(0, 255, 0), 2);
	}
	cv::imshow("graphShow", image2);
	//cv::setMouseCallback("graphShow",mouseEventInConnect);
	cv::waitKey(0);
	if (horizontal_ok&&vertical_ok){
		this->lineGraph = lineGraphs;
		this->Shift_percent = matchResult_vertical.group1[0]->matchPoint->points[0]->percentage_x - matchResult_vertical.group1[0]->points[0]->percentage_x;//�����Shift_percent,�@��ֵ�����x��ÿ��level��ʼ�cҪ���shift����grid�L�ȵİٷֱ�,
		if (this->Shift_percent < 0){
			this->Shift_percent += 1.0f;
		}
		//cout << "shift_percent:" << this->Shift_percent;
		//�@��ֵ������һ�ƿ����c�ĵ�һ��(result2.group1[0])��ƥ����c��percentage_x�Q����
	}
	cv::destroyAllWindows();
	MyMTSP mtsp;
	mtsp.Init(*mainGraph);
	if (mtsp.Solve()){
		//cout << "mtsp solve success! great!";
		vector<sequence> connectSequence = mtsp.getConnectSequence();
		cv::Mat  demoImg(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

		for each (sequence seq1 in connectSequence)
		{
			int startId;
			int nextId;
			if (seq1[0].direct)
			{
				startId = graph[seq1[0].index].front().id;
				nextId = graph[seq1[0].index].back().id;
			}
			else
			{
				startId = graph[seq1[0].index].back().id;
				nextId = graph[seq1[0].index].front().id;
			}
			int endId;
			if (seq1.back().direct){
				endId= graph[seq1.back().index].back().id;
			}
			else{
				endId= graph[seq1.back().index].front().id;
			}


			connectDirect[startId][-1] = nextId;
			if (flatArray[startId]->percentage_y != 0||flatArray[endId]->percentage_y!=1){
				extraStartDirect.push_back(pair<int, int>(startId,nextId));
			}
			for (size_t i = 0; i <seq1.size()-1; i++)
			{
				int nowId;
				int lastId;
				int nextId;
				if (seq1[i].direct){
					nowId = graph[seq1[i].index].back().id;
					lastId = graph[seq1[i].index].front().id;
				}
				else
				{
					nowId = graph[seq1[i].index].front().id;
					lastId = graph[seq1[i].index].back().id;
				}
				if (seq1[i+1].direct){
					if (nowId != graph[seq1[i+1].index].front().id)
					{
						cout << "�B�m�����н��c����ͬһ���c!";
						system("pause");
					}
					nextId = graph[seq1[i+1].index].back().id;
				}
				else
				{
					if (nowId != graph[seq1[i+1].index].back().id)
					{
						cout << "�B�m�����н��c����ͬһ���c!";
						system("pause");
					}
					nextId = graph[seq1[i+1].index].front().id;
				}
				connectDirect[nowId][lastId] = nextId;
			}
		}
		cv::destroyAllWindows();

	}
	else
	{
		cout << "solve fail......";
	}
	
}
