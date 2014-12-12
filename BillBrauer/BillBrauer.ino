/*
BillBrauer est une cuve de brassage avec moteur, une balance integré avec stockage continu des paramètres.
*/

#include <TFT.h>
#include <SPI.h>
//#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Time.h>
#include <TimeAlarms.h>
#include "BillBrauer.h"

/*Une fois le code acheve, il faudra enlever ces définitions et les remplacer dans le code. Il est également possible d'utiliser les registres directement pour etre plus efficace.
*/
#define RS  -1
#define ENC_A 2
#define ENC_S 3
#define ENC_B 4
#define MOTOR 5
#define HEATER 6
#define BUZZ 7
#define SD_CS 8
#define DC 9
#define TFT_CS 10
#define MOSI 11
#define MISO 12
#define CLK 13
#define LED_R A0
#define LED_G A1
#define LED_B A2
#define TEMP A3
#define SDA A4
#define SCL A5
#define SCALE A6
#define PB A7


//Definition des couleurs de base
// Color definitions
#define BLACK 0x0000
#define RED 0x001F 
#define BLUE 0xF800 
#define GREEN 0x07E0 
#define YELLOW 0x07FF 
#define MAGENTA 0xF81F 
#define CYAN 0xFFE0
#define WHITE 0xFFFF

//Definition des tempos de rafraichissement valeurs et ecran
#define TEMPERATURE_RATE 1
#define WEIGHT_RATE 1
#define SCREEN_RATE 0.1
#define WEIGHT_READINGS 10

// INITIALISATION LIBRAIRIES
//Definition de l'ecran
TFT Screen = TFT (10,9,-1);

//Thermometer
OneWire oneWire(A3);
DallasTemperature Thermometer(&oneWire);
static DeviceAddress ThermometerAdress={0x28,0x3E,0x40,0xCD,0x05,0x00,0x00,0x98};

// Variables interfaces
volatile unsigned char Previous_position=0;
volatile unsigned char Current_position=0;
volatile unsigned int Previous_screen=0;
volatile unsigned int Current_screen=0; //try to use char instead but i can't, maybe initialization
//
// Trigger permettant de declencher un rafraichissement complet de l'écran
volatile bool doRefresh=TRUE;
// Trigger permettant de declencher le rafraichissement de quelques champs
volatile bool doRefreshFocus=FALSE;
//Trigger pour permettre le rafraichissement de valeurs lors de la boucle principale de l'interface
volatile bool doRefreshValues=FALSE;
volatile bool doClick=FALSE;
//Contexte de navigation ou d'edition: trigger permettant d'orienter vers une incrementation de valeur
volatile bool doEdit=FALSE;



// Variables capteurs
float Temp_actual=0; //Temperature actuelle

float Temp_goal=0; // Temperature de consigne pour le thermostat
float Time_left=90;
bool Timer_set=FALSE;
float Weight_actual=0;
float Weight_tare=0;
//unsigned int Weight_readings[10]; // Liste de 10 lectures de la valeur de 0 à 1023
float Scale_define[2][2] = {{0,85},{2,96}};
 // Tares enregistrée TODO :à reporter dans un fichier de configuration à mettre sur la carte SD ??
// Variables effecteurs
unsigned char Motor_speed=0; // Correspond à la vitesse du moteur souhaitée de 0 à 255
bool Heating_state=FALSE; // Correspond à l'état de la resistance, il s'agit d'un booléen



// VARIABLES CAPTEURS (regulierement mise à jour dans la loop)
// Temperature actuelle : en degres
// Balance : analog

// VARIABLES ETATS EFFECTEURS 
// Vitesse moteur
// Etat resistance

// VARIABLES INTERFACE
// Ecran : avant , maintenant, ensuite
// Position focus  : depend de l'ecran 
// Set/Ok : correspond à changement position ou edition valeur, depend si l'utilisateur edite le champ ou pas, change à la suite d'un click sur une valeur à changer


// DEFINITION ECRANS (à ranger par position)
// Zone valeurs affichées : position x, position y, largeur, hauteur, pointeur variable globale
// TODO:le nombre de boutons donnera le nombre de positions par ecran

// Définitions des fonctions de "callback"
void goPage0(void) {changeScreen(0);};
void goPage1(void) {changeScreen(1);};
void goPage2(void) {changeScreen(2);};
void goPage3(void) {changeScreen(3);};
void goPage4(void) {changeScreen(4);};
void goEdit(void) {doEdit=TRUE;};
void goTare(void) {};
void goSetScale(void) {changeScreen(5);};

