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

//Definition des paramètres d'affichage des boutons
#define AREA_RADIUS 4
#define TEXT_PADDING 5
#define FOCUS_COLOR 0x07FF
#define EDIT_COLOR 0x07E0

//Definition des tempos de rafraichissement valeurs et ecran
#define TEMPERATURE_RATE 1
#define WEIGHT_RATE 1
#define SCREEN_RATE 0.1
#define WEIGHT_READINGS 10

//Definition des actions pour l'interface utilisateur
#define NONE 0
#define ENCODER_PLUS 1
#define ENCODER_MINUS 2
#define CLICK 3
#define PUSH_BACK 4

// INITIALISATION LIBRAIRIES
//Definition de l'ecran
TFT Screen = TFT (10,9,-1);

//Thermometer
OneWire oneWire(A3);
DallasTemperature Thermometer(&oneWire);
static DeviceAddress ThermometerAdress={0x28,0x3E,0x40,0xCD,0x05,0x00,0x00,0x98};

// VARIABLES ETATS INTERFACE
volatile unsigned int Current_Page=0;
/* TODO Find a way to use pointer nicely to point to the current page : needs an initialization part in the setup() fonction
volatile *Page CPage; 
volatile *Page Npage;
volatile *Position PButton;
volatile *Position CButton;
volatile *Position NButton;*/
volatile unsigned int Current_Pos=0;
volatile bool Edit=FALSE;
volatile unsigned int Action=NONE;

// VARIABLES CAPTEURS
float Temp_actual=0; //Temperature actuelle

float Temp_goal=0; // Temperature de consigne pour le thermostat
float Time_left=90;
bool Timer_set=FALSE;
float Weigth_actual=50;
float Weigth_tare=0;
//unsigned int Weight_readings[10]; // Liste de 10 lectures de la valeur de 0 à 1023
float Scale_define[2][2] = {{0,85},{2,96}};
 // Tares enregistrée TODO :à reporter dans un fichier de configuration à mettre sur la carte SD ??

// VARIABLES EFFECTEURS
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

// Définitions des écrans et des zones d'affichage correspondantes
// x[0,159] et y[0,127]


static Page interface[4] = {
	{0,0, //Page de demarrage
		0,{}, // Affichages simples
		0,{}, // Valeurs à rafraichir
		2,{ // Boutons
		{{10,10,140,50,BLACK,RED,2,"Manuel"},FALSE,NULL,1,1,1,ForwPos,BackPos,ClickPos}, 
		//{x,y,h,w,color,bg_color,text_size,text,dec,val_ptr,txt_ptr,prev,next,link,encP,encM,Click}
		{{10,65,140,50,BLACK,RED,2,"Automatique"},FALSE,NULL,0,0,1,ForwPos,BackPos,ClickPos}
		} 
	},
	{0,0, // Mode Manuel
		0,{}, // Affichages simples
		0,{}, // Valeurs à rafraichir
		3,{ // Boutons
		{{10,10,140,30,BLACK,RED,2,"Balance"},FALSE,NULL,2,1,2,ForwPos,BackPos,ClickPos},
		{{10,45,140,30,BLACK,RED,2,"Thermostat"},FALSE,NULL,0,2,0,ForwPos,BackPos,ClickPos},
		{{10,80,140,30,BLACK,RED,2,"Moteur"},FALSE,NULL,1,0,0,ForwPos,BackPos,ClickPos}
		} 
	},
	{1,0, // Balance
		1,{ // Affichages simples
		{100,10,50,30,BLACK,RED, 2,"kg"}// unité mesure instantanée
		}, 
		1,{ // Valeurs à rafraichir
		{{10,10,100,30,BLACK,RED,2,"kg"},TRUE,&Temp_actual}
		}, 
		2,{ // Boutons
		{{10,45,140,30,BLACK,RED,2,"Tarer"},FALSE,NULL,1,1,0,ForwPos,BackPos,ClickPos},
		{{10,80,140,30,BLACK,RED,2,"Regler"},FALSE,NULL,0,0,2,ForwPos,BackPos,ClickPos}
		} 
	},
	{2,0, // Reglage Balance Poids 1
		1,{ // Affichages simples
		{100,10,50,30,BLACK,RED, 2,"kg"}// unité mesure instantanée
		},
		1,{ // Valeurs à rafraichir
		{{10,10,90,30,BLACK,RED,2,"Heo"},TRUE,NULL}
		}, 
		0,{}
		/*2,{ // Boutons
		{{10,45,140,30,BLACK,RED,2,"Tarer"},FALSE,NULL,1,1,0,ForwPos,BackPos,ClickPos},
		{{10,80,140,30,BLACK,RED,2,"Regler"},FALSE,NULL,0,0,0,ForwPos,BackPos,ClickPos}
		} */
	}
};

