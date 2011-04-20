#include "subdivision.h"

vector<Point> objVertix;
vector<vector<int>> objVertixIndex;
vector<HalfEdge*> edges;
vector<Face*> faces;
vector<Vertex*> vertices;

int loop = 0;

int charToInt(char *ptr)
{
	int n=0;
	while(*ptr>='0'&&*ptr<='9')
	{
		n = n*10 + *ptr-'0';
		ptr++;
	}
	return n;
}

bool readObj(const char *filename)
{
	vector<int> tempVertixIndex;
	string tempLine;
	ifstream input;
	input.open(filename);
	if(!input)
		return false;
	Point temp;
	char buf[256];
	input>>buf;
	while(buf[0]!=EOF)
	{
		switch(buf[0])
		{
		case '#':
			getline(input,tempLine);
			input>>buf;
			break;
		case 'v':
			switch(buf[1])
			{
			case '\0':
				input>>temp.x>>temp.y>>temp.z>>buf;
				objVertix.push_back(temp);
				break;
			default:
				input>>buf;
				break;
			}
			break;
		case 'f':
			switch(buf[1])
			{
			case '\0':
				input>>buf;
				while(buf[0]>='0' && buf[0]<='9')
				{
					char *ptr = buf;
					tempVertixIndex.push_back(charToInt(ptr));
					input>>buf;
				}
				objVertixIndex.push_back(tempVertixIndex);
				tempVertixIndex.clear();
				break;
			default:
				input>>buf;
				break;
			}
			break;
		default:
			input>>buf;
			break;
		}
		if(input.eof())
			break;
	}
	return 1;
}

void initialzeHalfEdge()
{
	for(vector<Point>::iterator ptr=objVertix.begin(); ptr!=objVertix.end(); ++ptr)
	{
		Vertex* tempVertex = new Vertex();
		tempVertex->point.x = ptr->x;
		tempVertex->point.y = ptr->y;
		tempVertex->point.z = ptr->z;
		tempVertex->edge = NULL;
		vertices.push_back(tempVertex);
	}
	int tempFaceIndex = 0;

	for(vector<vector<int>>::iterator ptr1=objVertixIndex.begin(); ptr1!=objVertixIndex.end(); ++ptr1)
	{
		int tempHalfEdgeIndex = 0;
		Face *tempFace = new Face();

		vector<int>::iterator ptr2 = ptr1->begin();
		HalfEdge *firstEdge = new HalfEdge();

		firstEdge->vert = vertices[(*ptr2)-1];
		firstEdge->face = tempFace;
		firstEdge->pair = NULL;
		for(vector<Face*>::iterator facePtr=faces.begin(); facePtr!=faces.end(); ++facePtr)
		{
			HalfEdge *findPair = (*facePtr)->edge;
			do 
			{
				if((findPair->vert==vertices[(*ptr2)-1])&&(findPair->next->vert==vertices[ptr1->back()-1]))
				{
					findPair->next->pair= firstEdge;
					firstEdge->pair = findPair->next;
					break;
				}
				findPair = findPair->next;
			}while((findPair!=(*facePtr)->edge)&& findPair!=NULL);
			if(firstEdge->pair!=NULL)
				break;
		}

		vertices[(*(ptr1->end()-1))-1]->edge = firstEdge;
		//计算边的中点
		Point tempMidEdgePoint;
		tempMidEdgePoint.x = (vertices[(*ptr2)-1]->point.x + vertices[ptr1->back()-1]->point.x) / 2.0f;
		tempMidEdgePoint.y = (vertices[(*ptr2)-1]->point.y + vertices[ptr1->back()-1]->point.y) / 2.0f;
		tempMidEdgePoint.z = (vertices[(*ptr2)-1]->point.z + vertices[ptr1->back()-1]->point.z) / 2.0f;
		firstEdge->midEdgePoint = tempMidEdgePoint;

		firstEdge->index = tempHalfEdgeIndex;
		tempHalfEdgeIndex++;
		edges.push_back(firstEdge);
		HalfEdge *prevEdge = firstEdge;

		for(;(ptr2+1)!=ptr1->end(); ++ptr2)
		{
			HalfEdge *tempHalfEdge = new HalfEdge();
			tempHalfEdge->vert = vertices[(*(ptr2+1))-1];	
			prevEdge->next = tempHalfEdge;
			vertices[(*ptr2)-1]->edge = tempHalfEdge;
			tempHalfEdge->face = tempFace;

			if((ptr2+2)==ptr1->end())  //如果是最后一个，就设置next为第一个
				tempHalfEdge->next = firstEdge;

			tempHalfEdge->pair = NULL;
			for(vector<Face*>::iterator facePtr=faces.begin(); facePtr!=faces.end(); ++facePtr)
			{
				HalfEdge *findPair = (*facePtr)->edge;
				do 
				{
					if((findPair->vert==vertices[(*(ptr2+1))-1])&&(findPair->next->vert==vertices[(*ptr2)-1]))
					{
						findPair->next->pair = tempHalfEdge;
						tempHalfEdge->pair = findPair->next;
						break;
					}
					findPair = findPair->next;
				} while ((findPair!=(*facePtr)->edge)&&findPair!=NULL);
				if(tempHalfEdge->pair!=NULL)
						break;
			}
			prevEdge = tempHalfEdge;

			//计算中点
			tempMidEdgePoint.x = (vertices[(*(ptr2+1))-1]->point.x + vertices[(*ptr2)-1]->point.x) / 2.0f;
			tempMidEdgePoint.y = (vertices[(*(ptr2+1))-1]->point.y + vertices[(*ptr2)-1]->point.y) / 2.0f;
			tempMidEdgePoint.z = (vertices[(*(ptr2+1))-1]->point.z + vertices[(*ptr2)-1]->point.z) / 2.0f;
			tempHalfEdge->midEdgePoint = tempMidEdgePoint;
			tempHalfEdge->index = tempHalfEdgeIndex;
			tempHalfEdgeIndex++;
			edges.push_back(tempHalfEdge);
		}
		tempFace->edge = firstEdge;
		
		//计算平面重心点
		HalfEdge *caculMid = tempFace->edge;
		Point tempMidFacePoint;
		tempMidFacePoint.x = 0;
		tempMidFacePoint.y = 0;
		tempMidFacePoint.z = 0;
		int tempCount = 0;
		do 
		{
			tempMidFacePoint.x += caculMid->vert->point.x;
			tempMidFacePoint.y += caculMid->vert->point.y;
			tempMidFacePoint.z += caculMid->vert->point.z;
			tempCount++;
			caculMid = caculMid->next;	
		}while(caculMid!=tempFace->edge);
		tempMidFacePoint.x /= tempCount;
		tempMidFacePoint.y /= tempCount;
		tempMidFacePoint.z /= tempCount;
		tempFace->midFacePoint = tempMidFacePoint;
		tempFace->index = tempFaceIndex;
		tempFaceIndex++;
		faces.push_back(tempFace);
	}
}

