#include "stdafx.h"
#include "intersectionOptimizer.h"


intersectionOptimizer::intersectionOptimizer()
{
}
intersectionOptimizer::intersectionOptimizer(contour last, contour next, int num){
	girdNum = num;
	int debugIndex = 0;
	const float interval = 1.0f / (float)num;
	Vec3f_angle* nextSamplePoints = next.sample(sampleRate);
	contourList[0] = last;
	contourList[1] = next;

	for (int i = 0; i < num; i++){
		//if (i ==debugIndex)
		//Ӌ��deform
		cout << "��" << i << "��grid: ";

		float now_x = loopClampTo01(i*interval - interval);
		//cout << "now_x:" << now_x;
		float end_x = loopClampTo01(i*interval);
		//cout << "end_x:" << end_x;
		float next_end_x = loopClampTo01(i*interval + interval);

		gridGroups grid(i);
		OpenMesh::Vec3f p11 = last.getPosAtAngle(end_x);            //
		OpenMesh::Vec3f p12 = last.getPosAtAngle(next_end_x);
		int closetPointIndex = 0;//�ҵ�����һ�l�ȸ߾����xp11�W����þ��x������c
		/*for (int j = 1; j < sampleRate; j++){
		if (abs(nextSamplePoints[j].angle - now_x)<abs(nextSamplePoints[closetPointIndex].angle - now_x)){
		closetPointIndex = j;
		}
		}*/
		for (int i = 1; i < sampleRate; i++){
			if ((nextSamplePoints[i].pos - p11).length() < (nextSamplePoints[closetPointIndex].pos - p11).length()){
				closetPointIndex = i;
			}
		}

		if (nextSamplePoints[closetPointIndex].angle < now_x||nextSamplePoints[closetPointIndex].angle>end_x){//��ӽ����c��now_x��ǰ��
			//�ԏ�clostPointIndex����ʸ��� now_x ,end_x,next_end_x�@����ֵ
			now_x = loopClampTo01(nextSamplePoints[closetPointIndex].angle-interval);
			end_x = loopClampTo01(now_x + interval);
			next_end_x = loopClampTo01(end_x + interval);
		}
		int startIndex = now_x*sampleRate;
		if (now_x > end_x){//��ǰangleֵ�ȽY���r�Ĵ��f���^���н��^��еĽYβ nowx->���β��->endx ����now_x=0.8 ->1 ->end_x=0.3
			cout << "now_x >end_x:";
			for (int i = startIndex; i<sampleRate; i++)
			{
				floatGroup line(nextSamplePoints[i].angle);//�����Ե�i���c�����Ӌ��׃����
				if (nextSamplePoints[i + 1].angle>next_end_x){//ͬ���ʼ�c��angle ���next_end_x�t�f�����^�˽K�c
					for (int j = i + 1; j < sampleRate; j++){//���ߵ��K�c
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						//cout << "p11:" << p11 << " p12:" << p12 << "p21:" << p21 << "p22:" << p22;
						float weight = calDeform(p11, p12, p21, p22);//+coeff_edge*calLengrhDeform(interval,nextSamplePoints[i].angle,nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "weight:" << weight << endl;
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
					for (int j = 0; nextSamplePoints[j].angle <= next_end_x; j++){//�ُ����c�ߵ�next_end_x
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				else{
					for (int j = i+1; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				grid.groups.push_back(line);
			}
			for (int i = 0; nextSamplePoints[i].angle <= end_x; i++){
				floatGroup line(nextSamplePoints[i].angle);
				if (nextSamplePoints[i + 1].angle>next_end_x){
					for (int j = i + 1; j < sampleRate; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
					for (int j = 0; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				else{
					for (int j = i + 1; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				grid.groups.push_back(line);
			}
		}
		else
		{
			cout << "now_x<=end_x";
			for (int i = startIndex; nextSamplePoints[i].angle <= end_x; i++){
				//cout << "i=" << i << "angle=" << nextSamplePoints[i].angle << ",";
				floatGroup line(nextSamplePoints[i].angle);
				if (nextSamplePoints[i + 1].angle>next_end_x){
					for (int j = i + 1; j < sampleRate; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
					for (int j = 0; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				else{
					for (int j = i + 1; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22); //+ coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				grid.groups.push_back(line);
			}
		}/*
		cout << "gird size:" << grid.groups.size() << ";"<<endl;
		for each (floatGroup line in grid.groups)
		{
			cout << "line" << line.key <<" size:"<<line.values.size()<<":";
			for each (floatPair pair in line.values)
			{
				cout << "(" << pair.key << "," << pair.value << "),";
			}
			cout << endl;
		}*/
		deformRecords.push_back(grid);
		//��ʼ�����c��angle
		angleMappings.push_back(floatPair(i*interval,i*interval));
		//ӛ�grid�Ď׺���Ϣ
		OpenMesh::Vec3f p21 = next.getPosAtAngle(end_x);
		OpenMesh::Vec3f p22 = next.getPosAtAngle(next_end_x);
		float weight = calDeform(p11, p12, p21, p22);//+coeff_edge*calLengrhDeform(interval,end_x,next_end_x);
		float min_w=1000000;
		float p21_min_angle;
		float p22_min_angle;

		for each (floatGroup line in grid.groups)
		{

			for each(floatPair pair in line.values){
				if (pair.value < min_w){
					min_w = pair.value;
					p21_min_angle = line.key;
					p22_min_angle = pair.key;
				}
			}
		}
		if (i == 6){
			cout << endl;
		}
		//cout << "Ӌ�� grid" << i << ":" << "p11:" << p11 << " p12:" << p12 << " p21:" << p21 << " p22:" << p22 << "weight:" << weight << ";" << endl;
		rectGrid rect(p11, p12, p21, p22, weight, min_w);
		rect.minDeformPoint[0] = p11;
		rect.minDeformPoint[1] = p12;
		rect.minDeformPoint[2] = next.getPosAtAngle(p21_min_angle);
		rect.minDeformPoint[3] = next.getPosAtAngle(p22_min_angle);
		rect.minAnglePair[0] = p21_min_angle;
		rect.minAnglePair[1] = p22_min_angle;
		gridList.push_back(rect);
	}
	//delete nextSamplePoints;
	cout << "��ʼ��֮��:" << endl;
	for (int i = 0; i < gridList.size(); i++){
		cout << "grid" << i << ":��e:" << gridList[i].area << "׃����:" << gridList[i].weight << "; ";
		if (i % 2 == 0){
			cout << endl;
		}
	}
	//delete nextSamplePoints;
}
intersectionOptimizer::intersectionOptimizer(contour last, contour next, vector<float> originAngles, bool negAngel){
	cout << "originAngles size:" << originAngles.size() << endl;
	girdNum = originAngles.size();
	int debugIndex = 0;
	const float interval = 1.0f / (float)girdNum;
	Vec3f_angle* nextSamplePoints = next.sample(sampleRate);
	contourList[0] = last;
	contourList[1] = next;
	if (last.id == 13){
		cout << "id13!" << endl;
	}
	cout << "last.id:" << last.id;
	cout << "originAngles:";

	for each (float angle in originAngles)
	{
		cout << angle << ", ";
	}
	for (int g = 0; g < girdNum; g++){
		float now_x = originAngles[g] - interval;
		float end_x = originAngles[g];
		float next_end_x = originAngles[g] + interval;
		cout << "g=" << g << " now_x:" << now_x << " end_x:" << end_x << " next_end_x:" << next_end_x << endl;
		gridGroups grid(g);
		int nowRange_i = floorf(now_x);//��Ȧ���^�g,���箔nowRange��-1�rnow_x̎�[-1,0)
		OpenMesh::Vec3f p11 = last.getPosAtAngle(end_x);
		OpenMesh::Vec3f p12 = last.getPosAtAngle(next_end_x);
		int closetPointIndex = 0;//�ҵ�����һ�l�ȸ߾����xp11�W����þ��x������c
		for (int i = 1; i < sampleRate; i++){
			if ((nextSamplePoints[i].pos - p11).length() < (nextSamplePoints[closetPointIndex].pos - p11).length()){
				closetPointIndex = i;
			}
		}
		if (loopTransToLayer(nextSamplePoints[closetPointIndex].angle, nowRange_i)< now_x || loopTransToLayer(nextSamplePoints[closetPointIndex].angle,nowRange_i)>end_x){//��ӽ����c��now_x��ǰ��
			//�ԏ�clostPointIndex����ʸ��� now_x ,end_x,next_end_x�@����ֵ
			//cout << "���¶�λ";
			/*
			float candiate_dist[3];
			candiate_dist[0] = fabsf(loopTransToLayer(nextSamplePoints[closetPointIndex].angle, nowRange_i - 1) - now_x);
			candiate_dist[1] = fabsf(loopTransToLayer(nextSamplePoints[closetPointIndex].angle, nowRange_i) - now_x);
			candiate_dist[2] = fabsf(loopTransToLayer(nextSamplePoints[closetPointIndex].angle, nowRange_i + 1) - now_x);
			int minIdx = 0;
			for (int i = 1; i < 3; i++){
				if (candiate_dist[i] < candiate_dist[minIdx]){
					minIdx = i;
				}
			}*/
			float closetAngle= loopFindNearestCycle(end_x, nextSamplePoints[closetPointIndex].angle);
			if (abs(closetAngle - end_x) < minJumpPercentage*interval){//������c�ڿ������̹�����
				now_x = closetAngle - interval;
				end_x = now_x + interval;
				next_end_x = end_x + interval;
				nowRange_i = floorf(now_x);
				//cout << " now_x:" << now_x << " end_x:" << end_x << " next_end_x:" << next_end_x << endl;
			}
		}
		int startIndex = loopClampTo01(now_x)*sampleRate;

		for (int i = startIndex; loopTransToLayer(nextSamplePoints[i].angle, nowRange_i) <= end_x; i++){
			floatGroup line(loopTransToLayer(nextSamplePoints[i].angle, nowRange_i));
			int nowRange_j = nowRange_i;
			int j = i + 1;
			if (j == sampleRate){
				j = 0;
				nowRange_j++;
			}
			for (; loopTransToLayer(nextSamplePoints[j].angle,nowRange_j) <= next_end_x; j++){
				OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
				OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
				float weight = calDeform(p11, p12, p21, p22);
				floatPair pair(loopTransToLayer(nextSamplePoints[j].angle,nowRange_j), weight);
				line.values.push_back(pair);
				if (j == sampleRate - 1){
					j = 0;
					nowRange_j++;
				}
			}
			if (i == sampleRate - 1){
				i = 0;
				nowRange_i++;
			}
			grid.groups.push_back(line);
		}
		deformRecords.push_back(grid);
		//��ʼ�����c��angle
		angleMappings.push_back(floatPair(end_x, g*interval));
		//ӛ�grid�Ď׺���Ϣ
		OpenMesh::Vec3f p21 = next.getPosAtAngle(end_x);
		OpenMesh::Vec3f p22 = next.getPosAtAngle(next_end_x);
		float weight = calDeform(p11, p12, p21, p22);
		float min_w = 1000000;
		float p21_min_angle;
		float p22_min_angle;
		int count = 0;
		for each (floatGroup line in grid.groups)
		{
			for each(floatPair pair in line.values){

				if (pair.value < min_w){
					min_w = pair.value;
					p21_min_angle = line.key;
					p22_min_angle = pair.key;
				}
			}
		}

		//cout << "Ӌ�� grid" << i << ":" << "p11:" << p11 << " p12:" << p12 << " p21:" << p21 << " p22:" << p22 << "weight:" << weight << ";" << endl;
		rectGrid rect(p11, p12, p21, p22, weight, min_w);
		rect.minDeformPoint[0] = p11;
		rect.minDeformPoint[1] = p12;
		rect.minDeformPoint[2] = next.getPosAtAngle(p21_min_angle);
		rect.minDeformPoint[3] = next.getPosAtAngle(p22_min_angle);
		rect.minAnglePair[0] = p21_min_angle;
		rect.minAnglePair[1] = p22_min_angle;
		gridList.push_back(rect);
	}

}
intersectionOptimizer::intersectionOptimizer(contour last, contour next,vector<float> originAngles){
	cout << "originAngles size:" << originAngles.size()<<endl;
	girdNum = originAngles.size();
	int debugIndex = 0;
	const float interval = 1.0f / (float)girdNum;
	Vec3f_angle* nextSamplePoints = next.sample(sampleRate);
	contourList[0] = last;
	contourList[1] = next;
	cout << "originAngles:";
	for each (float angle in originAngles)
	{
		cout << angle << ", ";
	}
	bool flag_angle1_has_cross_zero = false;//��Փ��angle1�����^0���S1��
	float last_x=-1;//ӛ���һ��now_x,�Á��Д��Л]�п��^0���S
	for (int g = 0; g < girdNum; g++){
		//if (i ==debugIndex)
		//Ӌ��deform
		cout << "��" << g << "��grid: ";
		float now_x = loopClampTo01(originAngles[g] - interval);
		if (last_x > now_x){
			flag_angle1_has_cross_zero=true;
		}
		last_x=now_x;
		cout << "now_x:" << now_x<<"originAngles[i]:"<<originAngles[g];
		float end_x = loopClampTo01(originAngles[g]);
		cout << "end_x:" << end_x;
		float next_end_x = loopClampTo01(originAngles[g] + interval);
		cout << "next_end_x:" << next_end_x;
		gridGroups grid(g);
		
		OpenMesh::Vec3f p11 = last.getPosAtAngle(end_x);            //
		OpenMesh::Vec3f p12 = last.getPosAtAngle(next_end_x);
		int closetPointIndex = 0;//�ҵ�����һ�l�ȸ߾����xp11�W����þ��x������c
		/*for (int j = 1; j < sampleRate; j++){
		if (abs(nextSamplePoints[j].angle - now_x)<abs(nextSamplePoints[closetPointIndex].angle - now_x)){
		closetPointIndex = j;
		}
		}*/
		for (int i = 1; i < sampleRate; i++){
			if ((nextSamplePoints[i].pos - p11).length() < (nextSamplePoints[closetPointIndex].pos - p11).length()){
				closetPointIndex = i;
			}
		}
		/*
		if (nextSamplePoints[closetPointIndex].angle < now_x || nextSamplePoints[closetPointIndex].angle>end_x){//��ӽ����c��now_x��ǰ��
			//�ԏ�clostPointIndex����ʸ��� now_x ,end_x,next_end_x�@����ֵ
			now_x = loopClampTo01(nextSamplePoints[closetPointIndex].angle - interval);
			end_x = loopClampTo01(now_x + interval);
			next_end_x = loopClampTo01(end_x + interval);
		}*/
		int startIndex = now_x*sampleRate;

		if (now_x > end_x){//��ǰangleֵ�ȽY���r�Ĵ��f���^���н��^��еĽYβ nowx->���β��->endx ����now_x=0.8 ->1 ->end_x=0.3
			//cout << "now_x >end_x:";
			flag_angle1_has_cross_zero = true;//��һ��now_x>end_xҕ����^0���S�О�
			for (int i = startIndex; i<sampleRate; i++)
			{
				floatGroup line(nextSamplePoints[i].angle);//�����Ե�i���c�����Ӌ��׃����
				line.flag = true;
				if (nextSamplePoints[i + 1].angle>next_end_x){//ͬ���ʼ�c��angle ���next_end_x�t�f�����^�˽K�c
					for (int j = i + 1; j < sampleRate; j++){//���ߵ��K�c
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						//cout << "p11:" << p11 << " p12:" << p12 << "p21:" << p21 << "p22:" << p22;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "weight:" << weight << endl;
						floatPair pair(nextSamplePoints[j].angle, weight);
						pair.flag = true;
						line.values.push_back(pair);
					}
					for (int j = 0; nextSamplePoints[j].angle <= next_end_x; j++){//�ُ����c�ߵ�next_end_x
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				else{
					for (int j = i + 1; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				grid.groups.push_back(line);
			}
			for (int i = 0; nextSamplePoints[i].angle <= end_x; i++){
				floatGroup line(nextSamplePoints[i].angle);
				if (nextSamplePoints[i + 1].angle>next_end_x){
					for (int j = i + 1; j < sampleRate; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						pair.flag = true;
						line.values.push_back(pair);
					}
					for (int j = 0; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				else{
					for (int j = i + 1; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				grid.groups.push_back(line);
			}
		}
		else
		{
			//cout << "now_x<=end_x"<<" now_x:"<<now_x<<"end_x:"<<end_x;

			for (int i = startIndex; nextSamplePoints[i].angle <= end_x; i++){
				//cout << "i=" << i << "angle=" << nextSamplePoints[i].angle <<" ";
				//cout << "flag:" << flag_angle1_has_cross_zero<<", ";
				floatGroup line(nextSamplePoints[i].angle);
				if (!flag_angle1_has_cross_zero){//���߀�]�п��^0���S,��ʹ���λҲ����ӛ��0��֮ǰ
					line.flag = true;
				}
				if (nextSamplePoints[i + 1].angle>next_end_x){
					for (int j = i + 1; j < sampleRate; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						pair.flag = true;
						line.values.push_back(pair);
					}
					for (int j = 0; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						line.values.push_back(pair);
					}
				}
				else{

					for (int j = i + 1; nextSamplePoints[j].angle <= next_end_x; j++){
						OpenMesh::Vec3f p21 = nextSamplePoints[i].pos;
						OpenMesh::Vec3f p22 = nextSamplePoints[j].pos;
						if (last.id == 9 && g == 7){
							//cout << "angle1:" << nextSamplePoints[i].angle << " angle2:" << nextSamplePoints[j].angle << " rect deform:" << calDeform(p11, p12, p21, p22) << "length deform:" << coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle)<<endl;
						}
						float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						//cout << "calLengrhDeform(" << interval << "," << nextSamplePoints[i].angle << "," << nextSamplePoints[j].angle << "):" << calLengrhDeform(interval, nextSamplePoints[i].angle, nextSamplePoints[j].angle);
						floatPair pair(nextSamplePoints[j].angle, weight);
						pair.flag = line.flag;//�λ�@�^��angle1�Ġ�B,����λ�����ܿ��^0���S
						line.values.push_back(pair);
					}
				}
				grid.groups.push_back(line);
			}
		}/*
		 cout << "gird size:" << grid.groups.size() << ";"<<endl;
		 for each (floatGroup line in grid.groups)
		 {
		 cout << "line" << line.key <<" size:"<<line.values.size()<<":";
		 for each (floatPair pair in line.values)
		 {
		 cout << "(" << pair.key << "," << pair.value << "),";
		 }
		 cout << endl;
		 }*/
		deformRecords.push_back(grid);
		//��ʼ�����c��angle
		angleMappings.push_back(floatPair(end_x, g*interval));
		//ӛ�grid�Ď׺���Ϣ
		OpenMesh::Vec3f p21 = next.getPosAtAngle(end_x);
		OpenMesh::Vec3f p22 = next.getPosAtAngle(next_end_x);
		float weight = calDeform(p11, p12, p21, p22) + coeff_edge*calLengrhDeform(interval, end_x, next_end_x);
		float min_w = 1000000;
		float p21_min_angle;
		bool p21_cross_zero;
		float p22_min_angle;
		bool p22_cross_zero;
		int count = 0;
		for each (floatGroup line in grid.groups)
		{
			for each(floatPair pair in line.values){

				if (pair.value < min_w){
					min_w = pair.value;
					p21_min_angle = line.key;
					p21_cross_zero = line.flag;
					p22_min_angle = pair.key;
					p22_cross_zero = pair.flag;
				}
			}
		}
		//cout << "Ӌ�� grid" << i << ":" << "p11:" << p11 << " p12:" << p12 << " p21:" << p21 << " p22:" << p22 << "weight:" << weight << ";" << endl;
		rectGrid rect(p11, p12, p21, p22, weight, min_w);
		rect.minDeformPoint[0] = p11;
		rect.minDeformPoint[1] = p12;
		rect.minDeformPoint[2] = next.getPosAtAngle(p21_min_angle);
		rect.minDeformPoint[3] = next.getPosAtAngle(p22_min_angle);
		rect.minAnglePair[0] = p21_min_angle;
		rect.minAnglePair[1] = p22_min_angle;
		rect.minAngleCrossZero[0] = p21_cross_zero;
		rect.minAngleCrossZero[1] = p22_cross_zero;
		gridList.push_back(rect);
	}
	//delete nextSamplePoints;
	cout << "��ʼ��֮��:" << endl;
	for (int i = 0; i < gridList.size(); i++){
		cout << "grid" << i << ":��e:" << gridList[i].area << "׃����:" << gridList[i].weight << "; ";
		if (i % 2 == 0){
			cout << endl;
		}
	}
	//delete nextSamplePoints;
}

intersectionOptimizer::~intersectionOptimizer()
{
}

float intersectionOptimizer::calDeform(OpenMesh::Vec3f p11, OpenMesh::Vec3f p12, OpenMesh::Vec3f p21, OpenMesh::Vec3f p22){
	OpenMesh::Vec3f e1_ = p12 - p11;//contour1��߅
	OpenMesh::Vec3f e2_ = p21 - p22;//contour2��߅
	OpenMesh::Vec3f e_1 = p21 - p11;//��һ�Mintersection point������
	OpenMesh::Vec3f e_2 = p22 - p12;//�ڶ��Mintersection point������
	
	//return dot(e1_.normalize(), e2_.normalize())+dot(e_1.normalize(),e_2.normalize());
	return abs(dot(e1_.normalize(), e_1.normalize()))+abs(dot(e_1.normalize(),e2_.normalize())) +abs(dot(e2_.normalize(), e_2.normalize()))+abs(dot(e1_.normalize(),e_2.normalize()));
}
float intersectionOptimizer::calLengrhDeform(float avgLength, float angle1, float angle2){
	if (angle1 > angle2){//���^��0���S
		angle1 -= 1;//��angle1׃��ؓ��
	}
	return powf((angle2 - angle1) - avgLength, 2);
}
void intersectionOptimizer::solve(int iter_num){
	/*floatPair *largestWeightAnglePair=new floatPair[girdNum];
	for (int i = 0; i = deformRecords.size(); i++){
		gridGroups now_grid = deformRecords[i];
		floatPair maxAnglePair;
		float maxWeight = -1;
		for each (floatGroup group in now_grid.groups)
		{
			for each (floatPair calResult in group.values)
			{
				if (calResult.value > maxWeight){
					maxAnglePair = calResult;
				}
			}
		}
		largestWeightAnglePair[i] = maxAnglePair;
	}*/
	const float ignoreThreshold = 1.2f;
	for (int t = 0; t < iter_num; t++){
		cout <<endl<< "��" << t << "�ε���";
		int maxIndex = 0;
		for (int i = 1; i < gridList.size(); i++){
			if (gridList[i].weight > gridList[i].minWeight*ignoreThreshold){//�ӽ���С׃����һ���̶Ⱥ�Ͳ��ٸ�׃
				if (gridList[i].weight*gridList[i].area > gridList[maxIndex].weight*gridList[maxIndex].area){//�ҵ�����Ҫ���µ�grid
					maxIndex = i;
				}
			}
		}
		cout << "̎��" << maxIndex << ":";
		rectGrid last;
		int lastIndex;
		rectGrid next;
		int nextIndex;
		if (maxIndex <= 0){
			last = gridList.back();
			lastIndex = gridList.size() - 1;
		}
		else
		{
			last = gridList[maxIndex - 1];
			lastIndex = maxIndex - 1;
		}
		if (maxIndex >= gridList.size() - 1){
			next = gridList[0];
			nextIndex = 0;
		}
		else
		{
			next = gridList[maxIndex + 1];
			nextIndex = maxIndex + 1;
		}
		//׃�γ̶�ԽС����eԽ��ĸ��ӕ��ֵ�Խ���׃����
		//��eԽ���w�F���ǂ����ӵ���e���Ԯ�ǰ���ӵ���e,��������e
		//������e����׃����,�@��׃�����^С��������e�^��ĸ��ӕ��õ�һ���^��ę���
		float weight_last = (last.area / gridList[maxIndex].area) / last.weight;
		float weight_next = (next.area / gridList[maxIndex].area)/next.weight;
		float weight_current = 1/gridList[maxIndex].weight;

		//�ҵ����нM����׃�������ٵ�߅�M��
		gridGroups nowGroups= deformRecords[maxIndex];
		float angle1 = -1;
		float angle2 = -1;
		float minDeform = 1000000;
		for each (floatGroup group in nowGroups.groups)
		{
			for each(floatPair pair in group.values){
				if (pair.value<minDeform)
				{
					angle1 = group.key;
					angle2 = pair.key;
					minDeform = pair.value;
				}
			}
		}
		cout << "grid" << maxIndex << "��С׃������:" << minDeform;
		cout << "��ʼ�c1:" << gridList[maxIndex].points[0] << "��ʼ�c2:" << gridList[maxIndex].points[1]<<endl;
		cout << "��С׃���� angle1:" << angle1 << " λ��:" << contourList[1].getPosAtAngle(angle1)<< endl;
		cout << "��С׃���� angle2:" << angle2 << " λ��" << contourList[1].getPosAtAngle(angle2) << endl;
		//�������ƄӽǶ�
		float old_angle1 = angleMappings[maxIndex].value;
		float new_angle1 = (angle1 -old_angle1)*(weight_last/(weight_last+weight_current))+old_angle1;
		float old_angle2 = angleMappings[(maxIndex + 1) % angleMappings.size()].value;
		float new_angle2 = (angle2 - old_angle2)*(weight_next / (weight_next + weight_current)) + old_angle2;
		cout << "����left  angle:" << old_angle1 << "=>" << new_angle1<<endl;
		cout << "����right angle:" << old_angle2 << "=>" << new_angle2 << endl;
		angleMappings[maxIndex].value = new_angle1;//����angleMapping
		angleMappings[(maxIndex + 1) % angleMappings.size()].value = new_angle2;
		OpenMesh::Vec3f pos1 = contourList[1].getPosAtAngle(new_angle1);
		OpenMesh::Vec3f pos2 = contourList[1].getPosAtAngle(new_angle2);
		//���®�ǰ����
		OpenMesh::Vec3f p11 = gridList[maxIndex].points[0];
		OpenMesh::Vec3f p12 = gridList[maxIndex].points[1];
		OpenMesh::Vec3f p21 = pos1;
		OpenMesh::Vec3f p22 = pos2;
		float new_w = calDeform(p11, p12, p21, p22);
		float old_w = gridList[maxIndex].weight;
		gridList[maxIndex].updatePartsPoints(NULL,NULL,&p21,&p22,new_w);
		cout << "׃��������:" << old_w << " =>" << new_w<< endl;
		//����ǰ������
		p11 = last.points[0];
		p12 = last.points[1];
		p21 = last.points[2];
		p22 = pos1;
		new_w = calDeform(p11, p12, p21, p22);
		old_w = gridList[lastIndex].weight;
		gridList[lastIndex].updatePartsPoints(NULL, NULL, NULL, &p22, new_w);
		cout << "��׃��������:" << old_w << "=>" << new_w << endl;
		//���º󂀸���
		p11 = next.points[0];
		p12 = next.points[1];
		p21 = pos2;
		p22 = next.points[3];
		new_w = calDeform(p11, p12, p21, p22);
		old_w = gridList[nextIndex].weight;
		gridList[nextIndex].updatePartsPoints(NULL, NULL, &p21, NULL, new_w);
		cout << "��׃��������:" << old_w << "=>" << new_w << endl;

	}
}
void bubbleSort(vector<floatPair> &marray){
	for (int end_i = marray.size() - 1; end_i >= 0; end_i--)
	{
		//cout << "end_i:" << end_i;
		for (size_t i = 0; i < end_i; i++)
		{
			//cout << "i:" << i;
			if (marray[i].value > marray[i + 1].value){
				float temp = marray[i].value;
				marray[i].value = marray[i + 1].value;
				marray[i + 1].value = temp;
			}
			//cout << "���Q��marray[" << i << "]:" << marray[i].value << " marray[" << i + 1 << "]:" << marray[i + 1].value;
		}
		//cout << "����end_i:"<<end_i<<"��ĽY��:";
		/*for each (floatPair pair in marray)
		{
			cout << "(" << pair.key << "," << pair.value << ")";
		}*/
		cout << endl;
		cout << endl;
	}
}
void intersectionOptimizer::solveOnce(){
	//����energy function�ցѽ��c
	//cout << "solve once for index:" << contourList[0].id - 1 << "angleMapping size:"<<angleMappings.size()<<endl;
	for (int i = 0; i < gridList.size(); i++){
		int lastIdx = i - 1;
		if (lastIdx < 0){
			lastIdx += gridList.size();
		}

		//׃����Խ����eԽС�ĈDԴ���@�ø���ę���
		//cout << "grid i=" << i  << "minAngle1:"<<gridList[i].minAnglePair[0]<<"minAngle2:"<<gridList[i].minAnglePair[1]<<";"<<endl;
		float weight_last = gridList[lastIdx].minWeight / gridList[lastIdx].area;
		float weight_current = gridList[i].minWeight / gridList[i].area;
		float t = weight_last / (weight_last + weight_current);
		float angle = 0;
		//if (i==0)
		//{
		//if (gridList[lastIdx].minAngleCrossZero[1] != gridList[i].minAngleCrossZero[0]){
		/*if (gridList[lastIdx].minAnglePair[1]>gridList[i].minAnglePair[0]&&gridList[lastIdx].minAnglePair[1]-gridList[i].minAnglePair[0]>0.5){//�f����һ���c����һ���c֮�g���^��0���S
				float lastAngle = gridList[lastIdx].minAnglePair[1] - 1;
				float mixAngle = (lastAngle - gridList[i].minAnglePair[0])*t + gridList[i].minAnglePair[0];
				if (mixAngle < 0){
					angle = mixAngle + 1.0f;
				}
				else{
					angle = mixAngle;
				}
			}
		else{
				angle = (gridList[lastIdx].minAnglePair[1] - gridList[i].minAnglePair[0])*t + gridList[i].minAnglePair[0];
			}*/
		//cout << "grid"<<i<<"gridList[lastIdx].minAnglePair[1]:" << gridList[lastIdx].minAnglePair[1] << " gridList[i].minAnglePair[0]:" << gridList[i].minAnglePair[0] << " t:" << t << endl;
		float lastValue = gridList[lastIdx].minAnglePair[1];
		if (i == 0 && lastValue>gridList[i].minAnglePair[0]){//����
			lastValue = loopFindNearestCycle(gridList[i].minAnglePair[0],lastValue);
		}
		//cout << "lastValue:" << lastValue<<endl;
		angle = (lastValue - gridList[i].minAnglePair[0])*t + gridList[i].minAnglePair[0];
		//cout << "�����angle��:" << angle << endl;
		//}
		/*else
		{
			if ((contourList[0].id == 20)&&(i==1)){
				cout << "��19contour ��1grid cross zero1:" << gridList[lastIdx].minAngleCrossZero[1] << "grid cross zero2:" << gridList[i].minAngleCrossZero[0] << endl;
				cout << "��19contour ��1grid gridList[lastIdx].minAnglePair[1]:" << gridList[lastIdx].minAnglePair[1] << " gridList[i].minAnglePair[0]:" << gridList[i].minAnglePair[0] << " t:" << t<<endl;
			}
			angle = (gridList[lastIdx].minAnglePair[1] - gridList[i].minAnglePair[0])*t + gridList[i].minAnglePair[0];
		}*/

		angleMappings[i].value = angle;
	}
	//������С���������{���Y��

	/*for (size_t i = 0; i < angleMappings.size()-1; i++)
	{
		if (angleMappings[i].value > angleMappings[i + 1].value){
			float temp = angleMappings[i].value;
			angleMappings[i].value = angleMappings[i + 1].value;
			angleMappings[i + 1].value = temp;
		}
	}*/
	bubbleSort(angleMappings);
	if (contourList[0].id == 10){
		cout << "�����angleMappings:";
		for (int i = 0; i < angleMappings.size(); i++){
			cout << "i=" << i << ":" << angleMappings[i].value;
		}
		cout << endl;
	}
	
	if (((int)floor(angleMappings.front().value)) == ((int)floor(angleMappings.back().value)) && angleMappings.back().value>angleMappings.front().value){

	}
	else if (loopFindNearestCycle(angleMappings.front().value,angleMappings.back().value)>angleMappings.front().value){
		float front_aft_trans = loopTransToLayer(angleMappings.front().value, floor(angleMappings.back().value));
		float back_aft_trans = loopTransToLayer(angleMappings.back().value, floor(angleMappings.front().value));
		angleMappings[0].value = back_aft_trans;
		angleMappings[angleMappings.size() - 1].value = front_aft_trans;
	}
	//cout << "���Q��angleMappings:";
	//for (int i = 0; i < angleMappings.size(); i++){
	//	cout << "i=" << i << ":" << angleMappings[i].value;
	//}
	//
	const float SMALL_NUM = 0.000001f;
	int counter = 0;
	while (true)
	{
		//cout << "�ь�contour id" << contourList[0].id << "�� id" << contourList[1].id << "֮�g";
		float interval = 1.0f / (float)angleMappings.size();
		int minGridIndex = -1;
		for (int i = 0; i < angleMappings.size(); i++){
			float startAnglePoint = angleMappings[i].value;
			float endAnglePoint = angleMappings[(i + 1) % angleMappings.size()].value;
			//cout << "i:" << i << " startAnglePoint:" << startAnglePoint << " endAnglePoint:" << endAnglePoint << "len" << endAnglePoint - startAnglePoint << endl;
			if (i == angleMappings.size() - 1){
				//int layer = floorf(endAnglePoint);
				//startAnglePoint = loopTransToLayer(startAnglePoint,layer);
				endAnglePoint = loopFindNearestCycle(startAnglePoint, endAnglePoint);
				//cout << "startAnglePoint:" << startAnglePoint << " endAnglePoint:" << endAnglePoint << "endAnglePoint-startAnglePoint:" << endAnglePoint - startAnglePoint<<endl;
			}
			//if (contourList[0].id == 48)
				//cout << "i=" << i << "e-s=" << endAnglePoint - startAnglePoint << ",";
			if (endAnglePoint - startAnglePoint +SMALL_NUM < interval*minIntervalPercentage ){
				//cout << "i=" << i << "���ϔU���l��e-s:" << endAnglePoint - startAnglePoint << "threshold:" << interval*minIntervalPercentage<<endl;
				if (minGridIndex < 0)
				{
					minGridIndex = i;
				}
				else
				{
					float angleMin = angleMappings[(minGridIndex + 1) % angleMappings.size()].value - angleMappings[minGridIndex].value;
					if ((endAnglePoint - startAnglePoint) < angleMin)
					{
						minGridIndex = i;
					}
				}
			}
		}

		if (minGridIndex<0)
		{
			break;//������и��Ӷ��M������С�g������,�tֹͣޒȦ
		}
		else//�U����С����
		{
			if (contourList[0].id == 10){
				cout << "�_ʼ�U��" << minGridIndex << ":" << endl;
				float startAnglePoint = angleMappings[minGridIndex].value;
				float endAnglePoint = angleMappings[(minGridIndex + 1) % angleMappings.size()].value;
				cout << "length:" << endAnglePoint - startAnglePoint;
			}
			
			float expansionAngle_forward = 0;
			float expansionAngle_backward = 0;
			//int expansionCounter_forward = 0;
			//int expansionCounter_backward = 0;
			bool oddExpansion = angleMappings.size() % 2 == 0;//�攵�U������˼�Ǯ��Ƕȏă�߅�ߵĕr����ཻ������һ��,��ż���U���ཻ������һ�l߅
			float bestAngle1 = gridList[minGridIndex].minAnglePair[0];
			float bestAngle2 = gridList[minGridIndex].minAnglePair[1];
			float nowAngle1 = angleMappings[minGridIndex].value;
			bestAngle1 = loopFindNearestCycle(nowAngle1, bestAngle1);
			float nowAngle2 = angleMappings[(minGridIndex + 1) % angleMappings.size()].value;
			nowAngle2 = loopFindNearestCycle(nowAngle1,nowAngle2);
			bestAngle2 = loopFindNearestCycle(nowAngle2, bestAngle2);
			/*if (nowAngle2 - nowAngle1 < -0.5f){//�Д�nowAngle1->nowAngle2֮�g�Л]�п�0���S
				nowAngle1 = 1 - nowAngle1;
				if (abs(bestAngle1 - nowAngle1) < 0.5)//�ڿ�0���Sǰ����,�Д�bestAngle1�Ƿ���nowAngle1ͬһ��
					bestAngle1 = 1 - bestAngle1;
				if (abs(bestAngle2 - nowAngle1) < 0.5)//�Д�bestAngle2...
					bestAngle2 = 1 - bestAngle2;
			}*/
			cout << "interval*minIntervalPercentage:" << interval*minIntervalPercentage;
			if (bestAngle1 < nowAngle1 && nowAngle2 < bestAngle2){
				float offset1 = nowAngle1 - bestAngle1;
				float offset2 = bestAngle2 - nowAngle2;
				cout << "offset1:" << offset1 << "offset2:" << offset2 << "(offset2 / (offset1 + offset2)):" << (offset2 / (offset1 + offset2)) << "(offset1 / (offset1 + offset2)):" << offset1 << " abs:"<<abs(nowAngle2 - nowAngle1);
				expansionAngle_forward = (interval*minIntervalPercentage - abs(nowAngle2 - nowAngle1))*(offset2 / (offset1 + offset2));
				expansionAngle_backward = -(interval*minIntervalPercentage - abs(nowAngle2 - nowAngle1))*(offset1 / (offset1 + offset2));

			}
			else if (bestAngle1>nowAngle1 &&nowAngle2>bestAngle2)//�@�N��r�o��̎��
			{
				expansionAngle_forward = (interval*minIntervalPercentage - abs(nowAngle2 - nowAngle1)) / 2;
				expansionAngle_backward = -(interval*minIntervalPercentage - abs(nowAngle2 - nowAngle1)) / 2;
			}
			else if (bestAngle1>=nowAngle1)
			{
				expansionAngle_backward = bestAngle1 - nowAngle1;
				expansionAngle_forward = interval*minIntervalPercentage - (nowAngle2 - bestAngle1);

			} 
			else if (bestAngle2<=nowAngle2)
			{
				expansionAngle_forward = bestAngle2 - nowAngle2;
				expansionAngle_backward = - (interval*minIntervalPercentage-(bestAngle2-nowAngle1));
			}
			else
			{
				cout << "intersectionOptimizer::solveOnce() �o��̎���case nowAngle1:" << nowAngle1 << ", baseAngle1:" << bestAngle1 << ", nowAngle2:" << nowAngle2 << ", bestAngle2:" << bestAngle2 << ";" << endl;
				system("pause");
			}
			if (contourList[0].id == 10){
				cout << "nowAngle1:" << nowAngle1 << ", bestAngle1:" << bestAngle1 << ", nowAngle2:" << nowAngle2 << ", bestAngle2:" << bestAngle2 << ";" << endl;
				cout << "expansionAngle_forward:" << expansionAngle_forward << ",expansionAngle_backward:" << expansionAngle_backward << endl;
			}
			vector<float> absorb_value(angleMappings.size() - 1);//ÿ���������յ�׃��������
			float total_len = 0;//1 - interval*minIntervalPercentage;
			for (int i = 1; i < angleMappings.size(); i++){
				float angle1 = angleMappings[(minGridIndex + i) % angleMappings.size()].value;
				float angle2 = angleMappings[(minGridIndex + i + 1) % angleMappings.size()].value;
				if (minGridIndex + i == angleMappings.size() - 1){
					angle1 = loopTransToLayer(angle1, floorf(angle2));
				}
				if (angle2 - angle1 > interval*minIntervalPercentage+SMALL_NUM){//����^��С���С�g����Ԓ,�t�@�^�^���ǲ����֔�׃������
					total_len += angle2 - angle1;
				}
			}
			for (int i = 1; i < angleMappings.size(); i++){
				float angle1 = angleMappings[(minGridIndex + i) % angleMappings.size()].value;
				float angle2 = angleMappings[(minGridIndex + i + 1) % angleMappings.size()].value;
				if (minGridIndex + i==angleMappings.size()-1){
					angle1 = loopTransToLayer(angle1, floorf(angle2));
				}
				//cout << "i=" << i << "angle2-angle1:" << angle2 - angle1<<",";
				if (angle2 - angle1 <= interval*minIntervalPercentage+SMALL_NUM)
					absorb_value[i - 1] = 0;//��������0��ζ���������κ�׃��
				else
					absorb_value[i - 1] = (angle2 - angle1) / total_len;
			}
			//cout <<endl<< "absorb_value:";
			float total = 0;
			for (int i = 0; i < absorb_value.size(); i++){
				//cout << " " << i << ":" << absorb_value[i] << ",";
				total += absorb_value[i];
			}
			//cout << "���������ٷֱ�֮��:" << total<<endl;
			//cout << "expansionAngle_forward:" << expansionAngle_forward << "expansionAngle_backward:" << expansionAngle_backward<<endl;
			float expansionAngle_total = expansionAngle_forward - expansionAngle_backward;
			float now_expansionAngle = expansionAngle_forward;
			angleMappings[(minGridIndex + 1) % angleMappings.size()].value += now_expansionAngle;
			for (int i = 1; i < angleMappings.size(); i++){//�U��
				//cout << "i=" << i << "angle1:" << angleMappings[(minGridIndex + i) % angleMappings.size()].value << " +=" << now_expansionAngle;
				//angleMappings[(minGridIndex + i) % angleMappings.size()].value += now_expansionAngle;
				//now_expansionAngle -= expansionAngle_total*absorb_value[i - 1];//�������ԜpС�˔U���Ƕ�
				//angleMappings[(minGridIndex + i+1) % angleMappings.size()].value += now_expansionAngle;
				//cout << "angle2:" << angleMappings[(minGridIndex + i + 1) % angleMappings.size()].value << "+=" << now_expansionAngle << ";    ";
				now_expansionAngle -= expansionAngle_total*absorb_value[i - 1];
				angleMappings[(minGridIndex + i + 1) % angleMappings.size()].value += now_expansionAngle;
			}
		}
		if (contourList[0].id == 13){
			cout << "id:13 ��" << counter++ << "�ΔU���Y��:";
			for (int i = 0; i < angleMappings.size(); i++){
				cout << i << ":" << angleMappings[i].value << " ";
			}
			cout << endl;
		}
	}
	cout << "solveOnce �Y��.";
	if (contourList[0].id == 10){
		cout << "contour10��K�Y��:";
		for (int i = 0; i < angleMappings.size(); i++){
			cout << "i=" << i << ":" << angleMappings[i].value << "; ";
		}
		cout << endl;
	}
}
void intersectionOptimizer::printBestGridInf(){
	for (int i = 0; i < gridList.size(); i++){
		//cout << "��i��grid:" << gridList[i].
	}
}
vector<OpenMesh::Vec3f> intersectionOptimizer::showBestGridPoints(){
	vector<OpenMesh::Vec3f> result;
	for each (rectGrid grid in gridList)
	{
		for (size_t i = 0; i < 4; i++)
		{
			result.push_back(grid.minDeformPoint[i]);
		}
	}
	return result;
}
optimizerGroup::optimizerGroup(vector<contour*> groups,int elemNum){
	contours = groups;
	elemPerRing = elemNum;
	float interval = 1.0f / (float)elemPerRing;
	angleListForContours[0].resize(contours.size());
	angleListForContours[1].resize(contours.size());
	for (int a = 0; a < angleListForContours[0].size();a++){
		angleListForContours[0][a].resize(elemNum);
		angleListForContours[1][a].resize(elemNum);
		for (int i = 0; i < elemPerRing; i++){
			angleListForContours[0][a][i] = interval*i;
			angleListForContours[1][a][i] = interval*i;
		}
	}
	//printAngleListForContours();
}
/*void optimizerGroup::solve(){
	contourHistory.clear();
	vector<contour*> headers;
	for each (contour* c in contours)
	{
		cout << "contour" << c->id << " lastContours:";
		for each (contour* l in c->lastContours)
		{
			cout << l->id << ", ";
		}
		cout << endl;
		if (c->lastContours.size() == 0){
			headers.push_back(c);
		}
		if (c->nextContours.size()>1)
		{
			for each (contour* next in c->nextContours)
			{
				headers.push_back(next);
			}
		}
	}
	cout << "header size" << headers.size()<<":";
	for each (contour* c in headers)
	{
		cout << c->id << ",";
	}
	bool shutdown = false;
	for each(contour* current in headers)
	{
		while (current->nextContours.size()==1)
		{
			int index = current->id;
			contourHistory.push_back(index);
			cout << "current index = " << index;
			contour* next = current->nextContours[0];
			intersectionOptimizer optim(*current,*next,angleListForContours[index][0],false);
			optim.solveOnce();
			int next_index = next->id;
			cout << "nextindex = " << next_index;
			cout << "angleListForContours size:" << angleListForContours[0].size()<<"angleMappings size:"<<optim.angleMappings.size();
			for (int i = 0; i < elemPerRing; i++)
			{
				angleListForContours[0][next_index][i] = optim.angleMappings[i].value;
			}
			current = next;
		}
		if (shutdown){
			break;
		}
	}
	cout << "optimizerGroup solve over"<<endl;
	cout << "log contour index history:";
	for each (int index in contourHistory)
	{
		cout << index << ",";
	}
	cout << endl;
}*/

void optimizerGroup::stage_solve(contour* stageBegin){
	int nowIndex = stageBegin->id;
	contour* current = stageBegin;
	while (current->nextContours.size() == 1){//��춛]�з����contour
		//cout << "̎��contour:" << nowIndex << ", ";
		contour* next = current->nextContours[0];
		int next_index = next->id;
		if (angleListForContours[0][nowIndex].size() <= OPTIMIZER_SOTP_GRID_NUM){//ֱ�Ӳ�����
			int stageElemNum = angleListForContours[0][nowIndex].size();
			angleListForContours[1][next_index].resize(stageElemNum);
			angleListForContours[0][next_index].resize(stageElemNum);
			for (int i = 0; i < stageElemNum; i++)
			{
				angleListForContours[1][nowIndex][i] = angleListForContours[0][nowIndex][i];
				angleListForContours[0][next_index][i] = angleListForContours[1][nowIndex][i];
			}
		}
		else
		{
			intersectionOptimizer optim(*current, *next, angleListForContours[0][nowIndex], false);
			optim.solveOnce();
			int stageElemNum = angleListForContours[0][nowIndex].size();
			angleListForContours[1][nowIndex].resize(stageElemNum);
			angleListForContours[0][next_index].resize(stageElemNum);
			for (int i = 0; i < stageElemNum; i++)
			{
				angleListForContours[1][nowIndex][i] = optim.angleMappings[i].value;
				angleListForContours[0][next_index][i] = optim.angleMappings[i].value;
			}
		}
		current = next;
		nowIndex = next_index;
	}
	if (current->nextContours.size()>1){//����ͣ�ڷ���contour

		vector<vector<angleArea>> totalAreas(current->divergenceSegm.size());
		//cout << "current->divergenceSegm size():" << current->divergenceSegm.size();
		for (size_t i = 0; i < current->divergenceSegm.size(); i++)
		{
			//cout << "̎��i:" << i << " contour index:" << current->nextContours[i]->id - 1 << endl;
			vector<angleArea> Areas;
			vector<fp_mapping> segm = current->divergenceSegm[i];
			for (size_t j = 0; j < segm.size(); j++)
			{
				fp_mapping mapping = segm[j];
				//cout << "̎�� fp_mapping:(" << mapping[0][0] << "," << mapping[0][1] << ")>>(" << mapping[1][0] << "," << mapping[1][1] << ")";
				if (mapping[1][1] - mapping[1][0] < 0.01){

				}
				else
				{
					angleArea nowArea(mapping.group[1]);
					//cout << "angleListForContours[nowIndex].size():" << angleListForContours[nowIndex].size()<<endl;
					float boundary1 = mapping[0][0];
					float boundary2 = mapping[0][1];//mapping.group[0]�ĽǶ���ֱ�ӏ�pfts��anglebyCal���}�u��,����ֻ����[0-1]
					
					bool crossZero = boundary1 > boundary2;
					//cout << "boundary1:" << boundary1 << " boundary2:" << boundary2<<"crossZero:"<<crossZero;
					vector<float> angleInside;
					for each (float angle in angleListForContours[0][nowIndex])
					{
						float angle01 = loopClampTo01(angle);//angleListForContours��ֵ�K����λ�[0-1]֮�ȵ�,���˱��^������[0-1]֮�g
						if (crossZero){
							if (angle01>=boundary1 || angle01<=boundary2){
								if (angle01 > boundary2){//0�c֮ǰ���c
									angle01 -=1;//��angle01�ۯB��[-1,1]֮�g,�@�ӷ���Ƕ�֮�g�ȴ�С
								}
								//��������,���C�Ƕȏ�С��������
								if (angleInside.size() == 0){
									angleInside.push_back(angle01);
								}
								else
								{
									
									int insertIndex;
									for (insertIndex = 0; insertIndex < angleInside.size(); insertIndex++){
										if (angleInside[insertIndex] > angle01){
											angleInside.insert(angleInside.begin() + insertIndex, angle01);
											break;
										}
									}
									if (insertIndex == angleInside.size()){//����β��߀�]���ҵ������c
										angleInside.push_back(angle01);
									}
								}
							}
						}
						else
						{
							if (angle01 >= boundary1&&angle01 <= boundary2){
								if (angleInside.size() == 0){
									angleInside.push_back(angle01);
								}
								else
								{

									int insertIndex;
									for (insertIndex = 0; insertIndex < angleInside.size(); insertIndex++){
										if (angleInside[insertIndex] > angle01){
											angleInside.insert(angleInside.begin() + insertIndex, angle01);
											break;
										}
									}
									if (insertIndex == angleInside.size()){//����β��߀�]���ҵ������c
										angleInside.push_back(angle01);
									}
								}
							}
						}
					}
					if (crossZero)
					{
						boundary1 -= 1;
					}
					if (angleInside.size() > 1)
					{
						float group0Length = boundary2 - boundary1;
						float group1Length = mapping[1][1] - mapping[1][0];//mapping.group[1]����ȡ�������c�����c�����a����,����λ�[0,2]֮�g
						nowArea.fixGridAngles.resize(angleInside.size());
						//current->divergenceSegm[i][j].intersectionMapping.resize(angleInside.size());
						for (size_t p = 0; p < angleInside.size(); p++)
						{
							
							float angle_inNext = ((angleInside[p] - boundary1) / group0Length)*group1Length + mapping[1][0];
							float angle_ori = angleInside[p];
							if (angle_ori < 0){
								angle_ori += 1;
							}
							//cout<<"angle:"<<angle_ori << " angle01:" << angleInside[i]<<"angleInNext:"<<angle_inNext<<endl;
							nowArea.fixGridAngles[p] = angle_inNext;
							//current->divergenceSegm[i][j].intersectionMapping[p] = float_pair(angleInside[p], angle_inNext);
						}
						Areas.push_back(nowArea);
					}
					/*cout << "nowArea:";
					for each (float angle in nowArea.fixGridAngles)
					{
						cout << angle << ",";
					}
					cout << endl;*/
				}
			}//for (size_t j = 0; j < segm.size(); j++)
			totalAreas[i]= Areas;
		}
		for (size_t i = 0; i < totalAreas.size(); i++)
		{
			contour *next = current->nextContours[i];
			int next_index = next ->id;
			//cout << "̎�����ȸ߾�index:" << current->id - 1 << " next_index:" << next_index << endl;
			if (totalAreas[i].size() != 0){
			   angleArea  area0= totalAreas[i][0];
				angleListForContours[0][next_index] = assignGridWithInitArea(totalAreas[i]);

			}
			else
				angleListForContours[0][next_index] = newAngleList(elemPerRing);
			stage_solve(next);
		}
	}//if (current->nextContours.size()>1)
	else//����ͣ��ĩ��
	{
		//cout << "contour end" << endl;
		return;
	}
	

}
int indexOf(vector<float> list, float elem){
	for (int i = 0; i < list.size(); i++){
		if (list[i] == elem){
			return i;
		}
	}
	return -1;
}
void insertSort(vector<float> &list,float value){
	int index = 0;
	for (; index < list.size(); index++){
		if (list[index]>value){
			break;
		}
	}
	if (index==list.size())
	{
		list.push_back(value);
	}
	else
	{
		list.insert(list.begin() + index, value);
	}
}
void optimizerGroup::stage_solve(contour* stageBegin,float shift_percent){
	origenInf.resize(contours.size());
	int nowIndex = stageBegin->id;
	contour* current = stageBegin;
	int level = 0;
	vector<float> origenAngleList = angleListForContours[0][nowIndex];
	int origenIndex = nowIndex;
	while (current->nextContours.size() == 1){//��춛]�з����contour
		//cout << "̎��contour:" << nowIndex << ", ";
		contour* next = current->nextContours[0];
		int next_index = next->id;
		if (angleListForContours[0][nowIndex].size() <= OPTIMIZER_SOTP_GRID_NUM){//ֱ�Ӳ�����
			int stageElemNum = angleListForContours[0][nowIndex].size();//ԓȦ�ĵڶ��l�ȸ߾�ֱ���^�е�һ�l�Ľ��c�Ƕ�
			angleListForContours[1][nowIndex].resize(stageElemNum);
			angleListForContours[0][next_index].resize(stageElemNum);
			for (int i = 0; i < stageElemNum; i++)
			{
				angleListForContours[1][nowIndex][i] = angleListForContours[0][nowIndex][i];
				float len = loopFindNearestBiggerValue(angleListForContours[0][nowIndex][i], angleListForContours[0][nowIndex][(i + 1)% angleListForContours[0][nowIndex].size()])-angleListForContours[0][nowIndex][i];
				//cout << "v2=" << loopFindNearestBiggerValue(angleListForContours[0][nowIndex][i], angleListForContours[0][nowIndex][(i + 1) % angleListForContours[0][nowIndex].size()]) << " v1=" << angleListForContours[0][nowIndex][i] << " len=" << len << "; ";
				angleListForContours[0][next_index][i] = angleListForContours[0][nowIndex][i]+len*shift_percent;//��һȦ�ĽǶ�����һ��shift
			}
			origenInf[nowIndex].origenContourIndex = origenIndex;
			origenInf[nowIndex].origenAngleList = origenAngleList;
			origenInf[nowIndex].subLevel = level;
			origenInf[nowIndex].totalOffset = level*shift_percent;
		}
		/*else
		{
			intersectionOptimizer optim(*current, *next, angleListForContours[0][nowIndex], false);
			optim.solveOnce();
			int stageElemNum = angleListForContours[0][nowIndex].size();
			angleListForContours[0][next_index].resize(stageElemNum);
			angleListForContours[1][nowIndex].resize(stageElemNum);
			for (int i = 0; i < stageElemNum; i++)
			{
				angleListForContours[1][nowIndex][i] = optim.angleMappings[i].value;
			}
			for (int i = 0; i < stageElemNum; i++)
			{
				float len = loopFindNearestBiggerValue(angleListForContours[1][nowIndex][i], angleListForContours[1][nowIndex][(i + 1) % angleListForContours[1][nowIndex].size()]) - angleListForContours[1][nowIndex][i];
				angleListForContours[0][next_index][i] = angleListForContours[1][nowIndex][i] + len*shift_percent;
			}
		}*/
		current = next;
		nowIndex = next_index;
		level++;
	}
	origenInf[nowIndex].origenContourIndex = origenIndex;
	origenInf[nowIndex].origenAngleList = origenAngleList;
	origenInf[nowIndex].subLevel = level;
	origenInf[nowIndex].totalOffset = level*shift_percent;
	if (current->nextContours.size()>1){//����ͣ�ڷ���contour
		vector<vector<angleArea>> totalAreas(current->divergenceSegm.size());
		//cout << "current->divergenceSegm size():" << current->divergenceSegm.size();
		//cout << "divergence cindex:" << current->id - 1;
		if (nowIndex == 29){
			cout << "index 29 contour";
		}
		for (size_t i = 0; i < current->divergenceSegm.size(); i++)
		{
			//cout << "̎��i:" << i << " contour index:" << current->nextContours[i]->id - 1 << endl;
			vector<angleArea> Areas;
			vector<fp_mapping> segm = current->divergenceSegm[i];
			for (size_t j = 0; j < segm.size(); j++)
			{
				fp_mapping mapping = segm[j];
				if (nowIndex == 29)
					cout << "̎�� fp_mapping:(" << mapping[0][0] << "," << mapping[0][1] << ")>>(" << mapping[1][0] << "," << mapping[1][1] << ")";
				if (mapping[1][1] - mapping[1][0] < 0.01){

				}
				else if (loopFindNearestBiggerValue(mapping[0][0],mapping[0][1]) - mapping[0][0] < 0.01){
					//cout << "mapping[0][1]:" << mapping[0][1] << " mapping[0][0]:" << mapping[0][0];
					//cout << endl;
				}
				else
				{
					angleArea nowArea(mapping.group[1]);
					//cout << "angleListForContours[nowIndex].size():" << angleListForContours[nowIndex].size()<<endl;
					
					float boundary1 = mapping[0][0];
					float boundary2 = mapping[0][1];//mapping.group[0]�ĽǶ���ֱ�ӏ�pfts��anglebyCal���}�u��,����ֻ����[0-1]
					
					bool crossZero = boundary1 > boundary2;
					//cout << "boundary1:" << boundary1 << " boundary2:" << boundary2<<"crossZero:"<<crossZero;
					//for (int i = 0; i < angleListForContours[nowIndex].size(); i++){
					vector<int> gridInside;
					//for each (float angle in angleListForContours[0][nowIndex])
					/*for (int i = 0; i < angleListForContours[0][nowIndex].size();i++)
					{
						//int listSize = angleListForContours[0][nowIndex].size();
						float angle1 = angleListForContours[0][nowIndex][i];
						
						float angle01 = loopClampTo01(angle1);//angleListForContours��ֵ�K����λ�[0-1]֮�ȵ�,���˱��^������[0-1]֮�g
						if (crossZero){
							if (angle01>=boundary1 || angle01<=boundary2){
								if (angle01 > boundary2){//0�c֮ǰ���c
									angle01 -=1;//��angle01�ۯB��[-1,1]֮�g,�@�ӷ���Ƕ�֮�g�ȴ�С
								}
								//��������,���C�Ƕȏ�С��������
								if (angleInside.size() == 0){
									angleInside.push_back(angle01);
								}
								else
								{
									
									int insertIndex;
									for (insertIndex = 0; insertIndex < angleInside.size(); insertIndex++){
										if (angleInside[insertIndex] > angle01){
											angleInside.insert(angleInside.begin() + insertIndex, angle01);
											break;
										}
									}
									if (insertIndex == angleInside.size()){//����β��߀�]���ҵ������c
										angleInside.push_back(angle01);
									}
								}
							}
						}
						else
						{
							if (angle01 >= boundary1&&angle01 <= boundary2){
								if (angleInside.size() == 0){
									angleInside.push_back(angle01);
								}
								else
								{

									int insertIndex;
									for (insertIndex = 0; insertIndex < angleInside.size(); insertIndex++){
										if (angleInside[insertIndex] > angle01){
											angleInside.insert(angleInside.begin() + insertIndex, angle01);
											break;
										}
									}
									if (insertIndex == angleInside.size()){//����β��߀�]���ҵ������c
										angleInside.push_back(angle01);
									}
								}
							}
						}
					}*/
					for (int i = 0; i < origenInf[nowIndex].origenAngleList.size(); i++){
						float angle01_1 =loopClampTo01(getXat(nowIndex, i, 0));
						bool angle1_inside = false;
						if (crossZero){
							if (angle01_1 >= boundary1 || angle01_1 <= boundary2){
								angle1_inside = true;
								
							}
						}
						else
						{
							if (angle01_1 >= boundary1&&angle01_1 <= boundary2){
								angle1_inside = true;
								
							}
						}
						float angle01_2 = loopClampTo01(getXat(nowIndex, (i+1)%origenInf[nowIndex].origenAngleList.size(), 0));
						bool angle2_inside = false;
						if (crossZero){
							if (angle01_2 >= boundary1 || angle01_2 <= boundary2){
								angle2_inside = true;
							}
						}
						else
						{
							if (angle01_2 >= boundary1&&angle01_2 <= boundary2){
								angle2_inside = true;
							}
						}
						if (angle1_inside&&angle2_inside){
							gridInside.push_back(i);
						}
					}
					if (nowIndex == 29){
						cout << "gridInside:" << endl;
						for each (int gidx in gridInside)
						{
							cout << gidx << "," << endl;
						}
					}
					if (crossZero)//��ԭ���boundary2���boundary1��1����������ͽǶȱȴ�С
					{
						boundary1 -= 1;
					}
					if(true) //(angleInside.size() > 1)
					{
						vector<float> angleInside;
						current->divergenceSegm[i][j].gridMarkers.resize(gridInside.size());
						float group0Length = boundary2 - boundary1;
						float group1Length = mapping[1][1] - mapping[1][0];//mapping.group[1]����ȡ�������c�����c�����a����,����λ�[0,2]֮�g,�K������cross zero�Ć��}
						for (int g = 0; g < gridInside.size();g++)
						{
							//cout << "gridInside:" << gridInside[g];
							current->divergenceSegm[i][j].gridMarkers[g].gridIndex = gridInside[g];
							float angle1 = getXat(nowIndex, gridInside[g], 0);
							if (crossZero&&angle1 > boundary2)
							{
								angle1 -= 1;
							}
							
							float angle1_inNext = ((loopFindNearestBiggerValue(boundary1,angle1) -boundary1) / group0Length)*group1Length + mapping[1][0];
							//cout << "angle1:" << angle1 << " inNext:" << angle1_inNext <<"loopFindNearestBiggerValue(angle1,boundary1):"<< loopFindNearestBiggerValue(angle1, boundary1);
							current->divergenceSegm[i][j].gridMarkers[g].angleCorresponds[0] = angle1_inNext;
							if (indexOf(angleInside, angle1_inNext)<0)//angleInside�ț]��angle1_inNext
							{
								insertSort(angleInside, angle1_inNext);
							}
							float angle2 = getXat(nowIndex,gridInside[g],1);
							if (crossZero&&angle2 > boundary2)
							{
								angle2 -= 1;
							}
							
							float angle2_inNext = ((loopFindNearestBiggerValue(boundary1, angle2) - boundary1) / group0Length)*group1Length + mapping[1][0];
							//cout << "angle2:" << angle2<<" inNext:"<<angle2_inNext;
							current->divergenceSegm[i][j].gridMarkers[g].angleCorresponds[1] = angle2_inNext;
							if (indexOf(angleInside, angle2_inNext)<0)//angleInside�ț]��angle1_inNext
							{
								insertSort(angleInside, angle2_inNext);
							}
							cout << endl;
						}
						nowArea.fixGridAngles = angleInside;
						//nowArea.fixGridAngles.resize(angleInside.size());

						//current->divergenceSegm[i][j].intersectionMapping.resize(angleInside.size());
						//current->divergenceSegm[i][j].gridMarkers = gridInside;
						/*for (size_t p = 0; p < angleInside.size(); p++)
						{
							
							float angle_inNext = ((angleInside[p] - boundary1) / group0Length)*group1Length + mapping[1][0];
							float angle_ori = angleInside[p];
							if (angle_ori < 0){
								angle_ori += 1;
							}
							//cout<<"angle:"<<angle_ori << " angle01:" << angleInside[i]<<"angleInNext:"<<angle_inNext<<endl;
							nowArea.fixGridAngles[p] = angle_inNext;
							current->divergenceSegm[i][j].intersectionMapping[p] = float_pair(angleInside[p], angle_inNext);
						}*/
						if (nowArea.fixGridAngles.size() < 2){//������r,���angleInsideС�2��,�f���ǲ������B�ӵ��и�
							if (Areas.size()==0)//ֻ��Area�ǿյĵĕr��ŕ����Mȥ�������
							{
								Areas.push_back(nowArea);
							}
						}
						else{
							Areas.push_back(nowArea);
						}
					}
				}
			}//for (size_t j = 0; j < segm.size(); j++)
			totalAreas[i]= Areas;
			if (nowIndex == 29){
				cout << endl;
				for each (vector<angleArea> areas in totalAreas)
				{

					for each (angleArea area in areas)
					{
						cout << "(";
						for each (float angle in area.fixGridAngles)
						{
							cout << angle << ",";
						}
						cout << ")+";
					}
					cout << endl;
				}
				cout << "---------------------------------------------------------";
			}
			
		}

		for (size_t i = 0; i < totalAreas.size(); i++)
		{
			contour *next = current->nextContours[i];
			int next_index = next ->id;
			if (nowIndex == 29){
				cout << "̎�����ȸ߾�index:" << current->id<< " next_index:" << next_index << endl;
			}
			if (totalAreas[i].size() != 0){//������i���μ��ȸ߾��ą^�����
				if (totalAreas[i].size()==1&&totalAreas[i][0].fixGridAngles.size()==0){//ֻ��һ���ȸ߾�֮�g�ą^���҅^��ț]�Є��ֱ�����B��
					float length1 = current->length;//ֱ����������angleList
					float length2 = next->length;
					int pointNum = angleListForContours[0][nowIndex].size();
					int newNum = ceilf((length2 / length1)*pointNum);
					angleListForContours[0][next_index] = newAngleList(newNum);
				}
				else if (totalAreas[i].size() == 1 && totalAreas[i][0].fixGridAngles.size() == 1){//ֻ��һ���ȸ߾�֮�g�ą^���҅^���ֻ��һ�l���ֱ�����B��
					float length1 = current->length;//���ǂ������B���ڴμ��ȸ߾��ϵ��c������һ��angle�����µ�angleList
					float length2 = next->length;
					int pointNum = angleListForContours[0][nowIndex].size();
					int newNum = ceilf((length2 / length1)*pointNum);
					angleListForContours[0][next_index] = newAngleList(totalAreas[i][0].fixGridAngles[0], newNum);
				}
				else{//�Զ����^��Ą����B�����¼��ȸ߾��ȵĶ��c��һ��Ҫ���ڵ�angle,���N�]�ж��c�Ĳ����Ԅ���֮�g������g��Ȳ���µ�angle
					if (totalAreas[i][0].fixGridAngles.size()<2)//???
					{
						totalAreas[i].erase(totalAreas[i].begin());
					}
					vector<float> newRing = assignGridWithInitArea(totalAreas[i]);//ʹ�����Ѕ^��Ķ��c�Ȳ�����langleList
					vector<float> newRing_shift(newRing.size());
					for (size_t i = 0; i < newRing.size(); i++)
					{
						float angle1 = newRing[i];
						float angle2 = newRing[(i+1)%newRing.size()];
						if (i == newRing.size() - 1){
							angle2 = angle2 + 1;
						}
						newRing_shift[i] = angle1 + (angle2 - angle1)*shift_percent;
					}
					angleListForContours[0][next_index] = newRing_shift;
					if (nowIndex == 29){
						for each (angleArea area in totalAreas[i])
						{
							cout << "(";
							for each (float angle in area.fixGridAngles)
							{
								cout << angle << ",";
							}
							cout << ")+";
						}
						cout << endl;
						cout << "newRing:";
						for each (float angle in newRing)
						{
							cout << angle << ",";
						}
						cout << endl;
					}

					/*angleListForContours[0][next_index].resize(newRing.size());
					for (size_t j = 0; j < newRing.size(); j++)
					{
						float angle1 = newRing[j];
						float angle2 = loopFindNearestBiggerValue(newRing[j], newRing[(j + 1) % newRing.size()]);
						angleListForContours[0][next_index][j] = angle1 + (angle2 - angle1)*shift_percent;
					}*/
				}
			}
			else{
				float length1 = current->length;
				float length2 = next->length;
				int pointNum = angleListForContours[0][nowIndex].size();
				int newNum = ceilf((length2 / length1)*pointNum);
				angleListForContours[0][next_index] = newAngleList(newNum);
			}
			stage_solve(next, shift_percent);
		}
	}//if (current->nextContours.size()>1)
	else//����ͣ��ĩ��
	{
		//cout << "contour end" << endl;
		return;
	}
	

}
vector<float> optimizerGroup::getAngleListForContour(int index, bool contour1){
	if (contour1)
		return angleListForContours[0][index];
	else
		return angleListForContours[1][index];
}
void optimizerGroup::printAngleListForContours(){
	for (size_t i = 0; i < angleListForContours[0].size(); i++)
	{
		vector<float> ring1 = angleListForContours[0][i];
		vector<float> ring2 = angleListForContours[1][i];

		cout << "ring1 for contour" << contours[i]->id << ":";
		for each (float angle in ring1)
		{
			cout << angle << ",";
		}
		cout << endl;
		if (contours[i]->nextContours.size() == 1){
			cout << "ring2 for contour" << contours[i]->nextContours[0] << ":";
			for each (float angle in ring2)
			{
				cout << angle << ",";
			}
			cout << endl;
		}
		else{
			cout << "ring2 �]�����x.";
		}
	}
}
void optimizerGroup::printAngleListForContours_stage(vector<contour*> contours,int beginIndex){
	int nowIndex = beginIndex;
	cout << "print stage root index:" << nowIndex << "---------------------------";
	while (contours[nowIndex]->nextContours.size()==1)
	{
		cout << "angleList " << nowIndex << "(size"<<angleListForContours[nowIndex].size()<<"):";
		
		for (size_t i = 0; i < angleListForContours[0][nowIndex].size(); i++)
		{
			cout << i << ": " << angleListForContours[0][nowIndex][i] << "=>" << angleListForContours[1][nowIndex][i] << ";";
		}
		nowIndex = contours[nowIndex]->nextContours[0]->id;
	}
	if (contours[nowIndex]->nextContours.size()>0){
		for each (contour* next in contours[nowIndex]->nextContours)
		{
			cout << ">>next index" << next->id<< ":" << endl;
			printAngleListForContours_stage(contours, next->id);

		}
	}
}
float calDeformEnergy(contour current, float at_angle,float interval){
	if (current.nextContours.size() != 1){
		return -1;
	}
	contour next = *current.nextContours[0];
	float distance = (current.getPosAtAngle(at_angle) - next.getPosAtAngle(at_angle)).length();
	return distance / interval;
}
optimizerGroup::optimizerGroup(vector<contour*> contours, int elemPerRing, float interval){
	this->contours = contours;
	this->contourInterval = interval;
}
void optimizerGroup::calAllDeformEnergy(){
	cout << "contour size:" << contours.size();
	for each (contour *c in contours)
	{
		cout << "contour" << c->id<<":"<<endl;
		if (c->nextContours.size() != 1){
			vector<deformLabel> energyList(sampleRate);
			Vec3f_angle *sampleList = c->sample(sampleRate);
			for (int i = 0; i < sampleRate; i++){
				energyList[i] = deformLabel(sampleList[i], 1);
			}
			deformEnergys.push_back(energyList);
		}
		else
		{
			vector<deformLabel> energyList(sampleRate);
			Vec3f_angle *sampleList = c->sample(sampleRate);
			for (int i = 0; i < sampleRate; i++){
				float energy= calDeformEnergy(*c, sampleList[i].angle, contourInterval);
				energyList[i]=deformLabel(sampleList[i],energy);
				//cout << i << ":" << energyList[i].energy << ",";
				if (energy == 0){
					//cout << "�ҵ�energy��0���c";
				}
			}
			//cout << "energyList size:" << energyList.size();
			deformEnergys.push_back(energyList);
			
		}
	}
}
float optimizerGroup::getXat(int contour_index, int elem_index, float x){//ݔ���elem_index��ָ���ڎׂ�elem,x��λ춴�elem�еİٷֱ�
	offsetInf inf = origenInf[contour_index];
	/*cout << "ӡ��origenAngleList:";
	for (size_t i = 0; i < inf.origenAngleList.size(); i++)
	{
		cout << inf.origenAngleList[i] << ",";
	}
	cout << endl;*/
	float total_offset = inf.totalOffset;
	int index_offset = floorf(total_offset + x);
	elem_index += index_offset;//����offset
	elem_index = elem_index%inf.origenAngleList.size();
	//cout << "elem_index:" << elem_index;
	x = (total_offset + x) - index_offset;

	float angle1 = inf.origenAngleList[elem_index];
	float angle2 = 0; //inf.origenAngleList[elem_index + 1];
	if (elem_index==origenInf[contour_index].origenAngleList.size()-1)//����һ��grid
	{
		angle2 = inf.origenAngleList[0] + 1;
	}
	else
	{
		angle2 = inf.origenAngleList[elem_index + 1];
	}
	//cout << "angle1:" << angle1 << " angle2:" << angle2 << "angle1 + (angle2 - angle1)*x:" << angle1 + (angle2 - angle1)*x;
	return angle1 + (angle2 - angle1)*x;
}
vector<float> assignGridWithInitArea(vector<angleArea> initArea){//��춳�ʼԒ����ȸ߾��ĳ�ʼ���c�ց�.��֪���Ѕ^��initArea,�佻�c�ǶȽM��initArea[i].fixGridAngles,�؂��ɴ˃Ȳ���ĵȸ߾������н��c�ĽǶ�
	vector<float_pair> maxEmptyArea;//���Ŀհׅ^����ζ���p������С���Ж|���ą^��(minAngleBoundary)
	//vector<float_pair> minEmptyArea;
	vector<angleArea> sortedInitArea;
	//cout << "initArea size:" << initArea.size()<<" ";
	while (initArea.size()>0)//��������l,׌
	{
		int minStartAngleIndex = 0;
		for (size_t j = 1; j < initArea.size(); j++)
		{
			if (initArea[j].minAngleBoundary()[0] < initArea[minStartAngleIndex].minAngleBoundary()[0]){
				minStartAngleIndex = j;
			}
		}
		sortedInitArea.push_back(initArea[minStartAngleIndex]);
		initArea.erase(initArea.begin()+minStartAngleIndex);
	}
	/*cout << "sort ��Y��:";
	for each(angleArea area in sortedInitArea){
		cout << "[";
		for each (float angle in area.fixGridAngles)
		{
			cout << angle << ",";
		}
		cout << "];";
	}*/
	angleArea firstarea = sortedInitArea[0];
	//�h�и��_ʼ
	float minGridAngle = 1000000;//Ӌ��һ����С�Ľ��c�g�g�����׵ĽǶ�,����Ȳ���հ׽��c�Ļ���
	for each (angleArea area in sortedInitArea)
	{
		for (size_t i = 0; i < area.fixGridAngles.size()-1; i++)
		{
			float interval = area.fixGridAngles[i + 1] - area.fixGridAngles[i];
			if (interval < minGridAngle){
				minGridAngle = interval;
			}
		}
	}
	//cout << "minGridAngle:" << minGridAngle;

	vector<float> finalAngleList;
	for (int i = 0; i < sortedInitArea.size(); i++){
		//cout << "i=" << i << ":";
		angleArea now_fixArea = sortedInitArea[i];
		angleArea next_fixArea = sortedInitArea[(i + 1) % sortedInitArea.size()];
		finalAngleList.insert(finalAngleList.end(), now_fixArea.fixGridAngles.begin(), now_fixArea.fixGridAngles.end());
		float next_Angle = loopFindNearestCycle(now_fixArea.minAngleBoundary()[1], next_fixArea.minAngleBoundary()[0]);//loopTransToLayer(next_fixArea.minAngleBoundary()[0], floor(now_fixArea.minAngleBoundary()[1]));
		//cout << "next_Angle:" << next_Angle << "now_fixArea.minAngleBoundary()[1]:" << now_fixArea.minAngleBoundary()[1];
		float emptyLength = next_Angle - now_fixArea.minAngleBoundary()[1];
		//cout << "emptyLength:" << emptyLength;
		int newGridNum = emptyLength / minGridAngle;//�����emptyLength������ܷ��Îׂ��L�Ȟ�minGridAngle�ĸ���
		//cout << " newGridNum:" << newGridNum;
		float gridLen = emptyLength / newGridNum;
		//cout << "gridLen:" << gridLen;
		for (int j = 1; j < newGridNum; j++)//0�����ӻ���1�����ӵ�Ԓ�Ͳ��ò����µĽ��c,2�����Ӳ���һ�����c�х^��֞�2��,�Դ����
		{
			finalAngleList.push_back(now_fixArea.minAngleBoundary()[1] + gridLen*j);
		}
		//cout << endl;
	}
	return finalAngleList;
}
vector<float> newAngleList(int elemNum){
	float interval = 1.0f /(float)elemNum;
	vector<float> result(elemNum);
	for (int i = 0; i < elemNum; i++){
		result[i] = i*interval;
	}
	return result;
}
vector<float> newAngleList(float startAngle, int elemNum){
	float interval = 1.0f / (float)elemNum;
	vector<float> result(elemNum);
	for (int i = 0; i < elemNum; i++){
		result[i] = startAngle+ i*interval;
	}
	return result;
}
