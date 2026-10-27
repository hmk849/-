// PIXEL PFS - single-file C++17 + SDL2 (software raycaster, 320x180 pixel framebuffer)
// Desktop: g++ -O2 -std=c++17 main.cpp $(sdl2-config --cflags --libs) -o pixelpfs   (mouse = touch)
// Android: use the SDL2 android-project template, put this file in as the main source.
#include <SDL.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
typedef uint32_t u32;
#define RGB(r,g,b) (0xFF000000u|((u32)(r)<<16)|((u32)(g)<<8)|(u32)(b))
#define PI 3.14159265f
const int H=180; int W=320;
u32 fb[400*H]; float zb[400];
const u32 BG=RGB(15,25,35),ACC=RGB(255,70,85),WHT=RGB(236,232,225),PNL=RGB(31,40,51),DIM=RGB(130,140,150),BLU=RGB(70,160,255),BLK=0xFF000000u;

// ---------- settings ----------
struct Cfg{int ver=3,fps=60,color=0,aspect=0,gyro=0,xh=0,xhc=0,adsTog=0,invY=0,bot=1,hm=1,showFps=0,sel=0,op=0,map=0;
 float sens=1,ads=1,dist=.4f,spd=.4f,gyroS=1,xs=4,xg=2,xt=1,xa=1,vm=.8f,vs=1,vmu=.5f; float bx[7],by[7],bs[7],ba[7];} C;
const char*BL[]={"","FIRE","ADS","JUMP","SLIDE","RLD","INSP"};
void defLayout(){float d[7][4]={{.13f,.70f,26,.6f},{.87f,.62f,20,.7f},{.75f,.50f,13,.6f},{.93f,.36f,12,.6f},{.76f,.80f,12,.6f},{.90f,.86f,11,.6f},{.62f,.88f,10,.5f}};
 for(int i=0;i<7;i++){C.bx[i]=d[i][0];C.by[i]=d[i][1];C.bs[i]=d[i][2];C.ba[i]=d[i][3];}}
string cpath;
void save(){FILE*f=fopen(cpath.c_str(),"wb");if(f){fwrite(&C,sizeof C,1,f);fclose(f);}}
void load(){char*pp=SDL_GetPrefPath("PixelPFS","pfs");cpath=string(pp?pp:"")+"cfg.bin";Cfg t;FILE*f=fopen(cpath.c_str(),"rb");
 if(f&&fread(&t,sizeof t,1,f)==1&&t.ver==3)C=t;else defLayout();if(f)fclose(f);}

// ---------- data ----------
struct Wp{const char*n,*t;int dmg,pel;float rate,spr,zoom;int mag;bool au;float rec;};
Wp WP[]={{"AK-47","AR",26,1,10,.02f,1.5f,30,1,.9f},{"M4A1","AR",22,1,12,.015f,1.5f,30,1,.7f},
 {"MP5","SMG",17,1,14,.03f,1.2f,30,1,.5f},{"VECTOR","SMG",14,1,20,.035f,1.2f,33,1,.5f},
 {"AWM","SNIPER",110,1,.9f,.002f,4,5,0,3},{"KAR98","SNIPER",85,1,1.2f,.002f,3,5,0,2.5f},
 {"M870","SHOTGUN",12,8,1.2f,.08f,1.2f,6,0,3},{"SPAS12","SHOTGUN",10,8,2.5f,.09f,1.2f,8,0,2.5f}};
struct Op{const char*n,*role;u32 u,v,h;int hp;float spd;};
Op OP[]={{"GHOST","RECON",RGB(90,95,100),RGB(50,55,60),RGB(40,45,50),100,1.1f},{"VIPER","ASSAULT",RGB(60,100,60),RGB(40,70,40),RGB(35,55,35),100,1.f},
 {"RAVEN","STEALTH",RGB(60,50,95),RGB(30,25,50),RGB(25,22,40),90,1.05f},{"TITAN","HEAVY",RGB(150,125,80),RGB(110,90,55),RGB(90,75,45),125,.92f},
 {"NOVA","MEDIC",RGB(60,90,140),RGB(200,205,215),RGB(50,70,110),100,1.f}};
const char*SP[16]={".....hhhh.....","....hhhhhh....","....hssssh....","....kkkkkk....",".....ssss.....","...uvvvvvv....","..uuvvppvuggg.","..uuvvvvvgggg.",
 "...uvvvvvuggg.","....uuuuuu....","....uuuuuu....","....uu..uu....","....uu..uu....","....uu..uu....","....bb..bb....","...bbb..bbb..."};
u32 pc(char c,const Op&o,u32 vest){switch(c){case 'h':return o.h;case 's':return RGB(232,190,150);case 'k':return RGB(20,30,40);case 'v':return vest?vest:o.v;
 case 'u':return o.u;case 'p':return RGB(90,80,60);case 'g':return RGB(30,30,34);default:return RGB(25,25,25);}}
const char*MN[]={"NEON DOCKYARD","SANDSTORM BAZAAR","PIXEL RANGE"};
u32 WC[3][5]={{0,RGB(60,80,100),RGB(200,110,40),RGB(255,70,85),RGB(40,210,200)},{0,RGB(190,160,110),RGB(150,100,60),RGB(220,190,140),RGB(70,140,150)},{0,RGB(70,75,85),RGB(120,125,135),RGB(255,70,85),RGB(240,200,60)}};
u32 SK[3][4]={{RGB(8,14,26),RGB(30,50,70),RGB(30,40,52),RGB(14,20,28)},{RGB(200,160,110),RGB(240,210,160),RGB(200,170,120),RGB(150,115,75)},{RGB(10,12,18),RGB(36,42,54),RGB(48,54,66),RGB(22,26,34)}};
const int MW=24; char M[MW][MW]; int cm=0;
struct R{int x,y,w,h,t;};
void build(int m){cm=m;for(int y=0;y<MW;y++)for(int x=0;x<MW;x++)M[y][x]=(x==0||y==0||x==MW-1||y==MW-1)?1:0;vector<R> v;
 if(m==0)v={{6,3,2,2,2},{6,9,1,3,3},{6,17,2,3,2},{10,6,1,5,3},{10,14,1,5,3},{4,11,2,1,3},{11,11,2,2,4}};
 else if(m==1)v={{5,2,1,6,2},{5,12,1,2,2},{5,17,1,5,2},{8,8,3,1,3},{8,15,3,1,3},{9,4,2,2,4},{11,10,2,4,3}};
 else v={{6,6,2,2,2},{6,16,2,2,2},{11,4,2,2,4},{11,18,2,2,4},{11,11,2,2,3}};
 for(auto&r:v)for(int j=0;j<r.h;j++)for(int i=0;i<r.w;i++){M[r.y+j][r.x+i]=r.t;M[r.y+j][MW-1-(r.x+i)]=r.t;}}

