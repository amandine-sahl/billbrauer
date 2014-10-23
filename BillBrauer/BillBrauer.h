//define a type for pointer of function with an argument
typedef void (*ptrf)();

typedef struct {
        uint16_t  x; // position X
        uint16_t y; // postion Y
        uint16_t w; // largeur
        uint16_t h; // hauteur
        uint16_t f; // couleur du texte // passage en byte possible ???
        uint16_t b; // couleur d'arrière plan
	//String c;  
	char c[12]; // texte à rajouter ou texte dans le cas d'une zone non cliquable
	float (*v); // pointeur vers une valeur en int, si NULL il s'agit d'un texte simple
        ptrf pf; // fonction à appeler sans un paramètre à fixer !!!! Il faut trouver comment fixer des paramètres
     // la fonction à appeler est soit gotoscreen avec le numéro soit changevalue avec la valeur  à modifier et +1 ou -1  
}Area;
// Boutons textes cliquable (enter change screen) : position x, position y, largeur, hauteur, pointeur vers texte, ?? action go to screen
// Boutons valeurs ajustable (enter puis incremente puis enter) : position x, position y, largeur, hauteur, pointeur variable globale,  ??action set value


typedef struct {
        byte x; // page number
        byte p; // number of position needed because we need to fix the size of the array, TODO : we need to get a variable array size
        Area displayed[5]; // correspond aux affichages simples // pas plus de cinq ici par page
        Area buttons[5]; // correspond aux zones selectionnables, leur nombre conditionne la taille du tableau //pas plus de cing par page ici
  
}Page;

/*typedef struct {
	int actual_temp;
	float 


}State;*/


void drawButton(Area *button, byte has_focus);
void drawScreen();
void refreshScreen (int button_list[]);
//void Menu0(void);
void changePosition(int move);
void doEncoder(void);
void doClick(void);
void printhello(void);
void checkPB(void);
void changeScreen(int screen_index);