// Définitions des écrans et des zones d'affichage correspondantes
// x[0,159] et y[0,127]
static Page interface[6] = 
{
        {1,2,0,{},{},
            {
            {10,10,140,50,BLACK,RED,"Manuel" ,NULL ,FALSE, goPage1  },
            {10,65,140,50, BLACK,RED,"Automatique" ,NULL ,FALSE, goPage2  }
            }
        },
	{2,3,0,{},{},
            {
            {10,10,140,30,BLACK,RED,"Balance" ,NULL ,FALSE, goPage4 },
            {10,45,140,30, BLACK,RED,"Thermostat",NULL ,FALSE, goPage3 },
	    {10,80,140,30, BLACK, RED,"Moteur",NULL,FALSE, goPage0 }
            }
        },
	{3,2,0,{},{},
            {
            {10,10,140,30,BLACK,RED,  "Eau" ,NULL ,FALSE, goPage0 },
            {10,45,140,30, BLACK,RED,  "Malt",NULL ,FALSE, goPage0 }
            }
        },
	{4,3,1,{0},{},
            {
        	{10,10,140,30,BLACK,RED,  "Temp : " ,&Temp_actual ,FALSE, goPage0 },
         	{10,45,140,30, BLACK,RED,  "Cible: ",&Temp_goal ,TRUE, goEdit },
	    	{10,80,140,30, BLACK, RED,"Duree:",&Time_left,FALSE, goPage0 }
            }
	},
	{5,3,1,{0},{},
	   {
		{10,10,140,30,BLACK,RED,  "Poids: " ,&Weight_actual ,FALSE, goPage0 },
		{10,45,140,30,BLACK,RED, "Tarer",NULL,FALSE,goTare },
		{10,80,140,30, BLACK, RED,"Regler",NULL,FALSE, goSetScale }
	   }
	},
	{6,2,1,{0},{},
	   {
		{10,10,140,20,BLACK,RED,  "Poids 1: " ,&Weight_actual ,TRUE, goPage0 },
		{10,45,140,20,BLACK,RED, "Tarer",NULL,FALSE, goTare },
		{10,80,140,20, BLACK, RED,"Next",NULL,FALSE, goSetScale }
	   }
	},
};

//Page Current_screen;

// CONFIGURATION AFFICHAGE
// pour définir les bords des boutons
static unsigned int area_radius=4;
// pour l'espacement du texte par rapport au bord supérieur gauche
static unsigned int text_padding=5;
// défini la taille du texte : trois valeurs possibles : 1 (defaut), 2 ou 
static unsigned int text_size=2;
// couleur du cadre pour le bouton avec le focus
static unsigned int focus_color=YELLOW;

void drawButton(Area *button, bool has_focus) {
	unsigned int background_color;
	if (has_focus) { background_color=focus_color;} else { background_color=button->b;};
	Screen.fillRoundRect(button->x,button->y,button->w,button->h,area_radius,background_color);
	Screen.setTextColor ( BLACK, background_color);
	Screen.setTextSize (text_size);
	char float_text[5];
	char text_out[20];
	strcpy(text_out, button->c);
	//Si le bouton posséde une valeur à afficher : afficher cette valeur
	if (button->v !=NULL) {
		dtostrf((*button->v),4,1,float_text); 
		strcat(text_out, float_text);}
	Screen.text(text_out, button->x +text_padding, button->y +text_padding);
};

void refreshScreen() {
 // Efface l'ecran précédent et affiche l'ecran actuel
 // parcourt tous les boutons sans exception pour l'initialisation de l'écran
	Screen.background(0,0,0);
	for(unsigned int i ; i<interface[Current_screen].p; i++){
		// Draw with focus
		if (i==Current_position) {drawButton(&(interface[Current_screen].buttons[i]), 1);
		// Draw without focus
		} else { drawButton(&(interface[Current_screen].buttons[i]), 0);}
	}
};

void refreshFocus() {
 // Met le focus sur la position actuelle et l'enleve sur la position précédente
	drawButton(&(interface[Current_screen].buttons[Current_position]), 1);
	drawButton(&(interface[Current_screen].buttons[Previous_position]), 0);
};

void changeScreen(unsigned int screen_index) {
	//Declenche le trigger pour un rafraichissement lors de la boucle
	Previous_screen=Current_screen;
	Current_screen=screen_index;
	Current_position=0; 
	doRefresh=TRUE;
};

void refreshValues(void){
  // Reaffiche les boutons à rafraichir de la page
	unsigned char refreshListLength = interface[Current_screen].refreshListLength;
	if (refreshListLength) {
		for (unsigned int i; i<refreshListLength;i++){
			//TODO : find a way to deal with focus another way
			if (Current_position==interface[Current_screen].refreshList[i]) { 
				drawButton(&(interface[Current_screen].buttons[interface[Current_screen].refreshList[i]]), 1); } 
			else {	drawButton(&(interface[Current_screen].buttons[interface[Current_screen].refreshList[i]]), 0); }
		}
	}	
};

