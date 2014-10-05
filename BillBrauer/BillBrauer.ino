/*
BillBrauer est une cuve de brassage avec moteur, une balance integré avec stockage continu des paramètres.
*/

#include <TFT.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

// INITIALISATION LIBRAIRIES
//Definition de l'ecran
TFT Screen = TFT (10,9,-1);

//Thermometer
OneWire oneWire(A3);
DallasTemperature Thermometer(&oneWire);

DeviceAddress ThermometerAdress;



// Position de l'encodeur
volatile unsigned int Position = 0;

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
// il n'y aura pas de variable position de l'encodeur car 
// l'encodeur changera directement la valeur de la position correspondante a l'ecran


// DEFINITION ECRANS (à ranger par position)
// Zone valeurs affichées : position x, position y, largeur, hauteur, pointeur variable globale
// le nombre de boutons donnera le nombre de positions par ecran
/*
struct Area {
  // correspond à une zone de texte affichée
        byte  x; // position X
        byte y; // postion 
        byte w; // largeur
        byte h; // hauteur
        unsigned int f; // couleur du texte
        unsigned int b; // couleur d'arrière plan
        String c;  // texte (à déclarer à part) ??
        int (*v); // pointeur vers une valeur en int (si nul pas de valeur)
        // comme un bouton mais avec une adress NULL
};
*/
typedef struct {
        byte  x; // position X
        byte y; // postion Y
        byte w; // largeur
        byte h; // hauteur
        unsigned int f; // couleur du texte // passage en byte possible ???
        unsigned int b; // couleur d'arrière plan
        String c; // texte à rajouter ou texte dans le cas d'une zone non cliquable
        int (*v); // pointeur vers une valeur en int, si NULL il s'agit d'un texte simple
        void (*pf)(int, int); // fonction à appeler avec un paramètre à fixer
     // la fonction à appeler est soit gotoscreen avec le numéro soit changevalue avec la valeur  à modifier et +1 ou -1  
} Area;
// Boutons textes cliquable (enter change screen) : position x, position y, largeur, hauteur, pointeur vers texte, ?? action go to screen
// Boutons valeurs ajustable (enter puis incremente puis enter) : position x, position y, largeur, hauteur, pointeur variable globale,  ??action set value


typedef struct  {
        byte x; // page number
        byte p; // number of position needed because we need to fix the size of the array, TODO : we need to get a variable array size
        Area displayed[5]; // correspond aux affichages simples // pas plus de cinq ici par page
        Area buttons[5]; // correspond aux zones selectionnables, leur nombre conditionne la taille du tableau //pas plus de cing par page ici
  
} Page ;

int test_valeur = 1;
void function_test (int a, int b) {
  
};

// il faut passer les arguments  à la fonction au moment où elle est appeler, et elle prend forcément deux arguments à choisir dans les autres paramètre de la structure
static Page interface [] = 
{
        {1,
        2,
           { 
           {10,10,80,50, 250,250, "Hello",&test_valeur , function_test },
            {10,10,80,50, 250,250, "Hello",&test_valeur, function_test } 
            },
            {
            {10,10,80,50, 250,250, "Hello",&test_valeur , function_test },
            {10,10,80,50, 250,250, "Hello",&test_valeur , function_test }
            }
        }
};




void drawScreen(int screen_number) {
 // parcourt tous les boutons sans exception pour l'initialisation de l'écran

  
};

void refreshScreen (int screen_number, int button_list[]) {
  // parcourt la liste des boutons à rafraichir et rafraichi ceux là uniquement
}





void setup() {
  //INTERFACE UTILISATEUR
  // Initialisation de l'ecran
 Screen.begin();
 // Fond noir
 Screen.background(0,0,0);
 
 // Interruption de l'encodeur (les resistances pull up de l'arduino ne sont pas utilisés)
 pinMode(2, INPUT);
 pinMode(4,INPUT);
 attachInterrupt(0, doEncoder, CHANGE);
 // Interruption du bouton de l'encodeur
 pinMode(3,INPUT);
 attachInterrupt(1, doClick, CHANGE);
 
 // Initialisation du bouton poussoir
 pinMode(A7, INPUT);
 
 // CAPTEURS
 // Initialisation du thermometre 
Thermometer.begin();
// En fonction de la resolution, il est necessaire de mettre un delai de 93,75ms pour 9 bits, 187,5ms pour 10 bits, 375 pour 11 bits et 750ms pour 12 bits
Thermometer.setResolution(ThermometerAdress,10);
Thermometer.setWaitForConversion(false);  // rend la requete asynchrone, il faut donc mettre un delay dans la loop pour attendre apres un request
 


// Initialisation de la balance (pas utile)
//pinMode(A6,INPUT);

// EFFECTEURS




}

void loop() {
  // Request Temperature
  Thermometer.requestTemperatures();
  int analogWeight=analogRead(A6); // prend 1ms normalement, il faut en faire plusieurs et faire une moyenne
  delay(190); // a diminuer en fonction de la duree de analog read qu'il faut mesurer
  float temp=Thermometer.getTempC(ThermometerAdress);
  
}

void doEncoder() {
  /* If pinA and pinB are both high or both low, it is spinning
   * forward. If they're different, it's going backward.
   *
   * For more information on speeding up this process, see
   * [Reference/PortManipulation], specifically the PIND register.
   */
  if (digitalRead(ENC_A) == digitalRead(ENC_B)) {
    // si edition valeur , doit incrementer la valeur
    // si changement position focus, doit changer focus
    Position++;
  } else {
    Position--;
  }
}

void doClick() {
// soit provoque un changement d'ecran
// soit provoque une edition de valeurs
}