void dooSabin()
{
	int **table = (int**)malloc(sizeof(int*)*faces.size());
	bool **isEdgePair = (bool**)malloc(sizeof(bool*)*faces.size());
	for(int i=0; i<faces.size(); ++i)
	{
		table[i] = (int*)malloc(sizeof(int)*20);  //I cann't know how much edges in a face , so I malloc 20; it is enough!
		isEdgePair[i] = (bool*)malloc(sizeof(bool)*20);
	}
	for(int i=0; i<faces.size(); ++i)
	{
		for(int j=0; j<20; ++j)
		{
			table[i][j] = -1;
			isEdgePair[i][j] = false;
		}
	}
	objVertix.clear();
	objVertixIndex.clear();

	//for face-faces and initialize table
	int indexPoint = 1;
	for(vector<Face*>::iterator ptrF=faces.begin(); ptrF!=faces.end(); ++ptrF)
	{
		HalfEdge* tempHalfEdge = (*ptrF)->edge;
		vector<int> subVerticesIndex;
		do 
		{	
			Point subPoint;
			subPoint.x = (tempHalfEdge->midEdgePoint.x + tempHalfEdge->next->midEdgePoint.x + (*ptrF)->midFacePoint.x + tempHalfEdge->vert->point.x) / 4;
			subPoint.y = (tempHalfEdge->midEdgePoint.y + tempHalfEdge->next->midEdgePoint.y + (*ptrF)->midFacePoint.y + tempHalfEdge->vert->point.y) / 4;
			subPoint.z = (tempHalfEdge->midEdgePoint.z + tempHalfEdge->next->midEdgePoint.z + (*ptrF)->midFacePoint.z + tempHalfEdge->vert->point.z) / 4;
			objVertix.push_back(subPoint);
			table[(*ptrF)->index][tempHalfEdge->next->index] = indexPoint;
			subVerticesIndex.push_back(indexPoint);
			indexPoint++;
			tempHalfEdge = tempHalfEdge->next;
		}while(tempHalfEdge!=(*ptrF)->edge);
		objVertixIndex.push_back(subVerticesIndex);
	}
	//for vertices-face
	for(vector<Vertex*>::iterator ptrV=vertices.begin(); ptrV!=vertices.end(); ++ptrV)
	{
		HalfEdge *tempHalfEdge = (*ptrV)->edge;
		vector<int> subVerticesIndex1;
		do 
		{
			int tempIndexPoint;
			tempIndexPoint = table[tempHalfEdge->face->index][tempHalfEdge->index];
			subVerticesIndex1.push_back(tempIndexPoint);
			tempHalfEdge = tempHalfEdge->pair->next;
		}while(tempHalfEdge!=(*ptrV)->edge);
		//reverse
		vector<int> subVerticesIndex2;
		for(vector<int>::reverse_iterator ptrSubV=subVerticesIndex1.rbegin(); ptrSubV!=subVerticesIndex1.rend(); ++ptrSubV)
			subVerticesIndex2.push_back((*ptrSubV));
		objVertixIndex.push_back(subVerticesIndex2);
	}
	//for edge-faces
	for(vector<HalfEdge*>::iterator ptrE=edges.begin(); ptrE!=edges.end(); ++ptrE)
	{
		HalfEdge *tempHalfEdge = (*ptrE);
		if(!isEdgePair[tempHalfEdge->face->index][tempHalfEdge->index])
		{
			isEdgePair[tempHalfEdge->face->index][tempHalfEdge->index] = true;
			vector<int> subVerticesIndex;
			int tempIndexPoint1;
			int tempIndexPoint2;
			int tempIndexPoint3;
			int tempIndexPoint4;
			tempIndexPoint1 = table[tempHalfEdge->face->index][tempHalfEdge->index];
			tempIndexPoint2 = table[tempHalfEdge->next->face->index][tempHalfEdge->next->index];
			HalfEdge *tempHalfEdgePair = tempHalfEdge->pair;
			isEdgePair[tempHalfEdgePair->face->index][tempHalfEdgePair->index] = true;
			tempIndexPoint3 = table[tempHalfEdgePair->face->index][tempHalfEdgePair->index];
			tempIndexPoint4 = table[tempHalfEdgePair->next->face->index][tempHalfEdgePair->next->index];
			subVerticesIndex.push_back(tempIndexPoint1);
			subVerticesIndex.push_back(tempIndexPoint4);
			subVerticesIndex.push_back(tempIndexPoint3);
			subVerticesIndex.push_back(tempIndexPoint2);

			objVertixIndex.push_back(subVerticesIndex);
		}
	}
	for(int i=0; i<faces.size(); ++i)
	{
		free(table[i]);
		free(isEdgePair[i]);
	}
	free(table);
	free(isEdgePair);

	edges.clear();
	faces.clear();
	vertices.clear();
}

