//-static-libgcc -lSDL2 -lSDL2_image
/*************************************************
 * 
 * Project:Touhou-like Danmaku Game
 *  maker:qytlix
 *  begining time: 2023/12
 *  version  time: 2024/1/7
 *  update describe:
 *   [2024/1/14] Add Dian mode, Add rotate "throwBullet_followselfplane"
 *   [2024/1/13] Add Power attract mode
 *   [2024/1/13] Add Power random throw
 *   [2024/1/7] Add Power mode
 *   [2024/1/7] Change struct to class.(OOP!!!)
 * 
*************************************************/
#include <iostream>
#include <cmath>
#include <time.h>
#include <random>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define nullptr NULL
using namespace std;
/*vars*/
// window size
#define WINDOW_HEIGHT 960
#define WINDOW_WIDTH  1280

// map edges
#define UPLINE 92
#define DOWNLINE 880
#define LEFTLINE 88
#define RIGHTLINE 708

#define SELFBULLETMAX 50 // bullets from selfPlane max number
#define BULLETMAX 2000
#define POWERSMAX 100
#define DIANSMAX 100
#define ATTRACTLINE 300
// sdl vars
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *Background = nullptr;
SDL_Texture *bullet_a = nullptr;
SDL_Texture *background = nullptr;
SDL_Texture *plane = nullptr;
SDL_Texture *testEnemypic = nullptr;
SDL_Texture *PowerPic = nullptr;
SDL_Texture *DianPic = nullptr;

const Uint8* state;//keyboard event read

unsigned int power, dian;
bool quit = false;//main quit
unsigned int preTick;//which tick is now's tick
struct point{double x,y;};
struct Box{
	point pos;//centre position for a box
	double a;
};

//selfPlane move data
double v=5;
double rx,ry,lx,ly;
Box selfBox = {200,200,5};
bool lowSpeedMode;
// end move data

/*--------*/

/*function declarations*/
SDL_Texture * getimage(const char *);// load a image(png)

bool checkHit(Box,Box);// check 2 box if cross
bool initSDL();// init SDL including SDL_VEDIO, IMG_PNG, Window and icon

void closeSDL();// release SDL
void drawBox(Box);
void KBReflection();// check KB event and update place of SelfPlane
void putimage(SDL_Texture *, int, int, int = 0, int = 0);// put image to renderer
void putimage(SDL_Texture *, Box);// put image to renderer
void showPower();// TODO : put main's show part in funcs
void throwPower(double,double);// At this point throw a power
void throwDian(double,double); // At this point throw a dian

int randint(int,int);// get a random int

double _abs(double);// faster??
double distance(point,point);// Get 2 points distance(square)
double mol(point);// point length (to O)
double randdouble(double,double);// get a random double