// ---------- audio (fully procedural: no asset files) ----------
struct V{bool on=0;float dl=0,nv=0,kn=0,en=0,lc=.5f,lp=0,f0=0,f1=0,tv=0,kt=0,et=0,ph=0,vol=1,pl=1,pr=1;int wv=0;};
V vo[32];SDL_AudioDeviceID adev=0;bool musOn=1;
float rf(){return rand()/(float)RAND_MAX*2-1;}
// noise(decay nd, vol nv, lowpass lc) + tone(sweep f0->f1, decay td, vol tv, wave 0 sin/1 sq/2 saw); dl=delay s; pan -1..1
void snd(float dl,float nd,float nv,float lc,float f0,float f1,float td,float tv,int wv,float vol,float pan=0){
 if(!adev)return;SDL_LockAudioDevice(adev);int i=0;while(i<31&&vo[i].on)i++;V&v=vo[i];v=V();v.on=1;v.dl=dl;
 v.nv=nv*min(3.f,sqrtf(.5f/max(lc,.02f)));v.en=nd>0?1.f:0.f;v.kn=nd>0?expf(-1.f/(44100*nd)):0;v.lc=lc;
 v.f0=f0;v.f1=f1;v.tv=tv;v.et=td>0?1.f:0.f;v.kt=td>0?expf(-1.f/(44100*td)):0;v.wv=wv;v.vol=vol*C.vs;v.pl=min(1.f,1-pan);v.pr=min(1.f,1+pan);SDL_UnlockAudioDevice(adev);}
float mt=0,mg=0,bf=55,bph=0,be=0,af=110,aph=0,ae=0,kph=0,ke=0,ne=0,se=0;int ms=0;float dly[2][13230];int dpos=0;
float nf(int semi){return 55.f*powf(2.f,semi/12.f);}
void acb(void*,Uint8*st,int len){float*o=(float*)st;int n=len/8;static const int root[4]={0,-4,3,-2};static const int ch[4][3]={{0,3,7},{0,4,7},{0,4,7},{0,4,7}};
 for(int i=0;i<n;i++){float l=0,r=0;
  for(auto&v:vo){if(!v.on)continue;if(v.dl>0){v.dl-=1.f/44100;continue;}float s=0;
   if(v.en>0){v.lp+=(rf()-v.lp)*v.lc;s+=v.lp*v.en*v.nv;v.en*=v.kn;if(v.en<.0003f)v.en=0;}
   if(v.et>0){float f=v.f1+(v.f0-v.f1)*v.et;v.ph+=f/44100;v.ph-=floorf(v.ph);float w=v.wv==0?sinf(6.2832f*v.ph):v.wv==1?(v.ph<.5f?.6f:-.6f):(v.ph*2-1)*.7f;s+=w*v.et*v.tv;v.et*=v.kt;if(v.et<.0003f)v.et=0;}
   if(v.en==0&&v.et==0)v.on=0;l+=s*v.vol*v.pl;r+=s*v.vol*v.pr;}
  mg+=((musOn?C.vmu:0.f)-mg)*.0003f;   // chiptune BGM: bass + arp + kick/snare/hat, Am-F-C-G
  if(mg>.001f){mt+=1.f/44100;if(mt>=.125f){mt-=.125f;ms=(ms+1)&63;int bar=ms/16,q=ms%16,rt=root[bar];
    if(q%2==0){bf=nf(rt+(q%8==6?12:0));be=1;}af=nf(rt+24+ch[bar][(q*3/2)%3]+(q%8>=4?12:0));ae=1;if(q%4==0)ke=1;if(q%8==4)se=1;if(q%2==1)ne=1;}
   bph+=bf/44100;bph-=floorf(bph);aph+=af/44100;aph-=floorf(aph);kph+=(45+ke*110)/44100;kph-=floorf(kph);
   float m=(bph<.5f?.2f:-.2f)*be+(aph<.25f?.07f:-.07f)*ae+sinf(6.2832f*kph)*ke*.4f+rf()*ne*.05f+rf()*se*.12f;
   be*=.99981f;ae*=.99972f;ke*=.99968f;ne*=.99887f;se*=.99977f;l+=m*mg;r+=m*mg;}
  float dl_=dly[0][dpos],dr_=dly[1][dpos];dly[0][dpos]=l*.5f+dr_*.35f;dly[1][dpos]=r*.5f+dl_*.35f;dpos=(dpos+1)%13230;l+=dl_*.35f;r+=dr_*.35f;   // ping-pong echo = space/tail
  o[2*i]=tanhf(l*C.vm);o[2*i+1]=tanhf(r*C.vm);}}
void sfxShot(int wi,float vol,float pan){Wp&w=WP[wi];int hi=wi&1;
 if(w.pel>1){snd(0,.2f,1.2f,.35f,0,0,0,0,0,vol,pan);snd(0,.2f,.6f,.08f,125-hi*20,35,.17f,1.f,0,vol,pan);snd(.5f,.03f,.8f,.6f,0,0,0,0,0,vol*.6f,pan);snd(.62f,.03f,.8f,.5f,0,0,0,0,0,vol*.6f,pan);}
 else if(w.zoom>=3){snd(0,.09f,1.2f,.8f,0,0,0,0,0,vol,pan);snd(0,.5f,1.f,.06f,95,28,.28f,1.1f,0,vol,pan);snd(0,0,0,0,3200,500,.05f,.3f,2,vol,pan);snd(.55f,.02f,.8f,.7f,0,0,0,0,0,vol*.5f,pan);snd(.75f,.03f,.8f,.5f,0,0,0,0,0,vol*.5f,pan);}
 else if(w.rate>13){snd(0,.04f,.8f,.7f,0,0,0,0,0,vol,pan);snd(0,.07f,.5f,.2f,230+hi*40,75,.05f,.7f,0,vol,pan);}
 else{snd(0,.05f,1.f,.55f,0,0,0,0,0,vol,pan);snd(0,.15f,.6f,.1f,170+hi*30,45,.09f,.9f,0,vol,pan);snd(0,0,0,0,2200,300,.03f,.2f,2,vol,pan);}}
void sfxClick(){snd(0,.02f,.4f,.9f,1500,1500,.03f,.22f,1,.5f);}
void sfxThud(){snd(0,.12f,.7f,.08f,110,40,.1f,.8f,0,.8f);}
void sfxBoom(){snd(0,.9f,.9f,.04f,70,28,.6f,1.f,0,1.f);snd(0,.5f,.5f,.9f,0,0,0,0,0,.4f);}
void sfxHit(bool hs){if(hs){snd(0,0,0,0,2600,2600,.1f,.5f,0,.6f);snd(.06f,0,0,0,3400,3400,.14f,.5f,0,.6f);}else snd(0,0,0,0,1800,1500,.05f,.5f,0,.55f);}
void sfxKill(){snd(0,0,0,0,880,880,.12f,.35f,1,.5f);snd(.1f,0,0,0,1320,1320,.25f,.35f,1,.5f);}
void sfxHurt(){snd(0,.12f,.8f,.15f,120,50,.1f,.7f,0,.9f);}
void sfxReload(float rt){snd(0,.03f,.7f,.5f,0,0,0,0,0,.6f);snd(rt*.35f,.06f,.6f,.2f,200,90,.05f,.4f,0,.7f);snd(rt*.8f,.03f,.8f,.6f,0,0,0,0,0,.7f);snd(rt*.8f+.07f,.03f,.8f,.4f,0,0,0,0,0,.6f);}
void sfxStep(float v){snd(0,.045f,.5f,.08f,70,50,.04f,.25f,0,v);}
void sfxJump(){snd(0,.08f,.4f,.15f,0,0,0,0,0,.4f);}
void sfxLand(){snd(0,.1f,.6f,.07f,90,45,.08f,.6f,0,.6f);}
void sfxSlide(){snd(0,.5f,.7f,.18f,0,0,0,0,0,.7f);}
void sfxStart(){float n[]={392,523,659,784};for(int i=0;i<4;i++)snd(i*.08f,0,0,0,n[i],n[i],.15f,.35f,1,.6f);}
void sfxWin(){float n[]={523,659,784,1047};for(int i=0;i<4;i++)snd(i*.13f,0,0,0,n[i],n[i],.3f,.35f,1,.7f);}
void sfxLose(){float n[]={392,330,262,196};for(int i=0;i<4;i++)snd(i*.18f,0,0,0,n[i],n[i],.35f,.4f,2,.7f);}
void sfxWarn(){snd(0,0,0,0,1000,1000,.06f,.3f,1,.6f);}

