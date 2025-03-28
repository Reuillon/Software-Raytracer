//C Libraries
#include <ctime>
#include <math.h>
#include <stdio.h>

//C++ Libraries
#include <thread>

//External Libraries
#include "GL/gl.h"
#include "GL/glut.h"
#include <FreeImage.h>

//NUMBER OF BOUNCES FOR EACH RAY
int MAX_BOUNCES = 4;

//WINDOW DIMENSIONS
int width;
int height;
int processor_count;

//PIXEL RGB STRUCTURE
typedef struct pixel
{
	GLfloat r, g, b;
} pixel;

//GENERIC VECTOR STRUCTURE
typedef struct vector
{
	float x;
	float y;
	float z;
}vec;

//MATERIAL STRUCTURE
typedef struct mat
{
	float diffuse;
	float ambient;
	float specular;
	int fCo;
	int ref;
}mat;

//TRIANGLE STRUCTURE
typedef struct tri
{
	float p1[3];
	float p2[3];
	float p3[3];

	float x, y, z;

	float r, g, b;
}tri;

//LIGHT STRUCTURE
typedef struct light
{
	float x;
	float y;
	float z;
	
	
	float ambient;
	float intensity;
}light;

//CAMERA DATA STRUCTURE
typedef struct camera
{
	//CAMERA POSITION
	float x;
	float y;
	float z;

	//CAMERA DISPLACEMENT
	float dx;
	float dy;
	float dz;

	//BACKGROUND COLOR
	float r;
	float g;
	float b;

	//CAMERA SPEED
	float speed;

}camera;

//RAY DATA STRUCTURE
typedef struct ray
{

	//RAY ORIGIN
	float ox;
	float oy;
	float oz;

	//RAY VECTOR
	float x;
	float y;
	float z;

	//DIRECTION VECTOR
	float dx;
	float dy;
	float dz;


}ray;

//PLANE DATA STRUCTURE
typedef struct plane
{
	//PLANE POSITION
	float x;
	float y;
	float z;

	 

	//PLANE DIMENSIONS
	float A;
	float B;
	float C;
	float D;

	//COLOR FIELD
	float r;
	float g;
	float b;

	//MATERIAL OF OBJECT
	int material;
}plane;

//SPHERE DATA STRUCTURE
typedef struct sphere
{
	//SPHERE POSITION
	float x;
	float y;
	float z;

	//SPHERE RADIUS
	int radius;

	//COLOR FIELD
	float r;
	float g;
	float b;

	//MATERIAL OF OBJECT
	int material;
}sphere;

//OBJECT TYPES
ray* r;
camera* c;
sphere* s;
plane* p;
tri* t;
mat* m;
light* l;

//OBJECT QUANTITIES
short int sphereCount = 0;
short int planeCount = 0;
short int triCount = 0;
short int matCount = 0;
short int lightCount = 0;

//TEMPORARY DATA STRUCTURES TO ALLOW FOR RESIZABLE STRUCTURE ARRAY
sphere* sT;
plane* pT;
tri* tT;
mat* mT;
light* lT;
pixel* q;
pixel* o;

//Hit VALUE FOR EACH PIXEL ON SCREEN
int* hit;
int* cl;
float* tI;

//GENERAL LIGHT RAY VECTORS
vec* refVec;
vec* rVec;
vec* hVec;

//GETS PIXEL X AND Y AND CONVERTS INTO CORRESPONDING 1D ARRAY VALUE (USED FOR BETTER READABILITY)
int getPixel(int i, int j)
{
	return (i + (j * height));
}

//GETS PIXEL FROM THE SCENE USING glReadPixels AND CREATES AND SAVES IMAGE USING FREE IMAGE
void write_img(char* fileName)
{
	FIBITMAP* output;
	RGBQUAD imDat;

	output = FreeImage_Allocate(width, height, 24, 0, 0, 0);

	if (!output)
	{
		perror("ERROR WRITING TO FILE!");
		exit(1);
	}

	for (unsigned int i = 0; i < height; i++)
	{
		for (unsigned int j = 0; j < width; j++)
		{
			glReadPixels(j, i, 1, 1, GL_RGB, GL_FLOAT, &q[getPixel(i, j)]);
			imDat.rgbRed = (GLubyte)((q[getPixel(i, j)].r) * 255);
			imDat.rgbGreen = (GLubyte)((q[getPixel(i, j)].g) * 255);
			imDat.rgbBlue = (GLubyte)((q[getPixel(i, j)].b) * 255);
			
			FreeImage_SetPixelColor(output, j, i, &imDat);

		}
		if (i % 10 == 0)
		{
			printf("%d%%...\n", (int)(((float)i/ (float)height) * 100));
		}
	}

	printf("%d%%\n", (int)(((float)height / (float)height) * 100));
	printf("\n");
	if (!FreeImage_Save(FIF_TIFF, output, fileName, 0))
	{
		perror("ERROR COULD NOT COMPLETE FILE WRITE!");
		exit(1);
	}

	FreeImage_Unload(output);
}



//FUNCTION FOR EXPONENT OF 2
float square(float val)
{
	return pow(val, 2);
}

