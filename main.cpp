#include<bits/stdc++.h>
#include<graphics.h>
#include<windows.h>
#include<thread>

#define ull unsigned long long
#define ui unsigned int
#define ld long double
#define ll long long
#define lll __int128
#define db double
#define pii pair<int,int>
#define pll pair<ll,ll>
#define pdd pair<db,db>
#define vi vector<int>
#define vp vector<pii>
#define vt vector<tup>
#define fi first
#define se second
#define e emplace
#define eb emplace_back
#define ef emplace_front
#define all(x) x.begin(),x.end()
#define mp make_pair
using namespace std;

int N,M,realN,realM;
db Zoom;
const int tick=15;
const db pi=acos(-1),eps=1e-6;

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME)&0x8000)?1:0)
#define count(x) __builtin_popcount(x)
template<typename T> inline bool Mmax(T&a,T b){return a<b?a=b,1:0;}
template<typename T> inline bool Mmin(T&a,T b){return a>b?a=b,1:0;}
template<typename...Args,typename T> inline bool Mmax(T&a,T b,Args...args){return Mmax(a,b)|Mmax(a,args...);}
template<typename...Args,typename T> inline bool Mmin(T&a,T b,Args...args){return Mmin(a,b)|Mmin(a,args...);}
template<typename T> inline int symb(T d){return d>0?1:-1;}
mt19937 rnd(time(0));
inline int rd(int l,int r){return l+rnd()%(r-l+1);}
inline db range(db x,db y){return x<y?y-x:y-x+pi*2;}
inline db dis(pdd x,pdd y){return sqrt((x.fi-y.fi)*(x.fi-y.fi)+(x.se-y.se)*(x.se-y.se));}

PIMAGE img_Screen,img_Now[10],img_Background;
//0:background
//1:shadow
//2:people
//3:weapons
//4:flashing
//5:
//6:
//7:
//8:blood
//9:debug
PIMAGE img_Player;
PIMAGE img_Gun_shadow,img_Gun_shadow_rev,img_Gun,img_Gun_rev,img_Bullet;
PIMAGE img_Sword,img_Sword_shadow,img_Sword_flashing[17],img_Sword_stabbing[10];
PIMAGE img_Machete,img_Saw,img_Sight[2],img_Storage;
PIMAGE img_Enemy;
PIMAGE img_Velocity;

namespace Gaming
{
	db img_x,img_y;
	struct Velocity
	{
		db dx,dy;
		inline db rad(){return atan2(dy,dx);}
		inline db len(){return sqrt(dx*dx+dy*dy);}
		Velocity(db X=0,db Y=0,int typ=0)
		{
			if(!typ)dx=X,dy=Y;
			else dx=cos(X)*Y,dy=sin(X)*Y;
		}
		inline Velocity operator +(const Velocity v)const
		{return Velocity(dx+v.dx,dy+v.dy);}
		inline Velocity operator -(const Velocity v)const
		{return Velocity(dx-v.dx,dy-v.dy);}
		inline Velocity &operator +=(const Velocity v)
		{dx+=v.dx,dy+=v.dy;return *this;}
		inline Velocity &operator -=(const Velocity v)
		{dx-=v.dx,dy-=v.dy;return *this;}
		inline void apply(db&x,db&y,db t=1){x+=dx*t,y+=dy*t;}
		inline void fix(db l)
		{
			db L=len();
			if(L<eps)return;
			dx*=l/L,dy*=l/L;
		}
		inline void reduce(db l){fix(max((db)0,len()-l));}
		inline void show(db x,db y)
		{putimage_rotatetransparent(img_Now[9],img_Velocity,(int)(N/2+x-img_x),(int)(M/2+y-img_y),0,5,EGERGB(255,255,255),rad());}
	};
	
	
	int player_hp;
	Velocity player_v;
	db player_x,player_y;
	int lstmov,nowweapon=0;
	
	struct Player_bullet
	{
		Velocity v;
		db x,y;
		int tim,id;
	};
	vector<Player_bullet> player_bullets;
	int bullet_id;
	int gun_shooting,gun_nowdelta;
	