// ---------- drawing ----------
u32 mix(u32 a,u32 b,float t){auto f=[&](int s){return (int)(((a>>s)&255)*(1-t)+((b>>s)&255)*t);};return RGB(f(16),f(8),f(0));}
int ci(float v){return v<0?0:v>255?255:(int)v;}
void rect(int x,int y,int w,int h,u32 c){for(int j=max(y,0);j<min(y+h,H);j++)for(int i=max(x,0);i<min(x+w,W);i++)fb[j*W+i]=c;}
void arect(int x,int y,int w,int h,u32 c,float a){for(int j=max(y,0);j<min(y+h,H);j++)for(int i=max(x,0);i<min(x+w,W);i++)fb[j*W+i]=mix(fb[j*W+i],c,a);}
void px(int x,int y,u32 c){if(x>=0&&y>=0&&x<W&&y<H)fb[y*W+x]=c;}
void circ(int cx,int cy,int r,u32 c,float a,bool fill){for(int dy=-r;dy<=r;dy++){int xo=(int)sqrtf((float)(r*r-dy*dy));
 if(fill)arect(cx-xo,cy+dy,2*xo+1,1,c,a);else{int xi=(abs(dy)>=r-1)?-1:(int)sqrtf((float)((r-1)*(r-1)-dy*dy));
  if(xi<0)arect(cx-xo,cy+dy,2*xo+1,1,c,a);else{arect(cx-xo,cy+dy,xo-xi+1,1,c,a);arect(cx+xi,cy+dy,xo-xi+1,1,c,a);}}}}
#define G(a,b,c,d,e) (a<<12|b<<9|c<<6|d<<3|e)
const char*CH=" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-/.<>%+|";
const uint16_t FG[]={0,G(2,5,7,5,5),G(6,5,6,5,6),G(3,4,4,4,3),G(6,5,5,5,6),G(7,4,6,4,7),G(7,4,6,4,4),G(3,4,5,5,3),G(5,5,7,5,5),G(7,2,2,2,7),G(1,1,1,5,2),G(5,5,6,5,5),G(4,4,4,4,7),G(5,7,7,5,5),
 G(6,5,5,5,5),G(2,5,5,5,2),G(6,5,6,4,4),G(2,5,5,7,3),G(6,5,6,5,5),G(3,4,2,1,6),G(7,2,2,2,2),G(5,5,5,5,7),G(5,5,5,5,2),G(5,5,7,7,5),G(5,5,2,5,5),G(5,5,2,2,2),G(7,1,2,4,7),
 G(7,5,5,5,7),G(2,6,2,2,7),G(6,1,2,4,7),G(6,1,2,1,6),G(5,5,7,1,1),G(7,4,6,1,6),G(3,4,7,5,7),G(7,1,1,2,2),G(7,5,7,5,7),G(7,5,7,1,6),
 G(0,2,0,2,0),G(0,0,7,0,0),G(1,1,2,4,4),G(0,0,0,0,2),G(1,2,4,2,1),G(4,2,1,2,4),G(5,1,2,4,5),G(0,2,7,2,0),G(2,2,2,2,2)};
void text(int x,int y,string s,u32 c,int sc=1,int al=0){int w=s.size()*4*sc-sc;if(al==1)x-=w/2;if(al==2)x-=w;
 for(char ch:s){const char*p=strchr(CH,toupper(ch));int g=(p&&ch)?FG[p-CH]:0;for(int r=0;r<5;r++)for(int k=0;k<3;k++)if(g>>((4-r)*3+(2-k))&1)rect(x+k*sc,y+r*sc,sc,sc,c);x+=4*sc;}}
void soldier(int x,int y,int sc,const Op&o,u32 vest=0){for(int r=0;r<16;r++)for(int c=0;c<14;c++)if(SP[r][c]!='.')rect(x+c*sc,y+r*sc,sc,sc,pc(SP[r][c],o,vest));}

// ---------- input / ui ----------
struct T{bool on=0;int64_t id=0;float x=0,y=0,sx=0,sy=0,dx=0,dy=0,path=0;int role=-1;}ts[10];
float mx,my;bool md,mp,mr;SDL_Window*win;SDL_Renderer*ren;SDL_Texture*tex;SDL_Sensor*gy=0;
void mktex(){if(tex)SDL_DestroyTexture(tex);SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,W,H);}
void ev(const SDL_Event&e){if(e.type!=SDL_FINGERDOWN&&e.type!=SDL_FINGERMOTION&&e.type!=SDL_FINGERUP)return;
 int ow,oh;SDL_GetRendererOutputSize(ren,&ow,&oh);float vs=min(ow/(float)W,oh/(float)H),vx=(ow-W*vs)/2,vy=(oh-H*vs)/2;
 float x=(e.tfinger.x*ow-vx)/vs,y=(e.tfinger.y*oh-vy)/vs;T*t=0;for(auto&q:ts)if(q.on&&q.id==(int64_t)e.tfinger.fingerId)t=&q;
 if(e.type==SDL_FINGERDOWN){if(!t)for(auto&q:ts)if(!q.on){t=&q;break;}if(t){*t=T();t->on=1;t->id=e.tfinger.fingerId;t->x=t->sx=x;t->y=t->sy=y;}mx=x;my=y;md=mp=1;}
 else if(t){if(e.type==SDL_FINGERMOTION){t->dx+=x-t->x;t->dy+=y-t->y;t->path+=hypotf(x-t->x,y-t->y);}t->x=x;t->y=y;mx=x;my=y;if(e.type==SDL_FINGERUP){t->on=0;md=0;mr=1;}}}
bool hit(int x,int y,int w,int h){return mx>=x&&mx<x+w&&my>=y&&my<y+h;}
bool btn(int x,int y,int w,int h,string l,bool sel=0,int sc=1){bool ov=md&&hit(x,y,w,h);rect(x,y,w,h,sel?ACC:(ov?RGB(50,62,76):PNL));rect(x,y,2,h,sel?WHT:ACC);
 text(x+w/2+1,y+(h-5*sc)/2,l,WHT,sc,1);bool r=mr&&hit(x,y,w,h);if(r){mr=0;sfxClick();}return r;}
void slider(int x,int y,int w,string l,float&v,float lo,float hi){float t=(v-lo)/(hi-lo);text(x,y,l,WHT);text(x+w,y,to_string((int)(t*100))+"%",DIM,1,2);
 rect(x,y+8,w,3,PNL);rect(x,y+8,(int)(w*t),3,ACC);rect(x+(int)(w*t)-2,y+6,4,7,WHT);
 if(md&&mx>=x-4&&mx<=x+w+4&&my>=y+3&&my<=y+16)v=lo+(hi-lo)*max(0.f,min(1.f,(mx-x)/w));}
int choice(int x,int y,int w,string l,vector<string> o,int cur){text(x,y,l,WHT);int n=o.size(),bw=(w-(n-1)*2)/n;
 for(int i=0;i<n;i++)if(btn(x+i*(bw+2),y+8,bw,12,o[i],i==cur))cur=i;return cur;}