//SETS UP ENVIRONMENT BASED ON 'scene.txt'
void setupScene()
{
	processor_count = std::thread::hardware_concurrency();
	FILE* file = fopen("scene.txt", "r");
	if (file == NULL)
	{
		printf("ERROR COULD NOT CREATE FILE");
		exit(1);
	}

	char obj = 0;
	float dat;
	int mater;

	
	//READS FILE UNTIL THE END
	while (fscanf(file, "%c", &obj) != EOF)
	{

		switch (obj)
		{

			//ADDS SPHERE FROM TEXT FILE WITH ITS PROPERTIES
			case('s'):
			case('S'):
			{
				printf("SPHERE ADDED\n");
				//IF ANOTHER OBJECT IS FOUND THEN QUANTITY OF OBJECT INCREASES BY 1
				sphereCount += 1;
				free(s);

				//CREATES AN OBJECT ARRAY BASED ON AMOUNT OF OBJECTS FOUND IN FILE
				s = (struct sphere*)malloc((sphereCount * sizeof(struct sphere)));
				for (int i = 0; i < sphereCount; i++)
				{
					//RE ASSIGNS ARRAY WITH VALUES OF TEMP ARRAY
					if (i < sphereCount - 1)
					{
						s[i].x = sT[i].x;
						s[i].y = sT[i].y;
						s[i].z = sT[i].z;
						s[i].radius = sT[i].radius;
						s[i].r = sT[i].r;
						s[i].g = sT[i].g;
						s[i].b = sT[i].b;
						s[i].material = sT[i].material;
					}
					//ADDS NEW OBJECT STRUCTURE AND ASSIGNS OBJECT PROPERTIES
					else
					{
						fscanf(file, "%f", &dat);
						s[i].x = dat;
						fscanf(file, "%f", &dat);
						s[i].y = dat;
						fscanf(file, "%f", &dat);
						s[i].z = dat;
						fscanf(file, "%f", &dat);
						s[i].radius = square(dat);
						fscanf(file, "%f", &dat);
						s[i].r = dat;
						fscanf(file, "%f", &dat);
						s[i].g = dat;
						fscanf(file, "%f", &dat);
						s[i].b = dat;
						fscanf(file, "%d", &mater);
						s[i].material = mater;
					}
				}

				//CREATE ARRAY OF STRUCTS BASED ON ORIGINAL STUCT ARRAY AND COPIES ALL DATA FROM CURRENT ARRAY TO TEMP ARRAY
				free(sT);
				sT = (struct sphere*)malloc((sphereCount * sizeof(struct sphere)));
				for (int i = 0; i < sphereCount; i++)
				{
					sT[i].x = s[i].x;
					sT[i].y = s[i].y;
					sT[i].z = s[i].z;
					sT[i].radius = s[i].radius;
					sT[i].r = s[i].r;
					sT[i].g = s[i].g;
					sT[i].b = s[i].b;
					sT[i].material = s[i].material;
				}
	
	
		
				break;
			}
			case('m'):
			case('M'):
			{
				printf("MATERIAL ADDED\n");
				//IF ANOTHER OBJECT IS FOUND THEN QUANTITY OF OBJECT INCREASES BY 1
				matCount += 1;
				free(m);
	
				//CREATES AN OBJECT ARRAY BASED ON AMOUNT OF OBJECTS FOUND IN FILE
				m = (struct mat*)malloc((matCount * sizeof(struct mat)));
				for (int i = 0; i < matCount; i++)
				{
					//RE ASSIGNS ARRAY WITH VALUES OF TEMP ARRAY
					if (i < matCount - 1)
					{
						m[i].ambient = mT[i].ambient;
						m[i].diffuse = mT[i].diffuse;
						m[i].specular = mT[i].specular;
						m[i].fCo = mT[i].fCo;
						m[i].ref = mT[i].ref;
					}
					//ADDS NEW OBJECT STRUCTURE AND ASSIGNS OBJECT PROPERTIES
					else
					{
						fscanf(file, "%f", &dat);
						m[i].ambient = dat;
						fscanf(file, "%f", &dat);
						m[i].diffuse = dat;
						fscanf(file, "%f", &dat);
						m[i].specular = dat;
						fscanf(file, "%d", &mater);
						m[i].fCo = mater;
						fscanf(file, "%d", &mater);
						m[i].ref = mater;

					}
				}
	
				//CREATE ARRAY OF STRUCTS BASED ON ORIGINAL STUCT ARRAY AND COPIES ALL DATA FROM CURRENT ARRAY TO TEMP ARRAY
				free(mT);
				mT = (struct mat*)malloc((matCount * sizeof(struct mat)));
				for (int i = 0; i < matCount; i++)
				{
					mT[i].ambient = m[i].ambient;
					mT[i].diffuse = m[i].diffuse;
					mT[i].specular = m[i].specular;
					mT[i].fCo = m[i].fCo;
					mT[i].ref = m[i].ref;
				}
				
	

				break;
			}
			case 't':
			case 'T':
			{
				printf("TRIANGLE ADDED\n");
				//IF ANOTHER OBJECT IS FOUND THEN QUANTITY OF OBJECT INCREASES BY 1
				triCount += 1;
				free(t);
	
				//CREATES AN OBJECT ARRAY BASED ON AMOUNT OF OBJECTS FOUND IN FILE
				t = (struct tri*)malloc((triCount * sizeof(struct tri)));
				for (int i = 0; i < triCount; i++)
				{
					//RE ASSIGNS ARRAY WITH VALUES OF TEMP ARRAY
					if (i < triCount - 1)
					{
						t[i].x = tT[i].x;
						t[i].y = tT[i].y;
						t[i].z = tT[i].z;
	
						t[i].r = tT[i].r;
						t[i].g = tT[i].g;
						t[i].b = tT[i].b;
	
						for (int k = 0; k < 3; k++)
						{
							t[i].p1[k] = tT[i].p1[k];
							t[i].p2[k] = tT[i].p2[k];
							t[i].p3[k] = tT[i].p3[k];
						}
	
					}
					//ADDS NEW OBJECT STRUCTURE AND ASSIGNS OBJECT PROPERTIES
					else
					{
						fscanf(file, "%f", &dat);
						t[i].x = dat;
						fscanf(file, "%f", &dat);
						t[i].y = dat;
						fscanf(file, "%f", &dat);
						t[i].z = dat;
						fscanf(file, "%f", &dat);
						t[i].r = dat;
						fscanf(file, "%f", &dat);
						t[i].g = dat;
						fscanf(file, "%f", &dat);
						t[i].b = dat;
	
						for (int k = 0; k < 3; k++)
						{
							fscanf(file, "%c", &obj);
							fscanf(file, "%f", &dat);
							t[i].p1[k] = dat;
							fscanf(file, "%f", &dat);
							t[i].p2[k] = dat;
							fscanf(file, "%f", &dat);
							t[i].p3[k] = dat;
						}
					}
				}
	
				//CREATE ARRAY OF STRUCTS BASED ON ORIGINAL STUCT ARRAY AND COPIES ALL DATA FROM CURRENT ARRAY TO TEMP ARRAY
				free(tT);
				tT = (struct tri*)malloc((triCount * sizeof(struct tri)));
				for (int i = 0; i < triCount; i++)
				{
					tT[i].x = t[i].x;
					tT[i].y = t[i].y;
					tT[i].z = t[i].z;

					tT[i].r = t[i].r;
					tT[i].g = t[i].g;
					tT[i].b = t[i].b;
					
					for (int k = 0; k < 3; k++)
					{
						tT[i].p1[k] = t[i].p1[k];
						tT[i].p2[k] = t[i].p2[k];
						tT[i].p3[k] = t[i].p3[k];
					}
				}
					


				break;
			}
			//ADDS PLANE FROM TEXTFILE WITH ITS PROPERTIES
			case('p'):
			case('P'):
			{
				printf("PLANE ADDED\n");
				//IF ANOTHER OBJECT IS FOUND THEN QUANTITY OF OBJECT INCREASES BY 1
				planeCount += 1;
				free(p);
					
				//CREATES AN OBJECT ARRAY BASED ON AMOUNT OF OBJECTS FOUND IN FILE
				p = (struct plane*)malloc((planeCount * sizeof(struct plane)));
				for (int i = 0; i < planeCount; i++)
				{
					//RE ASSIGNS ARRAY WITH VALUES OF TEMP ARRAY
					if (i < planeCount - 1)
					{
						p[i].x = pT[i].x;
						p[i].y = pT[i].y;
						p[i].z = pT[i].z;
							
						
						p[i].A = pT[i].A;
						p[i].B = pT[i].B;
						p[i].C = pT[i].C;
						p[i].D = pT[i].D;
	
						p[i].r = pT[i].r;
						p[i].g = pT[i].g;
						p[i].b = pT[i].b;

						p[i].material = pT[i].material;
					}
					//ADDS NEW OBJECT STRUCTURE AND ASSIGNS OBJECT PROPERTIES
					else
					{
						fscanf(file, "%f", &dat);
						p[i].x = dat;
						fscanf(file, "%f", &dat);
						p[i].y = dat;
						fscanf(file, "%f", &dat);
						p[i].z = dat;
						fscanf(file, "%f", &dat);
						p[i].A = dat;
						fscanf(file, "%f", &dat);
						p[i].B = dat;
						fscanf(file, "%f", &dat);
						p[i].C = dat;
						fscanf(file, "%f", &dat);
						p[i].D = dat;
						fscanf(file, "%f", &dat);
						p[i].r = dat;
						fscanf(file, "%f", &dat);
						p[i].g = dat;
						fscanf(file, "%f", &dat);
						p[i].b = dat;
						fscanf(file, "%d", &mater);
						p[i].material = mater;
					}
				}
	
				//CREATE ARRAY OF STRUCTS BASED ON ORIGINAL STUCT ARRAY AND COPIES ALL DATA FROM CURRENT ARRAY TO TEMP ARRAY
				free(pT);
				pT = (struct plane*)malloc((planeCount * sizeof(struct plane)));
				for (int i = 0; i < planeCount; i++)
				{
					pT[i].x = p[i].x;
					pT[i].y = p[i].y;
					pT[i].z = p[i].z;
					pT[i].r = p[i].r;
					pT[i].g = p[i].g;
					pT[i].b = p[i].b;
					pT[i].material = p[i].material;
				}
				break;
			}
	
			//ADDS CAMERA OBJECT TO SCENE
			case('c'):
			case('C'):
			{
				printf("CAMERA ADDED\n");
				c = (struct camera*)malloc((sizeof(struct camera)));
				fscanf(file, "%f", &dat);
				width = dat;
				
				c->x = width / 2;
				fscanf(file, "%f", &dat);
				height = dat;
				c->y = height / 2;
				fscanf(file, "%f", &dat);
				c->z = dat;
				fscanf(file, "%f", &dat);
				c->dx = dat - (width / 2);
				fscanf(file, "%f", &dat);
				c->dy = dat - (height / 2);
				fscanf(file, "%f", &dat);
				c->dz = dat;
				fscanf(file, "%f", &dat);	
				c->speed = dat;
				fscanf(file, "%f", &dat);
				c->r = dat;
				fscanf(file, "%f", &dat);
				c->g = dat;
				fscanf(file, "%f", &dat);
				c->b = dat;
				r = (struct ray*)malloc((width * height) * (sizeof(struct ray)));
				o = (struct pixel*)malloc((width * sizeof(struct pixel)) * (height * sizeof(struct pixel)));
				break;
			}
	
			case 'l':
			case 'L':
			{
				printf("LIGHT ADDED\n");
				//IF ANOTHER OBJECT IS FOUND THEN QUANTITY OF OBJECT INCREASES BY 1
				lightCount += 1;
				free(l);

				//CREATES AN OBJECT ARRAY BASED ON AMOUNT OF OBJECTS FOUND IN FILE
				l  = (struct light*)malloc((lightCount * sizeof(struct light)));
				for (int i = 0; i < lightCount; i++)
				{
					//RE ASSIGNS ARRAY WITH VALUES OF TEMP ARRAY
					if (i < lightCount - 1)
					{
						l[i].ambient = lT[i].ambient;
						l[i].intensity = lT[i].intensity;
						l[i].x = lT[i].x;
						l[i].y = lT[i].y;
						l[i].z = lT[i].z;
					}
					//ADDS NEW OBJECT STRUCTURE AND ASSIGNS OBJECT PROPERTIES
					else
					{
						fscanf(file, "%f", &dat);
						l[i].ambient = dat;
						fscanf(file, "%f", &dat);
						l[i]. intensity = dat;
						fscanf(file, "%f", &dat);
						l[i].x = dat;
						fscanf(file, "%f", &dat);
						l[i].y = dat;
						fscanf(file, "%f", &dat);
						l[i].z = dat;

					}
				}

				//CREATE ARRAY OF STRUCTS BASED ON ORIGINAL STUCT ARRAY AND COPIES ALL DATA FROM CURRENT ARRAY TO TEMP ARRAY
				free(lT);
				lT = (struct light*)malloc((lightCount * sizeof(struct light)));
				for (int i = 0; i < lightCount; i++)
				{
					lT[i].ambient = m[i].ambient;
					lT[i].intensity = l[i].intensity;
					lT[i].x = l[i].x;
					lT[i].y = l[i].y;
					lT[i].z = l[i].z;
				}

				float lTEST = pow(l[0].x, 2) + pow(l[0].y, 2) + pow(l[0].z, 2);
				lTEST = sqrt(lTEST);
				
				l[0].x /= lTEST;
				l[0].y /= lTEST;
				l[0].z /= lTEST;
				



				break;
			}
		
			//ALLOWS FOR COMMENTING IN SCENE FILE
			case'/':
			{
				fscanf(file, "%c", &obj);
				while (obj != '/')
				{
					fscanf(file, "%c", &obj);
				}
				break;
			}	
			case('\n'):
			{
				break;
			}
			default:
				printf("PARSING ERROR UNKNOWN OBJECT TYPE\n");
				break;
		}
	}
	hit = (int *)malloc(width * height * sizeof(int));
	cl = (int *)malloc(width * height * sizeof(int));
	tI = (float *)malloc(width * height * sizeof(float));
	hVec = (struct vector*)malloc((width * height * sizeof(struct vector)));
	rVec = (struct vector*)malloc((width * height * sizeof(struct vector)));
	refVec = (struct vector*)malloc((width * height * sizeof(struct vector)));
	q = (struct pixel*)malloc((width * height * sizeof(struct pixel)));
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			hit[getPixel(i, j)] = 0;		
			cl[getPixel(i, j)] = -1;		
			tI[getPixel(i, j)] = 0;		
		}
	}
	
	printf("SCENE LOADED!\n");
	printf("PRESS H FOR CONTROLS\n");
}