void init(void)
{
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {50.0};
	GLfloat light_position[] ={1.5,1.5,1.5,0};
	GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat lmodel_ambient[] = {0.9, 0.9, 0.9, 1.0};
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	for(vector<vector<int>>::iterator ptr1=objVertixIndex.begin(); ptr1!=objVertixIndex.end(); ++ptr1)
	{
		vector<int>::iterator ptr2 = (*ptr1).begin();
		//normal
		Point tempVector[3];
		tempVector[0].x = objVertix[(*(ptr2+1))-1].x - objVertix[(*(ptr2))-1].x;
		tempVector[0].y = objVertix[(*(ptr2+1))-1].y - objVertix[(*(ptr2))-1].y;
		tempVector[0].z = objVertix[(*(ptr2+1))-1].z - objVertix[(*(ptr2))-1].z;
		tempVector[1].x = objVertix[(*(ptr2+2))-1].x - objVertix[(*(ptr2))-1].x;
		tempVector[1].y = objVertix[(*(ptr2+2))-1].y - objVertix[(*(ptr2))-1].y;
		tempVector[1].z = objVertix[(*(ptr2+2))-1].z - objVertix[(*(ptr2))-1].z;
		tempVector[2].x=tempVector[0].y*tempVector[1].z-tempVector[0].z*tempVector[1].y;
		tempVector[2].y=tempVector[0].z*tempVector[1].x-tempVector[0].x*tempVector[1].z;
		tempVector[2].z=tempVector[0].x*tempVector[1].y-tempVector[0].y*tempVector[1].x;
		double lenght = sqrt(tempVector[2].x*tempVector[2].x+tempVector[2].y*tempVector[2].y+tempVector[2].z*tempVector[2].z);
		tempVector[2].x /= lenght;
		tempVector[2].y /= lenght;
		tempVector[2].z /= lenght;
		glBegin(GL_POLYGON);
			glNormal3f(tempVector[2].x, tempVector[2].y, tempVector[2].z);
			for(;ptr2!=(*ptr1).end(); ++ptr2)
			{
				glVertex3f(objVertix[(*ptr2)-1].x, objVertix[(*ptr2)-1].y, objVertix[(*ptr2)-1].z);
			}
		glEnd();
	}
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(2.5, 2.5, 2.5, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
}

void idle()
{
	if(loop < 10)
	{
		clock_t start = clock();
		initialzeHalfEdge();
		dooSabin();
		printf("loop %d took %ld millisecond\n",loop,(clock()-start));
		loop++;
		Sleep(1000);
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	readObj("cube.obj");

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500,500);
	glutInitWindowPosition(100,100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutMainLoop();

	return 0;
}