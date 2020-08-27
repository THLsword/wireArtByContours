#pragma once
#include "polarMap.h";
//�Lԇͨ�^؝�����㷨�pС׃�γ̶�,�F�ڜpС׃�ζȵĹ����ѽ����U,���pС׃�ζȌ�����anglelist�Ĳ�����,�@�����������ȸ߾��f��,����Y���������Ӳ���
struct rectGrid
{
	OpenMesh::Vec3f points[4];
	float area = -1;
	float weight = -1;
	float minWeight = -1;
	OpenMesh::Vec3f minDeformPoint[4];
	float minAnglePair[2];
	float minAngleCrossZero[2];
	rectGrid(OpenMesh::Vec3f p11, OpenMesh::Vec3f p12, OpenMesh::Vec3f p21, OpenMesh::Vec3f p22, float w,float min_w){
		points[0] = p11;
		points[1] = p12;
		points[2] = p21;
		points[3] = p22;


		const int x = 0;
		const int y = 1;
		const int z = 2;
		OpenMesh::Vec3f p1 = p12 - p11;
		OpenMesh::Vec3f p2 = p21 - p11;
		OpenMesh::Vec3f faceVec1(p1[y]*p2[z] - p2[y]*p1[z], p1[z]*p2[x] - p2[z]*p1[x], p1[x]*p2[y]-p2[x]*p1[y]);
		
		p1 = p12 - p22;
		p2 = p21 - p22;
		OpenMesh::Vec3f faceVec2(p1[y]*p2[z] - p2[y]*p1[z], p1[z]*p2[x] - p2[z]*p1[x], p1[x]*p2[y] - p2[x]*p1[y]);
		area = faceVec1.length()*0.5f+faceVec2.length()*0.5f;
		weight = w;
		minWeight = min_w;
	}
	rectGrid(){

	}
	void updatePartsPoints(OpenMesh::Vec3f *p11, OpenMesh::Vec3f *p12, OpenMesh::Vec3f *p21, OpenMesh::Vec3f *p22,float w){
		if (p11 != NULL){
			points[0] = *p11;
		}
		if (p12 != NULL){
			points[1] = *p12;
		}
		if (p21 != NULL){
			points[2] = *p21;
		}
		if (p22 != NULL){
			points[3] = *p22;
		}
		const int x = 0;
		const int y = 1;
		const int z = 2;
		OpenMesh::Vec3f p1 = points[1]-points[0];
		OpenMesh::Vec3f p2 = points[2]-points[0];
		OpenMesh::Vec3f faceVec1(p1[y] * p2[z] - p2[y] * p1[z], p1[z] * p2[x] - p2[z] * p1[x], p1[x] * p2[y] - p2[x] * p1[y]);

		p1 = points[1]-points[3];
		p2 = points[2]-points[3];
		OpenMesh::Vec3f faceVec2(p1[y] * p2[z] - p2[y] * p1[z], p1[z] * p2[x] - p2[z] * p1[x], p1[x] * p2[y] - p2[x] * p1[y]);
		area = faceVec1.length()*0.5f + faceVec2.length()*0.5f;
		weight = w;

	}
};
struct floatPair{
	float key;
	float value;
	bool flag = false;
	floatPair(float k, float v){
		key=k;
		value=v;
	}
	floatPair(){
		key = -1;
		value = -1;
	}
};
struct floatGroup
{
	float key;
	vector<floatPair> values;
	bool flag = false;
	floatGroup(){

	}
	floatGroup(float k){
		key = k;
	}
};
struct gridGroups{
	int index;
	vector<floatGroup> groups;
	gridGroups(int idx){
		index = idx;
	}
	gridGroups(){
		index = -1;
	}
};
struct  deformLabel
{
	Vec3f_angle samplePoint;
	float energy;
	deformLabel(){

	}
	deformLabel(Vec3f_angle pt, float e){
		samplePoint = pt;
		energy = e;
	}
};
struct angleArea
{
	float_pair angleBoundary;
	vector<float> fixGridAngles;//���ܱ��Ąӵĸ��ӵ�����߅�ĽǶ�,�����Ѓɂ����Ӿ͕���3��ֵ���ʾ   ...| grid1 | grid2 |...
	float_pair minAngleBoundary(){
		return float_pair(fixGridAngles.front(), fixGridAngles.back());
	}
	angleArea(){

	}
	angleArea(float_pair boundary){
		angleBoundary = boundary;
	}
	angleArea(float_pair boundary,vector<float> angles){
		angleBoundary = boundary;
		fixGridAngles = angles;
	}

};
vector<float> assignGridWithInitArea(vector<angleArea> initArea);
vector<float> newAngleList(int elemNum);
vector<float> newAngleList(float startAngle, int elemNum);
class intersectionOptimizer//�Lԇ��؝�����㷨�p��׃����,������ʧ����K//�ѽ�����
{
public:
	float minJumpPercentage = 2.0f;//��ʼ����ȡ���ܟoҕ�����Ƕ��x������c�����interval����󹠇�
	float minIntervalPercentage = 0.6f;//solveOnce��grid���Ա��D����������interval��С�ı���
	float maxIntervalPercentage = 1.5f;
	float coeff_edge=1000;
	vector<gridGroups> deformRecords;
	vector<rectGrid> gridList;
	vector<floatPair> angleMappings;
	void printBestGridInf();
	vector<OpenMesh::Vec3f> showBestGridPoints();
	contour contourList[2];
	int girdNum = -1;
	int sampleRate = 1000;//��Ӌ����ؕr���1000���c;
	intersectionOptimizer();
	intersectionOptimizer(contour last, contour next,int num);
	intersectionOptimizer(contour last, contour next,vector<float> angleList);
	intersectionOptimizer(contour last, contour next, vector<float> angleList, bool negAngel);
	~intersectionOptimizer();
	float calDeform(OpenMesh::Vec3f p11,OpenMesh::Vec3f p12,OpenMesh::Vec3f p21,OpenMesh::Vec3f p22);
	float calLengrhDeform(float avgLength, float angle1, float angle2);
	void solve(int iter_num);
	void solveOnce();
};