//CALLBACK INTERFACE
//Pour les fonctions
void ForwPos(void) { // Avance le focus à la position suivante
	unsigned int Next_Pos=interface[Current_Page].button[Current_Pos].next; // Récupére la position du prochain bouton
	// Rafraichissement couleur bouton actuel et bouton suivant	 
	drawButton(&(interface[Current_Page].button[Current_Pos]), interface[Current_Page].button[Current_Pos].area.b); // Couleur de base pour le bouton actuel
	drawButton(&(interface[Current_Page].button[Next_Pos]),FOCUS_COLOR); // Couleur du focus pour le prochain bouton
	// Modification position actuelle
	Current_Pos=Next_Pos;
	Action=NONE;
};

void BackPos(void) { // Recule le focus à la position précédente
	unsigned int Previous_Pos=interface[Current_Page].button[Current_Pos].prev;
	// Rafraichissement couleur bouton actuel et bouton précédent
	drawButton(&(interface[Current_Page].button[Current_Pos]), interface[Current_Page].button[Current_Pos].area.b); // Couleur de base pour le bouton actuel
	drawButton(&(interface[Current_Page].button[Previous_Pos]),FOCUS_COLOR); // Couleur du focus pour le prochain bouton
	// Modification position actuelle
	Current_Pos=Previous_Pos;
	Action=NONE;
};

void ClickPos(void) { // Charge la page désignée par la position
	unsigned int Next_Page=interface[Current_Page].button[Current_Pos].link;
	Current_Pos=interface[Next_Page].init_pos;
	drawScreen(&(interface[Next_Page]));// Chargement nouvelle page
	// Modification page actuelle
	Current_Page=Next_Page;
	Action=NONE;	
};

void ForwVal(void) { // Incremente la valeur associée au bouton actuel ou avance d'une position
	if (Edit) {
	// Increment de la valeur pointée
	(*(interface[Current_Page].button[Current_Pos].value))++; //TODO : prévoir un increment au dizieme ou à l'unité
	// Rafraichissement valeur du bouton actuel
	refreshButton(&(interface[Current_Page].button[Current_Pos]), EDIT_COLOR);
	Action=NONE;
	}
	else {ForwPos();}
};

void BackVal(void) { // Decremente la valeur associée au bouton actuel ou recule d'une position
	if (Edit) {
	// Decrement de la valeur pointée
	(*(interface[Current_Page].button[Current_Pos].value))--;
	// Rafraichissement valeur du bouton actuel
	refreshButton(&(interface[Current_Page].button[Current_Pos]), EDIT_COLOR);
	Action=NONE;
	}
	else {BackPos();}
};

void ClickVal(void) { // Passe en mode edition ou en mode navigation
	if (Edit) { 
		// Rafraichissement couleur bouton actuel : edit à focus
		drawButton(&(interface[Current_Page].button[Current_Pos]),FOCUS_COLOR);
		Edit=FALSE;
	}
	else { 
		// Rafraichissement  couleur bouton actuel : focus à edit
		drawButton(&(interface[Current_Page].button[Current_Pos]),EDIT_COLOR);
		Edit=TRUE; 
	}
	Action=NONE;
};




