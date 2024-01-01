//-static-libgcc -lSDL2 -lSDL2_image
/***********************************
 * 
 * Project:Touhou-like Danmaku Game
 *  maker:qytlix
 *  begining time:2023/12
 * 
***********************************/
#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define nullptr NULL
using namespace std;
/*vars*/
// window size
#define WINDOW_HEIGHT 960
#define WINDOW_WIDTH  1280

// map edges
#define UPLINE 80
#define DOWNLINE 872
#define LEFTLINE 80
#define RIGHTLINE 696

#define SELFBULLETMAX 50 // bullets from selfPlane max number
#define BULLETMAX 2000
// sdl vars
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *Background = nullptr;

const Uint8* state;//keyboard event read

bool quit = false;//main quit
unsigned int preTick;//which tick is now's tick
struct S_Box{double x,y,a;};// Sqare hitBox
/*--------*/

/*function declarations*/
bool initSDL();// init SDL including SDL_VEDIO, IMG_PNG, Window and icon
void closeSDL();// release SDL
SDL_Texture * getimage(const char *);// load a image(png)
void putimage(SDL_Texture *, int, int, int = 0, int = 0);// put image to randerer
void moveSelfPlane();// check KB event and update place of SelfPlane
bool checkHit(S_Box*,S_Box*);// check 2 box if cross
void readEnemyBullets(const char *);
void drawBox(S_Box*);
/*--------*/
struct Enemys
{
	double a,b,hits;
	double dx,dy,size;
	S_Box box;
	bool exist;
	unsigned int step;
	// a <-> x
	// b <-> y
	void set(double _x, double _y, double _hits, double size, double _dx = 0, double _dy = 0) {
		a = _x;
		b = _y;
		this->size = size;
		hits = _hits;
		
		box.a = size/2;
		exist = true;
		dx = _dx;
		dy = _dy;
		step = 0;
	}
	void move(){
		a += dx;
		b += dy;
		box.x = a+size/2;
		box.y = b+size/2;
		step ++;
	}
	void throwBullet_followselfplane(double bspeed);
	void getHit(){
		hits --;
		if (hits == 0)exist = false;
	}
}testEnemy;
struct Bullet{
	double x,y,dx,dy;
	S_Box box;
	bool exist;
	double size;
	void SetBullet(double _x, double _y, double _dx, double _dy, double size) {
		x = _x;
		y = _y;
		dx = _dx;
		dy = _dy;
		box.x = x+size/2;
		box.y = y+size/2;
		box.a = size/4;
		this -> size = size;
		exist = 1;
	}
	void next() {
		x += dx;
		y += dy;
		box.x += dx;
		box.y += dy;
	}
	bool check() {
		if (x <= LEFTLINE - 100 or x >= RIGHTLINE + 100) return 0;
		if (y <= UPLINE - 100   or y >= DOWNLINE + 100)  return 0;
		return 1;
	}
}bullets[SELFBULLETMAX],enemybullets[BULLETMAX];
struct Timer {
	unsigned int last,tick;
	Timer (int _tick){
		last = 0;
		tick = _tick;
	}
	bool check() {
		unsigned int now = SDL_GetTicks();
		if (now > last + tick) {
			last = now;
			return 1;
		}
		return 0;
	}
	bool ocheck() {
		unsigned int now = SDL_GetTicks();
		if (now > last + tick) {
			return 1;
		}
		return 0;
	}
	void update() {
		unsigned int now = SDL_GetTicks();
		last = now;
	}
};
//selfPlane move data
double x=200,y=200,v=8;
double rx,ry,lx,ly;
S_Box selfBox = {x+10,y+10,5};
#undef main
int main(){
	if(!initSDL())return -1;
	SDL_Texture *bullet_a = getimage("./image/bullet_a.png");
	SDL_Texture *background = getimage("./image/background.png");
	SDL_Texture *plane = getimage("./image/plane.png");
	SDL_Texture *testEnemypic = getimage("./image/enemy.png");
	SDL_Event event;

	Timer ScreenTimer = Timer(15);
//	Timer SelfPlaneAnime = Timer(700);
	while(!quit) {
		//wheather quit
		if (ScreenTimer.check()) {
			preTick++;
			//update screen
			SDL_RenderPresent(renderer);
			//ready for next tick fresh
			//cls
			SDL_RenderClear(renderer);
			//put bullet from selfPlane
			for (int i = 0; i < SELFBULLETMAX; i ++)
			{
				if (bullets[i].exist) {
					bullets[i].next();
					putimage (bullet_a, bullets[i].x, bullets[i].y, 20, 20);
					if (!bullets[i].check())
					{
						bullets[i].exist = 0;
						continue;
					}
					//TODO : for testEnemy 
					if (testEnemy.exist and checkHit(&bullets[i].box,&testEnemy.box)){
						testEnemy.getHit();
						bullets[i].exist = 0;
						continue;
					}
					drawBox(&bullets[i].box);
				}
			}

			
			if (testEnemy.exist)
			{
				//move exemy
				testEnemy.move();
				if (testEnemy.a < LEFTLINE or testEnemy.a > RIGHTLINE or testEnemy.b > DOWNLINE or testEnemy.b < UPLINE) testEnemy.exist = 0;
				if (testEnemy.exist and !(testEnemy.step % 15))testEnemy.throwBullet_followselfplane(5);
				//put enemy
				if (testEnemy.exist) putimage(testEnemypic, testEnemy.a, testEnemy.b, testEnemy.size, testEnemy.size);
				drawBox(&testEnemy.box);
			}	
			//put enemy's bullets
			for (int i = 0; i < BULLETMAX; i ++)
			{
				if (enemybullets[i].exist) {
					enemybullets[i].next();
					putimage (bullet_a, enemybullets[i].x, enemybullets[i].y, 20, 20);
					if (!enemybullets[i].check()){
						enemybullets[i].exist = 0;
						continue;
					}
					if (enemybullets[i].exist and checkHit(&enemybullets[i].box,&selfBox)){
						cout << "Hit!\n";
						enemybullets[i].exist = 0;
						continue;
					}
					drawBox(&enemybullets[i].box);
				}
			}
			
			//put selfplane
			
//			cout <<x <<' ' << y<<endl;
//			cout <<"fps:"<< 1000/(now-lastTime) << endl;
			int nexx = x - lx * v + rx * v, nexy = y - ly * v + ry * v;
			lx = 0;ly = 0;
			rx = 0; ry = 0;
//			if (nexx >= LEFTLINE and nexx <= RIGHTLINE)x = nexx;
//			if (nexy >= UPLINE and nexy <= DOWNLINE)y = nexy;
			x = nexx;
			y = nexy;
			selfBox.x = x +10;
			selfBox.y = y +10;
			if (x < LEFTLINE) x = RIGHTLINE;
			if (x > RIGHTLINE) x = LEFTLINE;
			if (y > DOWNLINE) y = UPLINE;
			if (y < UPLINE) y = DOWNLINE;
			putimage(plane, x+5, y+5, 10, 10);
			drawBox(&selfBox);
			//put background
			putimage(background, 0, 0);
		}
		while (SDL_PollEvent(&event) > 0) {
			if (event.type == SDL_QUIT) {
				quit = true;
				break;
			}
		}
		moveSelfPlane();
	}
	
	SDL_DestroyTexture(plane);
	SDL_DestroyTexture(testEnemypic);
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(bullet_a);
	closeSDL();
}