	int sword_running,sword_type,sword_move;
	db sword_pos,sword_flspos;
	inline bool storaging(){return sword_running>40||sword_running<-40;}
	
	struct Enemy
	{
		db x,y,r;
		int hp,tophp;
		int stat,hitid;
		Velocity v;
		db redu;
		int tim;
	};
	vector<Enemy> enemies;
	
	const int dx[4]={0,0,-1,1},dy[4]={-1,1,0,0};
	const char CMov[5]="WSAD";
	inline void runenemy()
	{
		vector<Enemy> nwenemies;
		for(auto &E:enemies)
		{
			if(E.hp<=0)continue;
			if(E.stat==0)
			{
				E.v={0,0};
				bool flag=0;
				auto runaway=[&](Player_bullet B)->void
				{
					db k=B.v.rad(),dist=1.0*fabs(tan(k)*(E.x-B.x)-(E.y-B.y))/sqrt(tan(k)*tan(k)+1);
					if(dist<=E.r*2&&B.v.dx*(E.x-B.x)+B.v.dy*(E.y-B.y)>0)
					{
						flag=1;
						if((E.y-B.y)*B.v.dx>(E.x-B.x)*B.v.dy)
						E.v-={sin(k),-cos(k)};
						else E.v+={sin(k),-cos(k)};
					}
				};
				for(auto B:player_bullets)runaway(B);
				int mx=0,my=0;
				mousepos(&mx,&my);
				mx/=Zoom,my/=Zoom;
//				runaway({Velocity(mx-N/2,my-M/2)});
				db d=atan2(player_y-E.y,player_x-E.x);
				db dist=max(-2.0,min(2.0,dis(mp(E.x,E.y),mp(player_x,player_y))-30));
				E.v+={0.1*cos(d)*dist,0.1*sin(d)*dist};
			}
			else if(E.stat==1)
			{
				--E.tim;
				if(E.tim==0)E.stat=0;
			}
			if(E.v.len()>eps)E.v.fix(2);
//			E.v.show(E.x,E.y);
			E.v.apply(E.x,E.y);
			if(E.stat==1)
			E.v.fix(max((db)0,E.v.len()-E.redu));
			
			nwenemies.eb(E);
			if(abs(E.x-player_x)<N/2-10&&abs(E.y-player_y)<M/2-10)
			{
				setlinestyle(SOLID_LINE,0,1,img_Now[8]);
				setcolor(EGERGB(0,0,0),img_Now[8]);
				setfillcolor(EGERGB(min(255,(int)(460-460*E.hp/E.tophp)),min(255,(int)(460.0*E.hp/E.tophp)),0),img_Now[8]);
				rectangle((int)(N/2+E.x-img_x-12),(int)(M/2+E.y-img_y-20),(int)(N/2+E.x-img_x+12),(int)(M/2+E.y-img_y-15),img_Now[8]);
				solidrect((int)(N/2+E.x-img_x-12),(int)(M/2+E.y-img_y-20),(int)(N/2+E.x-img_x-12+24.0*E.hp/E.tophp),(int)(M/2+E.y-img_y-15),img_Now[8]);
				putimage_transparent(img_Now[2],img_Enemy,(int)(N/2+E.x-img_x-12),(int)(M/2+E.y-img_y-12),RGB(255,255,255));
			}
		}
		enemies=nwenemies;
	}
	inline bool Hitenemy(db x,db y,db d,db rng,db Dis,int damage,db _v,int redu,int tim,int hitid,bool reuse)
	{
		int flag=0;
		for(auto &E:enemies)
		{
			db dist=dis(mp(E.x,E.y),mp(x,y));
			db D=atan2(E.y-y,E.x-x);
			if(range(D,d)>rng&&range(d,D)>rng)continue;
			if(dist>Dis+E.r)continue;
			if(E.hitid==hitid)continue;
			E.hitid=hitid;
			E.hp-=damage,E.stat=1;
			E.tim=tim,E.redu=redu;
			E.v={cos(D)*_v,sin(D)*_v};
			if(reuse)flag=1;else return 1;
		}
		return flag;
	}
	inline void chkmove()
	{
		int s=0;
		for(int i=0;i<4;++i)s|=KEY_DOWN(CMov[i])<<(i>>1);
		s=count(s);
		player_v={0,0};
		int v=2-(KEY_DOWN(VK_LSHIFT)||storaging())+(KEY_DOWN(VK_LCONTROL)&&!storaging());
		for(int i=0;i<4;++i)if(KEY_DOWN(CMov[i]))player_v+={(db)dx[i],(db)dy[i]};
		if(s)player_v.apply(player_x,player_y,1.5/sqrt(s)*v);
		for(int i=0;i<10;++i)if(KEY_DOWN('0'+i))nowweapon=i;
	}
	inline void rungun(int mx,int my)
	{
		int fl=0;
		for(auto E:enemies)
		{
			db dist=dis(mp(E.x-img_x+N/2,E.y-img_y+M/2),mp(mx,my));
			fl|=dist<=E.r;
		}
		putimage_transparent(img_Now[3],img_Sight[fl],mx-10,my-10,EGERGB(255,255,255));
		mx-=N/2+player_x-img_x,my-=M/2+player_y-img_y;
		db d=atan2(my,mx);
		if(gun_nowdelta<0)++gun_nowdelta;
		if(gun_nowdelta>0)--gun_nowdelta;
		if(KEY_DOWN(VK_LBUTTON))
		{
			if(!gun_shooting)
			{
				int delta=1+1*(1+(player_v.len()>eps)+KEY_DOWN(VK_LCONTROL)-KEY_DOWN(VK_LSHIFT));
				gun_nowdelta=rd(-delta,delta);
				player_bullets.eb((Player_bullet){Velocity(d+1.0*gun_nowdelta/100,20,1),player_x+cos(d)*80,player_y+sin(d)*80,50,++bullet_id});
				gun_shooting=5;
			}
			else gun_shooting--;
		}
		else gun_shooting=0;
		int v=symb(mx);
		d+=v*1.0*abs(gun_nowdelta)/100;
		PIMAGE tmp=(mx>=0?img_Gun:img_Gun_rev);
		PIMAGE tmp2=(mx>=0?img_Gun_shadow:img_Gun_shadow_rev);
		putimage_rotatetransparent(img_Now[1],tmp2,N/2+player_x-img_x-v*6*sin(d),M/2+player_y-img_y+v*6*cos(d)+10,0,14,EGERGB(255,255,255),d);
		putimage_rotatetransparent(img_Now[3],tmp,N/2+player_x-img_x-v*6*sin(d),M/2+player_y-img_y+v*6*cos(d),0,14,EGERGB(255,255,255),d);
	}
	inline void sword_yielding(int sword_moveid)
	{
		++sword_move;
		if(sword_move==1)
		{
			sword_flspos=sword_pos;
			Hitenemy(player_x,player_y,sword_flspos,1,70,20,10+player_v.len(),1,20,sword_moveid,1);
		}
		if(sword_move<=4)sword_pos+=0.16*(5-sword_move);
		else if(sword_move<=12);
		else sword_type=0;
		if(sword_move<=10)
		putimage_rotatetransparent(img_Now[3],img_Sword_flashing[sword_move],N/2+player_x-img_x,M/2+player_y-img_y,0,80,EGERGB(0,0,0),sword_flspos);
		putimage_rotatetransparent(img_Now[1],img_Sword_shadow,N/2+player_x-img_x,M/2+player_y-img_y+10,35,8,EGERGB(255,255,255),sword_pos);
		putimage_rotatetransparent(img_Now[3],img_Sword,N/2+player_x-img_x,M/2+player_y-img_y,35,8,EGERGB(255,255,255),sword_pos);
	}
	inline void sword_stabbing(int sword_moveid)
	{
		int nw=35;
		++sword_move;
		if(sword_move==1)Hitenemy(player_x,player_y,sword_pos,0.1,100,40,15+player_v.len(),3,10,sword_moveid,1);
		if(sword_move<=5)nw-=5*sword_move;
		else if(sword_move<=9)nw-=30;
		else if(sword_move<=14)nw-=5*(14-sword_move);
		else sword_type=0;
		if(sword_move<=8)
		putimage_rotatetransparent(img_Now[3],img_Sword_stabbing[(sword_move+3)>>2],N/2+player_x-img_x,M/2+player_y-img_y,0,12,EGERGB(0,0,0),sword_pos);
		putimage_rotatetransparent(img_Now[1],img_Sword_shadow,N/2+player_x-img_x,M/2+player_y-img_y+10,nw,8,EGERGB(255,255,255),sword_pos);
		putimage_rotatetransparent(img_Now[3],img_Sword,N/2+player_x-img_x,M/2+player_y-img_y,nw,8,EGERGB(255,255,255),sword_pos);
	}
	PIMAGE tat;
	inline void runsword(int mx,int my)
	{
		static int sword_moveid=0;
		mx-=N/2+player_x-img_x,my-=M/2+player_y-img_y;
		db d=atan2(my,mx);
		if(sword_type==1)return sword_yielding(sword_moveid);
		if(sword_type==2)return sword_stabbing(sword_moveid);
		if(sword_type==3)return;
		if(sword_type==4)return;
		if(KEY_DOWN(VK_LBUTTON))
		{
			Mmax(sword_running,0);
			if(sword_running<100)++sword_running;
		}
		else if(KEY_DOWN(VK_RBUTTON))
		{
			Mmin(sword_running,0);
			if(sword_running>-100)--sword_running;
		}
		else
		{
			if(-5<sword_running&&sword_running<=10);
			else if(10<sword_running&&sword_running<=100)
			++sword_moveid,sword_type=1,sword_move=0,sword_pos=d;
			else if(-100<=sword_running&&sword_running<=-5)
			++sword_moveid,sword_type=2,sword_move=0,sword_pos=d;
			sword_running=0;
			if(sword_type)return runsword(mx,my);
		}
		int nw=35;
		if(sword_running>0)d=d-min(10,sword_running)*0.1;
		else nw=nw+min(5,-sword_running);
		db dist=range(sword_pos,d);
		if(dist<=pi)sword_pos+=min(0.5,dist);
		else sword_pos-=min(0.5,2*pi-dist);
		putimage_rotatetransparent(img_Now[1],img_Sword_shadow,N/2+player_x-img_x,M/2+player_y-img_y+10,nw,8,EGERGB(255,255,255),sword_pos);
		putimage_rotatetransparent(img_Now[3],img_Sword,N/2+player_x-img_x,M/2+player_y-img_y,nw,8,EGERGB(255,255,255),sword_pos);
	}
	inline void chkfight()
	{
		int mx=0,my=0;
		mousepos(&mx,&my);
		mx/=Zoom,my/=Zoom;
		if(nowweapon==1)rungun(mx,my);
		if(nowweapon==2)runsword(mx,my);
	}
	inline void runplayerbullets()
	{
		vector<Player_bullet> tmp;
		for(auto &[v,x,y,tim,id]:player_bullets)
		{
			v.apply(x,y);
			if(Hitenemy(x,y,v.rad(),4,0,10,5,1,5,id,0))continue;
			putimage_rotatetransparent(img_Now[3],img_Bullet,N/2+x-img_x,M/2+y-img_y,0,3,EGERGB(255,255,255),v.rad());
			if(--tim)tmp.eb((Player_bullet){v,x,y,tim,id});
		}
		player_bullets=tmp;
	}
	inline void setbackground()
	{
		img_x=img_x+(player_x-img_x)/20,img_y=img_y+(player_y-img_y)/20;
		putimage(img_Now[0],0,0,N,M,img_Background,((int)img_x%32+32)%32,((int)img_y%34+34)%34);
	}
	inline void putobjects()
	{
		
	}
	
