/*
BillBrauer est une cuve de brassage avec moteur, une balance integré avec stockage continu des paramètres.
*/

#include <TFT.h>
#include <SPI.h>
//#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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

// INITIALISATION LIBRAIRIES
//Definition de l'ecran
TFT Screen = TFT (10,9,-1);

//Thermometer
OneWire oneWire(A3);
DallasTemperature Thermometer(&oneWire);
DeviceAddress ThermometerAdress={0x28,0x3E,0x40,0xCD,0x05,0x00,0x00,0x98};

// Variables interfaces
volatile unsigned char Position=0;
volatile unsigned int Current_screen; //try to use char instead but i can't, maybe initialization
volatile bool doRefresh=TRUE;

// Variables capteurs
float temp; //Temperature actuelle

float temp_set; // Temperature de consigne pour le thermostat
unsigned int weight_reading[10]; // Liste de 10 lectures de la valeur de 0 à 1023
float weight=0; // Masse calculée à partir de la moyenne des lectures précédentes
unsigned int tare_weights[2][2]; // Tares enregistrée TODO :à reporter dans un fichier de configuration à mettre sur la carte SD ??
// Variables effecteurs
unsigned char motor_speed=0; // Correspond à la vitesse du moteur souhaitée de 0 à 255
bool heating_element=FALSE; // Correspond à l'état du moteur, il s'agit d'un booléen



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
void Menu0(void) {changeScreen(0);};
void Menu1(void) {changeScreen(1);};
void Menu2(void) {changeScreen(2);};
void Menu3(void) {changeScreen(3);};

// Définitions des écrans et des zones d'affichage correspondantes
// x[0,159] et y[0,127]
static Page interface[4] = 
{
        {1,2,{},
            {
            {10,10,140,50,BLACK,RED,"Manuel" ,NULL , Menu1  },
            {10,65,140,50, BLACK,RED,"Automatique" ,NULL , Menu2  }
            }
        },
	{2,3,{},
            {
            {10,10,140,30,BLACK,RED,"Balance" ,NULL , Menu0 },
            {10,45,140,30, BLACK,RED,"Thermostat",NULL , Menu3 },
	    {10,80,140,30, BLACK, RED,"Moteur",NULL, Menu0 }
            }
        },
	{3,3,{},
            {
            {10,10,140,30,BLACK,RED,  "Eau" ,NULL , Menu0 },
            {10,45,140,30, BLACK,RED,  "Malt",NULL , Menu0 },
	    {10,80,140,30, BLACK, RED,  "Back",NULL, Menu0 }
            }
        },
	{4,3,{},
            {
            {10,10,140,30,BLACK,RED,  "Temp : " ,&temp , Menu0 },
            {10,45,140,30, BLACK,RED,  "Masse",NULL , Menu0 },
	    {10,80,140,30, BLACK, RED,  "Back",NULL, Menu0 }
            }
        }
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
	if (button->v !=NULL) {dtostrf((*button->v),4,1,float_text); strcat(text_out, float_text);}
	Screen.text(text_out, button->x +text_padding, button->y +text_padding);
};

void drawScreen() {
 // parcourt tous les boutons sans exception pour l'initialisation de l'écran
	for(unsigned int i ; i<interface[Current_screen].p; i++){
		if (i==Position) {drawButton(&(interface[Current_screen].buttons[i]), 1);
		} else { drawButton(&(interface[Current_screen].buttons[i]), 0);}
	}
	doRefresh=FALSE;
};

void changeScreen(unsigned int screen_index) {
	//Declenche le trigger pour un rafraichissement lors de la boucle
	Current_screen=screen_index;
	Position=0; 
	doRefresh=TRUE;

};

//TODO : use this function with Current_screen
/*
void refreshScreen (int button_list[]) {
  // parcourt la liste des boutons à rafraichir et rafraichi ceux là uniquement; 
	for(int i; i<sizeof(button_list)+1;i++){
		drawButton(interface[Current_screen].buttons[button_list[i]],0);
	}
};*/



void setup() {
  //INTERFACE UTILISATEUR
  // Initialisation de l'ecran
	Screen.begin();
 // Trois valeurs de rotation possible 0, 1, 2, 3 correspondant à 0°, 90°, 180° et 270°
	Screen.setRotation(3);
 // Fond noir
	Screen.background(0,0,0);
 
 // Interruption de l'encodeur (les resistances pull up de l'arduino ne sont pas utilisés)
	pinMode(2, INPUT);
 	pinMode(4,INPUT);
 	attachInterrupt(0, doEncoder, RISING); //to get only half of the transition, changed from CHANGE to RISING or FALLING
 // Interruption du bouton de l'encodeur
 // TODO : check the pcb because the pin3 doesn't work !!!
// use the pushbutton for enter
	pinMode(3,INPUT);
	attachInterrupt(1, doClick, RISING); // only when pushed not released
 
// Initialisation du bouton poussoir
	pinMode(PB, INPUT);
 
 // CAPTEURS
 // Initialisation du thermometre 
	Thermometer.begin();
// En fonction de la resolution, il est necessaire de mettre un delai de 93,75ms pour 9 bits, 187,5ms pour 10 bits, 375 pour 11 bits et 750ms pour 12 bits
	Thermometer.setResolution(ThermometerAdress,10);
	Thermometer.setWaitForConversion(false);  // rend la requete asynchrone, il faut donc mettre un delay dans la loop pour attendre apres un request
 


// Initialisation de la balance (pas utile)
//pinMode(A6,INPUT);

// EFFECTEURS

//Dessine l'écran de demarrage
	Current_screen=0;
  //drawScreen(Current_screen);
}

void loop() {
  // Request Temperature
  Thermometer.requestTemperatures();
  //int analogWeight=analogRead(A6); // prend 1ms normalement, il faut en faire plusieurs et faire une moyenne
  delay(190); // a diminuer en fonction de la duree de analog read qu'il faut mesurer
  temp=Thermometer.getTempC(ThermometerAdress);
  if (analogRead(A7) >500) { doClick(); delay(100);}
  if (doRefresh) {drawScreen();}
  
}

void doEncoder(void) {
   if (digitalRead(ENC_A) == digitalRead(ENC_B)) {
    // si edition valeur , doit incrementer la valeur
    // si changement position focus, doit changer focus
    // incremente dans les autres cas
     changePosition(1);
  } else {
    // decremente dans les autres cas
     changePosition(0);
  }
}

void changePosition(bool move_forward){
   unsigned int screen_pos_max=interface[Current_screen].p-1;
   if (move_forward){
	if (Position==screen_pos_max){
		Position=0;
	} else{ Position++;};
   } else {
	if (Position==0){
		Position=screen_pos_max;
	} else{ Position--;};
   }
   doRefresh=TRUE;
}

void doClick(void) {
// soit provoque un changement d'ecran
// soit provoque une edition de valeurs
 // efface l'ecran
 	Screen.background(0,0,0);
	interface[Current_screen].buttons[Position].pf();
}

