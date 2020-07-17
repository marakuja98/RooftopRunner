#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "image.h"

#define VELICINAZGRADE 3.0
#define VISINAZGRADE 5
#define BROJZGRADA 25
#define MANJARAZDALJINA 3
#define VECARAZDALJINA 6

#define FILENAME0 "textures/zgrada1.bmp"
#define FILENAME1 "textures/nebo.bmp"
#define FILENAME2 "textures/pod1.bmp"
#define FILENAME3 "textures/zgrada2.bmp"
#define FILENAME4 "textures/zgrada3.bmp"

// `60` je za 60 fps
#define INTERVAL_AZURIRANJA (1000/60)

#define TIMER 0

#define pi M_PI

static int window_width, window_height;

/*deklaracija promenljivih koje uticu na igru*/
static float delta_z = 0.1f;
float trenutnaZKoordinata = 0;
float sumaRazdaljina;
float poslednjaZ;
float pad = 0;
int igraUToku = 0;
float parametarSkoka = 0;

/*deklaracija callback funkcija*/
static void on_timer(int value);
static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);

/*deklaracija funkcije za inicijalizaciju tekstura*/
static void inicijalizujTeksture(void);

/*deklaracija funkcije za jednokratno generisanje razdaljina i teksura*/
void nadjiRazdaljine();

/*deklaracija funkcija za crtanje*/
void iscrtajZgrade();
void nacrtajZgradu(int tekstura);
void nacrtajCicu();
void nacrtajNebo();
void nacrtajPod();

/*deklaracija funkcija za ispitivanje pada cice*/
int pronadjiJednakiIliManji(int l, int d,float z);
int proveriDaLiPada();

/*deklaracija niza identifikatora tekstura*/
static GLuint names[6];

/*deklaracija nizova generisanih funkcijom nadjiRazdaljine*/
int razdaljine[BROJZGRADA];
int teksture[BROJZGRADA];
int pozicijeIvicaZgrada[BROJZGRADA*2];

/*strukrura koja opisuje stanje cice koje je u pocetku HODA*/
typedef enum { HODA, SKACE, DUGACKO_SKACE, PADA, ZAVRSIO } Stanje;
Stanje stanje = HODA;

int main(int argc, char **argv){
    
    /*incijalizacija*/
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1200, 800);
    glutInitWindowPosition(200, 100);
    glutCreateWindow(argv[0]);

    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);
   
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    /*seed za random broj*/        
    srand(time(NULL));

    /*generisanje razdaljina i tekstura*/
    nadjiRazdaljine();
    inicijalizujTeksture();

    glutTimerFunc(INTERVAL_AZURIRANJA, on_timer, TIMER);

    glutMainLoop();

    return 0;
}


static void on_timer(int value){
    (void) (value); // Ovo se ovako pise jer se ne koristi parametar value

    if (!igraUToku && stanje != ZAVRSIO) { 
        // Ako igra nije u toku, samo ponovo okini tajmer za 50ms.
        glutTimerFunc(INTERVAL_AZURIRANJA, on_timer, TIMER);
        return;
    }

    if (stanje != PADA && stanje != ZAVRSIO) { 
        // U svim slucajeva osim pri padanju i kraja, cica se krece unapred
        trenutnaZKoordinata += delta_z;
    }

    /*U zavisnosti od stanja u kom je cica desava se akcija: */
    switch (stanje) {
        case HODA: {
            if (proveriDaLiPada()) {
                stanje = PADA;
            }
        } break;

        case SKACE: {
            parametarSkoka+=0.02f;
            if(parametarSkoka > 1.0) {
                parametarSkoka = 0;
                stanje = HODA;
            }
        } break;
        case DUGACKO_SKACE: {
            parametarSkoka+=0.014f;
            if(parametarSkoka > 1.0) {
                parametarSkoka = 0;
                stanje = HODA;
            }
        } break;

        case PADA: {
            pad += 0.1f;
            if (pad >= 2 * VISINAZGRADE) {
                sleep(2);
                exit(0);
            }
        } break;

        case ZAVRSIO:{
            parametarSkoka += 0.02f;
            if(parametarSkoka > 1.0) {
                parametarSkoka = 0;
            }
        }break;

        default:
            break;
    }

    glutPostRedisplay();

    /*rekurzivno pozivanje funkcije*/
    glutTimerFunc(INTERVAL_AZURIRANJA, on_timer, TIMER);
}

