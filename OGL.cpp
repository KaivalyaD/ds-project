// header files
// standard headers
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include<time.h>	// for time()
#include"OGL.h"

// LSystem
#include"LS/ls.h"

// GLEW
#include<GL/glew.h>	// for GL_MIRRORED_REPEAT

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>	// graphics library utility

// math headers
#define _USE_MATH_DEFINES
#include<math.h>

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define MAX_ITERATIONS_FRACTAL_TREE 6
#define MAX_SEQUENCE_LENGTH 65536

#define INITIAL_ANGLE_SECOND_HAND 30
#define INITIAL_ANGLE_MINUTE_HAND 330.0f
#define INITIAL_ANGLE_HOUR_HAND 200.0f

#define DEG_TO_RAD(x) (((x) * M_PI) / (180.0f))

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

int winWidth;
int winHeight;

/* textures */
GLuint textureAMCPresents;
GLuint textureHouseFloor;
GLuint textureAbhyasaMantra;
GLuint textureKeyboard;
GLuint textureTable;
GLuint textureScreen;
GLuint textureGrass;
GLuint textureBench;
GLuint textureKoshish;
GLuint textureKavi;
GLuint textureMyCredit;
GLuint textureTechnicalInfo;
GLuint textureMusicalInfo;
GLuint textureOthersCredit;
GLuint textureSirAndMadamsCredit;

/* lights */
GLfloat outdoorLightAmbient[] = { 0.7f, 0.5f, 0.2f, 1.0f };
GLfloat outdoorLightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat outdoorLightSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat outdoorLightPosition[] = { 0.0f, 1.0f, 0.0f, 1.0f };

GLfloat indoorLightAmbient[] = { 0.4f, 0.2f, 0.1f, 1.0f };
GLfloat indoorLightDiffuse[] = { 0.4f, 0.3f, 0.1f, 1.0f };
GLfloat indoorLightSpecular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat indoorLightPosition[] = { 1.3f, 0.85f, -6.43f, 1.0f };
GLfloat indoorLightDirection[] = { 0.0f, 0.0f, -1.0f, 1.0f };

/* camera */
GLfloat xCamera;
GLfloat yCamera;
GLfloat zCamera;

GLfloat xVantage;
GLfloat yVantage;
GLfloat zVantage;

GLfloat xUp;
GLfloat yUp;
GLfloat zUp;

float angleY;

/* scene counter */
int scene = 0;

/* intro/extro */
float alpha = 0.0f;

/* time keeper */
int elapsedSeconds = 0;

/* wall-clock */
int secondHandAngle = INITIAL_ANGLE_SECOND_HAND;
float minuteHandAngle = INITIAL_ANGLE_MINUTE_HAND;
float hourHandAngle = INITIAL_ANGLE_HOUR_HAND;

/* humanoid */
float angleHeadX = 0.0f;
float angleHeadY = 0.0f;
float angleShoulderX = 0.0f;
float angleShoulderZ = 0.0f;
float angleElbowX = 0.0f;
float angleElbowZ = 0.0f;
float angleButtocks = 0.0f;
float angleKnee = 0.0f;
float humanoidMotion = 0.0f;

/* ground */
const float indoorFloorLevel = 0.685f;
const float outdoorFloorLevel = -1.5f;

/* tree */
LSystem lSystem;
char drawingInstructions[MAX_SEQUENCE_LENGTH];
char *isBeautifulLeaf = NULL;
size_t sequenceLength = 0U;
float delta;
float windSpeed;
float deltaDueToWind;
int growthIteration;

// objects
enum {
	INDOOR_TABLE_BODY = 0,
	INDOOR_TABLE_LEG,
	INDOOR_TABLE_KEYBOARD,
	OUTDOOR_GROUND,
	OUTDOOR_BENCH
};

// L-System Alphabet
enum {
	/* variables */
	DRAW_FORWARD = 'F',
	DRAW_LEAF = 'L',
	DRAW_NULL = '\0',

	/* constants */
	PUSH = '[',
	POP = ']',
	TURN_LEFT_DELTA = '+',
	TURN_RIGHT_DELTA = '-',
	PITCH_UP_DELTA = '^',
	PITCH_DOWN_DELTA = '&',
	ROLL_LEFT_DELTA = '\\',
	ROLL_RIGHT_DELTA = '/',
	TURN_AROUND = '|',
};

GLUquadric *quadric;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	int initialize(void);	// declaring according to the order of use
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;
	int cxScreen, cyScreen;

	// code
	if (fopen_s(&gpLog, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("fopen_s: failed to open log file"), TEXT("File I/O Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpLog, "fopen_s: log file opened successfully\n");
	}

	// initialization of the wndclass structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// registering the wndclass
	RegisterClassEx(&wndclass);

	// getting the screen size
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Going 3D: Kaivalya Vishwakumar Deshpande"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(cxScreen - WIN_WIDTH) / 2,
		(cyScreen - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;
	
	// initialize
	iRetVal = initialize();
	if (iRetVal == -1)
	{
		fprintf(gpLog, "ChoosePixelFormat(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -2)
	{
		fprintf(gpLog, "SetPixelFormat(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -3)
	{
		fprintf(gpLog, "wglCreateContext(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpLog, "wglMakeCurrent(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -5)
	{
		fprintf(gpLog, "LoadGLTexture(): failed to load textureHouseFloor\n");
		uninitialize();
	}
	else if (iRetVal == -6)
	{
		fprintf(gpLog, "LoadGLTexture(): failed to load textureAbhyasaMantra\n");
		uninitialize();
	}
	else if (iRetVal == -7)
	{
		fprintf(gpLog, "LoadGLTexture(): failed to load textureTable\n");
		uninitialize();
	}
	else if (iRetVal == -8)
	{
		fprintf(gpLog, "LoadGLTexture(): failed to load textureKeyboard\n");
		uninitialize();
	}
	else if (iRetVal == -9)
	{
		fprintf(gpLog, "LoadGLTexture(): failed to load textureScreen\n");
		uninitialize();
	}
	else if (iRetVal == -10)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureGrass\n");
		uninitialize();
	}
	else if (iRetVal == -11)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureBench\n");
		uninitialize();
	}
	else if (iRetVal == -12)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureAMCPresents\n");
		uninitialize();
	}
	else if (iRetVal == -13)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureKoshish\n");
		uninitialize();
	}
	else if (iRetVal == -14)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureKavi\n");
		uninitialize();
	}
	else if (iRetVal == -15)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureMyCredit\n");
		uninitialize();
	}
	else if (iRetVal == -16)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureTechnicalInfo\n");
		uninitialize();
	}
	else if (iRetVal == -17)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureMusicalInfo\n");
		uninitialize();
	}
	else if (iRetVal == -18)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureOthersCredit\n");
		uninitialize();
	}
	else if (iRetVal == -19)
	{
		fprintf(gpLog, "LoadGLTexture(): failed for textureSirAndMadamsCredit\n");
		uninitialize();
	}
	else
	{
		fprintf(gpLog, "created OpenGL context successfully and made it the current context\n");
	}

	// show the window
	ShowWindow(hwnd, iCmdShow);

	// foregrounding and focussing the window
	SetForegroundWindow(hwnd);	// using ghwnd is obviously fine, but by common sense ghwnd is for global use while we have hwnd locally available in WndProc and here
	SetFocus(hwnd);

	// game loop
	while (bDone != TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow)
			{
				// render the scene
				display();

				// update the scene
				update();
			}
		}
	}

	// uninitialize
	uninitialize();

	return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	void resize(int, int);

	// variable declarations
	static int iterationsFractalTree = -1;
	size_t size = 0U;

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		fprintf(gpLog, "window in focus\n");
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		fprintf(gpLog, "window out of focus\n");
		break;
	case WM_ERASEBKGND:
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			fprintf(gpLog, "destroying after receiving esc\n");
			DestroyWindow(hwnd);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		default:
			break;
		}
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:	// disciplined code: sent as a signal that a window or an application should terminate
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
			fprintf(gpLog, "fullscreen mode on\n");
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
		fprintf(gpLog, "fullscreen mode off\n");
	}
}