struct offsetInf{
	int origenContourIndex = -1;
	vector<float> origenAngleList;//�Á�ӛ䛳�ʼһ�h��angle list
	int subLevel = 0;//�Á�ӛ䛮�ǰ�ȸ߾���λ���Դ�^�ȸ߾���ĵڎ׌ӵȸ߾�,Դ�^�ȸ߾���subLevel��0
	float totalOffset = 0;//totalOffset���۷e��offset_X,Դ�^�ȸ߾���ÿһ�����۷eһ��offset_X
};
class optimizerGroup{
protected:
	int elemPerRing;
	vector<contour*> contours;
	vector<vector<float>> angleListForContours[2];//ÿ��angleListForContour[0][x]��angleListForContour[1][x]���ÿ��ring,�@��level��x = contour.id-1������ʼcontour,angleListForContour[0][x]��ʾÿ��lelem����ʼcontour�ϵ����c�ĽǶ�
	//angleListForContour[1][x]��ÿ��elem��contour��һ�lcontour�ϵ���ʼ�Ƕ�,��Ȼ�@֮�m���c�H��һ��nextContour����r,���1������0���ĕr���H�ϲ����õ�angleListForContour[1]
	
	int OPTIMIZER_SOTP_GRID_NUM =1000;
	float contourInterval = 0;
public:
	optimizerGroup(vector<contour*> contours,int elemPerRing);
	optimizerGroup(vector<contour*> contours, int elemPerRing, float interval);
	//void solve();
	void stage_solve(contour* stageBegin);//�f��stage_solve,�ѽ�����
	void stage_solve(contour* stageBegin,float elem_startPoint_shift_percent);
	void printAngleListForContours();
	void printAngleListForContours_stage(vector<contour*> contours,int beginIndex);
	vector<float> getAngleListForContour(int index,bool contour1);
	void calAllDeformEnergy();
	int sampleRate = 1000;
	vector<vector<deformLabel>> deformEnergys;
	vector<Vec3f_angle*> sampleRecord;
	vector<int> contourHistory;
	vector<offsetInf> origenInf;
	float getXat(int contour_index,int elem_index,float x);
};
void bubbleSort(vector<floatPair> &marray);