	inline void Game()
	{
		initgraph(realN,realM);
		img_Screen=newimage(N,M);
		for(int i=0;i<10;++i)img_Now[i]=newimage(N,M);
		player_hp=200,player_x=0,player_y=0;
		settarget(NULL);
		ShowWindow(GetConsoleWindow(),SW_HIDE);
//		if(KEY_DOWN('Q')&&enemies.size()==0)1
		enemies.eb((Enemy){player_x-50,player_y-50,12,100,100,0});
		while(enemies.size()&&!KEY_DOWN(VK_ESCAPE))
		{
			
			for(int i=0;i<10;++i)
			{
				setfillcolor(EGERGB(135,242,111),img_Now[i]);
				bar(0,0,N,M,img_Now[i]);
			}
			
			putimage_transparent(img_Now[2],img_Player,N/2-12+player_x-img_x,M/2-12+player_y-img_y,RGB(255,255,255));
			setbackground();
			putobjects();
			
			chkmove();
			chkfight();
			runplayerbullets();
			runenemy();
			
			for(int i=0;i<10;++i)
			putimage_transparent(img_Screen,img_Now[i],0,0,EGERGB(135,242,111));
			putimage_rotatezoom(NULL,img_Screen,0,0,0,0,0,Zoom);
			Sleep(tick);
		}
	}
}
namespace Initing
{
	inline void Getimages()
	{
		img_Background=newimage(),getimage_pngfile(img_Background,"images/background.png");
		img_Player=newimage(),getimage_pngfile(img_Player,"images/player.png");
		img_Enemy=newimage(),getimage_pngfile(img_Enemy,"images/enemy.png");
		img_Velocity=newimage(),getimage_pngfile(img_Velocity,"images/velocity.png");
		img_Gun=newimage(),getimage_pngfile(img_Gun,"images/weapons/gun/gun.png");
		img_Gun_rev=newimage(),getimage_pngfile(img_Gun_rev,"images/weapons/gun/gun_rev.png");
		img_Gun_shadow=newimage(),getimage_pngfile(img_Gun_shadow,"images/weapons/gun/gun_shadow.png");
		img_Gun_shadow_rev=newimage(),getimage_pngfile(img_Gun_shadow_rev,"images/weapons/gun/gun_shadow_rev.png");
		img_Bullet=newimage(),getimage_pngfile(img_Bullet,"images/weapons/gun/bullet.png");
		img_Machete=newimage(),getimage_pngfile(img_Machete,"images/weapons/machete.png");
		img_Saw=newimage(),getimage_pngfile(img_Saw,"images/weapons/saw.png");
		img_Sight[0]=newimage(),getimage_pngfile(img_Sight[0],"images/weapons/sight1.png");
		img_Sight[1]=newimage(),getimage_pngfile(img_Sight[1],"images/weapons/sight2.png");
		img_Sword=newimage(),getimage_pngfile(img_Sword,"images/weapons/sword/sword.png");
		img_Sword_shadow=newimage(),getimage_pngfile(img_Sword_shadow,"images/weapons/sword/sword_shadow.png");
		char s[50];
		for(int i=1;i<=10;++i)
		{
			sprintf(s,"images/weapons/sword/flashing%d.png",i);
			img_Sword_flashing[i]=newimage(),getimage_pngfile(img_Sword_flashing[i],s);
		}
		for(int i=1;i<=4;++i)
		{
			sprintf(s,"images/weapons/sword/stabbing%d.png",i);
			img_Sword_stabbing[i]=newimage(),getimage_pngfile(img_Sword_stabbing[i],s);
		}
	}
	inline void Getsettings()
	{
		ifstream fin("settings/Screen.txt");
		fin>>N>>M>>Zoom;
		realN=N*Zoom;
		realM=M*Zoom;
	}
	inline void Getsaves()
	{
		
	}
	inline void init()
	{
		Getsettings();
		Getimages();
		Getsaves();
	}
}

int main()
{
	Gaming::tat=newimage(1,1);
	Initing::init();
	Gaming::Game();
	return 0;
}