// ---------- match state ----------
struct E{float x=0,y=0,a=0,hp=100,cd=0,rs=0,tx=0,ty=0,st=0,sg=1;int team=0,op=0,k=0,d=0;bool al=1;};
vector<E> es;int score[2];bool tr,paused,adsOn,hmh;float tl,hurt,hm,flash,pitch,eye=.5f,eh=.5f,z,vz,slideT,sdx,sdy,slideB,ads,sh,Zf=1,hpMax=100,inspT,rel,relMax=1,cdT,jx,jy;
int ammo;bool prevH[7];vector<pair<string,float>> feed;
enum{INTRO,LOBBY,ARMORY,OPS,SET,LAYOUT,MATCH,RESULT};int scr=INTRO;
bool wall(float x,float y){return M[(int)y][(int)x]!=0;}
bool los(float x0,float y0,float x1,float y1){int n=(int)(hypotf(x1-x0,y1-y0)*8);for(int i=1;i<n;i++){float t=(float)i/n;if(wall(x0+(x1-x0)*t,y0+(y1-y0)*t))return 0;}return 1;}
void mv(E&e,float dx,float dy){float r=.22f;if(!wall(e.x+dx+(dx>0?r:-r),e.y))e.x+=dx;if(!wall(e.x,e.y+dy+(dy>0?r:-r)))e.y+=dy;}
void spawn(int i){E&e=es[i];e.al=1;e.hp=i==0?OP[C.op].hp:100;e.rs=0;
 if(tr&&i>0){do{e.x=8+rand()%14+.5f;e.y=2+rand()%20+.5f;}while(wall(e.x,e.y));e.a=PI;}else{e.x=e.team?20.5f:3.5f;e.y=(i%2)?17.5f:6.5f;e.a=e.team?PI:0;}e.tx=e.x;e.ty=e.y;}
void startMatch(bool train){tr=train;build(train?2:C.map);es.assign(train?5:4,E());
 for(int i=0;i<(int)es.size();i++){es[i].team=(i==0||(!train&&i==1))?0:1;es[i].op=i==0?C.op:rand()%5;spawn(i);}
 hpMax=OP[C.op].hp;es[0].hp=hpMax;score[0]=score[1]=0;tl=180;ammo=WP[C.sel].mag;rel=0;pitch=0;ads=0;z=vz=slideT=0;eye=.5f;Zf=1;feed.clear();paused=0;adsOn=0;for(auto&t:ts)t.role=-1;scr=MATCH;sfxStart();}
void dmgf(int s,int t,float d,bool hd){E&v=es[t];if(!v.al)return;v.hp-=d;if(t==0){hurt=.5f;sfxHurt();}if(s==0){hm=.25f;hmh=hd;if(v.hp>0)sfxHit(hd);}
 if(v.hp<=0){if(s==0)sfxKill();v.al=0;v.rs=tr?1.5f:3;v.d++;es[s].k++;score[es[s].team]++;feed.push_back({string(s==0?"YOU":OP[es[s].op].n)+" > "+OP[v.op].n+(hd?" HS":""),4});}}
void ray(int s,float ang,float dm){E&p=es[s];float rx=cosf(ang),ry=sinf(ang),wd=30;
 for(float t=0;t<30;t+=.05f)if(wall(p.x+rx*t,p.y+ry*t)){wd=t;break;}
 int b=-1;float bt=wd;bool bh=0;
 for(int i=0;i<(int)es.size();i++){E&e=es[i];if(i==s||!e.al||e.team==p.team)continue;float dx=e.x-p.x,dy=e.y-p.y,t=dx*rx+dy*ry;if(t<.2f||t>bt||fabsf(-dx*ry+dy*rx)>.28f)continue;bool hd;
  if(s==0){float u=H*Zf/t,bot=H/2+pitch+u*eh,top=bot-u*.85f;if(H/2<top||H/2>bot)continue;hd=H/2<top+.2f*(bot-top);}else hd=rand()%10==0;b=i;bt=t;bh=hd;}
 if(b>=0)dmgf(s,b,dm*(bh?2:1),bh);}
float rnd(){return rand()/(float)RAND_MAX;}
void shoot(){Wp&w=WP[C.sel];ammo--;sfxShot(C.sel,1,0);float sp=w.spr*(1+min(1.f,hypotf(jx,jy))*.8f+(z>0?1:0))*(1-.6f*ads);
 for(int i=0;i<w.pel;i++)ray(0,es[0].a+(rnd()-.5f)*2*sp,(float)w.dmg);cdT=1/w.rate;pitch+=w.rec*2;es[0].a+=(rnd()-.5f)*w.rec*.02f;flash=.06f;sh=w.rec;}
void botShot(E&e){E&p=es[0];float dx=e.x-p.x,dy=e.y-p.y,d=hypotf(dx,dy),rel=remainderf(atan2f(dy,dx)-p.a,2*PI);sfxShot(1,.8f/(1+d*.35f),sinf(rel));}
void bot(int i,float dt){E&e=es[i];if(!e.al){if((e.rs-=dt)<=0)spawn(i);return;}
 float lv=C.bot==0?.6f:C.bot==1?1.f:1.5f;e.cd-=dt;e.st-=dt;int tg=-1;float bd=99;
 if(!tr)for(int j=0;j<(int)es.size();j++){E&o=es[j];if(!o.al||o.team==e.team)continue;float d=hypotf(o.x-e.x,o.y-e.y);if(d<bd&&los(e.x,e.y,o.x,o.y)){bd=d;tg=j;}}
 float spd=tr?.9f:2.4f;
 if(tg>=0){float w=atan2f(es[tg].y-e.y,es[tg].x-e.x),df=remainderf(w-e.a,2*PI),tn=3*lv*dt;e.a+=max(-tn,min(tn,df));
  if(e.st<=0){e.sg=rand()%2?1.f:-1.f;e.st=.4f+rand()%100/100.f;}mv(e,-sinf(e.a)*e.sg*spd*.6f*dt,cosf(e.a)*e.sg*spd*.6f*dt);
  if(fabsf(df)<.2f&&e.cd<=0){e.cd=.22f/lv;botShot(e);if(rnd()<max(.08f,min(.7f,.5f*lv-bd*.02f)))dmgf(i,tg,7,rand()%12==0);}}
 else{if(hypotf(e.tx-e.x,e.ty-e.y)<.5f||e.st<=0){do{e.tx=1.5f+rand()%22;e.ty=1.5f+rand()%22;}while(wall(e.tx,e.ty));e.st=4+rand()%30/10.f;}
  float w=atan2f(e.ty-e.y,e.tx-e.x);e.a=w;mv(e,cosf(w)*spd*dt,sinf(w)*spd*dt);}}