void getTemp() {
	Thermometer.requestTemperatures();
  	Alarm.delay(500);
	Temp_actual=Thermometer.getTempC(ThermometerAdress);
	Alarm.delay(100);
	doRefreshValues=TRUE;
};

void getWeight() {
	unsigned int analogValTotal;
	for (unsigned int i; i<WEIGHT_READINGS; i++) {
		analogValTotal+=analogRead(A6);
		Alarm.delay(40);
	}
	float analogAverage=analogValTotal/WEIGHT_READINGS;
	Weight_actual=mapWeight(analogAverage,Scale_define[0][1],Scale_define[1][1],Scale_define[0][0],Scale_define[1][0]);	
};

float mapWeight(float x, float in_min, float in_max, float out_min, float out_max){
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};


void receiveEncoder(void) {
   Previous_position=Current_position;
   if (digitalRead(ENC_A) == digitalRead(ENC_B)) {
    // si edition valeur , doit incrementer la valeur
    // si changement position focus, doit changer focus
    // incremente dans les autres cas
    // changePosition(1);
	if (Current_position==interface[Current_screen].p-1){Current_position=0;} 
	else{ Current_position++;};
  } else {
    // decremente dans les autres cas
	if (Current_position==0){ Current_position=interface[Current_screen].p-1;} 
	else{ Current_position--;};
  }
  doRefreshFocus=TRUE;
}

/*void changePosition(bool move_forward){
   unsigned int screen_pos_max=interface[Current_screen].p-1;
// TODO : modifier cette fonction en permettant l'édition de valeurs
   if (move_forward){
	
   } else {
	
   }
   doRefresh=TRUE;
}*/

void receiveClick(void) {doClick=TRUE;}

bool receiveBackClick(void) {if (analogRead(A7) >500){return TRUE;}else{return FALSE;}}

void doEnter(void) {
// soit provoque un changement d'ecran
// soit provoque une edition de valeurs
 // efface l'ecran
 	//Screen.background(0,0,0);
 // lance la fonction de callback pour l'écran et le bouton actuel
	interface[Current_screen].buttons[Current_position].pf();
}



void setup() {
  //INTERFACE UTILISATEUR
  // Initialisation de l'ecran
	Screen.begin();
 // Trois valeurs de rotation possible 0, 1, 2, 3 correspondant à 0°, 90°, 180° et 270°
	Screen.setRotation(3);
 // Fond noir
	Screen.background(0,0,0);
 
 // Interruption de l'encodeur (les resistances pull up de l'arduino ne sont pas utilisés)
	pinMode(2,INPUT);
 	pinMode(4,INPUT);
 	attachInterrupt(0, receiveEncoder, RISING); //to get only half of the transition, changed from CHANGE to RISING or FALLING
 // Interruption du bouton de l'encodeur
// use the pushbutton for enter
	pinMode(3,INPUT);
	attachInterrupt(1, receiveClick, RISING); // only when pushed not released
 
// Initialisation du bouton poussoir
// normalement utiliser pour les retours mais puisque le bouton de l'encodeur ne marche pas il faut utiliser celui ci pour la fonction doClick
	pinMode(PB, INPUT);
 
 // CAPTEURS
 // Initialisation du thermometre 
	Thermometer.begin();
// En fonction de la resolution, il est necessaire de mettre un delai de 93,75ms pour 9 bits, 187,5ms pour 10 bits, 375 pour 11 bits et 750ms pour 12 bits
	Thermometer.setResolution(ThermometerAdress,10);
	Thermometer.setWaitForConversion(false);  // rend la requete asynchrone, il faut donc mettre un delay dans la loop pour attendre apres un request ou faire d'autres choses entre temps ??


  // EVENEMENTS REPETITIFS
  	Alarm.timerRepeat(TEMPERATURE_RATE,getTemp);
	Alarm.delay(500);
	Alarm.timerRepeat(WEIGHT_RATE,getWeight);
// Initialisation de la balance (pas utile)
//pinMode(A6,INPUT);

// EFFECTEURS

//Dessine l'écran de demarrage
	Current_screen=0;
	//drawScreen();
  //drawScreen(Current_screen);
}

void loop() {
  //int analogWeight=analogRead(A6); // prend 1ms normalement, il faut en faire plusieurs et faire une moyenne

  // Correspond au bouton de retour à configurer
  if (receiveBackClick()) { changeScreen(Previous_screen); Alarm.delay(50);}
  if (doClick) {doEnter(); doClick=FALSE;}
  if (doRefresh) {refreshScreen(); doRefresh=FALSE;} 
  if (doRefreshFocus) {refreshFocus(); doRefreshFocus=FALSE;}
  if (doRefreshValues) {refreshValues(); doRefreshValues=FALSE;}
  Alarm.delay(10);
//TODO : choose a real timer

  //if (sizeof(interface[Current_screen].refreshList)) {refreshAreas();}
}