//CHECKS FOR RAY COLLISION WITH SPHERE
float sphereCollide(vec oRay, vec norRay, int o)
{
	
	float B;
	float C;
	float dx, dy, dz;
	float disc;
	float d2;
	float t0;
	float t1;
	dx = (oRay.x - s[o].x), dy = (oRay.y - s[o].y), dz = (oRay.z - s[o].z);

	//CALCULATES B AND C TO SOLVE QUADRATIC EQUATION SINCE A IS ALWAYS 1 IT IS IGNORED IN CALCULATION
	B = 2 * (((norRay.x) * (dx)) + (norRay.y * (dy)) + (norRay.z * (dz)));
	C = (square(dx) + square(dy) + square(dz) - s[o].radius);

	//DISCRIMINANT
	disc = square(B) - (4 * C);
	d2 = sqrt(disc);
	
	//FINDS COLLISION
	if (disc >= 0)
	{
		t0 = ((-B) - d2) / 2;
		if (t0 > 0)
		{
			return t0;
		}
		else
		{
			t1 = ((-B) + d2) / 2;
			if (t1 > 0)
			{
				return t1;
			}
		}
	}
	return -1;
}

//PLANE COLLISION FUNCTION RETURNS THE TIME POINT t OF WHEN THE VECTOR HITS
float planeCollide(vec oRay, vec refRay, int o2)
{
	float ta, tb, tc;
	float po, pd;
	float normal;
	//NORMALIZE 
	normal = pow(p[o2].A, 2) + pow(p[o2].B, 2) + pow(p[o2].C, 2);
	normal = sqrt(normal);
	ta = p[o2].A / normal;
	tb = p[o2].B / normal;
	tc = p[o2].C / normal;

	
	
	po = (ta * oRay.x) + (tb * oRay.y) + (tc * oRay.z) + (p[o2].D);
	pd = (ta * refRay.x) + (tb * refRay.y) + (tc * refRay.z) + (p[o2].D);
	if (pd == 0)
	{
		return 0;
	}
	else
	{
		return (((-1) * (po + p[o2].D)) / pd);
	}
	
}