static void on_keyboard(unsigned char key, int x, int y){
    switch (key) {
        
        /*Izlazak iz igre*/
        case 27:
            exit(0);
        
        /*Kreni*/
        case 'G':
        case 'g': {
            igraUToku = 1;
        }         break;

        /*Stani*/
        case 'S':
        case 's':
            igraUToku = 0;
            break;
    
        /*Kraci skok*/
        case 'J':
        case 'j':
            if (stanje == HODA) {
                stanje = SKACE;
            }
            break;

        /*Duzi skok*/
        case 'K':
        case 'k':
        if (stanje == HODA) {
            stanje = DUGACKO_SKACE;
        }
        break;

        /*Resetuj igru*/
        case 'R':
        case 'r':
            parametarSkoka = 0;
            trenutnaZKoordinata = 0;
            igraUToku = 0;
            stanje = HODA;
            pad = 0;
            glutPostRedisplay();
            break;
            
        default:
            break;
    }
}

static void on_reshape(int width, int height){
    window_width = width;
    window_height = height;
}

static void on_display(void){
    
    /*Podesavanje osvetljenja, zapremine pogleda, tacke gledista, vidnog polja*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    
    GLfloat light_position[] = { -5, 2*VISINAZGRADE+2.5, trenutnaZKoordinata+1, 0 };
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1 };
    GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1 };    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
            80,
            window_width/(float)window_height,
            1, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(-5, 2*VISINAZGRADE+2.5, 2+trenutnaZKoordinata-1,
              0, 2*VISINAZGRADE, trenutnaZKoordinata + VELICINAZGRADE,
              0, 1, 0);

    /*U slucaju da je cica dosao do kraja poslednje zgrade stanje se prebacuje u ZAVRSIO*/
    if (trenutnaZKoordinata + 0.5 >= poslednjaZ) {
        stanje = ZAVRSIO;
    }

    glPushMatrix();
        /*crta se cica, skalira, translira*/
        glTranslatef(0, 2 * VISINAZGRADE +1 + sin(pi * parametarSkoka) - pad, trenutnaZKoordinata + 0.2);
        glScalef(0.2,0.2,0.2);
        nacrtajCicu();
    glPopMatrix();

    glPushMatrix();
        /*crtaju se zgrade*/
        iscrtajZgrade();
    glPopMatrix();
    
    glPushMatrix();
        /*crta se nebo*/
        nacrtajNebo();
    glPopMatrix();

    glPushMatrix();
        /*crta se pod*/
        nacrtajPod();
    glPopMatrix();

    glutSwapBuffers();
}

void nadjiRazdaljine(){
    /*funkcija za pronalazenje random rasporeda zgrada i tekstura
    pri cemu je prva zgrada uvek iste teksture i dodiruje y osu*/
    int i = 0;

    int randR = 0;
    int randT = 0;
    int randInt;

    razdaljine[0] = 0;
    teksture[0] = 1;

    for(i = 1;i<BROJZGRADA;i++){
        randInt = rand();
        randR = randInt%2;
        randT = randInt%3+1;
        teksture[i] = randT;
        if(randR == 0){
            razdaljine[i] = MANJARAZDALJINA;
            sumaRazdaljina+=MANJARAZDALJINA;
        }
        else{
            razdaljine[i] = VECARAZDALJINA;
            sumaRazdaljina+=VECARAZDALJINA;
        }
    }

    /*promenljiva koja sluzi za identifikovanje kraja igre*/
    poslednjaZ = (BROJZGRADA)*VELICINAZGRADE + sumaRazdaljina;
}

void iscrtajZgrade(){
    /*funkcija koja crta zgrade tako sto svaku nacrta posebno 
    funkcijom nacrtajZgradu, potom je translira na odgovarajuce mesto
    u zavisnosti od rasporeda koji je odredjen u funkciji nadjiRazdaljine*/
    int i;
    int prethodnaDaljaStranica = 0;
    int trenutnaBlizaStranica = 0;
    for(i = 0;i<BROJZGRADA;i++){
        
        trenutnaBlizaStranica = prethodnaDaljaStranica + razdaljine[i];
        
        glPushMatrix();
            glTranslatef(0, 0, trenutnaBlizaStranica);
            nacrtajZgradu(teksture[i]);
        glPopMatrix();

        prethodnaDaljaStranica = trenutnaBlizaStranica + VELICINAZGRADE; 

        /*pamti gde koja zgrada pocinje i zavrsava sto se koristi posle u proveri
        da li cica treba da padne ili ne*/
        pozicijeIvicaZgrada[2*i] = trenutnaBlizaStranica;
        pozicijeIvicaZgrada[2*i+1] = trenutnaBlizaStranica + VELICINAZGRADE;
    }
}