void upd(float dt){E&p=es[0];bool hd[7]={0};float lkx=0,lky=0;jx=jy=0;
 for(auto&t:ts){if(!t.on)continue;
  if(t.role<0){if(t.sy<22&&t.sx>W-62&&t.sx<W-46){paused=1;t.role=99;}else{t.role=10;for(int b=1;b<7;b++)if(hypotf(t.sx-C.bx[b]*W,t.sy-C.by[b]*H)<C.bs[b]*1.3f)t.role=b;
   if(hypotf(t.sx-C.bx[0]*W,t.sy-C.by[0]*H)<C.bs[0]*1.7f)t.role=0;}}
  if(t.role==0){jx=(t.x-C.bx[0]*W)/C.bs[0];jy=(t.y-C.by[0]*H)/C.bs[0];float m=hypotf(jx,jy);if(m>1){jx/=m;jy/=m;}}
  else if(t.role>=1&&t.role<=6)hd[t.role]=1;
  if(t.role==10||t.role==1){float sp=hypotf(t.dx,t.dy)/max(dt,.001f);float mu=(1+C.dist*min(t.path/80,2.f))*(1+C.spd*min(sp/300,2.f));lkx+=t.dx*mu;lky+=t.dy*mu;}
  t.dx=t.dy=0;}
 bool pr[7];for(int i=1;i<7;i++){pr[i]=hd[i]&&!prevH[i];prevH[i]=hd[i];}
 Wp&w=WP[C.sel];
 if(p.al){
  if(C.adsTog){if(pr[2])adsOn=!adsOn;}else adsOn=hd[2];
  ads+=((adsOn?1.f:0.f)-ads)*min(1.f,dt*12);Zf=1+(w.zoom-1)*ads;
  float am=1-ads+ads*C.ads/(1+(w.zoom-1)*.6f),k=.006f*C.sens*am;
  p.a+=lkx*k;pitch+=(C.invY?1:-1)*lky*k*130;
  if(C.gyro&&gy){float v[3]={0,0,0};SDL_SensorGetData(gy,0,v,3);float sg=C.gyro==2?-1.f:1.f;p.a+=sg*v[0]*dt*C.gyroS*am;pitch+=sg*v[1]*dt*C.gyroS*am*150;}
  pitch=max(-60.f,min(60.f,pitch));
  float spm=3.4f*OP[C.op].spd*(1-.35f*ads),fx=cosf(p.a),fy=sinf(p.a);
  float mvx=fx*-jy-fy*jx,mvy=fy*-jy+fx*jx;
  if(pr[4]&&slideT<=0&&z<=0){slideT=.85f;sfxSlide();float m=hypotf(mvx,mvy);slideB=m>.2f;if(m>.2f){sdx=mvx/m;sdy=mvy/m;}else{sdx=fx;sdy=fy;}}
  if(slideT>0){slideT-=dt;float v=slideB?spm*(.6f+2.0f*slideT/.85f):0;mv(p,sdx*v*dt,sdy*v*dt);}else mv(p,mvx*spm*dt,mvy*spm*dt);
  eye+=((slideT>0?.3f:.5f)-eye)*min(1.f,dt*14);
  if(pr[3]&&z<=0&&slideT<=0){vz=2.8f;sfxJump();}if(z>0||vz>0){z+=vz*dt;vz-=9.8f*dt;if(z<=0){z=0;vz=0;sfxLand();}}eh=eye+z;{static float stp=0;float mm=hypotf(jx,jy);if(z<=0&&slideT<=0&&mm>.2f){stp+=dt*spm*mm;if(stp>1.2f){stp=0;sfxStep(.3f);}}}
  cdT-=dt;float rt=1.6f+(w.zoom>=3)*1.2f+(w.pel>1)*.6f;
  if(rel>0){rel-=dt;if(rel<=0)ammo=w.mag;}
  if(pr[5]&&ammo<w.mag&&rel<=0){rel=relMax=rt;sfxReload(rt);}
  if(hd[1]&&(w.au||pr[1])&&cdT<=0&&rel<=0&&ammo>0){shoot();inspT=0;}
  if(ammo<=0&&rel<=0){rel=relMax=rt;sfxReload(rt);}
  if(pr[6]&&rel<=0&&!hd[1])inspT=2.2f;if(inspT>0)inspT-=dt;if(hd[1]||adsOn)inspT=0;
  if(C.op==4)p.hp=min(hpMax,p.hp+4*dt);
 }else{p.rs-=dt;if(p.rs<=0){spawn(0);ammo=w.mag;rel=0;}}
 for(int i=1;i<(int)es.size();i++)bot(i,dt);
 flash-=dt;hurt-=dt;hm-=dt;sh*=max(0.f,1-dt*10);if(!tr){tl-=dt;if(tl>0&&tl<10&&(int)tl!=(int)(tl+dt))sfxWarn();}
 for(auto&f:feed)f.second-=dt;feed.erase(remove_if(feed.begin(),feed.end(),[](pair<string,float>&f){return f.second<=0;}),feed.end());
 if(!tr&&(tl<=0||score[0]>=15||score[1]>=15)){scr=RESULT;if(score[0]>score[1])sfxWin();else sfxLose();}}

// ---------- rendering ----------
int wl(int i){return WP[i].zoom>=3?62:WP[i].pel>1?50:WP[i].rate>13?32:44;}
void drawGun(int ax,int ay,int wi,float slope,int S){int L=wl(wi);u32 d1=RGB(40,44,52),d2=RGB(75,82,95);
 struct Pt{int x,y,w,h;u32 c;};vector<Pt> ps={{-L,-8,L,7,d1},{-L,-8,L,2,d2},{-L-14,-7,14,3,d2},{0,-7,12,6,d1},{-L/2,-1,7,12,d1},{-L/2+1,-11,8,3,d2},{-10,-1,5,10,d1},{-L+4,-6,6,2,ACC},{-9,3,6,6,RGB(232,190,150)}};
 if(WP[wi].pel>1)ps.push_back({-L/2-14,-1,14,4,d2});if(WP[wi].zoom>=3)ps.push_back({-L/2-10,-14,22,5,d2});
 for(auto&p:ps)for(int i=0;i<p.w*S;i++)rect(ax+p.x*S+i,ay+p.y*S+(int)(slope*(p.x*S+i)),1,p.h*S,p.c);}
void gunfp(){Wp&w=WP[C.sel];if(ads>.7f&&w.zoom>=3)return;int L=wl(C.sel);float k=inspT>0?sinf(inspT/2.2f*PI):0;
 float ax=(W*.82f)*(1-ads)+(W/2+L/2)*ads,ay=(H-8)*(1-ads)+(H+2)*ads-10*k+sh*3;
 if(rel>0)ay+=26*min(1.f,min(rel*3,(relMax-rel)*3));drawGun((int)ax,(int)ay,C.sel,.6f*k,1);
 if(flash>0)rect((int)ax-L-24,(int)ay-9,9,9,RGB(255,230,120));}
void xhair(int cx,int cy){u32 cols[]={WHT,RGB(60,255,100),RGB(60,230,230),ACC,RGB(250,220,60)};u32 c=cols[C.xhc];float a=C.xa;int s=(int)C.xs,g=(int)C.xg,t=(int)C.xt;
 switch(C.xh){case 0:arect(cx-g-s,cy-t/2,s,t,c,a);arect(cx+g,cy-t/2,s,t,c,a);arect(cx-t/2,cy-g-s,t,s,c,a);arect(cx-t/2,cy+g,t,s,c,a);break;
  case 1:arect(cx-t,cy-t,2*t+1,2*t+1,c,a);break;case 2:circ(cx,cy,g+s,c,a,0);break;
  default:arect(cx-g-s,cy-t/2,s,t,c,a);arect(cx+g,cy-t/2,s,t,c,a);arect(cx-t/2,cy+g,t,s,c,a);}}