float triCollide(int i, int j, int o3)
{
	float ta, tb, tc;
	float po, pd;
	float normal;
	vec v1, v2;
	float to;
	float td;
	t[getPixel(i, j)].p1[o3];
	v1.x = t[o3].p2[0] - t[o3].p1[0];
	v1.y = t[o3].p2[1] - t[o3].p1[1];
	v1.z = t[o3].p2[2] - t[o3].p1[2];
	v2.x = t[o3].p3[0] - t[o3].p1[0];
	v2.y = t[o3].p3[1] - t[o3].p1[1];
	v2.z = t[o3].p3[2] - t[o3].p1[2];
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;


	
	v1.x = v1.x / normal;
	v1.y = v1.y / normal;
	v1.z = v1.z / normal;



	to = (v1.x * r[getPixel(i, j)].ox) + (v1.y * r[getPixel(i, j)].oy) + (v1.z * r[getPixel(i, j)].oz) + 0;
	td = (v1.x * r[getPixel(i, j)].dx) + (v1.y * r[getPixel(i, j)].dy) + (v1.z * r[getPixel(i, j)].dz) + 0;



	if (td == 0)
	{
		return 0;
	}
	else
	{
		vec v;
		if (to >= 0 && to <= 1 && tb <= 1 && tb >= 0)
		{
			v.x = to * (t[o3].p2[0] - t[o3].p1[0]);
			v.y = to * (t[o3].p2[1] - t[o3].p1[1]);
			v.z = to * (t[o3].p2[2] - t[o3].p1[2]);
			v.x += td * (t[o3].p3[0] - t[o3].p1[0]);
			v.y += td * (t[o3].p3[1] - t[o3].p1[1]);
			v.z += td * (t[o3].p3[2] - t[o3].p1[2]);


			return (((-1) * (to)) / td);

			
		}
		
	}
	return 0.0f;

}














