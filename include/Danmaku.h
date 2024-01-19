/**
 * *****************************************************************************
 * @file        Danmaku.h
 * @brief       A cpp head file for Danmakugame.
 * @author      qytlix (3102225279@qq.com)
 * @date        2024-01-14
 * @copyright   qytlix
 * *****************************************************************************
 */


#ifndef DANMAKU_H 
#define DANMAKU_H 


/*----------------------------------include-----------------------------------*/

#include <SDL2\SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

/*-----------------------------------macro------------------------------------*/

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

/*----------------------------------typedef-----------------------------------*/

enum PRESS_TO_AVTIVE_TYPE {
    PRESS,
    RELEASE
};
enum MOUSE_STATE {
    DOWN,
    UP
}MouseState[2],lastMouseState[2];
// 0 means left, 1 means right;
struct point {
    double x,y;
};
struct Box {
	point pos;//centre position for a box
	double a;
};
struct Timer {
	unsigned int last,tick;
	Timer (int _tick);
	bool check(); //check and update
	bool ocheck(); // only check
	void update();
};

class flys{
	protected:
		Box hitbox,imgbox;// hitbox and image's box
		point v;//speed : vector
		SDL_Texture *pic;// Texture
		bool exist;// if exist
	public:
		void init(SDL_Texture *_pic, double x, double y, double hitsize, double imgsize, double dx, double dy);
		void putPic(); // put picture to renderer
		void update(); // Lets MOVE!
		void die(); // disappeared qaq
		bool ifexist(); // check it if exist
		Box getHitbox(); // where am I?
		Box getImgbox(); // where am I? (+1)
		bool check(); // check if it out of screen
};
class Bullet : public flys{
	public:
		void init(int choosepic, double x, double y, double hitsize, double imgsize, double dx, double dy);
};
class Enemys : public flys{
	protected:
		int hits;
		unsigned int step;
	public:
		void init(int choosepic, double x, double y, double hitsize, double imgsize, double dx, double dy, int _hits);
		void update();
		void getHit();
		int getstep();
		void throwBullet_followselfplane(double ,double = 0); // biu~
};
class Drops : public flys{
	protected:
		int type;// 0 = power, 1 = dian
		point a;// accelerate
		bool lock;// if lock, fly to selfPlane
	public:
		int getType(); // what am i
		void attract(); // be attracted
		void init(int type, double x, double y);
		void update();

};

class Item {
    protected:
        SDL_Rect rect;
        bool hang, active, lastAvtive;
        bool shown;
    public:
        void update();
        void hide();
        bool getHangState();
        bool getActiveState();
        bool getShownState();
        void init(SDL_Rect);
};
class textButton : public Item {
    protected:
        bool output;
        SDL_Color baseColor, hangColor, activeColor;
        PRESS_TO_AVTIVE_TYPE type;
        std::string text;
    public:
        bool update();
        void draw();
        void init(SDL_Rect, SDL_Color, SDL_Color, SDL_Color, PRESS_TO_AVTIVE_TYPE, std::string);
};
class pictureButton : public Item {
    // TODO
    // Coming Soon :)
};


/*----------------------------------variable----------------------------------*/

//sdkl vars
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *Background = nullptr;
SDL_Texture *bullet_a = nullptr;
SDL_Texture *background = nullptr;
SDL_Texture *plane = nullptr;
SDL_Texture *testEnemypic = nullptr;
SDL_Texture *PowerPic = nullptr;
SDL_Texture *DianPic = nullptr;

const Uint8* KBstate;//keyboard event read
Uint32 Mstate;//Mouse event read
int MouseX,MouseY;

bool drawBoxon; // hitBox if draw

// Add Points

int bulAddPoint;
int enemybulletAddPoint;
int powerAddPoint;
int dianAddPoint;

/*----------------------------------function----------------------------------*/

SDL_Texture * getimage(const char *);

bool checkHit(Box,Box);// check 2 box if cross
bool checkPointInRect(point,SDL_Rect);
bool initSDL();// init SDL including SDL_VEDIO, IMG_PNG, Window and icon

void closeSDL();// release SDL
void drawBox(Box);
void KBReflection();// check KB event and update place of SelfPlane
void putimage(SDL_Texture *, int, int, int = 0, int = 0);// put image to renderer
void putimage(SDL_Texture *, Box);// put image to renderer
void runGame();// start Game.
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
/*-----------------------------------detail-----------------------------------*/

