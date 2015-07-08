//Definition d'un pointeur pour une fonction ptrf
typedef void (*ptrf)();

/*
// Définition de la structure Area qui correspond soit à une bouton soit à un affichage simple
typedef struct {
        unsigned char  x; // position X
        unsigned char y; // position Y
        unsigned char w; // largeur
        unsigned char h; // hauteur
        unsigned int f; // couleur du texte // passage en byte possible ???
        unsigned int b; // couleur d'arrière plan
	//String c;  
	char c[12]; // texte à rajouter ou texte dans le cas d'une zone non cliquable
	float (*v); // pointeur vers une valeur en int, si NULL il s'agit d'un texte simple
	int e; // indique si la valeur est editable
        ptrf pf; // fonction à appeler sans un paramètre à fixer !!!! Il faut trouver comment fixer des paramètres
     // la fonction à appeler est soit gotoscreen avec le numéro soit changevalue avec la valeur  à modifier et +1 ou -1  
}Area;
// Boutons textes cliquable (enter change screen) : position x, position y, largeur, hauteur, pointeur vers texte, ?? action go to screen
// Boutons valeurs ajustable (enter puis incremente puis enter) : position x, position y, largeur, hauteur, pointeur variable globale,  ??action set value
*/

/*// Définition de la structure Page qui correspond à un écran avec des boutons et des affichages
typedef struct {
        unsigned char x; // page number
        unsigned char p; // number of position needed because we need to fix the size of the array, TODO : we need to get a variable array size
	unsigned char refreshListLength;
	unsigned char refreshList[5]; // position that contain a value that need to be refreshed
        Area displayed[5]; // correspond aux affichages simples // pas plus de cinq ici par page
        Area buttons[5]; // correspond aux zones selectionnables, leur nombre conditionne la taille du tableau //pas plus de cing par page ici
  
}Interface;
*/
typedef struct {
 	unsigned char  x; // position X
  unsigned char y; // position Y
  unsigned char w; // largeur
  unsigned char h; // hauteur
  unsigned int f; // couleur du texte 
  unsigned int b; // couleur d'arrière plan
	unsigned int s; // taille du texte
	char text[12]; // texte à afficher dans le cas d'un affichage simple
}Display;

typedef struct {
	Display area;
	bool decimal; // a utiliser pour savoir le type d'affichage de la valeur
	float (*value); 
}Value;

typedef struct {
	Display area;
	bool decimal;
	float (*value); // pointeur vers une valeur pour les zones éditables sinon NULL
	//char (*text[12]); //pointeur vers un texte pour les zones Start/Stop sinon NULL TODO : à revoir car cuasé problème
	unsigned int next; // next position for ForwardPos
	unsigned int prev; // previous position for BackwardPos
	unsigned int link; // go to page number for ClickPos
	ptrf encP; 
	ptrf encM;	
	ptrf clic;
}Position;

typedef struct {
  unsigned int previous; // previous page number for PushBack
  unsigned int init_pos; // initial position for ChangeScreen
	unsigned int numDisplays; // correspond au nombre réel de zones d'affichage simple
  Display display[5];// correspond aux affichages simples // pas plus de cinq ici par page
	unsigned int numValues; // nombre de valeur pour les for
	Value value[4]; // correspond aux variables rafraichies automatiquement
	unsigned int numButtons; //nombre de valeur pour les for
  Position button[4]; // correspond aux zones selectionnables
}Page;



// TODO : define a structure for the state but what to do with volatile
/*
typedef struct {
	unsigned char Screen;
	unsigned char Position;
	float Temp_actuel;
	float Temp_goal;
	unsigned int Time_left;
	
}State;
*/
/*typedef struct {
	int actual_temp;
	float 


}State;*/

//AFFICHAGE
void drawDisplay(Display *area);// Affiche les affichages simples
void drawValue(Value *value);// Affiche la zone de la valeur et la valeur
void refreshValue(Value *value);// Rafraichit uniquement la valeur
void refreshValues(void);
void drawButton(Position *button, unsigned int bg_color);// Affiche le bouton avec la bonne couleur de focus
void refreshButton(Position *button, unsigned int bg_color);// Rafraichit la valeur du bouton
void drawScreen(Page *screen);// Affiche la page demandée

void drawArea(Display *area, unsigned int bg_color);// Affiche une zone sans le texte
void drawText(Display *area, char text[12], unsigned int bg_color); // Affiche uniquement le texte sur la zone

//FONCTION SUIVI CAPTEURS
void getTemp();
void getWeight();
float mapWeight(float x, float in_min, float in_max, float out_min, float out_max);

//FONCTION ACTION UTILISATEURS
void receiveEncoder(void);
void receiveClick(void);
bool receiveBackClick(void);



