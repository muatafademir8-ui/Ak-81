/*
 * MUSTI AND CHATGPT - SQUAD OPS
 * PSP SDK C homebrew project
 * 480x272, arcade top-down sci-fi action prototype.
 *
 * Fictional opponents: rogue training robots (not real-world groups).
 * Controls: D-pad move, CROSS fire, CIRCLE reload, START pause,
 * SELECT switch language in menu.
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspge.h>
#include <pspctrl.h>
#include <psputils.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PSP_MODULE_INFO("MUSTI_SQUAD_OPS", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define SCR_W 480
#define SCR_H 272
#define PI 3.14159265f
#define MAX_BOTS 12
#define MAX_SHOTS 48
#define MAX_PARTICLES 120
#define MAX_ALLIES 2

typedef unsigned int u32;
typedef struct { float x,y,z; u32 color; } Vertex;
typedef struct { float x,y,vx,vy,life,maxlife; u32 color; int active; } Particle;
typedef struct { float x,y,vx,vy,life; int active,team; } Shot;
typedef struct { float x,y,hp,phase,speed; int active; } Bot;
typedef struct { float x,y,cool,phase; } Ally;

static unsigned int __attribute__((aligned(16))) list[262144];
static Vertex __attribute__((aligned(16))) verts[4096];
static int vcount=0;
static SceCtrlData pad,oldpad;
static int running=1, scene=0, menuSel=0, settingSel=0;
static int lang=0, soundOn=1, score=0, ammo=12, reloadTimer=0;
static int playerHP=100, shotCooldown=0, muzzle=0, frameNo=0;
static float px=240, py=218, cameraShake=0, spawnTimer=0;
static Bot bots[MAX_BOTS];
static Shot shots[MAX_SHOTS];
static Particle particles[MAX_PARTICLES];
static Ally allies[MAX_ALLIES];

static const u32 C_BG=0xFF111D2C, C_PANEL=0xFF101927, C_WHITE=0xFFF0F5FA;
static const u32 C_MUTED=0xFF8DA3B9, C_CYAN=0xFFE9D34B, C_BLUE=0xFFE06A38;
static const u32 C_RED=0xFF5557F0, C_GREEN=0xFF91E66B, C_ORANGE=0xFF55A8FF;
static const u32 C_YELLOW=0xFF6AEAFF, C_DARK=0xFF080E18, C_GRID=0xFF26384A;

static const char *langNames[]={"TURKCE","ENGLISH","PORTUGUES","ARABIC"};
static const char *labels[4][16]={
 {"BASLAT","AYARLAR","GERI","DIL","SES","ACIK","KAPALI","EGITIM ALANI","ROBOTLARI DURDUR","MERMI","SKOR","DURAKLATILDI","DEVAM ET","ANA MENU","GOREV TAMAM","GOREV BITTI"},
 {"START","SETTINGS","BACK","LANGUAGE","SOUND","ON","OFF","TRAINING ZONE","STOP THE ROGUE BOTS","AMMO","SCORE","PAUSED","RESUME","MAIN MENU","AREA SECURED","MISSION OVER"},
 {"INICIAR","OPCOES","VOLTAR","IDIOMA","SOM","LIGADO","DESLIGADO","ZONA DE TREINO","PARE OS ROBOS","MUNICAO","PONTOS","PAUSADO","CONTINUAR","MENU PRINCIPAL","AREA SEGURA","FIM DA MISSAO"},
 {"ابدأ","الإعدادات","رجوع","اللغة","الصوت","تشغيل","إيقاف","منطقة التدريب","أوقف الروبوتات","الذخيرة","النقاط","متوقف","متابعة","القائمة الرئيسية","تم تأمين المنطقة","انتهت المهمة"}
};
/* Compact 5x7 font: uppercase letters, digits and punctuation. */
static const unsigned char font5x7[38][5]={
 {0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},
 {0x3E,0x41,0x41,0x41,0x22},{0x7F,0x41,0x41,0x22,0x1C},
 {0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},
 {0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},
 {0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},
 {0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},
 {0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},
 {0x3E,0x41,0x41,0x41,0x3E},{0x7F,0x09,0x09,0x09,0x06},
 {0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},
 {0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},
 {0x3F,0x40,0x38,0x40,0x3F},{0x63,0x14,0x08,0x14,0x63},
 {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},
 {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
 {0x62,0x51,0x49,0x49,0x46},{0x22,0x41,0x49,0x49,0x36},
 {0x18,0x14,0x12,0x7F,0x10},{0x2F,0x49,0x49,0x49,0x31},
 {0x3E,0x49,0x49,0x49,0x32},{0x01,0x71,0x09,0x05,0x03},
 {0x36,0x49,0x49,0x49,0x36},{0x26,0x49,0x49,0x49,0x3E},
 {0x00,0x00,0x5F,0x00,0x00},{0x00,0x60,0x60,0x00,0x00}
};

static int justPressed(unsigned int b){ return (pad.Buttons & b) && !(oldpad.Buttons & b); }
static float clampf(float x,float a,float b){ return x<a?a:(x>b?b:x); }
static void addVertex(float x,float y,u32 c){ if(vcount<4090){ verts[vcount].x=x; verts[vcount].y=y; verts[vcount].z=0; verts[vcount].color=c; vcount++; } }

static void quad(float x,float y,float w,float h,u32 c){
 addVertex(x,y,c); addVertex(x+w,y,c); addVertex(x,y+h,c);
 addVertex(x+w,y,c); addVertex(x+w,y+h,c); addVertex(x,y+h,c);
}
static void drawFlush(void){
 if(vcount){ sceGuDrawArray(GU_TRIANGLES,GU_COLOR_8888|GU_VERTEX_32BITF, vcount,0,verts); vcount=0; }
}
static void box(float x,float y,float w,float h,u32 c){ quad(x,y,w,h,c); drawFlush(); }
static void outline(float x,float y,float w,float h,u32 c){
 box(x,y,w,2,c); box(x,y+h-2,w,2,c); box(x,y,2,h,c); box(x+w-2,y,2,h,c);
}
static int glyphIndex(char c){
 if(c>='A'&&c<='Z') return c-'A';
 if(c>='0'&&c<='9') return 26+c-'0';
 if(c==':') return 36; if(c==' ') return 37; return 37;
}
static void text(float x,float y,const char *s,u32 c,int scale){
 int i; for(i=0;s[i];i++){
  int g=glyphIndex(s[i]),col,row;
  for(col=0;col<5;col++) for(row=0;row<7;row++)
   if(font5x7[g][col]&(1<<row)) box(x+i*6*scale+col*scale,y+row*scale,scale,scale,c);
 }
}
static const char *L(int id){ return labels[lang][id]; }

static void background(void){
 int x,y;
 box(0,0,SCR_W,SCR_H,C_BG);
 for(y=18;y<SCR_H;y+=24) box(0,y,SCR_W,1,C_GRID);
 for(x=0;x<SCR_W;x+=32) box(x,18,1,SCR_H-18,C_GRID);
 /* industrial cover blocks and illuminated floor lanes */
 box(18,42,66,34,0xFF293C50); outline(18,42,66,34,0xFF526A80);
 box(395,38,67,40,0xFF293C50); outline(395,38,67,40,0xFF526A80);
 box(24,155,42,24,0xFF26384B); box(414,151,42,27,0xFF26384B);
 box(0,17,SCR_W,2,C_CYAN); box(0,269,SCR_W,2,C_CYAN);
}
static void character(float x,float y,u32 body,int isPlayer){
 /* layered suit silhouette, visor, shoulder plates and blaster */
 box(x-7,y-5,14,15,0xFF111827);
 box(x-5,y-12,10,9,0xFFE2EAF2);
 box(x-4,y-10,8,4,isPlayer?C_CYAN:C_ORANGE);
 box(x-9,y-4,4,9,body); box(x+5,y-4,4,9,body);
 box(x-6,y+8,5,7,0xFF566579); box(x+2,y+8,5,7,0xFF566579);
 box(x-5,y-4,10,12,body);
 box(x+5,y-2,13,4,0xFF202C3B);
 box(x+15,y-3,4,6,0xFF9AB0C7);
 if(isPlayer && muzzle>0){
  box(x+19,y-6,10,12,C_YELLOW); box(x+23,y-3,9,6,0xFFFFFFFF);
  box(x+27,y-1,5,2,C_WHITE);
 }
}
static void particleBurst(float x,float y,u32 color,int count){
 int i; for(i=0;i<MAX_PARTICLES;i++) if(!particles[i].active){
  int j; for(j=0;j<count && i<MAX_PARTICLES;j++,i++){
   float a=((float)rand()/RAND_MAX)*2*PI;
   float sp=0.7f+((float)rand()/RAND_MAX)*2.7f;
   particles[i].x=x; particles[i].y=y; particles[i].vx=cosf(a)*sp;
   particles[i].vy=sinf(a)*sp; particles[i].life=9+(rand()%18);
   particles[i].maxlife=particles[i].life; particles[i].color=color; particles[i].active=1;
  } break;
 }
}
static void spawnBot(void){
 int i; for(i=0;i<MAX_BOTS;i++) if(!bots[i].active){
  bots[i].x=30+rand()%420; bots[i].y=38+rand()%85; bots[i].hp=2;
  bots[i].phase=(float)(rand()%628)/100.0f; bots[i].speed=.25f+(rand()%40)/100.0f;
  bots[i].active=1; break;
 }
}
static int nearestBot(float x,float y){
 int i,best=-1; float bd=999999;
 for(i=0;i<MAX_BOTS;i++) if(bots[i].active){
  float dx=bots[i].x-x,dy=bots[i].y-y,d=dx*dx+dy*dy;
  if(d<bd){bd=d;best=i;}
 } return best;
}
static void fireShot(float x,float y,float dx,float dy,int team){
 int i; float len=sqrtf(dx*dx+dy*dy); if(len<.01f)return;
 for(i=0;i<MAX_SHOTS;i++) if(!shots[i].active){
  shots[i].x=x;shots[i].y=y;shots[i].vx=dx/len*6.2f;shots[i].vy=dy/len*6.2f;
  shots[i].life=55;shots[i].team=team;shots[i].active=1;break;
 }
}
static void resetGame(void){
 int i; memset(bots,0,sizeof(bots));memset(shots,0,sizeof(shots));memset(particles,0,sizeof(particles));
 px=240;py=218;playerHP=100;ammo=12;reloadTimer=0;score=0;spawnTimer=0;
 for(i=0;i<4;i++)spawnBot();
 allies[0].x=183;allies[0].y=218;allies[0].cool=8;
 allies[1].x=297;allies[1].y=218;allies[1].cool=25;
 scene=2;
}
static void hud(void){
 box(0,0,SCR_W,18,C_DARK);
 text(8,5,"TRAINING ZONE",C_CYAN,1);
 text(180,5,"SCORE",C_WHITE,1); text(216,5,"000000",C_YELLOW,1);
 /* score rendered separately for compact PSP bitmap font */
 {char s[16];sprintf(s,"%06d",score);box(213,3,38,12,C_DARK);text(215,5,s,C_YELLOW,1);}
 text(365,5,"SQUAD 3",C_GREEN,1);
 box(10,245,104,7,0xFF42536A);box(10,245,playerHP,7,C_GREEN);
 text(10,256,"HP",C_WHITE,1);
 {char a[16];sprintf(a,"%02d/12",ammo);text(383,247,"AMMO",C_WHITE,1);text(420,247,a,C_YELLOW,1);}
}
static void drawWorld(void){
 int i; background();
 for(i=0;i<MAX_ALLIES;i++)character(allies[i].x,allies[i].y,C_GREEN,0);
 for(i=0;i<MAX_BOTS;i++)if(bots[i].active){
  character(bots[i].x,bots[i].y,C_RED,0);
  box(bots[i].x-9,bots[i].y-19,18,3,0xFF465366);
  box(bots[i].x-9,bots[i].y-19,bots[i].hp*9,3,C_ORANGE);
 }
 for(i=0;i<MAX_SHOTS;i++)if(shots[i].active){
  u32 c=shots[i].team?C_GREEN:C_YELLOW;
  box(shots[i].x-5,shots[i].y-2,10,4,c);
  box(shots[i].x-8,shots[i].y-1,4,2,C_WHITE);
 }
 for(i=0;i<MAX_PARTICLES;i++)if(particles[i].active){
  float f=particles[i].life/particles[i].maxlife;
  box(particles[i].x,particles[i].y,2+f*2,2+f*2,particles[i].color);
 }
 character(px,py,C_BLUE,1);
 hud();
}
static void updateWorld(void){
 int i,j; sceCtrlReadBufferPositive(&pad,1);
 if(pad.Buttons&PSP_CTRL_LEFT)px-=2.0f;
 if(pad.Buttons&PSP_CTRL_RIGHT)px+=2.0f;
 if(pad.Buttons&PSP_CTRL_UP)py-=2.0f;
 if(pad.Buttons&PSP_CTRL_DOWN)py+=2.0f;
 px=clampf(px,16,464);py=clampf(py,42,237);
 if(justPressed(PSP_CTRL_START)){scene=3;return;}
 if(shotCooldown>0)shotCooldown--;if(muzzle>0)muzzle--;
 if(reloadTimer>0){reloadTimer--;if(reloadTimer==0)ammo=12;}
 if((pad.Buttons&PSP_CTRL_CIRCLE)&&reloadTimer==0&&ammo<12)reloadTimer=48;
 if((pad.Buttons&PSP_CTRL_CROSS)&&shotCooldown==0&&reloadTimer==0&&ammo>0){
  int n=nearestBot(px,py-10);float dx=0,dy=-1;
  if(n>=0){dx=bots[n].x-px;dy=bots[n].y-py;}
  fireShot(px+18,py-5,dx,dy,0);ammo--;shotCooldown=10;muzzle=3;
  particleBurst(px+26,py-5,C_YELLOW,5);
 }
 for(i=0;i<MAX_ALLIES;i++){
  allies[i].cool--;
  if(allies[i].cool<=0){
   int n=nearestBot(allies[i].x,allies[i].y-10);
   if(n>=0){fireShot(allies[i].x+16,allies[i].y-5,bots[n].x-allies[i].x,bots[n].y-allies[i].y,1);allies[i].cool=42;}
  }
 }
 spawnTimer++;
 /* Ten-bot training wave; stop spawning once the wave is cleared. */
 if(spawnTimer>220 && score<1000){spawnTimer=0;spawnBot();}
 for(i=0;i<MAX_BOTS;i++)if(bots[i].active){
  bots[i].phase+=.025f;bots[i].x+=sinf(bots[i].phase)*.35f;bots[i].y+=.10f;
  if(bots[i].y>py-24)bots[i].y=42;
 }
 for(i=0;i<MAX_SHOTS;i++)if(shots[i].active){
  Shot *s=&shots[i];s->x+=s->vx;s->y+=s->vy;s->life--;
  if(s->team==0){
   for(j=0;j<MAX_BOTS;j++)if(bots[j].active){
    float dx=s->x-bots[j].x,dy=s->y-bots[j].y;
    if(dx*dx+dy*dy<150){
     bots[j].hp--;particleBurst(bots[j].x,bots[j].y,C_ORANGE,9);s->active=0;
     if(bots[j].hp<=0){score+=100;particleBurst(bots[j].x,bots[j].y,C_CYAN,20);bots[j].active=0;}
     break;
    }
   }
  }
  if(s->life<=0||s->x<0||s->x>SCR_W||s->y<0||s->y>SCR_H)s->active=0;
 }
 for(i=0;i<MAX_PARTICLES;i++)if(particles[i].active){
  Particle *p=&particles[i];p->x+=p->vx;p->y+=p->vy;p->vx*=.94f;p->vy*=.94f;
  if(--p->life<=0)p->active=0;
 }
 if(score>=1000){
  int remaining=0;
  for(i=0;i<MAX_BOTS;i++) if(bots[i].active) remaining++;
  if(remaining==0) scene=5;
 }
}
static void frameStart(void){
 sceGuStart(GU_DIRECT,list);
 sceGuClearColor(C_BG);sceGuClearDepth(0);sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
 sceGuDisable(GU_DEPTH_TEST);sceGuDisable(GU_TEXTURE_2D);sceGuShadeModel(GU_SMOOTH);
 sceGuEnable(GU_SCISSOR_TEST);sceGuScissor(0,0,SCR_W,SCR_H);
 sceGuEnable(GU_BLEND);sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
}
static void frameEnd(void){
 drawFlush();sceGuFinish();sceGuSync(0,0);sceDisplayWaitVblankStart();sceGuSwapBuffers();
}
static void initGraphics(void){
 sceGuInit();sceGuStart(GU_DIRECT,list);
 sceGuDrawBuffer(GU_PSM_8888,(void*)0,512);sceGuDispBuffer(SCR_W,SCR_H,(void*)0x88000,512);
 sceGuDepthBuffer((void*)0x110000,512);sceGuOffset(2048-SCR_W/2,2048-SCR_H/2);
 sceGuViewport(2048,2048,SCR_W,SCR_H);sceGuDepthRange(65535,0);
 sceGuScissor(0,0,SCR_W,SCR_H);sceGuEnable(GU_SCISSOR_TEST);
 sceGuDisable(GU_DEPTH_TEST);sceGuFinish();sceGuSync(0,0);
 sceDisplayWaitVblankStart();sceGuDisplay(GU_TRUE);
}
static void menuScreen(void){
 background();box(48,30,384,212,C_DARK);outline(48,30,384,212,C_CYAN);
 text(126,48,"MUSTI AND CHATGPT",C_CYAN,2);
 text(151,76,"SQUAD OPS",C_WHITE,2);
 box(120,119,240,29,menuSel==0?C_BLUE:C_PANEL);text(202,130,L(0),C_WHITE,1);
 box(120,157,240,29,menuSel==1?C_BLUE:C_PANEL);text(198,168,L(1),C_WHITE,1);
 text(150,211,"PSP HOME BREW EDITION",C_MUTED,1);
}
static void settingsScreen(void){
 background();box(48,30,384,212,C_DARK);outline(48,30,384,212,C_CYAN);
 text(174,51,L(1),C_CYAN,2);
 box(83,105,314,30,settingSel==0?C_BLUE:C_PANEL);
 text(105,116,L(3),C_WHITE,1);text(230,116,langNames[lang],C_YELLOW,1);
 box(83,145,314,30,settingSel==1?C_BLUE:C_PANEL);
 text(105,156,L(4),C_WHITE,1);text(230,156,soundOn?L(5):L(6),C_YELLOW,1);
 text(190,207,L(2),C_MUTED,1);
}
static void overlay(const char *title,const char *subtitle){
 drawWorld();box(58,76,364,116,C_DARK);outline(58,76,364,116,C_CYAN);
 text(155,96,title,C_CYAN,2);text(95,129,subtitle,C_WHITE,1);
 text(111,162,"X RESUME      O MAIN MENU",C_MUTED,1);
}
int main(void){
 sceCtrlSetSamplingCycle(0);sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
 initGraphics();sceKernelDcacheWritebackAll();
 while(running){
  sceCtrlReadBufferPositive(&pad,1);
  frameStart();
  if(scene==0){
   background();text(132,92,"MUSTI",C_WHITE,3);text(222,92,"AND",C_CYAN,2);
   text(286,92,"CHATGPT",C_WHITE,2);text(126,137,"PRESENTS SQUAD OPS",C_MUTED,1);
   text(177,205,"PRESS START",C_YELLOW,1);
   if(justPressed(PSP_CTRL_START)||justPressed(PSP_CTRL_CROSS))scene=1;
  }else if(scene==1){
   menuScreen();
   if(justPressed(PSP_CTRL_UP)||justPressed(PSP_CTRL_DOWN))menuSel=1-menuSel;
   if(justPressed(PSP_CTRL_CROSS)){if(menuSel==0)resetGame();else scene=4;}
  }else if(scene==4){
   settingsScreen();
   if(justPressed(PSP_CTRL_UP)||justPressed(PSP_CTRL_DOWN))settingSel=1-settingSel;
   if(justPressed(PSP_CTRL_LEFT)||justPressed(PSP_CTRL_RIGHT)||justPressed(PSP_CTRL_CROSS)){
    if(settingSel==0)lang=(lang+1)%4;else soundOn=!soundOn;
   }
   if(justPressed(PSP_CTRL_CIRCLE)||justPressed(PSP_CTRL_START))scene=1;
  }else if(scene==2){
   updateWorld();drawWorld();
   if(playerHP<=0)scene=6;
  }else if(scene==3){
   overlay(L(11),"X RESUME      O MAIN MENU");
   if(justPressed(PSP_CTRL_CROSS))scene=2;
   if(justPressed(PSP_CTRL_CIRCLE))scene=1;
  }else if(scene==5||scene==6){
   char scoreText[32];sprintf(scoreText,"SCORE %06d",score);
   overlay(L(scene==5?14:15),scoreText);
   if(justPressed(PSP_CTRL_CROSS))resetGame();
   if(justPressed(PSP_CTRL_CIRCLE))scene=1;
  }
  frameEnd();oldpad=pad;frameNo++;
 }
 sceGuTerm();sceKernelExitGame();return 0;
}
