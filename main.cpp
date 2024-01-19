//-static-libgcc -lSDL2 -lSDL2_image
/*************************************************
 * 
 * Project:Touhou-like Danmaku Game
 *  maker:qytlix
 *  begining time: 2023/12
 *  version  time: 2024/1/7
 *  update describe:
 *   [2024/1/19] Add Button part.
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
#include "./include/Danmaku.h"
#define TESTOPEN // for test Button
using namespace std;

Box selfBox = {200,200,5};
Bullet bullets[SELFBULLETMAX],enemybullets[BULLETMAX];
Drops powers[POWERSMAX],dians[DIANSMAX];
Enemys testEnemy;

bool quit = false;//main quit
bool lowSpeedMode;

double v=5;
double rx,ry,lx,ly; // next step for selfPlane

unsigned int power, dian;// count power and dian
unsigned int preTick;//which tick is now's tick

Timer ShootTimer = Timer(15);
Timer switchTimer = Timer(150);
#ifdef TESTOPEN
textButton testB1,testB2;
#endif
#undef main
int main(){
	#ifdef TESTOPEN
	testB1.init({100,200,30,50},{150,200,150,255}, {0,200,0,255}, {200,0,0,255},PRESS,"aa");
	testB2.init({200,200,30,50},{150,200,150,255}, {0,200,0,255}, {200,0,0,255},RELEASE,"aa");
	#endif

	srand(time(0));// init rand()

	if(!initSDL())return -1;
	
	runGame();

	closeSDL();

	return 0;
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
void KBReflection() {
	if(KBstate[SDL_SCANCODE_X])quit = 1;
	if(KBstate[SDL_SCANCODE_LEFT])lx = 1;
	if(KBstate[SDL_SCANCODE_RIGHT])rx = 1;
	if(KBstate[SDL_SCANCODE_UP])ly = 1;
	if(KBstate[SDL_SCANCODE_DOWN])ry = 1;
	if(KBstate[SDL_SCANCODE_LSHIFT])v = 2.5, lowSpeedMode = 1;
	else v = 5, lowSpeedMode = 0;
	if(ShootTimer.check()) {
		if(KBstate[SDL_SCANCODE_Z]) {
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
		if(KBstate[SDL_SCANCODE_A]) {
			testEnemy.init(1,LEFTLINE+1,200,20,20,3,0,50);
		}
		if(KBstate[SDL_SCANCODE_D]) {
			throwPower(selfBox.pos.x,selfBox.pos.y-40);
		}
		if(KBstate[SDL_SCANCODE_F]) {
			throwDian(selfBox.pos.x,selfBox.pos.y-40);
		}
	}
	if (switchTimer.ocheck()) {
		if(KBstate[SDL_SCANCODE_S]) {
			drawBoxon = !drawBoxon;
			switchTimer.update();
		}
	}
}
void drawBox(Box A){
	if (!drawBoxon)return;
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect rect1 = { (int)(A.pos.x - A.a), (int)(A.pos.y - A.a), (int)(2*A.a), (int)(2*A.a)};
	SDL_RenderDrawRect(renderer, &rect1);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}
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
void runGame(){
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
			#ifdef TESTOPEN
			testB1.draw();
			testB2.draw();
			#endif
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
			switch (event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				default:
					break;
			}
		}
		#ifdef TESTOPEN
		if(testB1.update()){
			cout << "1:work!\n";
		}
		if(testB2.update()){
			cout << "2:work!\n";
		}
		#endif
		Mstate = SDL_GetMouseState(&MouseX,&MouseY);
		lastMouseState[0] = MouseState[0];
		lastMouseState[1] = MouseState[1];
		if(Mstate&SDL_BUTTON(1)) MouseState[0]=DOWN;
		else MouseState[0]=UP;
		if(Mstate&SDL_BUTTON(3)) MouseState[1]=DOWN;
		else MouseState[1]=UP;
		KBstate = SDL_GetKeyboardState(NULL);
		KBReflection();
	}
}
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
void Drops :: update(){

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