point operator+(point,point);
point operator-(point,point);
point operator/(point,double);
point operator*(point,double);
point towards(point,point,double);// from, to, speed
point rotate(point,double); // rotate an angle(deg)
/*--------*/
class flys{
	protected:
		Box hitbox,imgbox;// hitbox and image's box
		point v;//speed : vector
		SDL_Texture *pic;// Texture
		bool exist;// if exist
	public:
		void init(SDL_Texture *_pic, double x, double y, double hitsize, double imgsize, double dx = 0, double dy = 0) {
			exist = 1;
			pic = _pic;

			hitbox.pos.x = x;
			hitbox.pos.y = y;
			hitbox.a = hitsize;

			imgbox.pos.x = x;
			imgbox.pos.y = y;
			imgbox.a = imgsize;

			v.x = dx;
			v.y = dy;
		}
		void putPic() { // put picture to renderer
			putimage(this->pic,imgbox);
		}
		void update() { // Lets MOVE!
			hitbox.pos = v + hitbox.pos;
			imgbox.pos = v + imgbox.pos;
		}
		void die() { // disappeared qaq
			exist = 0;
		}
		bool ifexist() { // check it if exist
			return exist;
		}
		Box getHitbox() { // where am I?
			return hitbox;
		}
		Box getImgbox() { // where am I? (+1)
			return imgbox;
		}
		bool check() { // check if it out of screen
			if (hitbox.pos.x <= LEFTLINE - 100 or hitbox.pos.x >= RIGHTLINE + 100) return 0;
			if (hitbox.pos.y <= UPLINE - 100   or hitbox.pos.y >= DOWNLINE + 100)  return 0;
			return 1;
		}
};
class Bullet : public flys{
	public:
		void init(int choosepic, double x, double y, double hitsize, double imgsize, double dx, double dy) {
			exist = 1;
			// set pic
			switch (choosepic) {
				case 1: // bullet_a
					pic = bullet_a;
					break;
				default:
					break;
			}

			hitbox.pos.x = x;
			hitbox.pos.y = y;
			hitbox.a = hitsize;

			imgbox.pos.x = x;
			imgbox.pos.y = y;
			imgbox.a = imgsize;

			v.x = dx;
			v.y = dy;
		}
}bullets[SELFBULLETMAX],enemybullets[BULLETMAX];
class Enemys : public flys{
	private:
		int hits;
		unsigned int step;
	public:
		void init(int choosepic, double x, double y, double hitsize, double imgsize, double dx, double dy, int _hits) {
			exist = 1;
			// set pic
			switch (choosepic) {
				case 1:// testpic
					pic = testEnemypic;
					break;
				default:
					break;
			}

			hitbox.pos.x = x;
			hitbox.pos.y = y;
			hitbox.a = hitsize;

			imgbox.pos.x = x;
			imgbox.pos.y = y;
			imgbox.a = imgsize;

			v.x = dx;
			v.y = dy;

			hits = _hits;
		}
		void update(){
			flys::update();
			step++; // step for function : throwBullet_followselfplane 
		}
		void getHit() { // be hit by selfPlane qaq (nt!)
			hits --;
			if (hits == 0)
			{
				die();
				for(int i = 0; i < 5; i ++)throwPower(hitbox.pos.x,hitbox.pos.y); // awsl
				for(int i = 0; i < 5; i ++)throwDian(hitbox.pos.x,hitbox.pos.y);
			}
		}
		int getstep() {
			return step;
		}
		void throwBullet_followselfplane(double ,double = 0); // biu~
}testEnemy;
class Drops : public flys{
	private:
		int type;// 0 = power, 1 = dian
		point a;// accelerate
		bool lock;// if lock, fly to selfPlane
	public:
		int getType() { // what am i
			return type;
		}
		void attract() { // be attracted
			lock = 1;
		}
		void init(int type, double x, double y){
			lock = 0;
			exist = 1;
			v.y = -randdouble(5,10);
			v.x = randdouble(-10,10);
			switch (type)
			{
				case 0:
					pic = PowerPic;
					break;
				case 1:
					pic = DianPic;
					break;
				default:
					break;
			}

			hitbox.pos.x = x;
			hitbox.pos.y = y;
			hitbox.a = 10; // const size for Drops

			imgbox.pos.x = x;
			imgbox.pos.y = y;
			imgbox.a = 10;
		}
		void update(){

			flys::update();

			// check lock?
			if (lowSpeedMode and distance(hitbox.pos,selfBox.pos)<100)lock = 1;
			if (distance(hitbox.pos,selfBox.pos) < 25) lock = 1;
			// check end
			if (lock) {
				v = towards(hitbox.pos, selfBox.pos, 10);// face to selfPlane
				return;
			}
			if (v.y < 3)v.y += 0.25;// accele Y
			else v.y = 3;
			if (v.x > 0)v.x = (v.x-0.25)<0?0:(v.x-0.25);// accele X
			else if (v.x < 0)v.x = (v.x+0.25)>0?0:(v.x+0.25);
		}

}powers[POWERSMAX],dians[DIANSMAX];
struct Timer {
	unsigned int last,tick;
	Timer (int _tick){
		last = 0;
		tick = _tick;
	}
	bool check() { //check and update
		unsigned int now = SDL_GetTicks();
		if (now > last + tick) {
			last = now;
			return 1;
		}
		return 0;
	}
	bool ocheck() { // only check
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
#undef main
int main(){
	// init rand()
	srand(time(0));
	// end init rand()
	if(!initSDL())return -1;
	SDL_Event event;

	Timer ScreenTimer = Timer(15);
//	Timer SelfPlaneAnime = Timer(700);
	while(!quit) {
		// wheather quit
		if (ScreenTimer.check()) {
			preTick++; // next tick
			
			SDL_RenderPresent(renderer); // update screen

			//ready for next tick fresh
			
			SDL_RenderClear(renderer); // cls
			//throw powers
			for (int i = 0; i < POWERSMAX; i ++)
			{
				if (powers[i].ifexist())
				{
					powers[i].update();
					powers[i].putPic();
					if (!powers[i].check())
					{
						powers[i].die();
						continue;
					}
					if (checkHit(powers[i].getHitbox(),selfBox))
					{
						powers[i].die();
						printf("power ++\n");
						//TODO
						power ++;
						continue;
					}
					if (selfBox.pos.y <= ATTRACTLINE) powers[i].attract();
					drawBox(powers[i].getHitbox());
				}
			}
			// put dian
			for (int i = 0; i < DIANSMAX; i ++)
			{
				if (dians[i].ifexist())
				{
					dians[i].update();
					dians[i].putPic();
					if (!dians[i].check())
					{
						dians[i].die();
						continue;
					}
					if (checkHit(dians[i].getHitbox(),selfBox))
					{
						dians[i].die();
						printf("dian ++\n");
						//TODO
						dian ++;
						continue;
					}
					if (selfBox.pos.y <= ATTRACTLINE) dians[i].attract();
					drawBox(dians[i].getHitbox());
				}
			}
			//put bullet from selfPlane
			for (int i = 0; i < SELFBULLETMAX; i ++)
			{
				if (bullets[i].ifexist()) {
					bullets[i].update();
					bullets[i].putPic();
					// putimage (bullet_a, bullets[i].getHitbox());
					if (!bullets[i].check())
					{
						bullets[i].die();
						continue;
					}
					//TODO : for testEnemy 
					if (testEnemy.ifexist() and checkHit(bullets[i].getHitbox(),testEnemy.getHitbox())){
						testEnemy.getHit();//be hitted and check if die
						bullets[i].die();// clear the hitting bullet
						continue;
					}
					drawBox(bullets[i].getHitbox());
				}
			}
			
			if (testEnemy.ifexist())
			{
				//move enemy
				testEnemy.update();
				if (!testEnemy.check()){
					testEnemy.die();
				}
				if (testEnemy.ifexist() and !(testEnemy.getstep() % 15)) {
					testEnemy.throwBullet_followselfplane(5);
					testEnemy.throwBullet_followselfplane(5,10);
					testEnemy.throwBullet_followselfplane(5,-10);
				}
				//put enemy
				if (testEnemy.ifexist()) putimage(testEnemypic, testEnemy.getImgbox());
				drawBox(testEnemy.getHitbox());
			}	
			//put enemy's bullets
			for (int i = 0; i < BULLETMAX; i ++)
			{
				if (enemybullets[i].ifexist()) {
					enemybullets[i].update();
					enemybullets[i].putPic();
					// putimage (bullet_a, enemybullets[i].getImgbox());
					if (!enemybullets[i].check()){
						enemybullets[i].die();
						continue;
					}
					if (enemybullets[i].ifexist() and checkHit(enemybullets[i].getHitbox(),selfBox)){
						cout << "Hit!\n";
						enemybullets[i].die();
						continue;
					}
					drawBox(enemybullets[i].getHitbox());
				}
			}
			
			//put selfplane
			
//			cout <<x <<' ' << y<<endl;
//			cout <<"fps:"<< 1000/(now-lastTime) << endl;
			int nexx = selfBox.pos.x - lx * v + rx * v, nexy = selfBox.pos.y - ly * v + ry * v;
			lx = 0;ly = 0;
			rx = 0; ry = 0;
			if (nexx >= LEFTLINE and nexx <= RIGHTLINE)selfBox.pos.x = nexx;
			else if (nexx <= LEFTLINE)selfBox.pos.x = LEFTLINE;
			else if (nexx >= RIGHTLINE)selfBox.pos.x = RIGHTLINE;
			
			if (nexy >= UPLINE and nexy <= DOWNLINE)selfBox.pos.y = nexy;
			else if (nexy <= UPLINE)selfBox.pos.y = UPLINE;
			else if (nexy >= DOWNLINE)selfBox.pos.y = DOWNLINE;
			putimage(plane,selfBox);
			drawBox(selfBox);
			//put background
			putimage(background, 0, 0);
		}
		while (SDL_PollEvent(&event) > 0) {
			if (event.type == SDL_QUIT) {
				quit = true;
				break;
			}
		}
		KBReflection();
	}
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

	//Textures Load
	bullet_a = getimage("./image/bullet_a.png");
	background = getimage("./image/background.png");
	plane = getimage("./image/plane.png");
	testEnemypic = getimage("./image/enemy.png");
	PowerPic = getimage("./image/power.png");
	DianPic = getimage("./image/dian.png");
	return 1;
}
void closeSDL() {
	SDL_DestroyTexture(plane);
	SDL_DestroyTexture(testEnemypic);
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(bullet_a);
	SDL_DestroyTexture(PowerPic);
	SDL_DestroyTexture(DianPic);

	IMG_Quit();
	
	SDL_DestroyWindow( window );
	
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
void putimage(SDL_Texture *texture, Box box){
	SDL_Rect im_place = {(int)(box.pos.x - box.a), (int)(box.pos.y - box.a), 2*(int)box.a, 2*(int)box.a};
	SDL_RenderCopy(renderer, texture, nullptr, &im_place);
}
int bulAddPoint;
Timer ShootTimer = Timer(15);
Timer switchTimer = Timer(150);
bool drawBoxon;
void KBReflection() {
	state = SDL_GetKeyboardState(NULL);
	if(state[SDL_SCANCODE_X])quit = 1;
	if(state[SDL_SCANCODE_LEFT])lx = 1;
	if(state[SDL_SCANCODE_RIGHT])rx = 1;
	if(state[SDL_SCANCODE_UP])ly = 1;
	if(state[SDL_SCANCODE_DOWN])ry = 1;
	if(state[SDL_SCANCODE_LSHIFT])v = 2.5, lowSpeedMode = 1;
	else v = 5, lowSpeedMode = 0;
	if(ShootTimer.check()) {
		if(state[SDL_SCANCODE_Z]) {
			int ori = bulAddPoint;// original place
			while(bullets[bulAddPoint].ifexist()){
				bulAddPoint = (bulAddPoint + 1) % SELFBULLETMAX;
				if (ori == bulAddPoint)
				{
					//cout<<"Space G\n";
					return;
				}
			}
			bullets[bulAddPoint].init(1, selfBox.pos.x, selfBox.pos.y + 20, 8, 10, 0, -20);
		}
		if(state[SDL_SCANCODE_A]) {
			testEnemy.init(1,LEFTLINE+1,200,20,20,3,0,50);
		}
		if(state[SDL_SCANCODE_D]) {
			throwPower(selfBox.pos.x,selfBox.pos.y-40);
		}
		if(state[SDL_SCANCODE_F]) {
			throwDian(selfBox.pos.x,selfBox.pos.y-40);
		}
	}
	if (switchTimer.ocheck()) {
		if(state[SDL_SCANCODE_S]) {
			drawBoxon = !drawBoxon;
			switchTimer.update();
		}
	}
}
int enemybulletAddPoint;
void Enemys :: throwBullet_followselfplane (double bspeed, double angle) {
	point to = towards(hitbox.pos, selfBox.pos, bspeed);
	if (angle != 0)to = rotate(to, angle);
	int ori = enemybulletAddPoint;// original place
	while(enemybullets[enemybulletAddPoint].ifexist()){
		enemybulletAddPoint = (enemybulletAddPoint + 1) % BULLETMAX;
		if (ori == enemybulletAddPoint)
		{
			//cout<<"Space G\n";
			return;
		}
	}
	enemybullets[enemybulletAddPoint].init(1, hitbox.pos.x, hitbox.pos.y,10,20, to.x, to.y);
}
bool checkHit(Box A,Box B) {
	if (abs(A.pos.x-B.pos.x)>A.a+B.a)return 0;
	if (abs(A.pos.y-B.pos.y)>A.a+B.a)return 0;
	return 1;
}
void drawBox(Box A){
	if (!drawBoxon)return;
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect rect1 = { (int)(A.pos.x - A.a), (int)(A.pos.y - A.a), (int)(2*A.a), (int)(2*A.a)};
	SDL_RenderDrawRect(renderer, &rect1);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}
point operator+(point a,point b){
	point ans;
	ans.x = a.x + b.x;
	ans.y = a.y + b.y;
	return ans;
}
point operator-(point a,point b){
	point ans;
	ans.x = a.x - b.x;
	ans.y = a.y - b.y;
	return ans;
}
point operator/(point a,double b){
	point ans;
	ans.x = a.x/b;
	ans.y = a.y/b;
	return ans;
}
point operator*(point a,double b){
	point ans;
	ans.x = a.x*b;
	ans.y = a.y*b;
	return ans;
}
int powerAddPoint;
void throwPower(double x,double y){
	int ori = powerAddPoint;// original place
	while(powers[powerAddPoint].ifexist()){
		powerAddPoint = (powerAddPoint + 1) % POWERSMAX;
		if (ori == powerAddPoint)
		{
			//cout<<"Space G\n";
			return;
		}
	}
	powers[powerAddPoint].init(0,x,y);
}
int dianAddPoint;
void throwDian(double x,double y){
	int ori = dianAddPoint;// original place
	while(dians[dianAddPoint].ifexist()){
		dianAddPoint = (dianAddPoint + 1) % DIANSMAX;
		if (ori == dianAddPoint)
		{
			//cout<<"Space G\n";
			return;
		}
	}
	dians[dianAddPoint].init(1,x,y);
}
int randint(int min, int max) {
	return min+(rand()%(max-min+1));
}
double randdouble(double min, double max) {
	return min+(((double)(rand()%(((int)(max-min))*1000)))/1000);
}
double _abs(double number){
	return number>=0?number:-number;
}
double distance(point A, point B) {
	return _abs(A.x-B.x) + _abs(A.y-B.y);
}
double mol(point A){
	return sqrt(A.x*A.x+A.y*A.y);
}
point towards(point from, point to, double speed) {
	point ans = to - from;
	return ans *speed /mol(ans);
}
point rotate(point A, double angle) {
	point ans;
	double rad = angle*3.14159/180;
	ans.x = A.x*cos(rad)-A.y*sin(rad);
	ans.y = A.y*cos(rad)+A.x*sin(rad);
	return ans;
}