void ctl(int selB){for(int i=0;i<7;i++){int cx=(int)(C.bx[i]*W),cy=(int)(C.by[i]*H),r=(int)C.bs[i];float a=C.ba[i];u32 col=i==1?ACC:WHT;
 circ(cx,cy,r,col,a*.3f,1);circ(cx,cy,r,col,a,0);if(i==0)circ(cx+(int)(jx*r*.6f),cy+(int)(jy*r*.6f),r/2,WHT,a,1);else text(cx,cy-2,BL[i],WHT,1,1);
 if(i==selB)circ(cx,cy,r+3,ACC,1,0);}}
void render3d(){E&p=es[0];float ca=cosf(p.a),sa=sinf(p.a),pl=.66f*W/320.f/Zf;int hor=H/2+(int)pitch;
 for(int y=0;y<H;y++){u32 c=y<hor?mix(SK[cm][0],SK[cm][1],y/(float)max(hor,1)):mix(SK[cm][2],SK[cm][3],(y-hor)/(float)max(H-hor,1));for(int x=0;x<W;x++)fb[y*W+x]=c;}
 for(int x=0;x<W;x++){float cx=2.f*x/W-1,rx=ca-sa*pl*cx,ry=sa+ca*pl*cx;int ix=(int)p.x,iy=(int)p.y;
  float dX=rx==0?1e9f:fabsf(1/rx),dY=ry==0?1e9f:fabsf(1/ry);int stx=rx<0?-1:1,sty=ry<0?-1:1;
  float sdX=(rx<0?p.x-ix:ix+1-p.x)*dX,sdY=(ry<0?p.y-iy:iy+1-p.y)*dY;int side=0;
  while(!M[iy][ix]){if(sdX<sdY){sdX+=dX;ix+=stx;side=0;}else{sdY+=dY;iy+=sty;side=1;}}
  float pd=side?sdY-dY:sdX-dX;if(pd<.05f)pd=.05f;float wx=side?p.x+pd*rx:p.y+pd*ry;wx-=floorf(wx);
  int lh=(int)(H*Zf/pd),top=hor-(int)(lh*(1-eh)),bot=hor+(int)(lh*eh);u32 bc=WC[cm][(int)M[iy][ix]];float fog=max(.25f,1-pd/20)*(side?.78f:1);
  for(int y=max(top,0);y<min(bot,H);y++){float v=(y-top)/(float)lh;float edge=(fmodf(v*4,1)<.07f||fmodf(wx*4,1)<.05f)?.7f:1;fb[y*W+x]=mix(BLK,bc,fog*edge);}
  zb[x]=pd;}
 vector<pair<float,int>> o;for(int i=1;i<(int)es.size();i++)if(es[i].al)o.push_back({hypotf(es[i].x-p.x,es[i].y-p.y),i});sort(o.rbegin(),o.rend());
 for(auto&q:o){E&e=es[q.second];float dx=e.x-p.x,dy=e.y-p.y,ty=dx*ca+dy*sa,tx=(-dx*sa+dy*ca)/pl;if(ty<.15f)continue;
  int scx=(int)(W/2*(1+tx/ty));float u=H*Zf/ty;int shh=(int)(u*.85f),sw=max(1,shh*14/16),bt=hor+(int)(u*eh),tp=bt-shh,lf=scx-sw/2;u32 vest=e.team?BLU:ACC;
  for(int cx=lf;cx<lf+sw;cx++){if(cx<0||cx>=W||zb[cx]<ty)continue;int c=(cx-lf)*14/sw;
   for(int y=max(tp,0);y<min(bt,H);y++){char ch=SP[(y-tp)*16/max(shh,1)][c];if(ch!='.')fb[y*W+cx]=mix(pc(ch,OP[e.op],vest),BLK,min(.55f,ty/35));}}
  rect(lf,tp-4,sw,2,PNL);rect(lf,tp-4,(int)(sw*max(0.f,e.hp)/100),2,vest);}}
void hud(){E&p=es[0];Wp&w=WP[C.sel];
 if(w.zoom>=3&&ads>.5f){float R=64;for(int y=0;y<H;y++)for(int x=0;x<W;x++){float d=hypotf(x-W/2.f,y-H/2.f);if(d>R)fb[y*W+x]=mix(fb[y*W+x],BLK,ads);}rect(0,H/2,W,1,BLK);rect(W/2,0,1,H,BLK);}
 else gunfp();
 if(hurt>0)arect(0,0,W,H,ACC,min(.35f,hurt));
 xhair(W/2,H/2);
 if(hm>0&&C.hm){u32 c=hmh?ACC:WHT;for(int i=3;i<7;i++){px(W/2-i,H/2-i,c);px(W/2+i,H/2-i,c);px(W/2-i,H/2+i,c);px(W/2+i,H/2+i,c);}if(hmh)text(W/2,H/2-24,"HEADSHOT",ACC,1,1);}
 rect(6,6,34,14,RGB(120,30,40));rect(6,6,2,14,ACC);text(24,8,to_string(score[0]),WHT,2,1);rect(42,6,34,14,RGB(30,70,120));rect(42,6,2,14,BLU);text(60,8,to_string(score[1]),WHT,2,1);
 text(6,23,"KILLS "+to_string(p.k)+"  DEATHS "+to_string(p.d),WHT);int fy=33;for(auto&f:feed){text(6,fy,f.first,f.first.find("YOU")==0?ACC:DIM);fy+=8;}
 int tl_=max(0,(int)tl);rect(W-48,6,42,14,PNL);if(tr)text(W-27,10,"TRAIN",WHT,1,1);else text(W-27,8,to_string(tl_/60)+":"+(tl_%60<10?"0":"")+to_string(tl_%60),tl_<20?ACC:WHT,2,1);
 rect(W-62,6,12,14,PNL);text(W-56,8,"||",WHT,1,1);
 float br=fmodf(p.a*57.2958f+90+720,360);int cx=W/2;arect(cx-60,4,120,18,BG,.55f);
 for(int d=0;d<360;d+=15){float off=remainderf(d-br,360);if(fabsf(off)>48)continue;int x=cx+(int)(off*1.2f);if(d%90==0)text(x,6,string(1,"NESW"[d/90]),d==0?ACC:WHT,1,1);else rect(x,8,1,d%45==0?4:2,DIM);}
 rect(cx,14,1,3,ACC);text(cx,17,to_string((int)br),DIM,1,1);
 rect(6,H-14,70,5,PNL);rect(6,H-14,(int)(70*max(0.f,p.hp)/hpMax),5,p.hp>30?WHT:ACC);text(6,H-23,"HP "+to_string((int)max(0.f,p.hp)),WHT);
 text(W/2,H-20,to_string(ammo)+"/"+to_string(w.mag),rel>0?ACC:WHT,2,1);text(W/2,H-8,rel>0?"RELOADING":w.n,DIM,1,1);
 if(inspT>0)text(W/2,H-34,string("INSPECT ")+w.n,DIM,1,1);
 if(!p.al){arect(0,0,W,H,BG,.6f);text(W/2,70,"ELIMINATED",ACC,3,1);text(W/2,100,"RESPAWN IN "+to_string((int)p.rs+1),WHT,1,1);}
 ctl(-1);}
void grade(){if(!C.color)return;float S[]={1,1.45f,.8f,.75f},K[]={1,1.12f,.88f,1.15f},s=S[C.color],k=K[C.color];
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){u32&q=fb[y*W+x];float r=(q>>16)&255,g=(q>>8)&255,b=q&255,l=.3f*r+.59f*g+.11f*b;r=l+(r-l)*s;g=l+(g-l)*s;b=l+(b-l)*s;float v=1;
  if(C.color==3){float dx=(x-W/2)/(float)W,dy=(y-H/2)/(float)H;v=1-.9f*(dx*dx+dy*dy);}else if(C.color==2){r+=8;g+=4;}
  q=RGB(ci(((r-128)*k+128)*v),ci(((g-128)*k+128)*v),ci(((b-128)*k+128)*v));}}