// classes
void flys :: init(SDL_Texture *_pic, double x, double y, double hitsize, double imgsize, double dx = 0, double dy = 0) {
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
void flys :: putPic() {
	putimage(this->pic,imgbox);
}
void flys :: update() {
	hitbox.pos = v + hitbox.pos;
	imgbox.pos = v + imgbox.pos;
}
void flys :: die() {
	exist = 0;
}
bool flys :: ifexist() { // check it if exist
	return exist;
}
Box flys :: getHitbox() { // where am I?
	return hitbox;
}
Box flys :: getImgbox() { // where am I? (+1)
	return imgbox;
}
bool flys :: check() { // check if it out of screen
	if (hitbox.pos.x <= LEFTLINE - 100 or hitbox.pos.x >= RIGHTLINE + 100) return 0;
	if (hitbox.pos.y <= UPLINE - 100   or hitbox.pos.y >= DOWNLINE + 100)  return 0;
	return 1;
}
// class Bullet
void Bullet :: init(int choosepic, double x, double y, double hitsize, double imgsize, double dx, double dy) {
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
//class Enemys
void Enemys :: init(int choosepic, double x, double y, double hitsize, double imgsize, double dx, double dy, int _hits) {
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
void Enemys :: update(){
	flys::update();
	step++; // step for function : throwBullet_followselfplane 
}
void Enemys :: getHit() { // be hit by selfPlane qaq (nt!)
	hits --;
	if (hits == 0)
	{
		die();
		for(int i = 0; i < 5; i ++)throwPower(hitbox.pos.x,hitbox.pos.y); // awsl
		for(int i = 0; i < 5; i ++)throwDian(hitbox.pos.x,hitbox.pos.y);
	}
}

int Enemys :: getstep() {
	return step;
}
// class Drops
int Drops :: getType() { // what am i
	return type;
}
void Drops :: attract() { // be attracted
	lock = 1;
}
void Drops :: init(int type, double x, double y){
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

// class Item
void Item :: update() {
    if (checkPointInRect({(double)MouseX,(double)MouseY},rect)) hang = true;
    else hang = false;
    lastAvtive = active;
    if (hang and MouseState[0] == DOWN and lastMouseState[0] == UP) active = true;
    if (active and MouseState[0] == UP) active = false;
}
bool Item :: getHangState() {
    return hang;
}
bool Item :: getActiveState() {
    return active;
}
bool Item :: getShownState() {
    return shown;
}
void Item :: init(SDL_Rect A) {
    rect = A;
    shown = true;
}
void Item :: hide() {
    shown = false;
}
//class Button
bool textButton :: update() {
    Item::update();
    switch (type)
    {
    case PRESS:
        return active;
        break;
    case RELEASE:
        if (!active) return lastAvtive;
        break;
    default:
        break;
    }
    return 0;
}
void textButton :: draw() {
    if (!shown) return;
    SDL_Color nowColor = baseColor;
    if (active) nowColor = activeColor;
    else if (hang) nowColor = hangColor;
    SDL_SetRenderDrawColor(renderer, nowColor.r, nowColor.g, nowColor.b, nowColor.a);
	SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}
void textButton :: init(SDL_Rect A, SDL_Color _base, SDL_Color _hang, SDL_Color _active, PRESS_TO_AVTIVE_TYPE _type, std::string _text) {
    shown = true;
    rect = A;
    baseColor = _base;
    hangColor = _hang;
    activeColor = _active;
    type = _type;
    text = _text;
}
// struct Timer
Timer :: Timer (int _tick){
	last = 0;
	tick = _tick;
}
bool Timer :: check() { //check and update
	unsigned int now = SDL_GetTicks();
	if (now > last + tick) {
		last = now;
		return 1;
	}
	return 0;
}
bool Timer :: ocheck() { // only check
	unsigned int now = SDL_GetTicks();
	if (now > last + tick) {
		return 1;
	}
	return 0;
}
void Timer :: update() {
	unsigned int now = SDL_GetTicks();
	last = now;
}

// struct point
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

// normal functions
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
double _abs(double number){
	return number>=0?number:-number;
}
double distance(point A, point B) {
	return _abs(A.x-B.x) + _abs(A.y-B.y);
}
double mol(point A){
	return sqrt(A.x*A.x+A.y*A.y);
}
bool checkHit(Box A,Box B) {
	if (abs(A.pos.x-B.pos.x)>A.a+B.a)return 0;
	if (abs(A.pos.y-B.pos.y)>A.a+B.a)return 0;
	return 1;
}
bool checkPointInRect(point A, SDL_Rect B) {
    if (A.x <= B.x + B.w and A.x >= B.x and A.y <= B.y + B.h and A.y >= B.y) return 1;
    return 0;
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


#endif	/* DANMAKU_H */