int initialize(void)
{
	// function prototypes
	void resize(int, int);
	BOOL LoadGLTexture(GLuint *, TCHAR *, int);
	void productionRulesTree(SequenceNode **);
	void generateTree(int);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;
	RECT rc;
	float fogColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	int cLeaves;

	// code
	// initialize PIXELFORMATDESCRIPTOR
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;	// R
	pfd.cGreenBits = 8;	// G
	pfd.cBlueBits = 8;	// B
	pfd.cAlphaBits = 8;	// A
	pfd.cDepthBits = 32;	// 24 is another option

	// get DC
	ghdc = GetDC(ghwnd);

	// choose pixel format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
		return -1;

	// set chosen pixel format
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
		return -2;

	// create OpenGL rendering context
	ghrc = wglCreateContext(ghdc);	// calling my first bridging API
	if (ghrc == NULL)
		return -3;

	// make the rendering context the current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)	// the second bridging API
		return -4;

	// loading all textures
	if (!LoadGLTexture(&textureHouseFloor, MAKEINTRESOURCE(IDBM_HOUSE_FLOOR), -1))
		return -5;
	if (!LoadGLTexture(&textureAbhyasaMantra, MAKEINTRESOURCE(IDBM_ABHYASA_MANTRA), -1))
		return -6;
	if (!LoadGLTexture(&textureTable, MAKEINTRESOURCE(IDBM_TABLE), -1))
		return -7;
	if (!LoadGLTexture(&textureKeyboard, MAKEINTRESOURCE(IDBM_TABLE_KEYBOARD), -1))
		return -8;
	if (!LoadGLTexture(&textureScreen, MAKEINTRESOURCE(IDBM_TABLE_MONITOR_SCREEN), -1))
		return -9;
	if (!LoadGLTexture(&textureGrass, MAKEINTRESOURCE(IDBM_GRASS), OUTDOOR_GROUND))
		return -10;
	if (!LoadGLTexture(&textureBench, MAKEINTRESOURCE(IDBM_BENCH), -1))
		return -11;
	if (!LoadGLTexture(&textureAMCPresents, MAKEINTRESOURCE(IDBM_AMCPRESENTS), -1))
		return -12;
	if (!LoadGLTexture(&textureKoshish, MAKEINTRESOURCE(IDBM_KOSHISH), -1))
		return -13;
	if (!LoadGLTexture(&textureKavi, MAKEINTRESOURCE(IDBM_KAVI), -1))
		return -14;
	if (!LoadGLTexture(&textureMyCredit, MAKEINTRESOURCE(IDBM_MYCREDIT), -1))
		return -15;
	if (!LoadGLTexture(&textureTechnicalInfo, MAKEINTRESOURCE(IDBM_TECHNICALINFO), -1))
		return -16;
	if (!LoadGLTexture(&textureMusicalInfo, MAKEINTRESOURCE(IDBM_MUSICALINFO), -1))
		return -17;
	if (!LoadGLTexture(&textureOthersCredit, MAKEINTRESOURCE(IDBM_OTHERSCREDIT), -1))
		return -18;
	if (!LoadGLTexture(&textureSirAndMadamsCredit, MAKEINTRESOURCE(IDBM_SIRANDMADAMSCREDIT), -1))
		return -19;

	// setting clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// setting up depth
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// setting up lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, outdoorLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, outdoorLightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, outdoorLightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, outdoorLightPosition);
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, indoorLightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, indoorLightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, indoorLightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, indoorLightPosition);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 5.0f);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, indoorLightDirection);

	glEnable(GL_LIGHT0);
 	glEnable(GL_LIGHTING);

	// initializing L-System for tree
	lsGenLSystem(&lSystem, 1, productionRulesTree);
	lSystem.w[0] = DRAW_FORWARD;

	// generating the tree
	growthIteration = 4;
	generateTree(growthIteration);

	// parameters for drawing the tree
	delta = 28.0f;
	
	// seeding for random number generation
	srand(time(NULL));

	// deciding which leaves turn beautiful after storm
	// 3 is the number of branches a single branch splits into according to the production rules defined for this particular tree
	cLeaves = (int)powf(3, growthIteration);
	isBeautifulLeaf = (char *)malloc(sizeof(char) * cLeaves);
	for (int i = 0; i < cLeaves; i++)
		isBeautifulLeaf[i] = (rand() > (RAND_MAX / 2)) ? 1 : 0;

	// setting up the camera
	xCamera = 0.0f;
	yCamera = 0.5f;
	zCamera = 1.0f;

	xVantage = 0.0f;
	yVantage = 0.5f;
	zVantage = 0.0f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;

	// scene
	scene = 0;
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// setup fog
	glFogi(GL_FOG_MODE, GL_EXP);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.0f);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 50.0f);
	glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);

	// blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);

	// allocate a quadric for non-linear surfaces
	quadric = gluNewQuadric();

	// start BGM
	// PlaySound(MAKEINTRESOURCE(IDWAV_KOSHISH), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

	// warm-up resize
	GetClientRect(ghwnd, &rc);
	resize(rc.right - rc.left, rc.bottom - rc.top);

	return 0;
}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;	// to prevent a divide by zero when calculating the width/height ratio

	winWidth = width;
	winHeight = height;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void display(void)
{
	// function prototypes
	void camShotOne(void);
	void camShotTwo(void);
	void camShotThree(void);
	void camShotFour(void);
	void camShotFive(void);
	void camShotSix(void);
	void camShotEight(void);
	void camShotEleven(void);

	void drawHouse(void);
	void drawHumanoid(void);
	void updateHumanoid(void);
	void drawTree(void);

	void drawSky(void);
	void drawGround(void);
	void drawBench(void);

	void uninitialize(void);

	// variable declarations
	static BOOL isSceneTransitioning = TRUE;
	static GLfloat fogDensity = 0.0f;
	static GLfloat indoorAttenuation = 5.0f;

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setting the projection matrix to perspective
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(
		45.0f,
		(GLfloat)winWidth / (GLfloat)winHeight,
		0.1f,
		100.0f
	);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// camera control
	switch (scene)
	{
	case 0:
		if (isSceneTransitioning)
		{
			/* all scene initializations to be done here */
			isSceneTransitioning = FALSE;
			glDisable(GL_LIGHTING);

			alpha = 1.0f;
		}

		gluLookAt(
			0.0f, 0.0f, 3.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f
		);

		/* all scene updations based on the timer as follows */
		if (elapsedSeconds < 5)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureAMCPresents);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.0f, 0.2f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.0f, -0.2f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.0f, -0.2f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.0f, 0.2f, 0.0f);
			}
			glEnd();
			glDisable(GL_TEXTURE_2D);

			// fading in
			alpha -= 0.005f;
			if (alpha < 0.0)
				alpha = 0.0f;
		}
		else
		{
			/* all scene conclude with this else block having per-scene uninitializations (if any) or preparations for the next scene */
			// re-setting alpha for next scene fade-in before changing to it for unabrupt transition
			alpha = 1.0f;
			glEnable(GL_LIGHTING);

			scene = 1;
			isSceneTransitioning = TRUE;
		}
		break;
	
	case 1:
		if (isSceneTransitioning)
		{
			camShotOne();
			isSceneTransitioning = FALSE;

			// again setting alpha here as a convention so that its initial value at start of scene can easily be glanced
			alpha = 1.0f;

			// start BGM
			PlaySound(MAKEINTRESOURCE(IDWAV_BKGNDTRACK), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		}

		if (elapsedSeconds < 10)
		{
			zCamera += 0.0005f;

			// fading in
			alpha -= 0.005f;
			if (alpha < 0.0)
				alpha = 0.0f;
		}
		else if (elapsedSeconds < 14)
		{
			// fading out
			alpha += 0.005f;
			if (alpha > 1.0)
				alpha = 1.0f;
		}
		else
		{
			alpha = 1.0f;

			scene = 2;
			isSceneTransitioning = TRUE;
		}
		break;

	case 2:
		if (isSceneTransitioning)
		{
			camShotTwo();
			isSceneTransitioning = FALSE;

			alpha = 1.0f;
		}

		if (elapsedSeconds < 35)
		{
			xCamera -= 0.0005f;
			zVantage -= 0.001f;

			// fade in
			alpha -= 0.005f;
			if (alpha < 0.0)
				alpha = 0.0f;
		}
		else if (elapsedSeconds < 37)
		{
			// fade out
			alpha += 0.005f;
			if (alpha > 1.0)
				alpha = 1.0f;
		}
		else
		{
			alpha = 1.0f;

			isSceneTransitioning = TRUE;
			scene = 3;
		}
		break;

	case 3:
		if (isSceneTransitioning)
		{
			camShotThree();
			isSceneTransitioning = FALSE;

			alpha = 1.0f;
		}

		if (elapsedSeconds < 55)
		{
			zCamera += 0.0005f;

			if (elapsedSeconds < 48)
			{
				// fade in
				alpha -= 0.01f;
				if (alpha < 0.0)
					alpha = 0.0f;
			}
			else if(elapsedSeconds > 52)
			{
				// fade out
				alpha += 0.01f;
				if (alpha > 1.0)
					alpha = 1.0f;
			}
		}
		else
		{
			alpha = 1.0f;

			isSceneTransitioning = TRUE;
			scene = 4;
		}
		break;

	case 4:
		if (isSceneTransitioning)
		{
			camShotFour();
			isSceneTransitioning = FALSE;

			alpha = 1.0f;

			angleHeadX = -25.0f;
			angleShoulderX = -50.0f;
			angleShoulderZ = 15.0f;
			angleElbowZ = 70.0f;
			angleButtocks = -90.0f;
			angleKnee = 90.0f;
		}

		if (elapsedSeconds < 63)
		{
			xCamera += 0.0005f;
			zCamera += 0.0001f;

			// fade in
			alpha -= 0.01f;
			if (alpha < 0.0)
				alpha = 0.0f;
		}
		else if(elapsedSeconds < 65)
		{
			// don't cut for 2 seconds
		}
		else
		{
			isSceneTransitioning = TRUE;
			scene = 5;

			alpha = 0.0f;
		}
		break;

	case 5:
		if (isSceneTransitioning)
		{
			camShotFive();
			isSceneTransitioning = FALSE;

			alpha = 0.0f;
		}
		if (elapsedSeconds < 75)
		{
			xCamera += 0.0001f;
			zCamera += 0.0001f;
		}
		else
		{
			alpha = 0.0f;

			scene = 6;
			isSceneTransitioning = TRUE;
		}
		break;

	case 6:
		if (isSceneTransitioning)
		{
			isSceneTransitioning = FALSE;

			alpha = 0.0f;
		}
		if (elapsedSeconds < 110)
		{
			xCamera += 0.0001f;
			zCamera += 0.0001f;

			if (elapsedSeconds > 80 && elapsedSeconds < 84)
			{
				angleHeadX += 0.5f;
				if (angleHeadX > 0.0f)
					angleHeadX = 0.0f;
			}
			else if (elapsedSeconds > 84 && elapsedSeconds < 88)
			{
				angleHeadY -= 0.5f;
				if (angleHeadY < -65.0f)
					angleHeadY = -65.0f;
			}
			else if (elapsedSeconds > 88 && elapsedSeconds < 92)
			{
				angleHeadY += 0.5f;
				if (angleHeadY > 65.0f)
					angleHeadY = 65.0f;
			}
			else if (elapsedSeconds > 92 && elapsedSeconds < 96)
			{
				angleHeadY -= 0.5f;
				if (angleHeadY < 0.0f)
					angleHeadY = 0.0f;
			}
			else if (elapsedSeconds > 96 && elapsedSeconds < 100)
			{
				angleHeadX -= 0.5f;
				if (angleHeadX < -20.0f)
					angleHeadX = -20.0f;
			}
			else if(elapsedSeconds > 107 && elapsedSeconds < 110)
			{
				// fade out
				alpha += 0.005f;
				if (alpha > 1.0)
					alpha = 1.0f;
			}
		}
		else
		{
			alpha = 1.0f;

			scene = 7;
			isSceneTransitioning = TRUE;
		}
		break;

	case 7:
		if (isSceneTransitioning)
		{
			camShotSix();
			isSceneTransitioning = FALSE;
			
			glEnable(GL_FOG);
			alpha = 1.0f;
		}
		if (elapsedSeconds < 128)
		{
			xCamera = 4.57f - 3.0f * cosf(DEG_TO_RAD(angleY));
			yCamera += 0.003f;
			zCamera = 1.83f - 3.0f * sinf(DEG_TO_RAD(angleY));

			alpha -= 0.01f;
			if (alpha < 0.0f)
				alpha = 0.0f;

			fogDensity += 0.001f;
			if (fogDensity > 0.5f)
				fogDensity = 0.5f;

			glFogf(GL_FOG_DENSITY, fogDensity);
		}
		else if (elapsedSeconds < 130)
		{
			// don't cut for 2 more seconds
		}
		else
		{
			alpha = 0.0f;

			scene = 8;
			isSceneTransitioning = TRUE;
		}
		break;

	case 8:
		if (isSceneTransitioning)
		{
			isSceneTransitioning = FALSE;

			alpha = 0.0f;
		}
		if (elapsedSeconds < 140)
		{
			xCamera -= 0.0005f;
			zCamera -= 0.001f;

			fogDensity -= 0.001;
			if (fogDensity < 0.2f)
				fogDensity = 0.2f;

			glFogf(GL_FOG_DENSITY, fogDensity);
		}
		else if (elapsedSeconds < 145)
		{
			// fade out
			alpha += 0.004f;
			if (alpha > 1.0f)
				alpha = 1.0f;
		}
		else
		{
			alpha = 1.0f;

			scene = 9;
			isSceneTransitioning = TRUE;
		}
		break;

	case 9:
		if (isSceneTransitioning)
		{
			camShotEight();
			isSceneTransitioning = FALSE;
			glDisable(GL_FOG);

			alpha = 1.0f;
		}

		if (elapsedSeconds < 161)
		{
			xCamera += 0.0001f;

			// fade in
			alpha -= 0.01f;
			if (alpha < 0.0f)
				alpha = 0.0f;
		}
		else if (elapsedSeconds < 166)
		{
			// don't cut for 5 more seconds
		}
		else
		{
			alpha = 0.0f;

			scene = 10;
			isSceneTransitioning = TRUE;
		}
		break;

	case 10:
		if (isSceneTransitioning)
		{
			isSceneTransitioning = FALSE;

			alpha = 0.0f;
		}

		if (elapsedSeconds < 185)
		{
			xCamera += 0.0001f;

			angleHeadX += 0.5f;
			if (angleHeadX > 0.0f)
				angleHeadX = 0.0f;

			angleShoulderZ -= 0.5f;
			if (angleShoulderZ < 0.0f)
				angleShoulderZ = 0.0f;

			angleShoulderX += 0.5f;
			if (angleShoulderX > 0.0f)
				angleShoulderX = 0.0f;

			angleElbowZ -= 1.0f;
			if (angleElbowZ < 10.0f)
				angleElbowZ = 10.0f;
		}
		else if (elapsedSeconds < 187)
		{
			// fade out
			alpha += 0.01f;
			if (alpha > 1.0f)
				alpha = 1.0f;
		}
		else
		{
			alpha = 1.0f;

			scene = 11;
			isSceneTransitioning = TRUE;
		}
		break;

	case 11:
		if (isSceneTransitioning)
		{
			camShotEleven();
			isSceneTransitioning = FALSE;

			outdoorLightPosition[0] = 0.0f;
			outdoorLightPosition[0] = 10.0f;
			outdoorLightPosition[0] = 0.0f;
			glLightfv(GL_LIGHT0, GL_POSITION, outdoorLightPosition);

			glEnable(GL_LIGHT1);

			alpha = 1.0f;
		}

		if (elapsedSeconds < 203)
		{
			xCamera += 0.0001f;
			yCamera += 0.0001f;
			zCamera += 0.0001f;

			// fade in
			alpha -= 0.01f;
			if (alpha < 0.0f)
				alpha = 0.0f;

			for (int i = 0; i < 3; i++)
			{
				outdoorLightAmbient[i] -= 0.001f;
				if (outdoorLightAmbient[i] < 0.1f)
					outdoorLightAmbient[i] = 0.1f;
			}
			glLightfv(GL_LIGHT0, GL_AMBIENT, outdoorLightAmbient);

			outdoorLightDiffuse[0] -= 0.001f;
			if (outdoorLightDiffuse[0] < 0.1f)
				outdoorLightDiffuse[0] = 0.1f;

			outdoorLightDiffuse[1] -= 0.005f;
			if (outdoorLightDiffuse[1] < 0.1f)
				outdoorLightDiffuse[1] = 0.1f;

			outdoorLightDiffuse[2] -= 0.001f;
			if (outdoorLightDiffuse[3] > 0.1f)
				outdoorLightDiffuse[3] = 0.1f;

			glLightfv(GL_LIGHT0, GL_DIFFUSE, outdoorLightDiffuse);

			indoorAttenuation -= 0.1f;
			if (indoorAttenuation < 0.0f)
				indoorAttenuation = 0.0f;

			indoorLightPosition[0] = 1.07f;
			indoorLightPosition[1] = 0.56f;
			indoorLightPosition[2] = -0.2f;
			glLightfv(GL_LIGHT1, GL_POSITION, indoorLightPosition);
			glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, indoorAttenuation);
		}
		else if (elapsedSeconds < 205)
		{
			// fade out
			alpha += 0.01f;
			if (alpha > 1.0f)
				alpha = 1.0f;
		}
		else
		{
			alpha = 1.0f;

			scene = 12;
			isSceneTransitioning = TRUE;
		}
		break;

	case 12:
		if (isSceneTransitioning)
		{
			isSceneTransitioning = FALSE;

			outdoorLightAmbient[0] = 1.0f;
			outdoorLightAmbient[1] = 1.0f;
			outdoorLightAmbient[2] = 1.0f;

			outdoorLightDiffuse[0] = 1.0f;
			outdoorLightDiffuse[1] = 1.0f;
			outdoorLightDiffuse[2] = 1.0f;

			glLightfv(GL_LIGHT0, GL_AMBIENT, outdoorLightAmbient);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, outdoorLightDiffuse);

			alpha = 1.0f;
		}

		gluLookAt(
			0.0f, 0.0f, 3.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f
		);
		
		glEnable(GL_TEXTURE_2D);
		if (elapsedSeconds < 208)
		{
			// fade in
			alpha -= 0.01f;
			if (alpha < 0.0f)
				alpha = 0.0f;

			glBindTexture(GL_TEXTURE_2D, textureKoshish);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.0f, 0.2f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.0f, -0.2f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.0f, -0.2f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.0f, 0.2f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 211)
		{
			glBindTexture(GL_TEXTURE_2D, textureKavi);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.2f, 0.2f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.2f, -0.2f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.2f, -0.2f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.2f, 0.2f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 215)
		{
			glBindTexture(GL_TEXTURE_2D, textureMyCredit);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.5f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.5f, 1.0f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 219)
		{
			glBindTexture(GL_TEXTURE_2D, textureTechnicalInfo);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.5f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.5f, 1.0f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 222)
		{
			glBindTexture(GL_TEXTURE_2D, textureMusicalInfo);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.5f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.5f, 1.0f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 225)
		{
			glBindTexture(GL_TEXTURE_2D, textureOthersCredit);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.5f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.5f, 1.0f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 228)
		{
			glBindTexture(GL_TEXTURE_2D, textureSirAndMadamsCredit);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.5f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.5f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.5f, 1.0f, 0.0f);
			}
			glEnd();
		}
		else if (elapsedSeconds < 230)
		{
			// don't cut for 2 more seconds
		}
		else
		{
			alpha = 0.0f;

			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);

			isSceneTransitioning = TRUE;

			// uninitialize and exit
			uninitialize();
		}
		break;

	default:
		break;
	}
	
	if (scene != 0 && scene != 12)
	{
		// camera transform
		gluLookAt(
			xCamera, yCamera, zCamera,
			xVantage, yVantage, zVantage,
			xUp, yUp, zUp
		);

		glPushMatrix();
		{
			glTranslatef(0.0f, outdoorFloorLevel, 0.0f);
			drawSky();
			drawGround();
			drawBench();

			glTranslatef(0.4f, 0.03f, 0.27f);
			if (scene != 7 && scene != 8 && scene != 11)
			{
				drawHumanoid();
			}

			if (scene == 11)
			{
				glPushMatrix();
				{
					glTranslatef(1.7f, 1.2f, -6.7f);
					glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					drawHumanoid();
				}
				glPopMatrix();
			}

			glTranslatef(-0.8f, -0.28f, -1.45f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			drawTree();
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, indoorFloorLevel, 0.0f);
			drawHouse();
		}
		glPopMatrix();
	}
	else
	{
		// blackout
	}

	// after the color buffer is stained, apply logic for fade-ins and fade-outs
	// setting modelview matrix to identity
	glLoadIdentity();
	
	// setting projection matrix to orthographic
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (winWidth >= winHeight)
	{
		glOrtho(
			-100.0f,
			 100.0f,
			-100.0f * (GLfloat)winHeight / (GLfloat)winWidth,
			 100.0f * (GLfloat)winHeight / (GLfloat)winWidth,
			-100.0f,
			 100.0f
		);
	}
	else
	{
		glOrtho(
			-100.0f * (GLfloat)winWidth / (GLfloat)winHeight,
			 100.0f * (GLfloat)winWidth / (GLfloat)winHeight,
			-100.0f,
			 100.0f,
			-100.0f,
			 100.0f
		);
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_FUNC);
	glBegin(GL_QUADS);
	{
		glColor4f(0.0f, 0.0f, 0.0f, alpha);

		glVertex3f(-100.0f,  100.0f, 0.0f);
		glVertex3f(-100.0f, -100.0f, 0.0f);
		glVertex3f( 100.0f, -100.0f, 0.0f);
		glVertex3f( 100.0f,  100.0f, 0.0f);
	}
	glEnd();
	glEnable(GL_DEPTH_FUNC);
	glEnable(GL_LIGHTING);

	SwapBuffers(ghdc);
}

