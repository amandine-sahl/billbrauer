//Definition d'un pointeur pour une fonction ptrf
typedef void (*ptrf)();


// Définition de la structure Area qui correspond soit à une bouton soit à un affichage simple
typedef struct {
        unsigned char  x; // position X
        unsigned char y; // postion Y
        unsigned char w; // largeur
        unsigned char h; // hauteur
        unsigned int f; // couleur du texte // passage en byte possible ???
        unsigned int b; // couleur d'arrière plan
	//String c;  
	char c[12]; // texte à rajouter ou texte dans le cas d'une zone non cliquable
	float (*v); // pointeur vers une valeur en int, si NULL il s'agit d'un texte simple
        ptrf pf; // fonction à appeler sans un paramètre à fixer !!!! Il faut trouver comment fixer des paramètres
     // la fonction à appeler est soit gotoscreen avec le numéro soit changevalue avec la valeur  à modifier et +1 ou -1  
}Area;
// Boutons textes cliquable (enter change screen) : position x, position y, largeur, hauteur, pointeur vers texte, ?? action go to screen
// Boutons valeurs ajustable (enter puis incremente puis enter) : position x, position y, largeur, hauteur, pointeur variable globale,  ??action set value


// Définition de la structure Page qui correspond à un écran avec des boutons et des affichages
typedef struct {
        unsigned char x; // page number
        unsigned char p; // number of position needed because we need to fix the size of the array, TODO : we need to get a variable array size
        Area displayed[5]; // correspond aux affichages simples // pas plus de cinq ici par page
        Area buttons[5]; // correspond aux zones selectionnables, leur nombre conditionne la taille du tableau //pas plus de cing par page ici
  
}Page;

/*typedef struct {
	int actual_temp;
	float 


}State;*/


void drawButton(Area *button, bool has_focus);
void drawScreen();
void refreshScreen (int button_list[]);
//void Menu0(void);
void changePosition(bool move_forward);
void doEncoder(void);
void doClick(void);
void printhello(void);
void checkPB(void);
void changeScreen(unsigned int screen_index);