vec normalize(vec vector)
{
	float temp;
	temp = pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2);
	temp = sqrt(temp);
	vector.x /= temp;
	vector.y /= temp;
	vector.z /= temp;
	return vector;
}

void materialSphereCALC(vec oRay, vec ray, int k, int bounces, char obj, float collide, float ambient, float diffuse, float specular, float f, int i, int j)
{
	float c0;
	float dot;
	//HITSPOT VECTOR
	hVec[getPixel(i, j)].x = (ray.x * collide + oRay.x);
	hVec[getPixel(i, j)].y = (ray.y * collide + oRay.y);
	hVec[getPixel(i, j)].z = (ray.z * collide + oRay.z);

	//NORMAL OF HITSPOT OF THE SPHERE
	if (obj == 's')
	{
		rVec[getPixel(i, j)].x = ((hVec[getPixel(i, j)].x) - s[k].x) / s[k].radius;
		rVec[getPixel(i, j)].y = ((hVec[getPixel(i, j)].y) - s[k].y) / s[k].radius;
		rVec[getPixel(i, j)].z = ((hVec[getPixel(i, j)].z) - s[k].z) / s[k].radius;
	}
	if (obj == 'p')
	{
		rVec[getPixel(i, j)].x = ((hVec[getPixel(i, j)].x));
		rVec[getPixel(i, j)].y = -((hVec[getPixel(i, j)].y));
		rVec[getPixel(i, j)].z = ((hVec[getPixel(i, j)].z));
	}
	//NORMALIZE
	rVec[getPixel(i, j)] = normalize(rVec[getPixel(i, j)]);


	//REFLECTION COSINE AND RAY NORMAL VECTOR
	dot = (rVec[getPixel(i, j)].x * ray.x) + (rVec[getPixel(i, j)].y * ray.y) + (rVec[getPixel(i, j)].z * ray.z);
	refVec[getPixel(i, j)].x = (ray.x) - 2 * (dot)*rVec[getPixel(i, j)].x;
	refVec[getPixel(i, j)].y = (ray.y) - 2 * (dot)*rVec[getPixel(i, j)].y;
	refVec[getPixel(i, j)].z = (ray.z) - 2 * (dot)*rVec[getPixel(i, j)].z;

	//NORMALIZE
	refVec[getPixel(i, j)] = normalize(refVec[getPixel(i, j)]);


	if (m[s[k].material - 1].diffuse != 0)
	{
		//DIFFUSE COSINE
		c0 = (rVec[getPixel(i, j)].x * l[0].x) + (rVec[getPixel(i, j)].y * l[0].y) + (rVec[getPixel(i, j)].z * l[0].z);
		diffuse = m[s[k].material - 1].diffuse * l[0].intensity * c0;
	}

	if (m[s[k].material - 1].specular != 0)
	{
		//SPECULAR COSINE CALCULATION
		f = m[s[k].material - 1].fCo;
		c0 = pow((refVec[getPixel(i, j)].x * ray.x) + (refVec[getPixel(i, j)].y * ray.y) + (refVec[getPixel(i, j)].z * ray.z), f);
		specular = m[s[k].material - 1].specular * l[0].intensity * c0;
	}

	//COLOR CALCULATION
	cl[getPixel(i, j)] = k;
	float mat = diffuse + ambient + specular;


	
	if (bounces != MAX_BOUNCES)
	{
		q[getPixel(i, j)].r += (s[k].r * (mat) * 0.5);
		q[getPixel(i, j)].g += (s[k].g * (mat) * 0.5);
		q[getPixel(i, j)].b += (s[k].b * (mat) * 0.5);
	}
	else
	{
		q[getPixel(i, j)].r = (s[k].r * (mat) * 0.9);
		q[getPixel(i, j)].g = (s[k].g * (mat) * 0.9);
		q[getPixel(i, j)].b = (s[k].b * (mat) * 0.9);
	}


	tI[getPixel(i, j)] = collide;
	
}






