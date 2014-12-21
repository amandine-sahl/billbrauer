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
	bool decimal; // pour gérer l'affichage des int ou des décimales pour le temps (entier en minutes) et les temperatures (avec une decimale)
	float (*value); 
}Value;

typedef struct {
	Display area;
	bool decimal;
	float (*value); // pointeur vers une valeur pour les zones éditables sinon NULL
	char (*text[12]); //pointeur vers un texte pour les zones Start/Stop sinon NULL
	unsigned int prev; // next position for ForwardPos
	unsigned int next; // previous position for BackwardPos
	unsigned int link; // go to page number for ClickPos
	ptrf encP; 
	ptrf encM;	
	ptrf click;
}Position;

typedef struct {
        unsigned int previous; // previous page number for PushBack
        unsigned int init_pos; // initial position for ChangeScreen
	unsigned int numDisplays; // correspond au nombre réel de zones d'affichage simple
        Display display[5];// correspond aux affichages simples // pas plus de cinq ici par page
	unsigned int numValues; // nombre de valeur pour les for
	Value value[4]; // correspond aux variables rafraichies automatiquement
	unsigned int numButtons; //nombre de valeur pour les for
        Button button[4]; // correspond aux zones selectionnables
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


void drawButton(Area *button, bool has_focus);
void drawScreen(void);
void refreshValues(void);
void refreshScreen(void);
void refreshFocus(void);
//void Menu0(void);
//void changePosition(bool move_forward);
void receiveEncoder(void);
void receiveClick(void);
void printhello(void);
void checkPB(void);
void changeScreen(unsigned int screen_index);
void getTemp(void);
void doEnter(void);
void getWeight(void);