//Functions
bool initSDL() {
	if (IMG_Init(IMG_INIT_PNG) == -1) {
		printf("[ERROR] Init Image_PNG : %s\n",SDL_GetError());
		return 0;
	}
	//init the window
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		printf("[ERROR] Init SDL : %s\n",SDL_GetError());
		return 0;
	}
	//create a window
	window = SDL_CreateWindow( "TouHou : World Link Century", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		printf("[ERROR] Create Window : %s\n",SDL_GetError());
		return 0;
	}
	//set icon
	SDL_Surface *_icon= IMG_Load("./image/icon.png");
	if (_icon == nullptr) {
		printf("[ERROR] Image_jpg : %s\n",SDL_GetError());
		return 0;
	}
	SDL_SetWindowIcon(window, _icon);
	SDL_FreeSurface(_icon);
	_icon = nullptr;
	//create a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		printf("[ERROR] Create Renderer : %s\n",SDL_GetError());
		return 0;
	}
	return 1;
}
void closeSDL() {
	IMG_Quit();
	
	SDL_DestroyWindow( window );
	window = nullptr;
	
	SDL_Quit();
}
SDL_Texture * getimage(const char *path) {
	SDL_Surface *image = IMG_Load(path);
	if (image == NULL) {
		printf("IMG_Load <%s> failed\n" ,path);
		return nullptr;
	}
	SDL_Texture *back = SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);
	return back;
}
void putimage(SDL_Texture *texture, int x, int y, int textureW, int textureH) {
	if (textureW == 0 and textureH == 0)SDL_QueryTexture(texture ,NULL ,NULL ,&textureW, &textureH);
	else if (textureW == 0) SDL_QueryTexture(texture ,NULL ,NULL ,&textureW, NULL);
	else if (textureH == 0) SDL_QueryTexture(texture ,NULL ,NULL ,NULL , &textureH);
	SDL_Rect im_place = {x, y, textureW, textureH};
	SDL_RenderCopy(renderer, texture, nullptr, &im_place);
}
int bulAddPoint;
Timer ShootTimer = Timer(15);
Timer switchTimer = Timer(150);
bool drawBoxon;
void moveSelfPlane() {
	state = SDL_GetKeyboardState(NULL);
	/*if(state[SDL_SCANCODE_LSHIFT]) {
		pic.x = 256;
		v = 10;
	} else {
		pic.x = 0;
		v = 20;
	}*/
	//check KB
	if(state[SDL_SCANCODE_X])quit = 1;
	if(state[SDL_SCANCODE_LEFT])lx = 1;
	if(state[SDL_SCANCODE_RIGHT])rx = 1;
	if(state[SDL_SCANCODE_UP])ly = 1;
	if(state[SDL_SCANCODE_DOWN])ry = 1;
	if(state[SDL_SCANCODE_LSHIFT])v = 4;
	else v = 8;
	if(ShootTimer.check()) {
		if(state[SDL_SCANCODE_Z]) {
			int ori = bulAddPoint;// original place
			while(bullets[bulAddPoint].exist){
				bulAddPoint = (bulAddPoint + 1) % SELFBULLETMAX;
				if (ori == bulAddPoint)
				{
					//cout<<"Space G\n";
					return;
				}
			}
			bullets[bulAddPoint].SetBullet(x,y + 20,0,-20,20);
		}
		if(state[SDL_SCANCODE_A]) {
			testEnemy.set(LEFTLINE+1,200,50,40,3,0);
		}
	}
	if (switchTimer.ocheck()) {
		if(state[SDL_SCANCODE_S]) {
			drawBoxon = !drawBoxon;
		}

		switchTimer.update();
	}
}
int enemybulletAddPoint;
void Enemys :: throwBullet_followselfplane (double bspeed) {
	double toX = selfBox.x - box.x,toY = selfBox.y - box.y;
	double bLength = sqrt(pow(toX,2) + pow(toY,2));
	toX *= bspeed;
	toY *= bspeed;
	toX /= bLength;
	toY /= bLength;
	
	int ori = enemybulletAddPoint;// original place
	while(enemybullets[enemybulletAddPoint].exist){
		enemybulletAddPoint = (enemybulletAddPoint + 1) % BULLETMAX;
		if (ori == enemybulletAddPoint)
		{
			//cout<<"Space G\n";
			return;
		}
	}
	enemybullets[enemybulletAddPoint].SetBullet(box.x - 10, box.y - 10, toX, toY, 20);
}
bool checkHit(S_Box *A,S_Box *B) {
	if (abs(A->x-B->x)>A->a+B->a)return 0;
	if (abs(A->y-B->y)>A->a+B->a)return 0;
	return 1;
}
void drawBox(S_Box *A){
	if (!drawBoxon)return;
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect rect1 = { (int)(A->x - A->a), (int)(A->y - A->a), (int)(2*A->a), (int)(2*A->a)};
	SDL_RenderDrawRect(renderer, &rect1);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}