//SHOOTS OUT A RAY WHEN CALLED CAN BE CALLED RECURSIVELY TO CALCULATE REFLECTIONS
void rayShoot(vec oRay, vec ray, int bounces, int i, int j, int obj)
{
	float c0;
	float dot;
	vec lRay;
	float c1;
	float collide;
	float ambient = 0;
	float diffuse = 0;
	float specular = 0;
	float f = 0;
	int sC;
	int pC;
	
	//ray vector
	lRay.x = l[0].x;
	lRay.y = l[0].y;
	lRay.z = l[0].z;
	cl[getPixel(i, j)] = -1;
	
	//amount of collisionts
	pC = -1;
	sC = -1;


	
	//CHECKS ALL SPHERES IN THE SCENE TO SEE IF IT COLLIDES WITH A RAY
	for (int k = 0; k < sphereCount; k++)
	{
	
		collide = sphereCollide(oRay, ray, k);
		
		
		if(m[s[k].material - 1].ref == 0)
		{
			ambient = m[s[k].material - 1].ambient * l[0].ambient;
			if (collide > 0)
			{				
				hit[getPixel(i, j)] = 1;
				sC = k;	
				//IF NO RAY HAS HIT AN OBJECT YET THEN THEN THE FIRST OBJECT IS SET AND ITS COLOR IS APPLIED
				if (cl[getPixel(i, j)] == -1)
				{
					materialSphereCALC(oRay, ray, k, bounces,'s', collide, ambient, diffuse, specular, f, i, j);
				}
				
				//IF ANOTHER OBJECT IS CLOSER THAN THE CURRENT CLOSEST SET THIS OBJECT TO THE CLOSEST AND APPLY OBJECT COLOR
				if (tI[getPixel(i, j)] > collide)
				{
					materialSphereCALC(oRay, ray, k, bounces, 's', collide, ambient, diffuse, specular, f, i, j);

				}
			
			}
		}
		else
		{
			//stops ray from reflecting an object with itself
			if (k != obj  && bounces > 1)
			{
				ambient = m[s[k].material - 1].ambient * l[0].ambient;
				if (collide > 0)
				{	
					hit[getPixel(i, j)] = 1;
					sC = k;			
					//IF NO RAY HAS HIT AN OBJECT YET THEN THEN THE FIRST OBJECT IS SET AND ITS COLOR IS APPLIED
					if (cl[getPixel(i, j)] == -1)
					{
						materialSphereCALC(oRay, ray, k, bounces,'s', collide, ambient, diffuse, specular, f, i, j);
					}				
					//IF ANOTHER OBJECT IS CLOSER THAN THE CURRENT CLOSEST SET THIS OBJECT TO THE CLOSEST AND APPLY OBJECT COLOR
					if (tI[getPixel(i, j)] > collide)
					{
						materialSphereCALC(oRay, ray, k, bounces, 's', collide, ambient, diffuse, specular, f, i, j);

					}
				}				
			}
		}
		 
	}
	
	//CALCULATES ALL COLLISIONS WITH THE PLANES PRESENT IN THE SCENE
	for (int k = 0; k < planeCount; k++)
	{
			//CHECK COLLISION
			collide = planeCollide(oRay, ray, k);
			

				if (collide > 0)
				{
					
					hit[getPixel(i, j)] = 1;
					if (m[p[k].material - 1].ref != 0)
					{
						sC = k;	
						pC = k;
					}
					//AMBIENT LIGHT CO EFFICIENT
					ambient = m[p[k].material - 1].ambient * l[0].ambient;

					if (cl[getPixel(i, j)] == -1)
					{
						sC = k;	
						pC = k;
						//HITSPOT NORMALIZE
						hVec[getPixel(i, j)].x = (ray.x * collide + oRay.x);
						hVec[getPixel(i, j)].y = (ray.y * collide + oRay.y);
						hVec[getPixel(i, j)].z = (ray.z * collide + oRay.z);



						//NORMAL VEC
						rVec[getPixel(i, j)].x = ((hVec[getPixel(i, j)].x));
						rVec[getPixel(i, j)].y = -((hVec[getPixel(i, j)].y));
						rVec[getPixel(i, j)].z = ((hVec[getPixel(i, j)].z));

						rVec[getPixel(i, j)] = normalize(rVec[getPixel(i, j)]);




						dot = (rVec[getPixel(i, j)].x * ray.x) + (rVec[getPixel(i, j)].y * ray.y) + (rVec[getPixel(i, j)].z * ray.z);
						refVec[getPixel(i, j)].x = (ray.x) - 2 * (dot)*rVec[getPixel(i, j)].x;
						refVec[getPixel(i, j)].y = (ray.y) - 2 * (dot)*rVec[getPixel(i, j)].y;
						refVec[getPixel(i, j)].z = (ray.z) - 2 * (dot)*rVec[getPixel(i, j)].z;

						refVec[getPixel(i, j)] = normalize(refVec[getPixel(i, j)]);

						if (m[p[k].material - 1].diffuse != 0)
						{
							//DIFFUSE COSINE
							c0 = (rVec[getPixel(i, j)].x * l[0].x) + (rVec[getPixel(i, j)].y * l[0].y) + (rVec[getPixel(i, j)].z * l[0].z);
							diffuse = m[p[k].material - 1].diffuse * l[0].intensity * c0;
						}

						if (m[p[k].material - 1].specular != 0)
						{
							//SPECULAR COSINE CALCULATION
							f = m[p[k].material - 1].fCo;
							c0 = pow((refVec[getPixel(i, j)].x * ray.x) + (refVec[getPixel(i, j)].y * ray.y) + (refVec[getPixel(i, j)].z * ray.z), f);
							specular = m[p[k].material - 1].specular * l[0].intensity * c0;
						}

						//COLOR CALCULATION
						cl[getPixel(i, j)] = k;
						float mat = diffuse + ambient + specular;

						if (bounces != MAX_BOUNCES)
						{
							q[getPixel(i, j)].r += (p[k].r * (mat) * 0.5);
							q[getPixel(i, j)].g += (p[k].g * (mat) * 0.5);
							q[getPixel(i, j)].b += (p[k].b * (mat) * 0.5);
						}
						else
						{
							q[getPixel(i, j)].r = (p[k].r * (mat) * 0.9);
							q[getPixel(i, j)].g = (p[k].g * (mat) * 0.9);
							q[getPixel(i, j)].b = (p[k].b * (mat) * 0.9);
						}
						tI[getPixel(i, j)] = collide;

						//shadows
						for (int object = 0; object < sphereCount; object++)
						{
							c1 = sphereCollide(hVec[getPixel(i, j)], lRay, object);
							if (c1 > 0)
							{

								q[getPixel(i, j)].r = q[getPixel(i, j)].r / 2;
								q[getPixel(i, j)].g = q[getPixel(i, j)].g / 2;
								q[getPixel(i, j)].b = q[getPixel(i, j)].b / 2;
								object = sphereCount;
								return;
							}
						}
						
						if (bounces != MAX_BOUNCES)
						{
							q[getPixel(i, j)].r += (c->r * 0.1) * (diffuse + ambient + specular);
							q[getPixel(i, j)].g += (c->g * 0.1) * (diffuse + ambient + specular);
							q[getPixel(i, j)].b += (c->b * 0.1) * (diffuse + ambient + specular);
						}
						
					}
					if (tI[getPixel(i, j)] > collide)
					{
						sC = k;	
						pC = k;
						//HITSPOT NORMALIZE
						hVec[getPixel(i, j)].x = (ray.x * collide + oRay.x);
						hVec[getPixel(i, j)].y = (ray.y * collide + oRay.y);
						hVec[getPixel(i, j)].z = (ray.z * collide + oRay.z);



						//NORMAL VEC
						rVec[getPixel(i, j)].x = ((hVec[getPixel(i, j)].x));
						rVec[getPixel(i, j)].y = -((hVec[getPixel(i, j)].y));
						rVec[getPixel(i, j)].z = ((hVec[getPixel(i, j)].z));

						rVec[getPixel(i, j)] = normalize(rVec[getPixel(i, j)]);




						dot = (rVec[getPixel(i, j)].x * ray.x) + (rVec[getPixel(i, j)].y * ray.y) + (rVec[getPixel(i, j)].z * ray.z);
						refVec[getPixel(i, j)].x = (ray.x) - 2 * (dot)*rVec[getPixel(i, j)].x;
						refVec[getPixel(i, j)].y = (ray.y) - 2 * (dot)*rVec[getPixel(i, j)].y;
						refVec[getPixel(i, j)].z = (ray.z) - 2 * (dot)*rVec[getPixel(i, j)].z;

						refVec[getPixel(i, j)] = normalize(refVec[getPixel(i, j)]);

						if (m[p[k].material - 1].diffuse != 0)
						{
							//DIFFUSE COSINE
							c0 = (rVec[getPixel(i, j)].x * l[0].x) + (rVec[getPixel(i, j)].y * l[0].y) + (rVec[getPixel(i, j)].z * l[0].z);
							diffuse = m[p[k].material - 1].diffuse * l[0].intensity * c0;
						}

						if (m[p[k].material - 1].specular != 0)
						{
							//SPECULAR COSINE CALCULATION
							f = m[p[k].material - 1].fCo;
							c0 = pow((refVec[getPixel(i, j)].x * ray.x) + (refVec[getPixel(i, j)].y * ray.y) + (refVec[getPixel(i, j)].z * ray.z), f);
							specular = m[p[k].material - 1].specular * l[0].intensity * c0;
						}

						//COLOR CALCULATION
						cl[getPixel(i, j)] = k;
						float mat = diffuse + ambient + specular;


						if (bounces != MAX_BOUNCES)
						{
							q[getPixel(i, j)].r += (p[k].r * (mat) * 0.5);
							q[getPixel(i, j)].g += (p[k].g * (mat) * 0.5);
							q[getPixel(i, j)].b += (p[k].b * (mat) * 0.5);
						}
						else
						{
							q[getPixel(i, j)].r = (p[k].r * (mat) * 0.9);
							q[getPixel(i, j)].g = (p[k].g * (mat) * 0.9);
							q[getPixel(i, j)].b = (p[k].b * (mat) * 0.9);
						}

						tI[getPixel(i, j)] = collide;

						for (int object = 0; object < sphereCount; object++)
						{
							c1 = sphereCollide(hVec[getPixel(i, j)], lRay, object);
							if (c1 > 0)
							{
								q[getPixel(i, j)].r = q[getPixel(i, j)].r / 2;
								q[getPixel(i, j)].g = q[getPixel(i, j)].g / 2;
								q[getPixel(i, j)].b = q[getPixel(i, j)].b / 2;
								object = sphereCount;
								return;
							}
						}
						
						if (bounces != MAX_BOUNCES)
						{
							q[getPixel(i, j)].r += (c->r * 0.1) * (diffuse + ambient + specular);
							q[getPixel(i, j)].g += (c->g * 0.1) * (diffuse + ambient + specular);
							q[getPixel(i, j)].b += (c->b * 0.1) * (diffuse + ambient + specular);
						}
						
					}

				
					
			
				}
			
		
	}
	

	//RECURSIVE CALL FOR RAYTRACING REFLECTIONS 
	
	if (sC > -1)
	{
	
		if (m[s[sC].material - 1].ref != 0)
		{
			if (bounces > 1)
			{
				q[getPixel(i, j)].r *= 0.9;
				q[getPixel(i, j)].g *= 0.9;
				q[getPixel(i, j)].b *= 0.9;
				rayShoot(hVec[getPixel(i, j)],refVec[getPixel(i, j)], --bounces, i, j, cl[getPixel(i, j)]);
				
			}

		}
	}
	if (pC > -1)
	{
		if (m[p[pC].material - 1].ref != 0)
		{
			if (bounces > 1)
			{

				rayShoot(hVec[getPixel(i, j)], refVec[getPixel(i, j)], --bounces, i, j, cl[getPixel(i, j)]);
			}
		}
	}
	
}