void nacrtajCicu(){

    /*desna noga*/
     glPushMatrix();
        glColor3f(29/255.0, 47/255.0, 119/255.0);
        glRotatef(20*sin(-trenutnaZKoordinata), 1, 0, 0 );
        glTranslatef(0.75, -2.5, 0);
        glScalef(1, 4, 2);
        glutSolidCube(.5);
    glPopMatrix();
    
    /*leva ruka*/
     glPushMatrix();
        glColor3f(196/255.0,114/255.0,5/255.0);
        glTranslatef(0, 3.5, 0);
        glRotatef(20*sin(-trenutnaZKoordinata), 1, 0, 0 );
        glTranslatef(0, -3.5, 0);
        glTranslatef(-1.25, 0.75, 0);
        glScalef(1, 3, 2);
        glutSolidCube(.5);
    glPopMatrix();
    
     /*leva noga*/
     glPushMatrix();
        glColor3f(29/255.0, 47/255.0, 119/255.0);
        glRotatef(20*sin(trenutnaZKoordinata), 1, 0, 0 );
        glTranslatef(-0.75, -2.5, 0);
        glScalef(1, 4, 2);
        glutSolidCube(.5);
    glPopMatrix();
    
    /*desna ruka*/
     glPushMatrix();
        glColor3f(196/255.0,114/255.0,5/255.0);
        glTranslatef(0, 3.5, 0);
        glRotatef(20*sin(trenutnaZKoordinata), 1, 0, 0 );
        glTranslatef(0, -3.5, 0);
        glTranslatef(1.25, 0.75, 0);
        glScalef(1, 3, 2);
        glutSolidCube(.5);
    glPopMatrix();
    
    /*glava*/
    glPushMatrix();
        glColor3f(229/255.0,180/255.0,94/255.0);
        glTranslatef(0, 2, 0);
        glutSolidSphere(.6, 20, 20);
    glPopMatrix();
    
    /*telo*/
    glPushMatrix();
        glColor3f(196/255.0,114/255.0,5/255.0);
        glScalef(2, 3, 1);
        glutSolidCube(1);
    glPopMatrix();
}

int pronadjiJednakiIliManji(int l, int d, float z){
    /*pomocna funkcija za identifikovanje da li je cica na zgradi ili ne*/

    for (int i = 0; i < d; i++) {
        if (pozicijeIvicaZgrada[i] > z) {
            return i - 1;
        }
    }
    return -1;
}

int proveriDaLiPada() {

    /*kako se u nizu pozicijaIviceZgrada nalaze z koordinate ivica zgrada
    u poretku : na parnim mestima su koordinate pocetka zgrade,
    na neparnim krajevi, tako da ako se cica nalazi na 3.5 a zgrada je 0-3
    pronadje funkcijom pronadjiJednakiIliManji se pronadje 3, ali se identifikuje da
    je to na neparnom mestu, dakle treba da padne.*/

    int pozicijaPrvePrethodne = pronadjiJednakiIliManji(0,2*BROJZGRADA,trenutnaZKoordinata);
    return pozicijaPrvePrethodne % 2 != 0;
}