// ---------- screens ----------
void intro(float t){rect(0,0,W,H,BG);for(int i=0;i<60;i++)rect((i*53)%W,((int)(t*(40+i%4*25))+i*37)%H,1,3,i%5?DIM:ACC);
 const char*a="PIXEL",*b="PFS";static int dn=0;static bool bm=0;int nl=0;for(int i=0;i<5;i++)if(t>=.3f+i*.18f)nl++;for(int i=0;i<3;i++)if(t>=1.4f+i*.18f)nl++;while(dn<nl){sfxThud();dn++;}if(t>2.4f&&!bm){bm=1;sfxBoom();}
 for(int i=0;i<5;i++){float k=t-.3f-i*.18f;if(k<0)continue;text(W/2-80+i*32,30+(int)(max(0.f,1-k*5)*-50),string(1,a[i]),WHT,8);}
 for(int i=0;i<3;i++){float k=t-1.4f-i*.18f;if(k<0)continue;text(W/2-48+i*32,76+(int)(max(0.f,1-k*5)*-50),string(1,b[i]),ACC,8);}
 if(t>2.4f){rect(W/2-75,122,(int)(min(1.f,(t-2.4f)*3)*150),2,ACC);text(W/2,130,"PIXEL TACTICAL SHOOTER",DIM,1,1);}
 if(t>3&&(int)(t*2)%2==0)text(W/2,158,"TAP TO START",WHT,1,1);
 for(int y=0;y<H;y+=2)arect(0,y,W,1,BLK,.15f);if((t>3&&mr)||t>7){scr=LOBBY;mr=0;}}
void lobby(float t){for(int y=0;y<H;y++){u32 c=mix(BG,RGB(28,44,60),y/(float)H);for(int x=0;x<W;x++)fb[y*W+x]=c;}
 for(int y=20;y<150;y++){int w=30+(int)((y-20)*.9f);arect(W/2-w/2,y,w,1,WHT,.05f);}
 for(int j=1;j<10;j++)arect(0,100+(int)(j*j*.9f),W,1,ACC,.25f);for(int k=-8;k<=8;k++)for(int y=100;y<H;y++)arect(W/2+k*40*(y-100)/80,y,1,1,ACC,.25f);
 const Op&o=OP[C.op];int by=40+(int)(sinf(t*2)*1.5f);rect(W/2-40,152,80,3,RGB(10,15,20));soldier(W/2-49,by,7,o);
 text(W/2,160,string(o.n)+" - "+o.role,WHT,1,1);text(W/2,170,string("PRIMARY: ")+WP[C.sel].n,DIM,1,1);
 if(btn(6,6,72,16,"ARMORY"))scr=ARMORY;if(btn(6,26,72,16,"OPERATORS"))scr=OPS;if(btn(6,46,72,16,"RANGE")){startMatch(1);return;}
 if(btn(W-62,6,56,16,"SETTINGS"))scr=SET;
 int bx=W-112,my_=H-62;text(bx,my_-9,"MAP  2V2",DIM);rect(bx,my_,106,14,PNL);text(bx+53,my_+4,MN[C.map],WHT,1,1);
 if(btn(bx,my_,12,14,"<"))C.map=(C.map+1)%2;if(btn(bx+94,my_,12,14,">"))C.map=(C.map+1)%2;
 if(btn(bx,H-40,106,32,"START GAME",1,2)){startMatch(0);save();}}
void armory(){static int cat=0;rect(0,0,W,H,BG);rect(0,0,W,22,PNL);text(60,8,"ARMORY",WHT);if(btn(6,4,40,14,"BACK")){save();scr=LOBBY;return;}
 const char*CT[]={"AR","SMG","SNIPER","SHOTGUN"};static int pv=0;int tw=(W-120)/4;for(int i=0;i<4;i++)if(btn(W-4-(4-i)*tw,4,tw-2,14,CT[i],i==cat)){cat=i;pv=i*2;}
 for(int k=0;k<2;k++){int wi=cat*2+k;if(btn(8,32+k*24,100,20,string(C.sel==wi?"* ":"")+WP[wi].n,pv==wi))pv=wi;}
 Wp&w=WP[pv];int S=W>300?2:1,ax=118+(wl(pv)+14)*S+8;drawGun(ax,60,pv,0,S);
 struct B{const char*l;float v;};B bs[]={{"DAMAGE",min(1.f,w.dmg*w.pel/110.f*(w.pel>1?.45f:1))},{"FIRE RATE",min(1.f,w.rate/20)},{"MAGAZINE",min(1.f,w.mag/33.f)},{"ZOOM",(w.zoom-1)/3}};
 for(int i=0;i<4;i++){int y=88+i*14;text(118,y,bs[i].l,DIM);rect(118,y+7,W-128,3,PNL);rect(118,y+7,(int)((W-128)*bs[i].v),3,ACC);}
 if(btn(118,H-22,W-128,16,C.sel==pv?"EQUIPPED":"EQUIP",C.sel==pv))C.sel=pv;}
void ops(){rect(0,0,W,H,BG);rect(0,0,W,22,PNL);text(60,8,"OPERATORS",WHT);if(btn(6,4,40,14,"BACK")){save();scr=LOBBY;return;}
 int cw=(W-16-16)/5,sc=cw>=50?3:2;for(int i=0;i<5;i++){int x=8+i*(cw+4),y=30;rect(x,y,cw,138,i==C.op?RGB(60,28,36):PNL);if(i==C.op)rect(x,y,cw,2,ACC);
  soldier(x+(cw-14*sc)/2,y+10,sc,OP[i]);text(x+cw/2,y+70,OP[i].n,WHT,1,1);text(x+cw/2,y+80,OP[i].role,DIM,1,1);text(x+cw/2,y+96,"HP "+to_string(OP[i].hp),WHT,1,1);
  text(x+cw/2,y+106,"SPD "+to_string((int)(OP[i].spd*100)),WHT,1,1);text(x+cw/2,y+122,i==C.op?"SELECTED":"TAP",i==C.op?ACC:DIM,1,1);
  if(mr&&hit(x,y,cw,138)){C.op=i;mr=0;}}}
void layout(){static int sl=1;static bool dr=0;rect(0,0,W,H,RGB(20,30,40));text(W/2,6,"DRAG TO MOVE",DIM,1,1);jx=jy=0;ctl(sl);
 if(mp&&my<H-40){dr=0;float bd=1e9f;for(int i=0;i<7;i++){float d=hypotf(mx-C.bx[i]*W,my-C.by[i]*H);if(d<C.bs[i]+8&&d<bd){bd=d;sl=i;dr=1;}}}
 if(!md)dr=0;if(md&&dr){C.bx[sl]=max(.03f,min(.97f,mx/W));C.by[sl]=max(.05f,min(.95f,my/H));}
 text(8,H-44,string("EDITING: ")+(sl?BL[sl]:"MOVE"),WHT);slider(8,H-30,W/2-16,"SIZE",C.bs[sl],8,40);slider(W/2+8,H-30,W/2-16,"OPACITY",C.ba[sl],.1f,1);
 if(btn(W-46,4,40,12,"DONE")){save();scr=SET;}if(btn(W-92,4,40,12,"RESET"))defLayout();}