//UPDATES RENDER EVERY FRAME
void update(int tHeight, int lStart2)
{	
	float temp;
	
	//TRAVERSE THROUGH EACH PIXEL
	for (int i = lStart2; i < tHeight; i++)
	{
		
		for (int j = 0; j < width; j++)
		{
			
			//R0 AND RD VECTORS
			vec oRay;
			vec ray;
			//INITIALIZE CAMERA RAY BASED ON POSITION
			r[getPixel(i, j)].ox = c->x + c->dx;
			r[getPixel(i, j)].oy = c->y + c->dy;
			r[getPixel(i, j)].oz = c->dz;

			r[getPixel(i, j)].x = ((j)) + c->dx;
			r[getPixel(i, j)].y = ((i)) + c->dy;
			r[getPixel(i, j)].z = ((c->z) * (-1)) + c->dz;

			//CALCULATE DIRECTION VECTOR
			r[getPixel(i, j)].dx = (r[getPixel(i, j)].x - r[getPixel(i, j)].ox);
			r[getPixel(i, j)].dy = (r[getPixel(i, j)].y - r[getPixel(i, j)].oy);
			r[getPixel(i, j)].dz = (r[getPixel(i, j)].z - r[getPixel(i, j)].oz);

			//NORMALIZE
			temp = sqrt(square(r[getPixel(i, j)].dx) + (square(r[getPixel(i, j)].dy)) + (square(r[getPixel(i, j)].dz)));

			r[getPixel(i, j)].dx /= temp;
			r[getPixel(i, j)].dy /= temp;
			r[getPixel(i, j)].dz /= temp;


			//ASSIGN RAYS TO VECTORS TO MAKE IT SIMPLE TO PASS VECTORS THROUGH FUNCTIONS
			oRay.x = r[getPixel(i, j)].ox;
			oRay.y = r[getPixel(i, j)].oy;
			oRay.z = r[getPixel(i, j)].oz;

			ray.x = r[getPixel(i, j)].dx;
			ray.y = r[getPixel(i, j)].dy;
			ray.z = r[getPixel(i, j)].dz;

			hit[getPixel(i, j)] = 0;

			//CALL RAY TRACE ALGORITHM
			rayShoot(oRay, ray, MAX_BOUNCES, i, j, cl[getPixel(i, j)]);
			//IF OBJECT HAS COLLIDED DRAW THE OBJECT AT{j, i}
			if (hit[getPixel(i, j)] != 0)
			{
				o[getPixel(i, j)].r = q[getPixel(i, j)].r;
				o[getPixel(i, j)].g = q[getPixel(i, j)].g;
				o[getPixel(i, j)].b = q[getPixel(i, j)].b;
			}
			else
			{
				o[getPixel(i, j)].r = 0.05;
				o[getPixel(i, j)].g = 0.2;
				o[getPixel(i, j)].b = 0.5;
			}
			
		}
		
	}
		
	

	

	
	return;
}