void update(void)
{
	// function prototypes
	void updateHumanoid_Walk(int);

	// variable declarations
	static time_t now;

	// code
	// timer
	if (now != time(NULL))
	{
		elapsedSeconds += 1;
		now = time(NULL);
	}

	// wind-speed
	if ((rand() % 50) % 13 < 3)
	{
		if ((float)rand() / (float)RAND_MAX > 0.5f)
			windSpeed = (windSpeed + ((float)rand() / (float)RAND_MAX)) / 2.0f;
		else
			windSpeed = (windSpeed - ((float)rand() / (float)RAND_MAX)) * 1.1f;
	}

	// rotator
	angleY = angleY + 1.0f;
	if (angleY >= 360.0f)
		angleY = angleY - 360.0f;
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);

	// code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	if (lSystem.active)
	{
		lsDestroyLSystem(&lSystem);
	}

	if (isBeautifulLeaf)
	{
		free(isBeautifulLeaf);
		isBeautifulLeaf = NULL;
	}

	if (quadric)
	{
		gluDeleteQuadric(quadric);
		quadric = NULL;
	}

	if (textureBench)
	{
		glDeleteTextures(1, &textureBench);
		textureBench = 0;
	}

	if (textureAMCPresents)
	{
		glDeleteTextures(1, &textureAMCPresents);
		textureAMCPresents = 0;
	}

	if (textureKoshish)
	{
		glDeleteTextures(1, &textureKoshish);
		textureKoshish = 0;
	}

	if (textureKavi)
	{
		glDeleteTextures(1, &textureKavi);
		textureKavi = 0;
	}

	if (textureMyCredit)
	{
		glDeleteTextures(1, &textureMyCredit);
		textureMyCredit = 0;
	}

	if (textureTechnicalInfo)
	{
		glDeleteTextures(1, &textureTechnicalInfo);
		textureTechnicalInfo = 0;
	}

	if (textureMusicalInfo)
	{
		glDeleteTextures(1, &textureMusicalInfo);
		textureMusicalInfo = 0;
	}

	if (textureOthersCredit)
	{
		glDeleteTextures(1, &textureOthersCredit);
		textureOthersCredit = 0;
	}

	if (textureSirAndMadamsCredit)
	{
		glDeleteTextures(1, &textureSirAndMadamsCredit);
		textureSirAndMadamsCredit = 0;
	}

	if (textureBench)
	{
		glDeleteTextures(1, &textureBench);
		textureBench = 0;
	}

	if (textureGrass)
	{
		glDeleteTextures(1, &textureGrass);
		textureGrass = 0;
	}

	if (textureScreen)
	{
		glDeleteTextures(1, &textureScreen);
		textureScreen = 0;
	}

	if (textureKeyboard)
	{
		glDeleteTextures(1, &textureKeyboard);
		textureKeyboard = 0;
	}

	if (textureTable)
	{
		glDeleteTextures(1, &textureTable);
		textureTable = 0;
	}

	if (textureAbhyasaMantra)
	{
		glDeleteTextures(1, &textureAbhyasaMantra);
		textureAbhyasaMantra = 0;
	}

	if (textureHouseFloor)
	{
		glDeleteTextures(1, &textureHouseFloor);
		textureHouseFloor = 0;
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);	// if unitialize() was not called from WM_DESTROY
		ghwnd = NULL;
	}

	if (gpLog)
	{
		fprintf(gpLog, "fclose: closing log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}

/* useful functions for camera control */
void camShotOne(void)
{
	// code
	xCamera =  0.0f;
	yCamera =  0.9f;
	zCamera = -0.5;

	xVantage =  0.0f;
	yVantage =  0.9f;
	zVantage = -0.7f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotTwo(void)
{
	// code
	xCamera =  0.0f;
	yCamera =  0.4f;
	zCamera = -0.5f;

	xVantage =  3.5f;
	yVantage = -0.6f;
	zVantage = -2.85f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotThree(void)
{
	// code
	xCamera = 0.0f;
	yCamera = 0.6f;
	zCamera = 0.5f;

	xVantage =  0.0f;
	yVantage =  0.4f;
	zVantage = -0.5f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotFour(void)
{
	// code
	xCamera =  0.2f;
	yCamera =  0.52f;
	zCamera =  -0.9f;

	xVantage =  1.0f;
	yVantage =  0.5f;
	zVantage = -0.7f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotFive(void)
{
	// code
	xCamera = 7.0f;
	yCamera = -0.8f;
	zCamera = 1.0;

	xVantage = 5.0f;
	yVantage = -1.2f;
	zVantage = 0.0f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotSix(void)
{
	// code
	xCamera = 6.0f;
	yCamera = -0.3f;
	zCamera = 2.0;

	xVantage = 4.57f;
	yVantage = 0.1f;
	zVantage = 1.89f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotEight(void)
{
	// code
	xCamera = 7.0f;
	yCamera = -0.8f;
	zCamera = 0.5;

	xVantage = 5.0f;
	yVantage = -1.2f;
	zVantage = 0.5f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

void camShotEleven(void)
{
	// code
	xCamera = 10.0f;
	yCamera = 0.0f;
	zCamera = 0.5;

	xVantage = 5.0f;
	yVantage = -0.2f;
	zVantage = 0.5f;

	xUp = 0.0f;
	yUp = 1.0f;
	zUp = 0.0f;
}

/* useful functions for drawing objects */
void switchMaterial(const float *materialProps)
{
	// code
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialProps);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialProps + 4);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialProps + 8);
	glMaterialf(GL_FRONT, GL_SHININESS, *(materialProps + 12));
}

BOOL LoadGLTexture(GLuint *textureId, TCHAR *resourceId, int object)
{
	// variable declarations
	HBITMAP hBitmap = NULL;
	BITMAP bmp;

	// code
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), resourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (!hBitmap)
	{
		fprintf(gpLog, "Error code: %d\n", GetLastError());
		return FALSE;
	}

	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glGenTextures(1, textureId);
	glBindTexture(GL_TEXTURE_2D, *textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	switch (object)
	{
	case OUTDOOR_GROUND:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;

	default:
		break;
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

	glBindTexture(GL_TEXTURE_2D, 0);
	DeleteObject(hBitmap);

	return TRUE;
}

void drawTexturedCuboid(float width, float height, float depth, int object)
{
	// variable declarations
	GLfloat xStart = (GLfloat)0.0f;
	GLfloat xEnd = (GLfloat)width;
	GLfloat zStart = (GLfloat)0.0f;
	GLfloat zEnd = (GLfloat)depth;

	// code
	glBegin(GL_QUADS);
	{
		/* rear face */
		glNormal3f(0.0f, 0.0f, -1.0f);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 1.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, 0.0f, zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 1.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, 0.0f, zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 0.1f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, (GLfloat)(-height), zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 0.1f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, (GLfloat)(-height), zStart);

		/* right face */
		glNormal3f(1.0f, 0.0f, 0.0f);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(1.0f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(1.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, 0.0f, zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(1.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, 0.0f, zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.9f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, (GLfloat)(-height), zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.9f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(1.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, (GLfloat)(-height), zStart);

		/* front face */
		glNormal3f(0.0f, 0.0f, 1.0f);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, 0.0f, zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, 0.0f, zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 0.1f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, (GLfloat)(-height), zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 0.1f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, (GLfloat)(-height), zEnd);

		/* left face */
		glNormal3f(-1.0f, 0.0f, 0.0f);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.1f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(1.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, 0.0f, zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.1f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 1.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, 0.0f, zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, (GLfloat)(-height), zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.0f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(1.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, (GLfloat)(-height), zEnd);

		/* top face */
		glNormal3f(0.0f, 1.0f, 0.0f);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.55f);
			break;

		case INDOOR_TABLE_KEYBOARD:
			glTexCoord2f(0.06f, 0.63f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, 0.0f, zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.55f);
			break;

		case INDOOR_TABLE_KEYBOARD:
			glTexCoord2f(0.06f, 0.23f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, 0.0f, zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.45f);
			break;

		case INDOOR_TABLE_KEYBOARD:
			glTexCoord2f(0.94f, 0.23f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, 0.0f, zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.45f);
			break;

		case INDOOR_TABLE_KEYBOARD:
			glTexCoord2f(0.94f, 0.63f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, 0.0f, zStart);

		/* bottom face */
		glNormal3f(0.0f, -1.0f, 0.0f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.45f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, (GLfloat)(-height), zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(2.0f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.45f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 1.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, (GLfloat)(-height), zStart);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 1.5f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.45f, 0.55f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(2.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xEnd, (GLfloat)(-height), zEnd);
		switch (object)
		{
		case INDOOR_TABLE_BODY:
			glTexCoord2f(0.0f, 0.0f);
			break;

		case INDOOR_TABLE_LEG:
			glTexCoord2f(0.55f, 0.55f);
			break;

		case OUTDOOR_BENCH:
			glTexCoord2f(0.0f, 0.0f);
			break;

		default:
			break;
		}
		glVertex3f(xStart, (GLfloat)(-height), zEnd);
	}
	glEnd();
}

/* functions that draw objects */
void drawHouse(void)
{
	// function prototypes
	void switchMaterial(const float *);
	void drawTexturedCuboid(float, float, float, int);
	void drawWallClock(void);
	void updateWallClock(void);

	// variable declarations
	const GLfloat xExtentHouse = 3.0f / 2.0f;
	const GLfloat yExtentHouse = 2.0f / 2.0f;
	const GLfloat zExtentHouse = 3.0f / 2.0f;

	const GLfloat xExtentDesk = 0.8f;
	const GLfloat yExtentDesk = 0.025f;
	const GLfloat zExtentDesk = 0.6f;

	static int clockTick = 0;

	const float materialWalls[] = {
		0.2f, 0.2f, 0.2f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		50.0f
	};

	const float materialMonitor[] = {
		0.1f, 0.1f, 0.1f, 1.0f,
		0.1f, 0.1f, 0.1f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f,
		50.0f
	};

	const float materialMouse[] = {
		0.0f, 0.0f, 0.0f, 1.0f,
		0.1f, 0.1f, 0.1f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f,
		32.0f
	};

	const float materialCupboard[] = {
		0.1f, 0.1f, 0.1f, 1.0f,
		0.494f, 0.482f, 0.462f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f,
		78.0f
	};

	const float materialSurfaces[] = {
		0.8f, 0.8f, 0.8f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.6f, 0.6f, 0.6f, 1.0f,
		78.0f
	};

	// code
	switchMaterial(materialWalls);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureHouseFloor);
	glBegin(GL_QUADS);
	{
		// front inside
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-xExtentHouse, -yExtentHouse, -zExtentHouse);
		glVertex3f( xExtentHouse, -yExtentHouse, -zExtentHouse);
		glVertex3f( xExtentHouse,  yExtentHouse, -zExtentHouse);
		glVertex3f(-xExtentHouse,  yExtentHouse, -zExtentHouse);

		// front outside
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(-xExtentHouse, -yExtentHouse, -zExtentHouse - 0.001f);
		glVertex3f(-xExtentHouse, yExtentHouse, -zExtentHouse - 0.001f);
		glVertex3f(xExtentHouse, yExtentHouse, -zExtentHouse - 0.001f);
		glVertex3f(xExtentHouse, -yExtentHouse, -zExtentHouse - 0.001f);

		// left inside
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-xExtentHouse, -yExtentHouse,  zExtentHouse);
		glVertex3f(-xExtentHouse, -yExtentHouse, -zExtentHouse);
		glVertex3f(-xExtentHouse,  yExtentHouse, -zExtentHouse);
		glVertex3f(-xExtentHouse,  yExtentHouse,  zExtentHouse);

		// left outside
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-xExtentHouse - 0.001f, -yExtentHouse, zExtentHouse);
		glVertex3f(-xExtentHouse - 0.001f, yExtentHouse, zExtentHouse);
		glVertex3f(-xExtentHouse - 0.001f, yExtentHouse, -zExtentHouse);
		glVertex3f(-xExtentHouse - 0.001f, -yExtentHouse, -zExtentHouse);

		// rear inside
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f( xExtentHouse, -yExtentHouse,  zExtentHouse);
		glVertex3f(-xExtentHouse, -yExtentHouse,  zExtentHouse);
		glVertex3f(-xExtentHouse,  yExtentHouse,  zExtentHouse);
		glVertex3f( xExtentHouse,  yExtentHouse,  zExtentHouse);

		// rear outside
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(xExtentHouse, -yExtentHouse, zExtentHouse + 0.001f);
		glVertex3f(xExtentHouse, yExtentHouse, zExtentHouse + 0.001f);
		glVertex3f(-xExtentHouse, yExtentHouse, zExtentHouse + 0.001f);
		glVertex3f(-xExtentHouse, -yExtentHouse, zExtentHouse + 0.001f);

		// right inside 1
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse, yExtentHouse, -zExtentHouse);
		glVertex3f(xExtentHouse, yExtentHouse, zExtentHouse);
		glVertex3f(xExtentHouse, 0.2f, zExtentHouse);
		glVertex3f(xExtentHouse, 0.2f, -zExtentHouse);

		// right outside 1
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse + 0.001f, yExtentHouse, -zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, 0.2f, -zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, 0.2f, zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, yExtentHouse, zExtentHouse);

		// right inside 2
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse,  0.2f, -zExtentHouse);
		glVertex3f(xExtentHouse, -0.5f, -zExtentHouse);
		glVertex3f(xExtentHouse, -0.5f, -zExtentHouse + 0.15f);
		glVertex3f(xExtentHouse,  0.2f, -zExtentHouse + 0.15f);

		// right outside 2
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse + 0.001f, 0.2f, -zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, 0.2f, -zExtentHouse + 0.15f);
		glVertex3f(xExtentHouse + 0.001f, -0.5f, -zExtentHouse + 0.15f);
		glVertex3f(xExtentHouse + 0.001f, -0.5f, -zExtentHouse);

		// right inside 3
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse, -yExtentHouse, -zExtentHouse);
		glVertex3f(xExtentHouse, -yExtentHouse, zExtentHouse);
		glVertex3f(xExtentHouse, -0.5f, zExtentHouse);
		glVertex3f(xExtentHouse, -0.5f, -zExtentHouse);

		// right outside 3
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse + 0.001f, -yExtentHouse, -zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, -0.5f, -zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, -0.5f, zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, -yExtentHouse, zExtentHouse);

		/* glVertex3f(xExtentHouse, 0.2f, -zExtentHouse + 0.15f);
		glVertex3f(xExtentHouse, -0.5f, -zExtentHouse + 0.15f);
		glVertex3f(xExtentHouse, -0.5f, -zExtentHouse + 1.5f);
		glVertex3f(xExtentHouse,  0.2f, -zExtentHouse + 1.5f); */

		// right inside 4
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse,  0.2f, -zExtentHouse + 1.4f);
		glVertex3f(xExtentHouse, -0.5f, -zExtentHouse + 1.4f);
		glVertex3f(xExtentHouse, -0.5f,  zExtentHouse);
		glVertex3f(xExtentHouse,  0.2f,  zExtentHouse);

		// right outside 4
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse + 0.001f, 0.2f, -zExtentHouse + 1.4f);
		glVertex3f(xExtentHouse + 0.001f, 0.2f, zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, -0.5f, zExtentHouse);
		glVertex3f(xExtentHouse + 0.001f, -0.5f, -zExtentHouse + 1.4f);

		// bottom
		glNormal3f(0.0f, 1.0f, 0.0f);		
		glTexCoord2f(4, 4);
		glVertex3f( xExtentHouse, -yExtentHouse, -zExtentHouse);
		glTexCoord2f(0, 4);
		glVertex3f(-xExtentHouse, -yExtentHouse, -zExtentHouse);
		glTexCoord2f(0, 0);
		glVertex3f(-xExtentHouse, -yExtentHouse,  zExtentHouse);
		glTexCoord2f(4, 0);
		glVertex3f( xExtentHouse, -yExtentHouse,  zExtentHouse);

		// top inside
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(-xExtentHouse, yExtentHouse, -zExtentHouse);
		glVertex3f( xExtentHouse, yExtentHouse, -zExtentHouse);
		glVertex3f( xExtentHouse, yExtentHouse,  zExtentHouse);
		glVertex3f(-xExtentHouse, yExtentHouse,  zExtentHouse);

		// top outside
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(-xExtentHouse, yExtentHouse + 0.001f, -zExtentHouse);
		glVertex3f(-xExtentHouse, yExtentHouse + 0.001f, zExtentHouse);
		glVertex3f(xExtentHouse, yExtentHouse + 0.001f, zExtentHouse);
		glVertex3f(xExtentHouse, yExtentHouse + 0.001f, -zExtentHouse);

		// the lower house
		glTranslatef(0.0f, -yExtentHouse, 0.0f);
		// drawTexturedCuboid(3.0f, 0.025f, 4.0f, -1);

		// front
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(-xExtentHouse, -2.2f, -zExtentHouse);
		glVertex3f(-xExtentHouse, -1.005f, -zExtentHouse);
		glVertex3f(xExtentHouse, -1.005f, -zExtentHouse);
		glVertex3f(xExtentHouse, -2.2f, -zExtentHouse);

		// right
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(xExtentHouse, -1.01f, -zExtentHouse);
		glVertex3f(xExtentHouse, -1.01f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -2.2f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -2.2f, -zExtentHouse);

		glVertex3f(xExtentHouse, -0.5f, zExtentHouse);
		glVertex3f(xExtentHouse, -0.5f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -1.0f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -1.0f, zExtentHouse);

		// rear
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-xExtentHouse, -2.2f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -2.2f,  zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -1.01f, zExtentHouse + 1.0f);
		glVertex3f(-xExtentHouse, -1.01f, zExtentHouse + 1.0f);

		glVertex3f(-xExtentHouse, -0.5f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -0.5f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -1.0f, zExtentHouse + 1.0f);
		glVertex3f(-xExtentHouse, -1.0f, zExtentHouse + 1.0f);

		// left
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-xExtentHouse, -1.01f, -zExtentHouse);
		glVertex3f(-xExtentHouse, -2.2f, -zExtentHouse);
		glVertex3f(-xExtentHouse, -2.2f, zExtentHouse + 1.0f);
		glVertex3f(-xExtentHouse, -1.01f, zExtentHouse + 1.0f);

		glVertex3f(-xExtentHouse, -0.5f, zExtentHouse);
		glVertex3f(-xExtentHouse, -1.0f, zExtentHouse);
		glVertex3f(-xExtentHouse, -1.0f, zExtentHouse + 1.0f);
		glVertex3f(-xExtentHouse, -0.5f, zExtentHouse + 1.0f);

		// balcony floor
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-xExtentHouse, -1.0f, zExtentHouse);
		glVertex3f(-xExtentHouse, -1.0f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -1.0f, zExtentHouse + 1.0f);
		glVertex3f(xExtentHouse, -1.0f, zExtentHouse);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	// wall clock
	glPushMatrix();
	{
		glTranslatef(0.0f, yExtentHouse - 0.7f, -zExtentHouse);
		
		glPushMatrix();
		{
			gluCylinder(quadric, 0.18f, 0.18f, 0.025f, 10, 10);
			glTranslatef(0.0f, 0.0f, 0.025f);
			glBegin(GL_POLYGON);
			{
				for (int i = 18; i <= 360; i = i + 36)
					glVertex3f(0.18f * cosf(DEG_TO_RAD(i)), 0.18f * sinf(DEG_TO_RAD(i)), 0.0f);
			}
			glEnd();
		}
		glPopMatrix();

		glTranslatef(0.0f, 0.0f, 0.027f);
		drawWallClock();

		if (elapsedSeconds - clockTick != 0)
		{
			updateWallClock();
			clockTick = elapsedSeconds;
		}
	}
	glPopMatrix();

	// desk
	glBindTexture(GL_TEXTURE_2D, textureTable);
	glPushMatrix();
	{
		glTranslatef(-xExtentDesk / 2.0f, -0.65f, -zExtentHouse);
		drawTexturedCuboid(xExtentDesk, yExtentDesk, zExtentDesk, INDOOR_TABLE_BODY);

		// monitor
		glBindTexture(GL_TEXTURE_2D, 0);
		switchMaterial(materialMonitor);
		glPushMatrix();
		{
			glTranslatef(0.15f, 0.35f, 0.0f);
			drawTexturedCuboid(xExtentDesk - 0.3f, 0.3f, 0.025f, -1);

			glTranslatef(0.01f, -0.01f, 0.026f);
			switchMaterial(materialSurfaces);
			glBindTexture(GL_TEXTURE_2D, textureScreen);
			glBegin(GL_QUADS);
			{
				glNormal3f(0.0f, 0.0f, 1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(0.0f, -0.28f, 0.0f);
				
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(0.48f, -0.28f, 0.0f);
				
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(0.48f, 0.0f, 0.0f);
			}
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			switchMaterial(materialWalls);
		}
		glPopMatrix();
		switchMaterial(materialWalls);
		glBindTexture(GL_TEXTURE_2D, textureTable);

		// keyboard
		glBindTexture(GL_TEXTURE_2D, textureKeyboard);
		glPushMatrix();
		{
			glTranslatef(0.15f, 0.0125f, 0.3f);
			glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
			drawTexturedCuboid(xExtentDesk - 0.45f, 0.0125f, 0.125f, INDOOR_TABLE_KEYBOARD);
		}
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, textureTable);

		// mouse
		glBindTexture(GL_TEXTURE_2D, 0);
		switchMaterial(materialMouse);
		glPushMatrix();
		{			
			glTranslatef(0.65f, 0.0125f, 0.4f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			glRotatef(-15.0f, 0.0f, 0.0f, 1.0f);
			glScalef(1.0f, 2.0f, 1.0f);
			gluCylinder(quadric, 0.02f, 0.02f, 0.01f, 10, 10);
			glScalef(1.0f, 1.0f, 0.5f);
			gluSphere(quadric, 0.02f, 10, 10);
		}
		glPopMatrix();
		switchMaterial(materialWalls);
		glBindTexture(GL_TEXTURE_2D, textureTable);
		
		// right-front leg
		glPushMatrix();
		{
			glTranslatef(xExtentDesk - 0.025f, -yExtentDesk, zExtentDesk - 0.025f);
			drawTexturedCuboid(0.025f, 0.325f, 0.025f, INDOOR_TABLE_LEG);
		}
		glPopMatrix();

		// right-rear leg
		glPushMatrix();
		{
			glTranslatef(xExtentDesk - 0.025f, -yExtentDesk, 0.0f);
			drawTexturedCuboid(0.025f, 0.325f, 0.025f, INDOOR_TABLE_LEG);
		}
		glPopMatrix();

		// left-rear leg
		glPushMatrix();
		{
			glTranslatef(0.0f, -yExtentDesk, 0.0f);
			drawTexturedCuboid(0.025f, 0.325f, 0.025f, INDOOR_TABLE_LEG);
		}
		glPopMatrix();

		// left-front leg
		glPushMatrix();
		{
			glTranslatef(0.0f, -yExtentDesk, zExtentDesk - 0.025f);
			drawTexturedCuboid(0.025f, 0.325f, 0.025f, INDOOR_TABLE_LEG);
		}
		glPopMatrix();
	}
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);

	// chair
	glBindTexture(GL_TEXTURE_2D, textureTable);
	glPushMatrix();
	{
		glTranslatef(-0.125f, -0.75f, -zExtentHouse + zExtentDesk + 0.3f);
		glRotatef(15.0f, 0.0f, 1.0f, 0.0f);
		drawTexturedCuboid(0.25f, 0.025f, 0.25f, INDOOR_TABLE_BODY);

		// right-rear leg
		glPushMatrix();
		{
			glTranslatef(0.225f, -0.025f, 0.0f);
			drawTexturedCuboid(0.025f, 0.18f, 0.025f, INDOOR_TABLE_BODY);
		}
		glPopMatrix();

		// left-rear leg
		glPushMatrix();
		{
			glTranslatef(0.0f, -0.025f, 0.0f);
			drawTexturedCuboid(0.025f, 0.18f, 0.025f, INDOOR_TABLE_BODY);
		}
		glPopMatrix();

		// left-front leg
		glPushMatrix();
		{
			glTranslatef(0.0f, -0.025f, 0.225f);
			drawTexturedCuboid(0.025f, 0.18f, 0.025f, INDOOR_TABLE_BODY);
		}
		glPopMatrix();

		// right-front leg
		glPushMatrix();
		{
			glTranslatef(0.225f, -0.025f, 0.225f);
			drawTexturedCuboid(0.025f, 0.18f, 0.025f, INDOOR_TABLE_BODY);
		}
		glPopMatrix();
	}
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);

	// cupboard
	switchMaterial(materialCupboard);
	glPushMatrix();
	{
		glTranslatef(-xExtentHouse, -yExtentHouse + 1.0f, -zExtentHouse + 0.01f);
		drawTexturedCuboid(0.325f, 1.0f, 0.6f, -1);

		// left door
		glPushMatrix();
		{
			glTranslatef(0.33f, 0.0f, 0.29f);
			drawTexturedCuboid(0.025f, 1.0f, 0.31f, -1);

			// left door knob
			glTranslatef(0.04f, -0.5f, 0.05f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			gluSphere(quadric, 0.02f, 10, 10);
		}
		glPopMatrix();

		// right door
		glPushMatrix();
		{
			glTranslatef(0.33f, 0.0f, -0.01f);
			drawTexturedCuboid(0.025f, 1.0f, 0.29f, -1);

			// right door knob
			glTranslatef(0.04f, -0.5f, 0.25f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			gluSphere(quadric, 0.02f, 10, 10);
		}
		glPopMatrix();
	}
	glPopMatrix();

	// front wall decoration
	switchMaterial(materialSurfaces);
	glBindTexture(GL_TEXTURE_2D, textureAbhyasaMantra);
	glBegin(GL_QUADS);
	{
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-0.4f, 0.08f, -zExtentHouse + 0.0001f);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-0.4f, -0.08f, -zExtentHouse + 0.0001f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(0.4f, -0.08f, -zExtentHouse + 0.0001f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(0.4f, 0.08f, -zExtentHouse + 0.0001f);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawWallClock(void)
{
	// code
	glDisable(GL_LIGHTING);
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	{
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);

		// points separated by 30 degrees (1 hour/5 minutes)
		for (int i = 0; i < 360; i = i + 30)
			glVertex3f(0.135f * cosf(DEG_TO_RAD(i)), 0.135f * sinf(DEG_TO_RAD(i)), 0.0f);
	}
	glEnd();

	glPointSize(2.0f);
	glBegin(GL_POINTS);
	{
		// points separated by 5 degrees (1 second)
		for (int i = 0; i < 360; i = i + 6)
		{
			if (i % 30 != 0)
				glVertex3f(0.135f * cosf(DEG_TO_RAD(i)), 0.135f * sinf(DEG_TO_RAD(i)), 0.0f);
		}
	}
	glEnd();

	glScalef(0.25f, 0.25f, 1.0f);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	{
		// second hand
		glColor3f(0.0f, 0.0f, 0.0f);

		glVertex3f(0.4f * cosf(DEG_TO_RAD(secondHandAngle)), 0.4f * sinf(DEG_TO_RAD(secondHandAngle)), 0.0f);
		glVertex3f(-0.08f * cosf(DEG_TO_RAD(secondHandAngle)), -0.08f * sinf(DEG_TO_RAD(secondHandAngle)), 0.0f);
	}
	glEnd();
	glEnable(GL_LIGHTING);

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	{
		// minute hand
		glVertex3f(0.45f * cosf(DEG_TO_RAD(minuteHandAngle)), 0.45f * sinf(DEG_TO_RAD(minuteHandAngle)), 0.0f);
		glVertex3f(0.045 * cosf(DEG_TO_RAD(minuteHandAngle + 45.0f)), 0.045 * sinf(DEG_TO_RAD(minuteHandAngle + 45.0f)), 0.0f);

		glVertex3f(0.045 * cosf(DEG_TO_RAD(minuteHandAngle + 45.0f)), 0.045 * sinf(DEG_TO_RAD(minuteHandAngle + 45.0f)), 0.0f);
		glVertex3f(-0.1f * cosf(DEG_TO_RAD(minuteHandAngle)), -0.1f * sinf(DEG_TO_RAD(minuteHandAngle)), 0.0f);

		glVertex3f(-0.1f * cosf(DEG_TO_RAD(minuteHandAngle)), -0.1f * sinf(DEG_TO_RAD(minuteHandAngle)), 0.0f);
		glVertex3f(0.045 * cosf(DEG_TO_RAD(minuteHandAngle - 45.0f)), 0.045 * sinf(DEG_TO_RAD(minuteHandAngle - 45.0f)), 0.0f);

		glVertex3f(0.045 * cosf(DEG_TO_RAD(minuteHandAngle - 45.0f)), 0.045 * sinf(DEG_TO_RAD(minuteHandAngle - 45.0f)), 0.0f);
		glVertex3f(0.45f * cosf(DEG_TO_RAD(minuteHandAngle)), 0.45f * sinf(DEG_TO_RAD(minuteHandAngle)), 0.0f);

		// hour hand
		glVertex3f(0.35f * cosf(DEG_TO_RAD(hourHandAngle)), 0.35f * sinf(DEG_TO_RAD(hourHandAngle)), 0.0f);
		glVertex3f(0.035 * cosf(DEG_TO_RAD(hourHandAngle + 45.0f)), 0.035 * sinf(DEG_TO_RAD(hourHandAngle + 45.0f)), 0.0f);

		glVertex3f(0.035 * cosf(DEG_TO_RAD(hourHandAngle + 45.0f)), 0.035 * sinf(DEG_TO_RAD(hourHandAngle + 45.0f)), 0.0f);
		glVertex3f(-0.1f * cosf(DEG_TO_RAD(hourHandAngle)), -0.1f * sinf(DEG_TO_RAD(hourHandAngle)), 0.0f);

		glVertex3f(-0.1f * cosf(DEG_TO_RAD(hourHandAngle)), -0.1f * sinf(DEG_TO_RAD(hourHandAngle)), 0.0f);
		glVertex3f(0.035 * cosf(DEG_TO_RAD(hourHandAngle - 45.0f)), 0.035 * sinf(DEG_TO_RAD(hourHandAngle - 45.0f)), 0.0f);

		glVertex3f(0.035 * cosf(DEG_TO_RAD(hourHandAngle - 45.0f)), 0.035 * sinf(DEG_TO_RAD(hourHandAngle - 45.0f)), 0.0f);
		glVertex3f(0.35f * cosf(DEG_TO_RAD(hourHandAngle)), 0.35f * sinf(DEG_TO_RAD(hourHandAngle)), 0.0f);
	}
	glEnd();

	glScalef(1.14f, 1.14f, 1.0f);
	glBegin(GL_LINES);
	{
		// circle
		for (float i = 0.0f; i < 360.0f; i = i + 0.5f)
		{
			glVertex3f(0.53f * cosf(DEG_TO_RAD(i)), 0.53f * sinf(DEG_TO_RAD(i)), 0.0f);
			glVertex3f(0.56f * cosf(DEG_TO_RAD(i)), 0.56f * sinf(DEG_TO_RAD(i)), 0.0f);
		}
	}
	glEnd();
}

void updateWallClock(void)
{
	// code
	secondHandAngle = (secondHandAngle - 6);
	if (secondHandAngle < -(360 - INITIAL_ANGLE_SECOND_HAND - 4))
		secondHandAngle = INITIAL_ANGLE_SECOND_HAND;

	minuteHandAngle = minuteHandAngle - (4.0f / 60.0f);
	if (minuteHandAngle < -(360.0f - INITIAL_ANGLE_MINUTE_HAND - 0.1f))
		minuteHandAngle = INITIAL_ANGLE_MINUTE_HAND;

	hourHandAngle = hourHandAngle - (4.0f / (60.0f * 12.0f));
	if (hourHandAngle < -(360.0f - INITIAL_ANGLE_MINUTE_HAND - 0.1f))
		hourHandAngle = INITIAL_ANGLE_MINUTE_HAND;
}

void drawHumanoid(void)
{
	// function prototypes
	void switchMaterial(const float *);

	// local variables
	float materialSkin[] = {
		0.72f, 0.62f, 0.42f, 1.0f,  // ambient
		0.20f, 0.20f, 0.20f, 1.0f,  // diffuse
		0.20f, 0.20f, 0.20f, 1.0f,  // specular
		20.0f  // shininess
	};

	float materialShirtUpper[] = {
		0.01f, 0.03f, 0.2f, 1.0f,  // ambient
		0.20f, 0.20f, 0.20f, 1.0f,  // diffuse
		0.20f, 0.20f, 0.20f, 1.0f,  // specular
		80.0f  // shininess
	};

	float materialShirtLower[] = {
		0.01f, 0.03f, 0.15f, 1.0f,  // ambient
		0.20f, 0.20f, 0.20f, 1.0f,  // diffuse
		0.20f, 0.20f, 0.20f, 1.0f,  // specular
		80.0f  // shininess
	};

	float materialTrousers[] = {
		0.07f, 0.07f, 0.07f, 1.0f,  // ambient
		0.20f, 0.20f, 0.20f, 1.0f,  // diffuse
		0.20f, 0.20f, 0.20f, 1.0f,  // specular
		100.0f  // shininess
	};

	float materialShoes[] = {
		0.01f, 0.01f, 0.01f, 1.0f,  // ambient
		0.20f, 0.20f, 0.20f, 1.0f,  // diffuse
		0.50f, 0.50f, 0.50f, 1.0f,  // specular
		200.0f  // shininess
	};

	// code
	// face
	glPushMatrix();
	{
		switchMaterial(materialSkin);
		glPushMatrix();
		{
			glTranslatef(0.0f, 0.4f, 0.0f);
			glRotatef(90.0f, 0.0f, -1.0f, 0.0f);

			glRotatef(angleHeadX, 0.0f, 0.0f, 1.0f);  /* head control */
			glRotatef(angleHeadY, 0.0f, 1.0f, 0.0f);
			glPushMatrix();
			{
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
				glScalef(1.0f, 1.0f, 1.1f);
				gluSphere(quadric, 0.1f, 20, 20);

				// mouth
				glDisable(GL_LIGHTING);
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glColor3f(0.0f, 0.0f, 0.0f);
					glVertex3f(0.0866f, -0.005f, 0.05f);
					glVertex3f(0.0866f, 0.005f, 0.05f);
				}
				glEnd();
				glEnable(GL_LIGHTING);
			}
			glPopMatrix();

			// nose
			glPushMatrix();
			{
				glTranslatef(0.1f, 0.0f, 0.0f);
				gluSphere(quadric, 0.01f, 10, 10);
			}
			glPopMatrix();
		}
		glPopMatrix();

		// torso
		glPushMatrix();
		{
			glTranslatef(0.0f, 0.2f, 0.0f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

			switchMaterial(materialShirtUpper);
			gluSphere(quadric, 0.1f, 20, 20);

			switchMaterial(materialShirtLower);
			gluCylinder(quadric, 0.1f, 0.08f, 0.2f, 20, 20);
		}
		glPopMatrix();

		// right hand
		glPushMatrix();
		{
			// forearm
			glPushMatrix();
			{
				glTranslatef(-0.12f, 0.21f, 0.0f);
				glRotatef(angleShoulderX, 1.0f, 0.0f, 0.0f);	 /* right shoulder X control */
				glRotatef(angleShoulderZ, 0.0f, 0.0f, 1.0f);	 /* right shoulder Z control */
				glPushMatrix();
				{
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

					switchMaterial(materialShirtUpper);
					gluSphere(quadric, 0.04f, 10, 10);

					switchMaterial(materialSkin);
					gluCylinder(quadric, 0.02f, 0.02f, 0.12f, 10, 10);
				}
				glPopMatrix();

				// elbow
				glTranslatef(0.0f, -0.12f, 0.0f);
				glRotatef(angleElbowX, 1.0f, 0.0f, 0.0f);  /* right elbow X control */
				glRotatef(angleElbowZ, 0.0f, 0.0f, 1.0f);  /* right elbow Z control */
				glPushMatrix();
				{
					glPushMatrix();
					{
						glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
						gluSphere(quadric, 0.025f, 10, 10);
					}
					glPopMatrix();

					// arm
					glTranslatef(0.0f, -0.01f, 0.0f);
					glPushMatrix();
					{
						glPushMatrix();
						{
							glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
							gluCylinder(quadric, 0.02f, 0.01f, 0.1f, 10, 10);

							// palm
							glTranslatef(0.0f, 0.0f, 0.1f);
							gluSphere(quadric, 0.02f, 10, 10);
						}
						glPopMatrix();
					}
					glPopMatrix();
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		// left hand
		glPushMatrix();
		{
			// forearm
			glPushMatrix();
			{
				glTranslatef(0.12f, 0.21f, 0.0f);
				glRotatef(angleShoulderX, 1.0f, 0.0f, 0.0f);	 /* left shoulder X control */
				glRotatef(-angleShoulderZ, 0.0f, 0.0f, 1.0f);	 /* left shoulder Z control */
				glPushMatrix();
				{
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

					switchMaterial(materialShirtUpper);
					gluSphere(quadric, 0.04f, 10, 10);

					switchMaterial(materialSkin);
					gluCylinder(quadric, 0.02f, 0.02f, 0.12f, 10, 10);
				}
				glPopMatrix();

				// elbow
				glTranslatef(0.0f, -0.12f, 0.0f);
				glRotatef(angleElbowX, 1.0f, 0.0f, 0.0f);  /* left elbow X control */
				glRotatef(-angleElbowZ, 0.0f, 0.0f, 1.0f);  /* left elbow Z control */
				glPushMatrix();
				{
					glPushMatrix();
					{
						glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
						gluSphere(quadric, 0.025f, 10, 10);
					}
					glPopMatrix();

					// arm
					glTranslatef(0.0f, -0.01f, 0.0f);
					glPushMatrix();
					{
						glPushMatrix();
						{
							glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
							gluCylinder(quadric, 0.02f, 0.01f, 0.1f, 10, 10);

							// palm
							glTranslatef(0.0f, 0.0f, 0.1f);
							gluSphere(quadric, 0.02f, 10, 10);
						}
						glPopMatrix();
					}
					glPopMatrix();
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		// right leg
		switchMaterial(materialTrousers);
		glPushMatrix();
		{
			glTranslatef(-0.035f, 0.0f, 0.0f);
			glRotatef(angleButtocks, 1.0f, 0.0f, 0.0f);  /* right buttock control */

			// fibioid
			glPushMatrix();
			{
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
				gluSphere(quadric, 0.045f, 10, 10);
				gluCylinder(quadric, 0.03f, 0.02f, 0.15f, 10, 10);

				// knee
				glTranslatef(0.0f, 0.0f, 0.15f);
				glRotatef(angleKnee, 1.0f, 0.0f, 0.0f);  /* right knee control */
				gluSphere(quadric, 0.025f, 10, 10);

				// tibioid
				glPushMatrix();
				{
					gluCylinder(quadric, 0.015f, 0.015f, 0.15f, 10, 10);

					// foot
					switchMaterial(materialShoes);
					glTranslatef(0.0f, 0.0f, 0.15f);
					gluSphere(quadric, 0.02f, 10, 10);
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		// left leg
		switchMaterial(materialTrousers);
		glPushMatrix();
		{
			glTranslatef(0.035f, 0.0f, 0.0f);
			glRotatef(angleButtocks, 1.0f, 0.0f, 0.0f);  /* left buttock control */

			// fibioid
			glPushMatrix();
			{
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
				gluSphere(quadric, 0.045f, 10, 10);
				gluCylinder(quadric, 0.03f, 0.02f, 0.15f, 10, 10);

				// knee
				glTranslatef(0.0f, 0.0f, 0.15f);
				glRotatef(angleKnee, 1.0f, 0.0f, 0.0f);  /* left knee control */
				gluSphere(quadric, 0.025f, 10, 10);

				// tibioid
				glPushMatrix();
				{
					gluCylinder(quadric, 0.015f, 0.015f, 0.15f, 10, 10);

					// foot
					switchMaterial(materialShoes);
					glTranslatef(0.0f, 0.0f, 0.15f);
					gluSphere(quadric, 0.02f, 10, 10);
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();
}

/* functions updating individual objects' states */
void updateHumanoid(void)
{
	// code
	switch (scene)
	{
	case 5:
		angleHeadX = -25.0f;
		angleShoulderX = -50.0f;
		angleShoulderZ = 15.0f;
		angleElbowZ = 70.0f;
		angleButtocks = -90.0f;
		angleKnee = 90.0f;
		break;
	}
}

/*
void updateHumanoid_Walk(int quarterWalkCycleResolution)
{
	// variable declarations
	const float maxAngleButtocks = 30.0f;
	const float deltaAngleButtocks = maxAngleButtocks / quarterWalkCycleResolution;

	const float maxAngleKnee = 20.0f;
	const float deltaAngleKnee = maxAngleKnee / quarterWalkCycleResolution;

	const float maxAngleShoulder = 30.0f;
	const float deltaAngleShoulder = maxAngleShoulder / quarterWalkCycleResolution;

	static int currentWalkCycle = 1;

	// code
	if (currentWalkCycle == 1)  /* 1st quarter walk cycle 
	{
		angleButtocks += deltaAngleButtocks;

		if (angleKnee > 0.0f)
		{
			angleKnee -= deltaAngleKnee;
		}

		if (angleShoulder < maxAngleShoulder)
		{
			angleShoulder += deltaAngleShoulder;
		}

		if (maxAngleButtocks - angleButtocks <= 0.01f)
			currentWalkCycle = 2;
	}
	else if (currentWalkCycle == 2)  /* 2nd quarter walk cycle
	{
		angleButtocks -= deltaAngleButtocks;

		if (angleKnee < maxAngleKnee)
		{
			angleKnee += deltaAngleKnee;
		}

		if (angleShoulder > 0.0f)
		{
			angleShoulder -= deltaAngleShoulder;
		}

		if (angleButtocks <= 0.01f)
			currentWalkCycle = 3;
	}
	else if (currentWalkCycle == 3)  /* 3rd quarter walk cycle
	{
		angleButtocks -= deltaAngleButtocks;

		if (angleKnee < maxAngleKnee)
		{
			angleKnee += deltaAngleKnee;
		}

		if (angleShoulder > -maxAngleShoulder)
		{
			angleShoulder -= deltaAngleShoulder;
		}

		if (maxAngleButtocks + angleButtocks <= 0.01f)
			currentWalkCycle = 4;
	}
	else if (currentWalkCycle == 4)  /* last quarter walk cycle
	{
		angleButtocks += deltaAngleButtocks;

		if (angleKnee > 0.0f)
		{
			angleKnee -= deltaAngleKnee;
		}

		if (angleShoulder < 0.0f)
		{
			angleShoulder += deltaAngleShoulder;
		}

		if (angleButtocks >= -0.01f)
			currentWalkCycle = 1;
	}
} */

/* 
 * `dim' dimensional parameteric interpolator: 
 * when t <= 0.0f, vOut = vIn1;
 * when t >= 1.0f, vOut = vIn2;
 * when 0.0f < t < 1.0f, vOut = `dim' dimensional position of a linearly interpolated point on the line connecting vIn1 and vIn2.
 */
void mix(const float *vIn1, const float *vIn2, const float t, float *vOut, const size_t dim)
{
	// code
	if (t < 0.0f)
		vOut = (float *)vIn1;
	else if (t > 1.0f)
		vOut = (float *)vIn2;

	for (int i = 0; i < dim; i++)
	{
		vOut[i] = (1 - t) * vIn1[i] + t * vIn2[i];
	}
}

void drawSky(void)
{
	// variable declarations
	static float colorModifierHigherClouds_Red = 0.0f;
	static float colorModifierHigherClouds_Green = 0.0f;
	static float colorModifierHigherClouds_Blue = 0.0f;

	static float colorModifierLowerClouds_Red = 0.0f;
	static float colorModifierLowerClouds_Green = 0.0f;
	static float colorModifierLowerClouds_Blue = 0.0f;

	// code
	if (scene == 7)
	{
		if (colorModifierLowerClouds_Red < 0.5f)
			colorModifierLowerClouds_Red += 0.02f;
		if (colorModifierLowerClouds_Green < 0.1f)
			colorModifierLowerClouds_Green += 0.01f;
		if (colorModifierLowerClouds_Blue < 0.0f)
			colorModifierLowerClouds_Blue += 0.01f;

		if(colorModifierHigherClouds_Red < 0.0f)
			colorModifierHigherClouds_Red += 0.001f;
		if (colorModifierHigherClouds_Green < 0.1f)
			colorModifierHigherClouds_Green += 0.001f;
		if (colorModifierHigherClouds_Blue < 0.5f)
			colorModifierHigherClouds_Blue += 0.002f;
	}
	else if (scene == 11)
	{
		if (colorModifierLowerClouds_Red < 0.7f)
			colorModifierLowerClouds_Red += 0.005f;
		if (colorModifierLowerClouds_Green < 0.3f)
			colorModifierLowerClouds_Green += 0.0025f;
		if (colorModifierLowerClouds_Blue < 0.1f)
			colorModifierLowerClouds_Blue += 0.001f;

		if (colorModifierHigherClouds_Blue < 0.05f)
			colorModifierHigherClouds_Blue += 0.001f;
	}
	else
	{
		colorModifierLowerClouds_Red = 0.0f;
		colorModifierLowerClouds_Green = 0.0f;
		colorModifierLowerClouds_Blue = 0.0f;

		colorModifierHigherClouds_Red = 0.0f;
		colorModifierHigherClouds_Green = 0.0f;
		colorModifierHigherClouds_Blue = 0.0f;
	}

	glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
	{
		// east
		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(50.0f, -1.0f, -50.0f);

		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(50.0f, -1.0f, 50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(50.0f, 30.0f,  50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(50.0f, 30.0f, -50.0f);

		// north
		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(50.0f, -1.0f, -50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(50.0f, 30.0f, -50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(-50.0f, 30.0f, -50.0f);

		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(-50.0f, -1.0f, -50.0f);

		// west
		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(-50.0f, -1.0f, -50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(-50.0f, 30.0f, -50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(-50.0f, 30.0f, 50.0f);

		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(-50.0f, -1.0f, 50.0f);

		// south
		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(50.0f, -1.0f, 50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(50.0f, 30.0f, 50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(-50.0f, 30.0f, 50.0f);

		glColor3f(0.7f - colorModifierLowerClouds_Red, 0.3f - colorModifierLowerClouds_Green, 0.1f - colorModifierLowerClouds_Blue);
		glVertex3f(-50.0f, -1.0f, 50.0f);

		// up
		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(50.0f, 30.0f, 50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(-50.0f, 30.0f, 50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(-50.0f, 30.0f, -50.0f);

		glColor3f(0.1f - colorModifierHigherClouds_Red, 0.2f - colorModifierHigherClouds_Green, 0.7f - colorModifierHigherClouds_Blue);
		glVertex3f(50.0f, 30.0f, -50.0f);
	}
	glEnd();
	glEnable(GL_LIGHTING);
}

void drawGround(void)
{
	// code
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureGrass);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-50.0f, 0.0f, 50.f);

		glTexCoord2f(100.0f, 0.0f);
		glVertex3f(50.0f, 0.0f, 50.0f);

		glTexCoord2f(100.0f, 100.0f);
		glVertex3f(50.0f, 0.0f, -50.0f);

		glTexCoord2f(0.0f, 100.0f);
		glVertex3f(-50.0f, 0.0f, -50.0f);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void drawBench(void)
{
	// function prototypes
	void drawTexturedCuboid(float, float, float, int);

	// code
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureBench);
	{
		glTranslatef(6.0f, 0.25f, 1.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		drawTexturedCuboid(0.8f, 0.25f, 0.4f, OUTDOOR_BENCH);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void generateTree(int iterations)
{
	// variable declarations
	Sequence *seq = NULL;
	size_t size = 0U;

	// code
	if (iterations > MAX_ITERATIONS_FRACTAL_TREE)
		iterations = MAX_ITERATIONS_FRACTAL_TREE;

	lsGenSequence(&lSystem, iterations, &seq);
	{
		size = lsSequenceSize() + 1;

		if (size > MAX_SEQUENCE_LENGTH)
		{
			fprintf(gpLog, "sequence length exceeded: %zd\n", size);
			size = MAX_SEQUENCE_LENGTH;
		}

		lsSequenceString(seq, size, drawingInstructions);
		sequenceLength = size;
	}
	lsDestroySequence(&seq);
}

void productionRulesTree(SequenceNode **ppNode)
{
	// variable declarations
	static int currentIteration;

	// code
	switch (lsSequenceNodeSymbol(*ppNode))
	{
	case DRAW_FORWARD:
		lsReplaceSymbol(ppNode, DRAW_FORWARD);

		lsAddSymbol(ppNode, PUSH);
		{
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);
			lsAddSymbol(ppNode, PITCH_DOWN_DELTA);
			lsAddSymbol(ppNode, ROLL_LEFT_DELTA);
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);

			lsAddSymbol(ppNode, DRAW_FORWARD);
			lsAddSymbol(ppNode, DRAW_LEAF);
		}
		lsAddSymbol(ppNode, POP);

		lsAddSymbol(ppNode, PUSH);
		{
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);
			lsAddSymbol(ppNode, PITCH_DOWN_DELTA);
			lsAddSymbol(ppNode, ROLL_RIGHT_DELTA);
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);

			lsAddSymbol(ppNode, DRAW_FORWARD);
			lsAddSymbol(ppNode, DRAW_LEAF);
		}
		lsAddSymbol(ppNode, POP);

		lsAddSymbol(ppNode, PUSH);
		{
			lsAddSymbol(ppNode, PITCH_UP_DELTA);
			lsAddSymbol(ppNode, ROLL_RIGHT_DELTA);

			lsAddSymbol(ppNode, DRAW_FORWARD);
			lsAddSymbol(ppNode, DRAW_LEAF);
		}
		lsAddSymbol(ppNode, POP);
		break;

	case DRAW_LEAF:
		break;

	default:
		break;
	}
}

void drawTree(void)
{
	// function declarations
	void mix(const float *, const float *, const float, float *, const size_t);

	// variable declarations
	float branchBaseRadius = 0.07f;
	float branchTopRadius = 0.06f;
	float branchHeight = 0.6f;

	int cPushes = 0;
	int currentLeaf = 0;

	const float materialBranch[] = {
		0.3f, 0.1f, 0.0f, 1.0f,
		0.6f, 0.3f, 0.1f, 1.0f,
		0.3f, 0.3f, 0.3f, 1.0f,
		25.0f
	};

	const float materialLeafNormal[] = {
		0.2f, 0.5f, 0.1f, 1.0f,
		0.3f, 0.6f, 0.2f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f,
		30.0f
	};

	const float materialLeafBeautiful[] = {
		0.7f, 0.5f, 0.6f, 1.0f,
		0.6f, 0.4f, 0.5f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f,
		30.0f
	};

	float materialLeaf[13];
	static float t = 0.0f;

	// code
	for (int i = 0; i < sequenceLength; i++)
	{
		if(scene != 7)
			deltaDueToWind = (cPushes * windSpeed) / 150.0f;  /* this has to be THE worst hack of my life till date */
		else
			deltaDueToWind = (cPushes * windSpeed) / 5.0f;

		switch (drawingInstructions[i])
		{
		case DRAW_NULL:
			break;

		case DRAW_FORWARD:
			switchMaterial(materialBranch);
			glPushMatrix();
			{
				glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
				gluCylinder(quadric, branchBaseRadius, branchTopRadius, branchHeight, 10, 10);
			}
			glPopMatrix();

			glTranslatef(0.0f, branchHeight, 0.0f);
			break;

		case DRAW_LEAF:
			if (cPushes == growthIteration)  /* again, part of the worst hacks */
			{
				if (scene == 8)
				{
					if (isBeautifulLeaf[currentLeaf])
					{
						for (int i = 0; i < 12; i = i + 4)
						{
							materialLeaf[i + 0] = (1 - t) * materialLeafNormal[i] + t * materialLeafBeautiful[i];
							materialLeaf[i + 1] = (1 - t) * materialLeafNormal[i + 1] + t * materialLeafBeautiful[i + 1];
							materialLeaf[i + 2] = (1 - t) * materialLeafNormal[i + 2] + t * materialLeafBeautiful[i + 2];
							materialLeaf[i + 3] = 1.0f;
						}
						materialLeaf[12] = materialLeafBeautiful[12];

						switchMaterial(materialLeaf);

						t = t + 0.0002f;
						if (t >= 1.0f)
							t = 1.0f;
					}
					else
						switchMaterial(materialLeafNormal);
				}
				else
					switchMaterial(materialLeafNormal);

				glPushMatrix();
				{
					glColor3f(0.0f, 1.0f, 0.0f);
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
					gluSphere(quadric, 0.05f, 5, 5);
				}
				glPopMatrix();

				currentLeaf++;
			}
			break;

		case PUSH:
			cPushes += 1;
			branchBaseRadius = branchTopRadius;
			branchTopRadius = branchTopRadius - 0.01f;
			branchHeight = branchHeight - 0.06f;
			glPushMatrix();
			break;

		case POP:
			cPushes -= 1;
			branchHeight = branchHeight + 0.06f;
			branchTopRadius = branchTopRadius + 0.01f;
			branchBaseRadius = branchTopRadius;
			glPopMatrix();
			break;

		case TURN_LEFT_DELTA:
			glRotatef(delta + deltaDueToWind, 0.0f, 1.0f, 0.0f);
			break;

		case TURN_RIGHT_DELTA:
			glRotatef(-delta + deltaDueToWind, 0.0f, 1.0f, 0.0f);
			break;

		case PITCH_UP_DELTA:
			glRotatef(delta + deltaDueToWind, 1.0f, 0.0f, 0.0f);
			break;

		case PITCH_DOWN_DELTA:
			glRotatef(-delta + deltaDueToWind, 1.0f, 0.0f, 0.0f);
			break;

		case ROLL_LEFT_DELTA:
			glRotatef(delta + deltaDueToWind, 0.0f, 0.0f, 1.0f);
			break;

		case ROLL_RIGHT_DELTA:
			glRotatef(-delta + deltaDueToWind, 0.0f, 0.0f, 1.0f);
			break;

		default:
			break;
		}
	}
}