/*
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
		{10,10,140,20,BLACK,RED,  "Poids 1: " ,&Weight_actual ,FALSE, goPage0 },
		{10,45,140,20,BLACK,RED, "Tarer",NULL,FALSE, goTare },
		{10,80,140,20, BLACK, RED,"Next",NULL,FALSE, goSetScale }
	   }
	},
};
*/
//Page Current_screen;

// CONFIGURATION AFFICHAGE
/*
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
	//unsigned char refreshListLength = interface[Current_screen].refreshListLength;
	//if (refreshListLength!=0) {
		for (unsigned int i; i<interface[Current_screen].refreshListLength;i++){
			//TODO : find a way to deal with focus another way
			//if (Current_position==interface[Current_screen].refreshList[i]) { 
				drawButton(&(interface[Current_screen].buttons[interface[Current_screen].refreshList[i]]), 1); 
//} 
//			else {	drawButton(&(interface[Current_screen].buttons[interface[Current_screen].refreshList[i]]), 0); }
		}
	//}	
};
*/

//AFFICHAGE
void drawDisplay(Display *area) { // Affiche les affichages simples
	drawArea(area, area->b);
	drawText(area, area->text,area->b);
};

void drawValue(Value *value) { // Affiche la zone de la valeur et la valeur
	Display *area=&(value->area);
	drawArea(area,area->b);
	refreshValue(value);
};

void refreshValue(Value *value) { // Rafraichit uniquement la valeur
	//drawText(&(value->area), value->area.text, value->area.b);
	
	//Screen.setTextColor (value->area.f, value->area.b); Doesn't work anymore
  Screen.stroke(value->area.f);
  //Screen.background(value->area.b);
	Screen.setTextSize (value->area.s);
	char float_text[5];
	//double floatings=10.0;
	//dtostrf(floatings,4,1,float_text);
	dtostrf((*value->value),4,1,float_text);
	Screen.text(float_text,value->area.x+TEXT_PADDING,value->area.y+TEXT_PADDING);
};

void refreshValues() {
	unsigned int valNum=interface[Current_Page].numValues;
	if (valNum) {
		for (unsigned int i; i<valNum; i++) {
			drawValue(&(interface[Current_Page].value[i]));		
		}
	}	
};

void drawButton(Position *button, unsigned int bg_color) { // Affiche le bouton avec la bonne couleur de focus
	drawArea(&(button->area), bg_color);
	refreshButton(button, bg_color);
};

void refreshButton(Position *button, unsigned int bg_color) { // Rafraichit la valeur du bouton
	drawText(&(button->area), button->area.text, bg_color);
};

void drawScreen(Page *screen) { // Affiche la page demandée
	Screen.background(0,0,0); // Efface l'ecran
	if (screen->numValues) { // Affiche les valeurs à rafraichir automatiquement
		for (unsigned int i; i<screen->numValues; i++) {
			drawValue(&(screen->value[i]));		
		}
	}
	
	if (screen->numButtons) { // Affiche les boutons
		for (unsigned int i; i<screen->numButtons; i++) {
			unsigned int bg_color=screen->button[i].area.b;
			if (i==Current_Pos) {bg_color=FOCUS_COLOR;}
			drawButton(&(screen->button[i]),bg_color);
		}
	}

	if (screen->numDisplays) { // Affiche les affichages simples
		for (unsigned int i; i<screen->numDisplays; i++) {
			drawDisplay(&(screen->display[i]));
		}
	}
};

void drawArea(Display *area, unsigned int bg_color) { // Affiche une zone sans le texte
	Screen.fillRoundRect(area->x,area->y,area->w,area->h,AREA_RADIUS,bg_color);
};