void threading()
{
	std::thread threads[processor_count];
	
	
	
	

	float start = clock();
	
	//allocate threads and divides render by processor size
	for (int i = 0; i < processor_count; i++)
	{	
		threads[i] = std::thread(update, (i+1) * (height / processor_count),  i * (height / processor_count));
		//RUN ASYNC
		//threads[i].detach();
	}
	for (int i = 0; i < processor_count; i++)
	{
		//RUN WHEN ALL THREADS ARE FINISHED
		threads[i].join();
		
	}

	
	glBegin(GL_POINTS);
	
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{	
			glColor3f(o[getPixel(i, j)].r, o[getPixel(i, j)].g, o[getPixel(i, j)].b);
			glVertex2i(j, i);
		}
	}
	glEnd();
	glutSwapBuffers();
	printf("%f\n",((float)(1 / (((clock() - start))/CLOCKS_PER_SEC))));
}


//KEYBOARD PRESSES
void controls(unsigned char key, int x, int y)
{
	//WASD CONTROLS
	//Q AND E TO ASCEND AND DESCEND
	switch (key)
	{
		case 'A':
		case 'a':
		{
			c->dx -= c->speed;
			break;
		}
		case 'D':
		case 'd':
		{
			c->dx += c->speed;
			break;
		}
		case 'W':
		case 'w':
		{
	
			c->dz -= c->speed;
			break;
		}
		case 'S':
		case 's':
		{
			c->dz += c->speed;
			break;
		}
		case 'q':
		case 'Q':
		{
			c->dy += c->speed;
			break;
		}
		case 'E':
		case 'e':
		case ' ':
		{
				c->dy -= c->speed;
			break;
		}
	
		case 'h':
		case 'H':
		{
			printf
			(
				"CONTROLS:\n"
				"		  USE WASD KEYS TO MOVE FORWARD, BACK AND SIDE TO SIDE\n"
				"		  USE E AND Q TO MOVE UP AND DOWN\n"
				"		  ALTERNATIVELY SPACE MAY BE USED TO MOVE UP\n"
			);
			break;
		}

		//RENDERS SCENE TO IMAGE FILE
		case 'P':
		case 'p':
		{
			printf("WRITING IMAGE TO RAYTRACEDSCENE.tiff... \n");
			
			write_img((char*)("RAYTRACEDSCENE.tiff"));
			printf("IMAGE SUCCESSFULLY WRITTEN TO: RAYTRACEDSCENE.tiff!\n");
			break;
		}
	}
}

//MAIN
int main(int argc, char** argv)
{
	//LOADS SCENE FROM TEXT FILE
	setupScene();
	//OPENGL/GLUT INITIALIZATION
	glutInit(&argc, argv);
	glutInitWindowSize(width, height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("RAYTRACER");
	glutDisplayFunc(threading);
	//CAMERA CONTROL
	glutKeyboardFunc(controls);
	
	//MAIN UPDATE LOOP
	glutIdleFunc(threading);
	
	glOrtho(0.0f, width, height, 0.0f, 0.0f, 1.0f);

	//BACKGROUND COLOR
	//glClearColor(c->r, c->g, c->b, 1.0f);

	

	
	glutMainLoop();
	
	free(rVec);
	free(hVec);
	free(refVec);
	free(r);
	free(c);
	free(s);
	free(p);
	free(t);
	free(m);
	free(l);
	free(sT);
	free(pT);
	free(tT);
	free(mT);
	free(lT);
	free(q);
	free(o);
	free(hit);
	free(cl);
	free(tI);
}