void settings(){static int tab=0;rect(0,0,W,H,BG);rect(0,0,W,22,PNL);text(8,8,"SETTINGS",WHT);if(btn(W-46,4,40,14,"BACK")){save();scr=LOBBY;return;}
 const char*TB[]={"GRAPHICS","CONTROLS","SENSITIVITY","CROSSHAIR","AUDIO","MORE","DEVELOPERS"};for(int i=0;i<7;i++)if(btn(6,28+i*21,68,18,TB[i],i==tab))tab=i;
 int x=84,w=W-x-10,y=30;
 if(tab==0){int f[]={30,60,90,120},cur=1;for(int i=0;i<4;i++)if(f[i]==C.fps)cur=i;C.fps=f[choice(x,y,w,"FRAME RATE",{"30","60","90","120"},cur)];
  C.color=choice(x,y+30,w,"COLOR",{"NORMAL","VIVID","SOFT","REAL"},C.color);
  int a=choice(x,y+60,w,"ASPECT",{"16:9","4:3"},C.aspect);if(a!=C.aspect){C.aspect=a;W=a?240:320;mktex();}C.showFps=choice(x,y+90,w,"FPS COUNTER",{"OFF","ON"},C.showFps);}
 else if(tab==1){C.adsTog=choice(x,y,w,"ADS MODE",{"HOLD","TOGGLE"},C.adsTog);C.invY=choice(x,y+30,w,"INVERT Y",{"OFF","ON"},C.invY);
  if(btn(x,y+64,w,18,"EDIT LAYOUT / SIZE / OPACITY"))scr=LAYOUT;if(btn(x,y+88,w,18,"RESET LAYOUT"))defLayout();}
 else if(tab==2){slider(x,y,w,"LOOK SENS (SWIPE)",C.sens,.2f,3);slider(x,y+20,w,"ADS SENS",C.ads,.2f,2);slider(x,y+40,w,"DISTANCE ACCEL",C.dist,0,2);slider(x,y+60,w,"SPEED ACCEL",C.spd,0,2);
  C.gyro=choice(x,y+82,w,"GYROSCOPE",{"OFF","ON","INVERT"},C.gyro);slider(x,y+112,w,"GYRO SENS",C.gyroS,.2f,3);}
 else if(tab==3){C.xh=choice(x,y,w,"STYLE",{"CROSS","DOT","RING","T"},C.xh);C.xhc=choice(x,y+28,w,"COLOR",{"WHT","GRN","CYN","RED","YEL"},C.xhc);int sw=w-56;
  slider(x,y+56,sw,"SIZE",C.xs,1,12);slider(x,y+74,sw,"GAP",C.xg,0,10);slider(x,y+92,sw,"THICKNESS",C.xt,1,3);slider(x,y+110,sw,"OPACITY",C.xa,.2f,1);
  rect(x+w-44,y+72,44,44,RGB(60,70,80));xhair(x+w-22,y+94);}
 else if(tab==4){slider(x,y,w,"MASTER",C.vm,0,1);slider(x,y+22,w,"SFX",C.vs,0,1);slider(x,y+44,w,"MUSIC",C.vmu,0,1);if(btn(x,y+74,w/2-2,18,"TEST SHOT"))sfxShot(C.sel,1,0);if(btn(x+w/2+2,y+74,w/2-2,18,"TEST KILL"))sfxKill();}
 else if(tab==5){C.bot=choice(x,y,w,"BOT LEVEL",{"EASY","NORMAL","HARD"},C.bot);C.hm=choice(x,y+30,w,"HIT MARKER",{"OFF","ON"},C.hm);}
 else{text(x,y,"DEVELOPERS",DIM);const char*dv[]={"CLAUDE","HUANG MINGKANG","CHATGPT"};for(int i=0;i<3;i++){rect(x,y+16+i*26,w,22,PNL);rect(x,y+16+i*26,3,22,ACC);text(x+10,y+22+i*26,dv[i],WHT,2);}}}
void result(){rect(0,0,W,H,BG);bool win_=score[0]>score[1];text(W/2,40,score[0]==score[1]?"DRAW":win_?"VICTORY":"DEFEAT",win_?WHT:ACC,6,1);
 text(W/2,90,to_string(score[0])+" - "+to_string(score[1]),WHT,3,1);text(W/2,115,"KILLS "+to_string(es[0].k)+"  DEATHS "+to_string(es[0].d),DIM,1,1);
 if(btn(W/2-60,135,120,22,"BACK TO LOBBY",1)){scr=LOBBY;}}

int main(int,char**){SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,"1");SDL_SetHint(SDL_HINT_ORIENTATIONS,"LandscapeLeft LandscapeRight");
 SDL_Init(SDL_INIT_VIDEO);load();W=C.aspect?240:320;
 win=SDL_CreateWindow("PIXEL PFS",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,960,540,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
 ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);mktex();
 if(SDL_InitSubSystem(SDL_INIT_SENSOR)==0)for(int i=0;i<SDL_NumSensors();i++)if(SDL_SensorGetDeviceType(i)==SDL_SENSOR_GYRO)gy=SDL_SensorOpen(i);
 if(SDL_InitSubSystem(SDL_INIT_AUDIO)==0){SDL_AudioSpec w{};w.freq=44100;w.format=AUDIO_F32SYS;w.channels=2;w.samples=512;w.callback=acb;adev=SDL_OpenAudioDevice(0,0,&w,0,0);if(adev)SDL_PauseAudioDevice(adev,0);}
 Uint32 last=SDL_GetTicks();float gt=0,fps=0;
 for(bool run=1;run;){Uint32 now=SDL_GetTicks();float dt=min(.05f,(now-last)/1000.f);last=now;gt+=dt;fps=fps*.9f+(dt>0?1/dt:0)*.1f;
  SDL_Event e;while(SDL_PollEvent(&e)){if(e.type==SDL_QUIT)run=0;ev(e);}
  switch(scr){case INTRO:intro(gt);break;case LOBBY:lobby(gt);break;case ARMORY:armory();break;case OPS:ops();break;case SET:settings();break;case LAYOUT:layout();break;
   case MATCH:if(!paused)upd(dt);if(scr==MATCH){render3d();hud();if(paused){arect(0,0,W,H,BG,.7f);text(W/2,40,"PAUSED",WHT,3,1);
     if(btn(W/2-50,80,100,20,"RESUME",1)){paused=0;for(auto&t:ts)t.role=99;}if(btn(W/2-50,106,100,20,"EXIT")){scr=LOBBY;Zf=1;}}grade();}break;
   case RESULT:result();break;}
  if(C.showFps)text(W/2,H-6,to_string((int)fps),DIM,1,1);
  SDL_UpdateTexture(tex,0,fb,W*4);SDL_SetRenderDrawColor(ren,0,0,0,255);SDL_RenderClear(ren);int ow,oh;SDL_GetRendererOutputSize(ren,&ow,&oh);
  float vs=min(ow/(float)W,oh/(float)H);SDL_Rect d={(int)((ow-W*vs)/2),(int)((oh-H*vs)/2),(int)(W*vs),(int)(H*vs)};SDL_RenderCopy(ren,tex,0,&d);SDL_RenderPresent(ren);
  musOn=scr!=MATCH;mp=mr=0;Uint32 el=SDL_GetTicks()-now,tg=1000/C.fps;if(el<tg)SDL_Delay(tg-el);}
 save();SDL_Quit();return 0;}