void drawText(Display *area, char text[12], unsigned int bg_color) { // Affiche uniquement le texte sur la zone
	if (!text) {text=area->text;}	
	//Screen.setTextColor ( area->f, bg_color);
  Screen.stroke(area->f);
  //Screen.background(bg_color);
	Screen.setTextSize (area->s);
	Screen.text(text,area->x+TEXT_PADDING,area->y+TEXT_PADDING);
};

//ALERTE SONORE
void playTone() { // Joue une note A4 pendant 1000ms
	tone(7,440,200);
	//noTone(7);
};

//SUIVI CAPTEURS
void getTemp() {
	Thermometer.requestTemperatures();
  	Alarm.delay(500);
	Temp_actual=Thermometer.getTempC(ThermometerAdress);
	Alarm.delay(100);
};

void getWeight() {
	unsigned int analogValTotal;
	for (unsigned int i; i<WEIGHT_READINGS; i++) {
		analogValTotal+=analogRead(A6);
		Alarm.delay(40);
	}
	float analogAverage=analogValTotal/WEIGHT_READINGS;
	Weigth_actual=mapWeight(analogAverage,Scale_define[0][1],Scale_define[1][1],Scale_define[0][0],Scale_define[1][0]);
};

float mapWeight(float x, float in_min, float in_max, float out_min, float out_max){
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};


//ACTION UTILISATEURS
void receiveEncoder(void) {
	if (digitalRead(ENC_A) == digitalRead(ENC_B)) { Action=ENCODER_PLUS; } // Déclenche le trigger correspondant au prochain tour de boucle
	else { Action=ENCODER_MINUS; }
}

void receiveClick(void) {Action=CLICK;}

bool receiveBackClick(void) {if (analogRead(A7) >500){return TRUE;}else{return FALSE;}}

//SETUP
void setup() {
  //INTERFACE UTILISATEUR
  // Initialisation de l'ecran
	Screen.begin();
 // Trois valeurs de rotation possible 0, 1, 2, 3 correspondant à 0°, 90°, 180° et 270°
	Screen.setRotation(3);
 // Fond noir
	Screen.background(0,0,0);
  Screen.stroke (WHITE);
  Screen.setTextSize (2);
  Screen.text("Billbrauer alpha",25,25);
  Alarm.delay(5000);
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


  // EVENEMENTS CAPTEURS
  	Alarm.timerRepeat(TEMPERATURE_RATE,getTemp);
	Alarm.delay(500);
	Alarm.timerRepeat(WEIGHT_RATE,getWeight);
	// TODO : TODO TODO Find why it doesnt refresh
	Alarm.timerRepeat(2,refreshValues);
// Initialisation de la balance (pas utile)
//pinMode(A6,INPUT);

// EFFECTEURS

//Dessine l'écran de demarrage
	drawScreen(&(interface[0]));
}


//LOOP
void loop() {
  //int analogWeight=analogRead(A6); // prend 1ms normalement, il faut en faire plusieurs et faire une moyenne
	if (receiveBackClick()) { Action=PUSH_BACK;} // Pour la lisibilite seulement : test de l'état du bouton Back
	switch (Action) { // Vérifie si une action utilisateur a été demandée et lance le callback correspondant à l'état actuel le cas échéant
		case NONE: // Pas d'action utilisateur à lancer
			break;
		case ENCODER_PLUS: // Encodeur avancé d'un cran
			interface[Current_Page].button[Current_Pos].encP();
			break;
		case CLICK: // Click avec l'encodeur
			interface[Current_Page].button[Current_Pos].clic();
			break;
		case ENCODER_MINUS: // Encodeur reculé d'un cran
			interface[Current_Page].button[Current_Pos].encM();
			break;
		case PUSH_BACK: // Bouton Back enfoncé
			drawScreen(&(interface[interface[Current_Page].previous]));
			Action=NONE;
			break;
	}
	Alarm.delay(10);
}