void nacrtajZgradu(int tekstura){
    /*funkcija koja sluzi da icrta svaku zgradu sa odgovarajucom teksturom
    sklapajuci 4 pravougaonika*/

    /*redni broj teksture*/
    int rbTeksture;
    if(tekstura == 1){
        rbTeksture = 1;
    }
    else if(tekstura == 2){
        rbTeksture = 4;
    }
    else{
        rbTeksture = 5;
    }

    /*Krov svake zgrade je isti, radi preglednosti: */
    glBindTexture(GL_TEXTURE_2D, names[1]);
    glBegin(GL_QUADS);
        // Gornja strana
        glNormal3f(0, 1, 0);

        glTexCoord2f(0.8, 0);
        glVertex3f(-VELICINAZGRADE/2.0,2*VISINAZGRADE , 0);

        glTexCoord2f(1, 0);
        glVertex3f(VELICINAZGRADE/2.0,2*VISINAZGRADE , 0);

        glTexCoord2f(1, 1);
        glVertex3f(VELICINAZGRADE/2.0,2*VISINAZGRADE , VELICINAZGRADE);

        glTexCoord2f(0.8, 1);
        glVertex3f(-VELICINAZGRADE/2.0,2*VISINAZGRADE , VELICINAZGRADE);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    /*lepljenje tekstura na ostale zidove zgrada: */

    glBindTexture(GL_TEXTURE_2D, names[rbTeksture]);
    glBegin(GL_QUADS);

        // Zapadna strana
        glNormal3f(0, 0, 1);
        
        glTexCoord2f(0, 0);
        glVertex3f(-VELICINAZGRADE/2.0, 0, 0);
        
        glTexCoord2f(0.7, 0);
        glVertex3f(VELICINAZGRADE/2.0, 0, 0);
        
        glTexCoord2f(0.7, 0.5);
        glVertex3f(VELICINAZGRADE/2.0, 2*VISINAZGRADE, 0);
        
        glTexCoord2f(0, 0.5);
        glVertex3f(-VELICINAZGRADE/2.0, 2*VISINAZGRADE, 0);
        
        // Juzna strana
        glNormal3f(1, 0, 0);
        
        glTexCoord2f(0, 0);
        glVertex3f(-VELICINAZGRADE/2.0, 0, 0);
        
        glTexCoord2f(0.7, 0);
        glVertex3f(-VELICINAZGRADE/2.0, 0, VELICINAZGRADE);
        
        glTexCoord2f(0.7, 0.5);
        glVertex3f(-VELICINAZGRADE/2.0, 2*VISINAZGRADE, VELICINAZGRADE);
        
        glTexCoord2f(0, 0.5);
        glVertex3f(-VELICINAZGRADE/2.0, 2*VISINAZGRADE, 0);

        // Istocna strana
        glNormal3f(0, 0, 1);
        
        glTexCoord2f(0, 0);
        glVertex3f(-VELICINAZGRADE/2.0, 0, VELICINAZGRADE);
        
        glTexCoord2f(0.7, 0);
        glVertex3f(VELICINAZGRADE/2.0, 0, VELICINAZGRADE);
        
        glTexCoord2f(0.7, 0.5);
        glVertex3f(VELICINAZGRADE/2.0, 2*VISINAZGRADE, VELICINAZGRADE);
        
        glTexCoord2f(0, 0.5);
        glVertex3f(-VELICINAZGRADE/2.0, 2*VISINAZGRADE, VELICINAZGRADE);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

}

static void inicijalizujTeksture(void){
    /*funkcija sa sedmog casa vezbi koja koristi biblioteku image.c*/

    Image * image;
    glEnable(GL_DEPTH_TEST);

    /*ukljucivanje odredjenih flegova za lepljenje tekstura*/
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

    /*inicijalizacija*/
    image = image_init(0, 0);
    glGenTextures(6, names);

    /*TEKSTURA ZGRADE_1*/
    image_read(image, FILENAME0);
    glBindTexture(GL_TEXTURE_2D, names[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /*TEKSTURA NEBA*/
    image_read(image, FILENAME1);
    glBindTexture(GL_TEXTURE_2D, names[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /*TEKSTURA PODA*/
    image_read(image, FILENAME2);
    glBindTexture(GL_TEXTURE_2D, names[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    
    /*TEKSTURA ZGRADE_2*/
    image_read(image, FILENAME3);
    glBindTexture(GL_TEXTURE_2D, names[4]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /*TEKSTURA ZGRADE_3*/
    image_read(image, FILENAME4);
    glBindTexture(GL_TEXTURE_2D, names[5]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /* Iskljucujemo aktivnu teksturu */
    glBindTexture(GL_TEXTURE_2D, 0);
    image_done(image);
}

void nacrtajNebo(){
    /*funkcija za isrtavanje neba*/

    glBindTexture(GL_TEXTURE_2D, names[2]);
    glBegin(GL_QUADS);

        glNormal3f(1, 0, 0);
        
        glTexCoord2f(0, 0);
        glVertex3f(20, 0, -40);
        
        glTexCoord2f(1, 0);
        glVertex3f(20, 0, ((BROJZGRADA+1)*VELICINAZGRADE + sumaRazdaljina+1)+120);
        
        glTexCoord2f(1, 1);
        glVertex3f(20, 30, ((BROJZGRADA+1)*VELICINAZGRADE + sumaRazdaljina+1)+120);
        
        glTexCoord2f(0, 1);
        glVertex3f(20, 30,-40);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void nacrtajPod(){
    /*funkcija za isrtavanje poda*/

    glBindTexture(GL_TEXTURE_2D, names[3]);
    glBegin(GL_QUADS);

        glNormal3f(0, 1, 0);
        
        glTexCoord2f(0, 50);
        glVertex3f(20, 0, -40);
        
        glTexCoord2f(50, 50);
        glVertex3f(20, 0, ((BROJZGRADA+1)*VELICINAZGRADE + sumaRazdaljina+1)+120);
        
        glTexCoord2f(50, 1);
        glVertex3f(-20, 0, ((BROJZGRADA+1)*VELICINAZGRADE + sumaRazdaljina+1)+120);
        
        glTexCoord2f(0, 0);
        glVertex3f(-20, 0,-40);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}